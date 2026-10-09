#pragma once

#include <iow/ip/udp/server/options.hpp>
#include <iow/ip/endpoint.hpp>
#include <iow/io/reader/asio/aspect.hpp>
#include <iow/io/writer/asio/aspect.hpp>
#include <iow/io/rw/aspect.hpp>
#include <iow/io/basic/aspect.hpp>
#include <iow/io/descriptor/tags.hpp>
#include <iow/io/socket/dgram/aspect.hpp>
#include <iow/logger.hpp>
#include <iow/system.hpp>
#include <fas/aop.hpp>
#include <mutex>
#include <vector>

namespace iow{ namespace ip{ namespace udp{ namespace server{

struct _open_;
struct _sync_resolver_;
struct ad_sync_resolver
{
  template<typename T, typename Opt>
  boost::asio::ip::udp::endpoint operator()(T& t, const Opt& opt) const
  {
    boost::system::error_code ec;
    auto endpoint = ::iow::ip::resolve_endpoint<boost::asio::ip::udp>(
      t.descriptor().get_executor(), opt.addr, opt.port, ec);
    if ( ec )
    {
      IOW_LOG_ERROR("UDP listen resolve failed for " << opt.addr << ":" << opt.port
                    << ": " << ec.message())
      throw boost::system::system_error(ec);
    }
    return endpoint;
  }
};


struct ad_open
{
  template<typename T, typename Opt>
  void operator()(T& t, const Opt& opt) const
  {
    const auto endpoint = t.get_aspect().template get<_sync_resolver_>()(t, opt);

    IOW_LOG_MESSAGE("UDP open " << opt.addr << ":" << opt.port)
    t.descriptor().open(endpoint.protocol());
    t.descriptor().bind(endpoint);
  }
};


struct aspect : fas::aspect<
    fas::advice<_open_, ad_open>,
    fas::advice<_sync_resolver_, ad_sync_resolver>,
    fas::type< ::iow::io::descriptor::_descriptor_type_, boost::asio::ip::udp::socket >,
    fas::value< ::iow::io::socket::dgram::_current_endpoint_, std::shared_ptr< boost::asio::ip::udp::endpoint > >,
    fas::type< ::iow::io::_options_type_, options >,
    ::iow::io::socket::dgram::aspect,
    ::iow::io::reader::asio::aspect,
    ::iow::io::writer::asio::aspect,
    ::iow::io::rw::aspect,
    ::iow::io::basic::aspect< std::recursive_mutex >::advice_list
>{};

}}}}
