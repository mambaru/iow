#pragma once
#include <vector>

namespace iow{ namespace io{

struct buffer_stat
{
  size_t total_size = 0;
  size_t total_capacity = 0;
  size_t chunk_count = 0;
  size_t chunk_count_capacity = 0;
  std::vector<size_t> chunk_sizes;
  std::vector<size_t> chunk_capacities;

  buffer_stat& operator += (const buffer_stat& other )
  {
    total_size += other.total_size;
    total_capacity += other.total_capacity;
    chunk_count += other.chunk_count;
    chunk_count_capacity += other.chunk_count_capacity;
    if ( !other.chunk_sizes.empty() )
    {
      std::copy(other.chunk_sizes.begin(), other.chunk_sizes.end(), std::back_inserter(chunk_sizes));
      std::copy(other.chunk_capacities.begin(), other.chunk_capacities.end(), std::back_inserter(chunk_capacities));
    }
    return *this;
  }
};

}}
