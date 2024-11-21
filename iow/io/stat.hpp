#pragma once
#include <iow/io/aux/buffer_stat.hpp>

namespace iow{ namespace io{

struct connection_stat
{
  size_t connection_count = 1;
  buffer_stat reader;
  buffer_stat writer;

  connection_stat& operator += (const connection_stat& other)
  {
    connection_count += other.connection_count;
    reader += other.reader;
    writer += other.writer;
    return *this;
  }
};

}}
