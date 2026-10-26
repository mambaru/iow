#pragma once

#include <iow/io/client/connection.hpp>
#include <iow/ip/tcp/client/options.hpp>
#include <iow/ip/tcp/connection/aspect.hpp>
#include <iow/ip/endpoint.hpp>
#include <iow/io/basic/tags.hpp>
#include <fas/aop.hpp>
#include <iow/system.hpp>
#include <iow/logger.hpp>

namespace iow{ namespace ip{ namespace tcp{ namespace client{

struct ad_sync_resolver
{
  template<typename T, typename Opt>
  boost::asio::ip::tcp::endpoint operator()(T& t, const Opt& opt) const
  {
    boost::system::error_code ec;
    auto endpoint = ::iow::ip::resolve_endpoint<boost::asio::ip::tcp>(
      t.descriptor().get_executor(), opt.addr, opt.port, ec);
    if ( ec )
    {
      IOW_LOG_ERROR("Client Resolve: " << ec.message())
    }
    else
    {
      IOW_LOG_DEBUG("Client Resolve: " << opt.addr << ":" << opt.port)
    }
    return endpoint;
  }
};


struct aspect : fas::aspect<
  fas::type< ::iow::io::_options_type_, options >,
  fas::advice< ::iow::io::client::_sync_resolver_, ad_sync_resolver>,
  ::iow::ip::tcp::connection::aspect
>{};

}}}}
