
/*
#include <iostream>
#include <iow/io/descriptor/holder.hpp>
#include <iow/io/acceptor/aspect.hpp>
#include <iow/asio.hpp>

typedef ::iow::io::acceptor::options options;

typedef ::iow::ip::tcp::connection::connection<> test_connection;

typedef ::iow::io::acceptor::acceptor<
  test_connection,
  fas::aspect<
    fas::type< ::iow::io::_options_type_, options >,
    fas::type< ::iow::io::descriptor::_descriptor_type_, boost::asio::ip::tcp::acceptor >
> > tcp_acceptror;
*/
/*typedef ::iow::io::descriptor::holder< fas::aspect<
  fas::type< ::iow::io::_options_type_, fas::empty_type >,
  ::iow::io::acceptor::aspect::advice_list,
  fas::type< ::iow::io::descriptor::_descriptor_type_, boost::asio::ip::tcp::acceptor >
> > tcp_acceptror;
*/

#include <iostream>
#include <iow/ip/tcp/acceptor/acceptor.hpp>
#include <iow/ip/tcp/server/server.hpp>

int main()
{
  typedef ::iow::ip::tcp::acceptor::acceptor<> tcp_acceptor;
  typedef tcp_acceptor::descriptor_type accept_descriptor;
  typedef ::iow::ip::tcp::acceptor::options<> options;
  boost::asio::io_context io_context;
  accept_descriptor desc = accept_descriptor(io_context);
  auto acceptor = std::make_shared<tcp_acceptor>( std::move(desc)  );
  options opt;
  opt.addr = "0.0.0.0";
  opt.port = "12345";
  acceptor->listen(opt);
  acceptor->start(opt);
  io_context.run();

  ::iow::ip::tcp::server::server<> s(io_context);
  s.get_stat();

}
