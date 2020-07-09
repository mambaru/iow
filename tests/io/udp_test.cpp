#include <iostream>
#include <iow/ip/udp/server/server.hpp>
int main()
{
  typedef iow::ip::udp::server::server server_type;
  typedef iow::ip::udp::server::options options_type;
  options_type opt;
  opt.threads = 3;
  opt.addr = "0.0.0.0";
  opt.port = "12345";
  boost::asio::io_context ios;
  typedef boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_type;
  work_type wrk(ios.get_executor());

  server_type srv(ios);
  srv.start(opt);
  ios.run();
  return 0;
}
