#pragma once

#include <memory>

namespace iow{ namespace io{ namespace acceptor{

struct ad_async_accept
{
  template<typename T, typename P, typename H>
  void operator()(T& t, P p, H handler)
  {
    std::cout << "DEBUG acceptor::ad_async_accept" << std::endl;
    t.descriptor().async_accept( p->descriptor(), std::move(handler) );
  }
};

  
}}}
