#include "global_pool.hpp"
#include "data_pool.hpp"
#include <iow/mutex/spinlock.hpp>

namespace iow{ namespace io{

typedef pool_map< data_type, std::mutex/*::iow::spinlock*/ > pool_type;
typedef std::shared_ptr<pool_type> pool_ptr;
pool_ptr static_pool;

void global_pool::initialize(data_map_options opt)
{
 
  
  if ( static_pool == nullptr )
    static_pool = std::make_shared<pool_type>();
  static_pool->set_options(opt);
  
}

data_ptr global_pool::create(size_t bufsize, size_t maxbuf)
{
  if ( static_pool == nullptr )
    return std::make_unique<data_type>(bufsize);
  return static_pool->create(bufsize, maxbuf);
}

void global_pool::free(data_ptr d)
{
  if ( static_pool == nullptr )
    return;
  //std::cout << "free " << d->capacity() << std::endl;
  return static_pool->free( std::move(d) );
}

create_fun global_pool::get_create()
{
  if ( static_pool == nullptr )
    return nullptr;
  using namespace std::placeholders;
  return std::bind( &pool_type::create, static_pool, _1, _2);
}

free_fun global_pool::get_free()
{
  if ( static_pool == nullptr )
    return nullptr;
  using namespace std::placeholders;
  return std::bind( &pool_type::free, static_pool, _1);
}

}}
