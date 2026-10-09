#pragma once

#include <iow/io/acceptor/tags.hpp>
#include <iow/ip/endpoint.hpp>
#include <iow/asio.hpp>
#include <iow/logger.hpp>
#include <iow/system.hpp>

namespace iow{ namespace ip{ namespace tcp{ namespace acceptor{

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
      IOW_LOG_ERROR("Listen resolve failed for " << opt.addr << ":" << opt.port
                    << ": " << ec.message())
      throw boost::system::system_error(ec);
    }
    return endpoint;
  }
};

}}}}
