#pragma once

#include <iow/io/aux/write_buffer.hpp>


namespace iow{ namespace io{ namespace writer{ namespace data{

struct options:
  ::iow::io::write_buffer_options
{
  //!!! Вынести на более высокий уровень
#error **
  size_t maxsize = 0;

  size_t wrnsize = 1024*1024;
};

}}}}
