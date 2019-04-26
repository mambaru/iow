#pragma once

#include <memory>
#include <fas/system/nullptr.hpp>

namespace iow{ namespace io{ namespace acceptor{

struct ad_async_accept
{
  template<typename T, typename P, typename H>
  void operator()(T& t, P p, H handler) const
  {
#if defined(__GNUC__) && (__GNUC___ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9))
    t.descriptor().async_accept( p->descriptor(), std::move(handler), nullptr );
#else
    t.descriptor().async_accept( p->descriptor(), std::move(handler), static_cast<void*>(0) );
#endif
  }
};

  
}}}
