#pragma once

#include <iow/ip/udp/client/options.hpp>
#include <iow/io/reader/asio/aspect.hpp>
#include <iow/io/writer/asio/aspect.hpp>
#include <iow/io/rw/aspect.hpp>
#include <iow/io/basic/aspect.hpp>
#include <iow/io/descriptor/tags.hpp>
#include <iow/io/socket/dgram/aspect.hpp>
#include <fas/aop.hpp>
#include <mutex>
#include <vector>

namespace iow{ namespace ip{ namespace udp{ namespace client{

struct _open_;
struct _sync_resolver_;
struct ad_sync_resolver
{
  template<typename T, typename Opt>
  boost::asio::ip::udp::endpoint operator()(T& t, const Opt& opt) const
  {
    boost::system::error_code ec;
    boost::asio::ip::udp::resolver resolver( t.descriptor().get_executor() );
    boost::asio::ip::udp::resolver::results_type results = resolver.resolve(opt.addr, opt.port, ec);
    boost::asio::ip::udp::endpoint endpoint = *(results.begin());

    if ( ec && opt.args.error_handler!=nullptr )
    {
      IOW_LOG_ERROR("UDP resolve error:" << ec.message() );
      opt.args.error_handler(ec);
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
    t.descriptor().open(endpoint.protocol());
    //t.descriptor().bind(endpoint);
    /*boost::system::error_code ec;
    t.descriptor().connect(endpoint, ec);
    if ( !ec )
    {
      if ( opt.args.connect_handler!=nullptr )
        opt.args.connect_handler();
    }
    else
    {
      IOW_LOG_ERROR("UDP connect error:" << ec.message() );
      if ( opt.args.error_handler!=nullptr )
        opt.args.error_handler(ec);
    }*/
    t.get_aspect().template get< ::iow::io::socket::dgram::_current_endpoint_>()
      = std::make_shared<boost::asio::ip::udp::endpoint>(endpoint);
    if ( opt.args.connect_handler!=nullptr )
      opt.args.connect_handler();

    IOW_LOG_MESSAGE("UDP connect " << opt.addr << ":" << opt.port)
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
