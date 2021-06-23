
#include <iostream>
#include <iow/ip/tcp/server/server.hpp>
#include <iow/ip/tcp/server/options.hpp>
#include <iow/ip/tcp/client/client.hpp>
#include <wflow/workflow.hpp>
#include <thread>
#include <mutex>
#include <memory>

boost::asio::io_context g_io_context;
std::atomic<int> connect_count;
void server();
void server()
{
  typedef ::iow::ip::tcp::server::server<> tcp_server;
  typedef ::iow::ip::tcp::server::options<> options;

  auto acceptor = std::make_shared<tcp_server>( g_io_context  );
  options opt;
  opt.addr = "0.0.0.0";
  opt.port = "12345";

  std::cout << "server start..." << std::endl;
  acceptor->start(opt);
  std::cout << "server run..." << std::endl;
  g_io_context.run();
  std::cout << "server done" << std::endl;
}


int main()
{
  connect_count = 1;

  typedef ::iow::ip::tcp::client::client<> client_type;
  typedef ::iow::ip::tcp::client::options options_type;

  auto tcp_client = std::make_shared<client_type>(g_io_context);


  options_type opt;
  opt.connect_count = connect_count;
  opt.args.connect_handler = []()
  {
    std::cout << "connect ready" << std::endl;
    --connect_count;
    if ( connect_count==0 )
    {
      boost::asio::post(g_io_context, []()
      {
        std::cout << "stop io ..." << std::endl;
        sleep(1);
        g_io_context.stop();
        std::cout << "stop io ... ready" << std::endl;
      }/*, nullptr*/);
    }
  };
  opt.addr = "0.0.0.0";
  opt.port = "12345";
  opt.reconnect_timeout_ms = 1000;
  auto workflow = std::make_shared< wflow::workflow>(g_io_context);
  opt.args.workflow = workflow;
  opt.connection.input_handler=[](iow::io::data_ptr, iow::io::io_id_t, ::iow::io::output_handler_t)
  {
  };
  opt.connection.fatal_handler = [](int code, std::string msg )
  {
    std::cout << "---> " << code  << ": " << msg << std::endl;
  };
  opt.connection.reader.trimsep=true;
  // std::cout << "client connect..." << std::endl;
  //tcp_client->connect(opt);
  std::cout << "client start..." << std::endl;
  tcp_client->start(opt);
  tcp_client->send( iow::io::make("Hello World!") );
  std::thread t(server);
  sleep(4);

  std::cout << "client main run..." << std::endl;
  g_io_context.run();
  if ( connect_count != 0 )
  {
    std::cout << "TEST FAIL" << std::endl;
    std::cout.flush();
    abort();
  }
  else
  {
    std::cout << "OK" << std::endl;
  }
  t.join();
  return 0;
}
