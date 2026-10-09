#include <iow/ip/tcp/server/server.hpp>
#include <iow/ip/tcp/server/options.hpp>
#include <fas/testing.hpp>

#include <boost/asio.hpp>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

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

UNIT(get_stat_threads0, "get_stat при threads=0 считает соединения на origin")
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
  opt.threads = 0;
  opt.max_connections = 16;
  opt.connection.nonblocking = true;

  std::atomic<int> startups{0};
  opt.connection.startup_handler = [&](::iow::io::io_id_t, ::iow::io::output_handler_t) noexcept {
    ++startups;
  };
  opt.connection.input_handler =
    [](::iow::io::data_ptr, ::iow::io::io_id_t, ::iow::io::output_handler_t) noexcept {};

  server->start(opt);

  t << equal<assert, size_t>(server->get_stat().connection_count, 0) << FAS_FL;

  tcp::socket c1(io);
  boost::system::error_code ec;
  c1.connect(tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), port), ec);
  t << is_false<assert>(static_cast<bool>(ec)) << ec.message() << FAS_FL;
  t << stop;

  poll_for(io, std::chrono::milliseconds(300));
  t << is_true<assert>(startups.load() >= 1) << FAS_FL;
  t << equal<assert, size_t>(server->get_stat().connection_count, 1) << FAS_FL;

  server->stop();
  io.stop();
}

UNIT(get_stat_threads_mt, "get_stat при threads>0 суммирует по всем dup-acceptor'ам")
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
  opt.max_connections = 64;
  opt.connection.nonblocking = true;

  std::atomic<int> startups{0};
  opt.connection.startup_handler = [&](::iow::io::io_id_t, ::iow::io::output_handler_t) noexcept {
    ++startups;
  };
  opt.connection.input_handler =
    [](::iow::io::data_ptr, ::iow::io::io_id_t, ::iow::io::output_handler_t) noexcept {};

  server->start(opt);

  // раньше смотрели только origin→manager: при threads>0 всегда 0/мусор
  t << equal<assert, size_t>(server->get_stat().connection_count, 0) << FAS_FL;

  const int clients_n = 6;
  std::vector<tcp::socket> clients;
  clients.reserve(clients_n);
  auto ep = tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), port);
  for (int i = 0; i < clients_n; ++i)
  {
    clients.emplace_back(io);
    boost::system::error_code ec;
    clients.back().connect(ep, ec);
    t << is_false<assert>(static_cast<bool>(ec)) << ec.message() << FAS_FL;
    t << stop;
  }

  poll_for(io, std::chrono::milliseconds(800));
  t << equal<assert, int>(startups.load(), clients_n) << FAS_FL;
  t << equal<assert, size_t>(server->get_stat().connection_count, static_cast<size_t>(clients_n))
    << "mt get_stat connection_count=" << server->get_stat().connection_count << FAS_FL;

  server->stop();
  io.stop();
}

BEGIN_SUITE(get_stat, "server get_stat suite")
  ADD_UNIT(get_stat_threads0)
  ADD_UNIT(get_stat_threads_mt)
END_SUITE(get_stat)

BEGIN_TEST
  RUN_SUITE(get_stat)
END_TEST
