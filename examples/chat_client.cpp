// TCP-чат клиент на iow. Строки из stdin уходят на сервер, ответы печатаются.
// Запуск: ./chat_client [host] [port]

#include <iow/ip/tcp/client/client.hpp>
#include <iow/ip/tcp/client/options.hpp>
#include <iow/io/types.hpp>

#include <boost/asio.hpp>
#include <atomic>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char* argv[])
{
  using namespace iow::io;
  typedef iow::ip::tcp::client::client<> client_t;
  typedef iow::ip::tcp::client::options options_t;

  const std::string host = (argc > 1) ? argv[1] : "127.0.0.1";
  const std::string port = (argc > 2) ? argv[2] : "30000";

  boost::asio::io_context io;
  auto client = std::make_shared<client_t>(io);
  std::atomic_bool ready{false};

  options_t opt;
  opt.addr = host;
  opt.port = port;
  opt.async_connect = true;
  opt.reconnect_timeout_ms = 1000;
  opt.connection.reader.sep = "\n";
  opt.connection.writer.sep = "\n";

  // Отложенный reconnect без wflow: обычный asio-таймер.
  opt.args.delayed_handler =
    [&io](std::chrono::milliseconds ms, std::function<void()> handler)
  {
    auto timer = std::make_shared<boost::asio::steady_timer>(io, ms);
    timer->async_wait([timer, handler](const boost::system::error_code& ec)
    {
      if ( !ec )
        handler();
    });
  };

  opt.args.connect_handler = [&]() noexcept
  {
    ready = true;
    std::cout << "connected to " << host << ":" << port << "\n";
  };

  opt.connection.input_handler =
    [](data_ptr d, io_id_t, output_handler_t) noexcept
  {
    if ( d != nullptr )
      std::cout << std::string(d->begin(), d->end()) << "\n";
  };

  client->start(opt);

  std::thread input([&]()
  {
    std::string line;
    while ( std::getline(std::cin, line) )
    {
      if ( !ready )
        continue;
      boost::asio::post(io, [client, line]()
      {
        client->send( make(line) );
      });
    }
    boost::asio::post(io, [&]() { io.stop(); });
  });

  io.run();
  input.join();
  client->stop();
}
