#pragma once

#include <memory>
#include <fas/system/nullptr.hpp>

namespace iow{ namespace io{ namespace acceptor{

struct ad_async_accept
{
  template<typename T, typename P, typename H>
  void operator()(T& t, P p, H handler) const
  {
    t.descriptor().async_accept( p->descriptor(), std::move(handler)/*, fas_null_param*/ );
  }
};


}}}
