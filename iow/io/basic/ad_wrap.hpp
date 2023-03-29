#pragma once

#include <iow/io/basic/tags.hpp>
#include <utility>

namespace iow{ namespace io{ namespace basic{

struct ad_wrap
{
  template<typename T, typename Handler, typename AltHandler>
  auto operator()(T& t, Handler&& h, AltHandler&& nh) const
    -> decltype( t.get_aspect().template get<_context_>().holder.wrap( h, nh ) ) 
  {
    return t.get_aspect().template get<_context_>().holder.wrap( std::forward<Handler>(h), std::forward<AltHandler>(nh) ) ;
  }
};

}}}
