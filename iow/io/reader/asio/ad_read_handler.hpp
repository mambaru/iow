#pragma once

#include <iow/io/reader/asio/tags.hpp>
#include <iow/io/reader/tags.hpp>
#include <iow/io/basic/tags.hpp>

#include <iow/logger.hpp>
#include <iow/system.hpp>
#include <iow/asio.hpp>

namespace iow{ namespace io{ namespace reader{ namespace asio{


struct ad_read_handler
{
  template<typename T, typename P>
  void operator()(T& t, P p, ::iow::system::error_code ec , std::size_t bytes_transferred) const
  {
    if ( !ec )
    {
      p.second = bytes_transferred;
      
      IOW_LOG_TRACE("READ[" << std::string(p.first, p.first + p.second) << "] id=" << t.get_id_(t) )
      
      t.get_aspect().template get< ::iow::io::reader::_complete_>()(t, std::move(p));
    }
    else
    {
      p.second = 0;
      t.get_aspect().template get< _error_handler_>()(t, std::move(p), std::move(ec));
    }
  }
};

}}}}
