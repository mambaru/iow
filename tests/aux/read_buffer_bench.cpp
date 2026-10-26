#include <iostream>
#include <iow/io/aux/read_buffer.hpp>
#include <iow/io/aux/data_pool.hpp>
#include <chrono>
#include <cstdlib>
#include <string>

#ifndef NDEBUG
#define TOTAL 1
#else
#define TOTAL 1000
#endif

typedef ::iow::io::data_type data_type;
typedef ::iow::io::data_ptr data_ptr;
typedef ::iow::io::read_buffer  read_buffer;
typedef ::iow::io::data_pool data_pool;

enum class pool_mode { both, with_pool, without_pool };

void run(size_t packsize, size_t readsize, size_t total, size_t count,
         size_t bufsize, size_t minbuf, size_t maxbuf, bool use_pool);

void run(size_t packsize, size_t readsize, size_t total, size_t count,
         size_t bufsize, size_t minbuf, size_t maxbuf, bool use_pool)
{
  std::cout << "*****************************************" << std::endl;

  data_pool pool;
  data_pool::options_type pool_opt;
  pool_opt.poolsize = 64;
  pool.set_options(pool_opt);

  read_buffer buf;
  if ( bufsize != 0 )
  {
    read_buffer::options_type rb_opt;
    buf.get_options(rb_opt);
    rb_opt.bufsize = bufsize;
    rb_opt.minbuf = minbuf;
    rb_opt.maxbuf = maxbuf;
    rb_opt.sep = "\r\n";
    if ( use_pool )
    {
      rb_opt.create = [&pool](size_t sz, size_t mx){ return pool.create(sz, mx); };
      rb_opt.free = [&pool](data_ptr d){ pool.free(std::move(d)); };
    }
    buf.set_options(rb_opt);
    std::cout << "bufsize=" << rb_opt.bufsize
              << ", minbuf=" << rb_opt.minbuf
              << ", maxbuf=" << rb_opt.maxbuf
              << std::endl;
    std::cout << "Размер пакета: " << packsize << " 'чтение' за раз: " << readsize << std::endl;
  }

  size_t pack_count = 0;
  std::vector<char> indata;
  for (size_t i =0 ; i < count; ++i)
  {
    for (size_t j =0 ; j < packsize - 2; ++j)
      indata.push_back( static_cast<char>('0'+ j%packsize) );
    indata.push_back('\r');
    indata.push_back('\n');
    ++pack_count;
  }

  std::cout << "start... " << std::endl;
  auto start = std::chrono::high_resolution_clock::now();

  size_t parse_pack = 0;
  size_t read_block = 0;
  for (size_t i=0; i < total; ++i )
  {
    std::ptrdiff_t pos = 0;
    while ( static_cast<size_t>(pos) < indata.size() )
    {
      auto p = buf.next();
      if ( p.second + static_cast<size_t>(pos) > indata.size())
        p.second = indata.size() - static_cast<size_t>(pos);
      if ( p.second > readsize )
        p.second = readsize;

      ++read_block;
      std::copy(
        indata.begin() + pos,
        indata.begin() + pos + static_cast< std::ptrdiff_t >(p.second),
        p.first
      );
      buf.confirm(p);

      auto d = buf.detach();
      while ( d!=nullptr )
      {
        if ( d->size() != packsize || !std::equal(d->begin(), d->end(), indata.begin()) )
        {
          std::cout << "Error: [[" << std::string(d->begin(), d->end()) << "]] " << d->size() << std::endl;
          std::cout << "pos: " << pos << std::endl;
          std::cout << "i: " << i << std::endl;
          std::cout << "parse packs: " << parse_pack << std::endl;
          std::cout << "blocks: " << read_block << std::endl;
          abort();
        }
        ++parse_pack;
        if ( use_pool )
          pool.free(std::move(d));
        d = buf.detach();
      }
      pos += static_cast<std::ptrdiff_t>(p.second);
    }
  }

  auto finish = std::chrono::high_resolution_clock::now();

  auto span = std::chrono::duration_cast<std::chrono::microseconds>( finish - start).count();
  if ( span == 0 )
    span = 1;

  std::cout << "use data pool: " << std::boolalpha << use_pool << std::endl;
  if ( use_pool )
  {
    std::cout << "pool allocs: " << pool.allocs()
              << " reuses: " << pool.reuses()
              << " frees: " << pool.frees()
              << " pooled: " << pool.pooled()
              << std::endl;
  }
  std::cout << "time: " << span << " microseconds" << std::endl;
  std::cout << "reads blocks: " << read_block << std::endl;
  std::cout << "expected packs: " << pack_count * total << std::endl;
  std::cout << "parse packs: " << parse_pack << std::endl;
  std::cout << "read rate: " << read_block*1000000ul / static_cast<size_t>(span)  << std::endl;
  std::cout << "parse rate: " << parse_pack*1000000ul / static_cast<size_t>(span)  << std::endl;
}

static void run_case(size_t packsize, size_t readsize, size_t total, size_t count,
                     size_t bufsize, size_t minbuf, size_t maxbuf, pool_mode mode)
{
  if ( mode == pool_mode::both || mode == pool_mode::without_pool )
    run(packsize, readsize, total, count, bufsize, minbuf, maxbuf, false);
  if ( mode == pool_mode::both || mode == pool_mode::with_pool )
    run(packsize, readsize, total, count, bufsize, minbuf, maxbuf, true);
}

static void usage(const char* argv0)
{
  std::cerr << "Usage: " << argv0 << " [both|pool|nopool]\n"
            << "  both   — каждый кейс без пула и с пулом (по умолчанию)\n"
            << "  pool   — только с data_pool\n"
            << "  nopool — только без пула\n";
}

static pool_mode parse_mode(int argc, char** argv)
{
  if ( argc <= 1 )
    return pool_mode::both;

  const std::string a = argv[1];
  if ( a == "both" || a == "--both" )
    return pool_mode::both;
  if ( a == "pool" || a == "--pool" )
    return pool_mode::with_pool;
  if ( a == "nopool" || a == "--nopool" || a == "no-pool" || a == "--no-pool" )
    return pool_mode::without_pool;
  if ( a == "-h" || a == "--help" )
  {
    usage(argv[0]);
    std::exit(0);
  }

  usage(argv[0]);
  std::exit(2);
}

int main(int argc, char** argv)
{
  const pool_mode mode = parse_mode(argc, argv);
  std::cout << "pool mode: "
            << (mode == pool_mode::both ? "both"
                : mode == pool_mode::with_pool ? "pool" : "nopool")
            << std::endl;

  run_case(10,  1,   TOTAL, 1000, 1,    1,    1, mode);
  run_case(10,  10,  TOTAL, 1000, 10,   10,   10, mode);
  run_case(10,  10,  TOTAL, 1000, 10,   5,    15, mode);
  run_case(10,  10,  TOTAL, 1000, 128,  128,  256, mode);
  run_case(128, 10,  TOTAL, 1000, 10,   128,  128, mode);
  run_case(128, 10,  TOTAL, 1000, 10,   10,   10, mode);
  run_case(128, 10,  TOTAL, 1000, 4096, 4096, 4096, mode);
  run_case(128, 128, TOTAL, 1000, 4096, 1024, 4096*2, mode);
}
