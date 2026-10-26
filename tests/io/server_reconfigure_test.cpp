#include <iow/ip/tcp/server/server.hpp>
#include <iow/ip/tcp/server/options.hpp>
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

bool socket_still_connected(tcp::socket& s)
{
  boost::system::error_code ec;
  s.non_blocking(true, ec);
  char buf[1];
  auto n = s.read_some(boost::asio::buffer(buf), ec);
  if ( ec == boost::asio::error::would_block || ec == boost::asio::error::try_again )
    return true;
  if ( !ec && n == 0 )
    return false;
  // eof / reset / not connected → dead
  return !ec;
}

template<typename Options>
void setup_handlers(Options& opt, std::atomic<int>& startups)
{
  opt.connection.nonblocking = true;
  opt.connection.startup_handler = [&](::iow::io::io_id_t, ::iow::io::output_handler_t) noexcept {
    ++startups;
  };
  opt.connection.input_handler =
    [](::iow::io::data_ptr, ::iow::io::io_id_t, ::iow::io::output_handler_t) noexcept {};
}

} // namespace

UNIT(reconfigure_increase_keeps_clients, "threads↑ не сбрасывает существующие соединения")
{
  using namespace fas::testing;

  boost::asio::io_context io;
  auto port = bind_ephemeral_port(io);

  typedef ::iow::ip::tcp::server::server<> tcp_server;
  typedef ::iow::ip::tcp::server::options<> options;

  auto server = std::make_shared<tcp_server>(io);
  options opt;
  opt.addr = "127.0.0.1";
  opt.port = std::to_string(port);
  opt.threads = 2;
  opt.max_connections = 16;
  std::atomic<int> startups{0};
  setup_handlers(opt, startups);

  server->start(opt);

  tcp::socket c1(io);
  boost::system::error_code ec;
  c1.connect(tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), port), ec);
  t << is_false<assert>(static_cast<bool>(ec)) << ec.message() << FAS_FL;
  t << stop;

  poll_for(io, std::chrono::milliseconds(300));
  t << is_true<assert>(startups.load() >= 1) << FAS_FL;
  t << stop;

  opt.threads = 4;
  startups = 0;
  server->reconfigure(opt);

  t << is_true<assert>(socket_still_connected(c1)) << "client dropped on threads increase" << FAS_FL;
  t << stop;

  tcp::socket c2(io);
  c2.connect(tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), port), ec);
  t << is_false<assert>(static_cast<bool>(ec)) << ec.message() << FAS_FL;
  t << stop;

  poll_for(io, std::chrono::milliseconds(500));
  t << is_true<assert>(startups.load() >= 1) << "accept after increase failed" << FAS_FL;

  server->stop();
  io.stop();
}

UNIT(reconfigure_decrease_still_accepts, "threads↓ сбрасывает лишние acceptor'ы, listen жив")
{
  using namespace fas::testing;

  boost::asio::io_context io;
  auto port = bind_ephemeral_port(io);

  typedef ::iow::ip::tcp::server::server<> tcp_server;
  typedef ::iow::ip::tcp::server::options<> options;

  auto server = std::make_shared<tcp_server>(io);
  options opt;
  opt.addr = "127.0.0.1";
  opt.port = std::to_string(port);
  opt.threads = 3;
  opt.max_connections = 16;
  std::atomic<int> startups{0};
  setup_handlers(opt, startups);

  server->start(opt);

  opt.threads = 1;
  server->reconfigure(opt);

  startups = 0;
  tcp::socket c1(io);
  boost::system::error_code ec;
  c1.connect(tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), port), ec);
  t << is_false<assert>(static_cast<bool>(ec)) << ec.message() << FAS_FL;
  t << stop;

  poll_for(io, std::chrono::milliseconds(500));
  t << is_true<assert>(startups.load() >= 1) << "accept after decrease failed" << FAS_FL;

  server->stop();
  io.stop();
}

UNIT(reconfigure_options_keeps_clients, "смена options без threads не сбрасывает клиентов")
{
  using namespace fas::testing;

  boost::asio::io_context io;
  auto port = bind_ephemeral_port(io);

  typedef ::iow::ip::tcp::server::server<> tcp_server;
  typedef ::iow::ip::tcp::server::options<> options;

  auto server = std::make_shared<tcp_server>(io);
  options opt;
  opt.addr = "127.0.0.1";
  opt.port = std::to_string(port);
  opt.threads = 2;
  opt.max_connections = 16;
  std::atomic<int> startups{0};
  setup_handlers(opt, startups);

  server->start(opt);

  tcp::socket c1(io);
  boost::system::error_code ec;
  c1.connect(tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), port), ec);
  t << is_false<assert>(static_cast<bool>(ec)) << ec.message() << FAS_FL;
  t << stop;

  poll_for(io, std::chrono::milliseconds(300));
  t << is_true<assert>(startups.load() >= 1) << FAS_FL;
  t << stop;

  opt.max_connections = 32;
  server->reconfigure(opt);

  t << is_true<assert>(socket_still_connected(c1)) << "client dropped on options reconfigure" << FAS_FL;

  server->stop();
  io.stop();
}

BEGIN_SUITE(server_reconfigure, "server reconfigure suite")
  ADD_UNIT(reconfigure_increase_keeps_clients)
  ADD_UNIT(reconfigure_decrease_still_accepts)
  ADD_UNIT(reconfigure_options_keeps_clients)
END_SUITE(server_reconfigure)

BEGIN_TEST
  RUN_SUITE(server_reconfigure)
END_TEST
