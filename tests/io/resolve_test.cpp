#include <iow/ip/endpoint.hpp>
#include <iow/ip/tcp/server/server.hpp>
#include <iow/ip/tcp/server/options.hpp>
#include <iow/ip/tcp/client/client.hpp>
#include <iow/ip/tcp/client/options.hpp>
#include <fas/testing.hpp>

#include <boost/asio.hpp>
#include <atomic>
#include <chrono>
#include <thread>

namespace {

using tcp = boost::asio::ip::tcp;

unsigned short bind_ephemeral_port(boost::asio::io_context& io)
{
  tcp::acceptor tmp(io);
  tmp.open(tcp::v4());
  tmp.set_option(tcp::acceptor::reuse_address(true));
  tmp.bind(tcp::endpoint(tcp::v4(), 0));
  auto port = tmp.local_endpoint().port();
  tmp.close();
  return port;
}

void poll_for(boost::asio::io_context& io, std::chrono::milliseconds total)
{
  const auto deadline = std::chrono::steady_clock::now() + total;
  while (std::chrono::steady_clock::now() < deadline)
  {
    io.poll();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}

} // namespace

UNIT(try_make_ip_endpoint_v4, "IP v4 собирается без DNS")
{
  using namespace fas::testing;
  boost::system::error_code ec;
  tcp::endpoint ep;
  t << is_true<assert>(::iow::ip::try_make_ip_endpoint<boost::asio::ip::tcp>(
        "127.0.0.1", "8080", ep, ec)) << FAS_FL;
  t << is_false<assert>(static_cast<bool>(ec)) << FAS_FL;
  t << equal<expect, unsigned short>(ep.port(), 8080) << FAS_FL;
  t << is_true<expect>(ep.address().is_v4()) << FAS_FL;
}

UNIT(try_make_ip_endpoint_hostname, "hostname не считается IP")
{
  using namespace fas::testing;
  boost::system::error_code ec;
  tcp::endpoint ep;
  t << is_false<assert>(::iow::ip::try_make_ip_endpoint<boost::asio::ip::tcp>(
        "localhost", "8080", ep, ec)) << FAS_FL;
}

UNIT(try_make_ip_endpoint_bad_port, "битый порт на IP — ошибка без DNS")
{
  using namespace fas::testing;
  boost::system::error_code ec;
  tcp::endpoint ep;
  t << is_false<assert>(::iow::ip::try_make_ip_endpoint<boost::asio::ip::tcp>(
        "127.0.0.1", "not-a-port", ep, ec)) << FAS_FL;
  t << is_true<assert>(static_cast<bool>(ec)) << FAS_FL;
}

UNIT(client_connect_ip_async, "клиент к IP: async_connect без sync resolve")
{
  using namespace fas::testing;

  boost::asio::io_context io;
  auto port = bind_ephemeral_port(io);

  typedef ::iow::ip::tcp::server::server<> tcp_server;
  typedef ::iow::ip::tcp::server::options<> server_opt;
  auto server = std::make_shared<tcp_server>(io);
  server_opt sopt;
  sopt.addr = "127.0.0.1";
  sopt.port = std::to_string(port);
  sopt.connection.nonblocking = true;
  sopt.connection.input_handler =
    [](::iow::io::data_ptr, ::iow::io::io_id_t, ::iow::io::output_handler_t) noexcept {};
  server->start(sopt);

  typedef ::iow::ip::tcp::client::client<> client_type;
  typedef ::iow::ip::tcp::client::options client_opt;
  auto client = std::make_shared<client_type>(io);
  client_opt copt;
  copt.addr = "127.0.0.1";
  copt.port = std::to_string(port);
  copt.async_connect = true;
  copt.reconnect_timeout_ms = 0;
  copt.connection.input_handler =
    [](::iow::io::data_ptr, ::iow::io::io_id_t, ::iow::io::output_handler_t) noexcept {};

  std::atomic<bool> connected{false};
  copt.args.connect_handler = [&]() noexcept { connected = true; };

  client->start(copt);
  poll_for(io, std::chrono::milliseconds(500));

  t << is_true<assert>(connected.load()) << "client did not connect via IP" << FAS_FL;

  client->stop();
  server->stop();
  io.stop();
}

UNIT(client_connect_localhost_async_resolve, "клиент к localhost: async_resolve + connect")
{
  using namespace fas::testing;

  boost::asio::io_context io;
  auto port = bind_ephemeral_port(io);

  typedef ::iow::ip::tcp::server::server<> tcp_server;
  typedef ::iow::ip::tcp::server::options<> server_opt;
  auto server = std::make_shared<tcp_server>(io);
  server_opt sopt;
  sopt.addr = "127.0.0.1";
  sopt.port = std::to_string(port);
  sopt.connection.nonblocking = true;
  sopt.connection.input_handler =
    [](::iow::io::data_ptr, ::iow::io::io_id_t, ::iow::io::output_handler_t) noexcept {};
  server->start(sopt);

  typedef ::iow::ip::tcp::client::client<> client_type;
  typedef ::iow::ip::tcp::client::options client_opt;
  auto client = std::make_shared<client_type>(io);
  client_opt copt;
  copt.addr = "localhost";
  copt.port = std::to_string(port);
  copt.async_connect = false; // hostname всё равно идёт через async_resolve→async_connect
  copt.reconnect_timeout_ms = 0;
  copt.connection.input_handler =
    [](::iow::io::data_ptr, ::iow::io::io_id_t, ::iow::io::output_handler_t) noexcept {};

  std::atomic<bool> connected{false};
  copt.args.connect_handler = [&]() noexcept { connected = true; };

  client->start(copt);
  poll_for(io, std::chrono::milliseconds(1500));

  t << is_true<assert>(connected.load()) << "client did not connect via localhost resolve" << FAS_FL;

  client->stop();
  server->stop();
  io.stop();
}

UNIT(listen_resolve_system_error, "listen resolve с битым host — system_error с ec, не raw throw из asio")
{
  using namespace fas::testing;

  boost::asio::io_context io;
  typedef ::iow::ip::tcp::server::server<> tcp_server;
  typedef ::iow::ip::tcp::server::options<> server_opt;
  auto server = std::make_shared<tcp_server>(io);
  server_opt sopt;
  sopt.addr = "this.host.definitely.does.not.exist.invalid";
  sopt.port = "1";
  sopt.connection.input_handler =
    [](::iow::io::data_ptr, ::iow::io::io_id_t, ::iow::io::output_handler_t) noexcept {};

  bool got_system_error = false;
  try
  {
    server->start(sopt);
  }
  catch (const boost::system::system_error& )
  {
    got_system_error = true;
  }

  t << is_true<assert>(got_system_error) << "expected system_error from failed listen resolve" << FAS_FL;

  server->stop();
  io.stop();
}

BEGIN_SUITE(resolve, "async/IP resolve suite")
  ADD_UNIT(try_make_ip_endpoint_v4)
  ADD_UNIT(try_make_ip_endpoint_hostname)
  ADD_UNIT(try_make_ip_endpoint_bad_port)
  ADD_UNIT(client_connect_ip_async)
  ADD_UNIT(client_connect_localhost_async_resolve)
  ADD_UNIT(listen_resolve_system_error)
END_SUITE(resolve)

BEGIN_TEST
  RUN_SUITE(resolve)
END_TEST
