#pragma once

#include <iow/io/client/multi_base.hpp>
#include <memory>

namespace iow{ namespace io{ namespace client{

template<typename Client>
class multi_client: public multi_base<Client, false>
{
  typedef multi_base<Client, false> super;

public:
  typedef typename super::io_context_type io_context_type;
  
  explicit multi_client(io_context_type& io)
    : super(io)
  {}
  
  template<typename Opt>
  void start(Opt opt)
  {
    if ( opt.show_connect_log) 
    { IOW_LOG_MESSAGE("Client start for " << opt.connect_count << " connections " << opt.addr << ":" << opt.port << "" ) }

    super::start(std::move(opt), opt.connect_count);
  }

  template<typename Opt>
  void reconfigure(Opt opt)
  {
    super::reconfigure(std::move(opt), opt.connect_count);
  }
};

}}}
