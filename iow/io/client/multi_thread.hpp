#pragma once

#include <iow/io/client/multi_base.hpp>
#include <memory>

namespace iow{ namespace io{ namespace client{

template<typename Client>
class multi_thread: public multi_base<Client, true>
{
  using super = multi_base<Client, true>;
public:
  typedef typename super::io_context_type io_context_type;
  
  explicit multi_thread(io_context_type& io)
    : super(io)
  {}
  
  template<typename Opt>
  void start(Opt opt)
  {
    super::start(std::move(opt), threads_(opt) );
  }

  template<typename Opt>
  void reconfigure(Opt opt, size_t )
  {
    super::reconfigure(std::move(opt), threads_(opt) );
  }
  
private: 
  
  template<typename Opt>
  static int threads_(Opt& opt) 
  {
    return opt.threads==0 ? 1 : opt.threads;
  }
};

}}}
