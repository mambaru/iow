#pragma once
#include <iow/io/basic/context.hpp>
#include <iow/io/types.hpp>

namespace iow{ namespace io{ namespace descriptor{

template<typename ID, typename DataType, typename DataPtr>
struct context
{
  typedef ID       io_id_type;
  typedef DataType data_type;
  typedef DataPtr  data_ptr;

  typedef std::function< void(data_ptr) > output_handler_type;
  typedef std::function< void(data_ptr, io_id_type, output_handler_type )> input_handler_type;
  typedef std::function< void(io_id_type, output_handler_type) > startup_handler_type;
  typedef std::function< void(io_id_type) > shutdown_handler_type;
  typedef std::function< void(int, std::string)> fatal_handler_type;

  output_handler_type output_handler;
  input_handler_type input_handler;
  startup_handler_type  startup_handler;
  shutdown_handler_type shutdown_handler;
  fatal_handler_type    fatal_handler;
};

}}}
