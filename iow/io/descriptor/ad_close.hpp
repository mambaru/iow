#pragma once
#include <iow/logger.hpp>
#include <exception>

namespace iow{ namespace io{ namespace descriptor{

struct ad_close
{
  template<typename T>
  void operator()(T& t) const
  {
    try
    {
      if ( t.descriptor().is_open() )
      {
        t.descriptor().close();
      }
    }
    catch(const std::exception& e)
    {
      IOW_LOG_ERROR("descriptor close failed: " << e.what()
                    << " — continue shutdown");
    }
    catch(...)
    {
      IOW_LOG_ERROR("descriptor close failed: unknown exception"
                    << " — continue shutdown");
    }
  }
};

}}}
