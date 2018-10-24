#pragma once

#include <iow/io/socket/dgram/socket.hpp>
#include <iow/ip/udp/client/aspect.hpp>
#include <iow/io/client/client.hpp>
#include <fas/aop.hpp>

namespace iow{ namespace ip{ namespace udp{ namespace client{

template<typename A = fas::aspect<> >
class connection_base
  : public ::iow::io::socket::dgram::socket_base< typename fas::merge_aspect<A, aspect>::type >
{
public:
  typedef connection_base<A> self;
  typedef ::iow::io::socket::dgram::socket_base< typename fas::merge_aspect<A, aspect>::type > super;
  typedef typename super::descriptor_type descriptor_type;
  typedef typename super::data_ptr data_ptr;
  
  explicit connection_base(descriptor_type&& desc)
    : super( std::move( desc ) )
  {}
  
  template<typename T, typename Opt>
  void connect_(T& t, Opt&& opt)
  {
    this->get_aspect().template get<_open_>()(t, std::forward<Opt>(opt) );
  }
};


template<typename A = fas::aspect<> >
using client = ::iow::io::client::client< connection_base<A> >;

/*class client: public client_base<>
{
  typedef client_base<> super;
public:
  typedef super::io_service_type io_service_type; 
  explicit client(io_service_type& io)
    : super( io )
  {}
};*/


}}}}
