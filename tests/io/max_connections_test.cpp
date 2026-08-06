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

} // namespace

UNIT(max_connections_reject, "второй accept отклоняется при max_connections=1")
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
  opt.max_connections = 1;
  opt.connection.nonblocking = true;

  std::atomic<int> startups{0};
  opt.connection.startup_handler = [&](::iow::io::io_id_t, ::iow::io::output_handler_t) noexcept {
    ++startups;
  };
  opt.connection.input_handler =
    [](::iow::io::data_ptr, ::iow::io::io_id_t, ::iow::io::output_handler_t) noexcept {};

  server->start(opt);

  tcp::socket c1(io);
  tcp::socket c2(io);
  auto ep = tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), port);

  boost::system::error_code ec1;
  boost::system::error_code ec2;
  c1.connect(ep, ec1);
  t << is_false<assert>(static_cast<bool>(ec1)) << ec1.message() << FAS_FL;
  t << stop;

  // дать accept/start первому соединению
  for (int i = 0; i < 50 && startups.load() == 0; ++i)
  {
    io.poll();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  c2.connect(ep, ec2);
  t << is_false<assert>(static_cast<bool>(ec2)) << ec2.message() << FAS_FL;
  t << stop;

  for (int i = 0; i < 50; ++i)
  {
    io.poll();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  t << equal<assert, int>(startups.load(), 1) << FAS_FL;
  t << equal<assert, size_t>(server->get_stat().connection_count, 1) << FAS_FL;

  // отклонённый сокет должен быть закрыт сервером
  char buf[1];
  boost::system::error_code rec;
  c2.read_some(boost::asio::buffer(buf), rec);
  t << is_true<expect>(static_cast<bool>(rec)) << "rejected socket still readable" << FAS_FL;

  server->stop();
  io.stop();
}

BEGIN_SUITE(max_connections, "max_connections suite")
  ADD_UNIT(max_connections_reject)
END_SUITE(max_connections)

BEGIN_TEST
  RUN_SUITE(max_connections)
END_TEST
