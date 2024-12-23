#pragma once

#include <iow/memory.hpp>
#include <iow/io/types.hpp>
#include <queue>
#include <memory>
#include <string>
#include <iostream>

namespace iow{ namespace io{

struct write_buffer_options
{
  typedef std::function< data_ptr(size_t, size_t) > create_fun;
  typedef std::function< void(data_ptr) > free_fun;
  
  std::string sep;
  size_t bufsize = 8*1024;
  size_t maxbuf  = 8*1024; 
  size_t minbuf  = 0;
  size_t maxsize = 0; 
  bool first_as_is = true; // Если maxbuff или minbuff != 0 и bufsize!=0
 // bool chunk_stats = false; ///  собирать ли информацию по чанкам

  create_fun create;
  free_fun free;
};

}}
