#pragma once

#include <memory>

#include <fas/aop.hpp>
#include <fas/integral/bool_.hpp>
#include <iostream>
#include <iow/logger.hpp>

namespace iow{ namespace io{ namespace descriptor{ 

struct ad_after_start
{
  template<typename T>
  void operator()(T& t)  const
  {
    auto& cntx = t.get_aspect().template get< _context_ >();
    if ( cntx.startup_handler )
    {
      cntx.startup_handler( t.get_id_(t), cntx.output_handler);
    }
  }
};
  
}}}
