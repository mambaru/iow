#pragma once

#include <iow/io/acceptor/tags.hpp>
#include <iow/asio.hpp>

namespace iow{ namespace ip{ namespace tcp{ namespace acceptor{

struct ad_sync_resolver
{
  template<typename T, typename Opt>
  boost::asio::ip::tcp::endpoint operator()(T& t, const Opt& opt) const
  {
    boost::asio::ip::tcp::resolver resolver( t.descriptor().get_executor() );
    boost::asio::ip::tcp::resolver::results_type results = resolver.resolve(opt.addr, opt.port);
    boost::asio::ip::tcp::endpoint endpoint = *(results.begin());
    return endpoint;
  }
};

}}}}
