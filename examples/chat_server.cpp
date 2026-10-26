// Простейший TCP-чат на iow: строки, разделённые '\n'.
// Запуск: ./chat_server [port]

#include <iow/ip/tcp/server/server.hpp>
#include <iow/ip/tcp/server/options.hpp>
#include <iow/io/types.hpp>

#include <boost/asio.hpp>
#include <iostream>
#include <map>
#include <mutex>
#include <string>

int main(int argc, char* argv[])
{
  using namespace iow::io;
  typedef iow::ip::tcp::server::server<> server_t;
  typedef iow::ip::tcp::server::options<> options_t;

  const std::string port = (argc > 1) ? argv[1] : "30000";

  boost::asio::io_context io;
  auto server = std::make_shared<server_t>(io);

  std::mutex mutex;
  std::map<io_id_t, output_handler_t> peers;

  options_t opt;
  opt.addr = "0.0.0.0";
  opt.port = port;
  opt.connection.reader.sep = "\n";
  opt.connection.writer.sep = "\n";

  opt.connection.startup_handler = [&](io_id_t id, output_handler_t out) noexcept
  {
    std::lock_guard<std::mutex> lk(mutex);
    peers[id] = std::move(out);
    std::cout << "+ client " << id << " (" << peers.size() << ")\n";
  };

  opt.connection.shutdown_handler = [&](io_id_t id) noexcept
  {
    std::lock_guard<std::mutex> lk(mutex);
    peers.erase(id);
    std::cout << "- client " << id << " (" << peers.size() << ")\n";
  };

  opt.connection.input_handler = [&](data_ptr d, io_id_t /*id*/, output_handler_t) noexcept
  {
    if ( d == nullptr )
      return;

    const std::string msg(d->begin(), d->end());
    std::cout << msg << "\n";

    std::lock_guard<std::mutex> lk(mutex);
    for (auto& p : peers)
      p.second( make(msg) );
  };

  server->start(opt);
  std::cout << "chat_server on :" << port << "\n";
  io.run();
}
