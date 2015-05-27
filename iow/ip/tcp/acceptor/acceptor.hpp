#pragma once

#include <iow/ip/tcp/acceptor/aspect.hpp>
#include <iow/ip/tcp/connection/connection.hpp>
#include <iow/io/acceptor/acceptor.hpp>
#include <fas/aop.hpp>

namespace iow{ namespace ip{ namespace tcp{ namespace acceptor{

//struct connection: public ::iow::ip::tcp::connection::connection<>{};
  
typedef ::iow::ip::tcp::connection::connection<> connection;
  
template<
  typename ConnectionType = connection, 
  typename A = fas::aspect<> 
>
using acceptor = ::iow::io::acceptor::acceptor<
  ConnectionType,
  typename fas::merge_aspect<
    A,
    aspect
  >::type
>;
  
}}}}
