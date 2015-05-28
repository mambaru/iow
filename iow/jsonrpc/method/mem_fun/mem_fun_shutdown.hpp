#pragma once

#include <iow/jsonrpc/method/mem_fun/mem_fun_helper.hpp>

namespace iow{ namespace jsonrpc{

template<
  typename Target, 
  void (Target::*mem_ptr)( io::io_id_t ) 
>
struct mem_fun_shutdown
{
  template<typename T>
  void operator()(T& t, io::io_id_t id) const
  {
    if (auto trg = t.provider() )
    {
      (trg.get()->*mem_ptr)( id );
    }
    else
    {
    }
  }
};

}} // iow

