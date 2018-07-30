#pragma once

#include <memory>
#include <iow/asio.hpp>
#include <iow/io/rw/tags.hpp>
#include <iow/logger.hpp>

namespace iow{ namespace io{ namespace socket{

struct ad_initialize
{
  template<typename T, typename O>
  void operator()(T& t, O&& opt)  const
  {
    if ( opt.receive_buffer_size != 0 )
    {
      boost::asio::socket_base::receive_buffer_size option( static_cast<int>(opt.receive_buffer_size) );
      boost::system::error_code ec;
      t.descriptor().set_option(option, ec);
      this->check(ec, "receive_buffer_size", opt.receive_buffer_size);
    }

    if ( opt.send_buffer_size != 0 )
    {
      boost::asio::socket_base::send_buffer_size option( static_cast<int>(opt.send_buffer_size) );
      boost::system::error_code ec;
      t.descriptor().set_option(option, ec);
      this->check(ec, "send_buffer_size", opt.send_buffer_size);
    }
    
    t.get_aspect().template get< ::iow::io::rw::_initialize_ >()(t, opt);
  }
  
  static void check(const boost::system::error_code& ec, const char* name, size_t value)
  {
    only_for_log(name, value);
    if ( !ec )
    {
      IOW_LOG_DEBUG("socket::set_option " << name << "=" << value );
      return;
    }
    IOW_LOG_FATAL("socket::set_option " << name << "=" << value << " error: " << ec.message() );  
  }
};
 
}}}
