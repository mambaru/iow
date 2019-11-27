#include <iostream>
#include <iow/io/aux/read_buffer.hpp>

#include <chrono>

#include <fcntl.h>
#include <unistd.h>


#ifndef NDEBUG
#define TOTAL 1
#else
#define TOTAL 1000
#endif
#define DATA_SIZE 50000

typedef ::iow::io::data_type data_type;
typedef ::iow::io::read_buffer  read_buffer;
typedef ::iow::io::data_ptr data_ptr;

int main()
{
  data_type data(DATA_SIZE);
  std::fill_n(data.begin(), 'A', DATA_SIZE);
  data.push_back('\r');
  data.push_back('\n');
  std::vector<data_ptr> data_arr(TOTAL);
  for (auto& i: data_arr)
  {
    i = data_ptr(new data_type(data.begin(), data.end()));
  }

  read_buffer buf;
  read_buffer::options_type opt;

  buf.get_options(opt);
  opt.bufsize = 512;
  opt.minbuf = 128;
  opt.maxbuf = 1024;
  opt.sep = "\r\n";
  buf.set_options(opt);

  std::cout << "start... " << std::endl;

  auto start = std::chrono::high_resolution_clock::now();
  long detach_span = 0;
  long init_span = 0;
  for (int i=0; i < TOTAL; ++i)
  {

    auto start1 = std::chrono::high_resolution_clock::now();
    auto beg = data.begin();
    auto end = data.end();
    while ( beg!=end )
    {
      auto p = buf.next();
      auto itr = beg;
      std::ptrdiff_t dist = std::distance(beg, end);
      if ( size_t(dist) > p.second ) { itr += std::ptrdiff_t(p.second); }
      else { itr = end; }
      std::copy(beg, itr, p.first);
      p.second = size_t( std::distance(beg, itr) );
      buf.confirm(p);
      beg = itr;
    }
    auto finish1 = std::chrono::high_resolution_clock::now();
    init_span += std::chrono::duration_cast<std::chrono::microseconds>( finish1 - start1).count();

    start1 = std::chrono::high_resolution_clock::now();
    buf.detach();
    finish1 = std::chrono::high_resolution_clock::now();
    detach_span += std::chrono::duration_cast<std::chrono::microseconds>( finish1 - start1).count();
  }
  auto finish = std::chrono::high_resolution_clock::now();
  auto span = std::chrono::duration_cast<std::chrono::microseconds>( finish - start).count();

  std::cout << "count: " << TOTAL << std::endl;
  std::cout << "time: " << span << " microseconds" << std::endl;
  std::cout << "rate: " << TOTAL*1000000 / span  << std::endl;
  std::cout << "init time: " << init_span << " microseconds" << std::endl;
  std::cout << "init rate: " << TOTAL*1000000 / init_span  << std::endl;
  std::cout << "detach time: " << detach_span << " microseconds" << std::endl;
  std::cout << "detach rate: " << TOTAL*1000000 / detach_span  << std::endl;
}
