#include <iow/io/aux/data_pool.hpp>
#include <iow/io/aux/read_buffer.hpp>
#include <iow/io/aux/write_buffer.hpp>
#include <iow/io/rw/ad_initialize.hpp>
#include <iow/io/rw/options.hpp>
#include <iow/io/rw/tags.hpp>
#include <iow/io/reader/data/ad_initialize.hpp>
#include <iow/io/reader/data/tags.hpp>
#include <iow/io/writer/data/ad_initialize.hpp>
#include <iow/io/writer/data/tags.hpp>
#include <fas/testing.hpp>
#include <fas/aop.hpp>
#include <cstring>

typedef ::iow::io::data_type data_type;
typedef ::iow::io::data_ptr data_ptr;
typedef ::iow::io::data_pool data_pool;
typedef ::iow::io::read_buffer read_buffer;
typedef ::iow::io::write_buffer write_buffer;

namespace {

void bind_pool(read_buffer::options_type& opt, data_pool& pool)
{
  opt.create = [&pool](size_t sz, size_t mx){ return pool.create(sz, mx); };
  opt.free = [&pool](data_ptr d){ pool.free(std::move(d)); };
}

void bind_pool(write_buffer::options_type& opt, data_pool& pool)
{
  opt.create = [&pool](size_t sz, size_t mx){ return pool.create(sz, mx); };
  opt.free = [&pool](data_ptr d){ pool.free(std::move(d)); };
}

struct aspect_rw_pool : fas::aspect<
  fas::value< ::iow::io::reader::data::_read_buffer_, read_buffer >,
  fas::value< ::iow::io::writer::data::_write_buffer_, write_buffer >,
  fas::advice< ::iow::io::reader::data::_initialize_, ::iow::io::reader::data::ad_initialize >,
  fas::advice< ::iow::io::writer::data::_initialize_, ::iow::io::writer::data::ad_initialize >,
  fas::advice< ::iow::io::rw::_initialize_, ::iow::io::rw::ad_initialize >
>{};

typedef fas::aspect_class<aspect_rw_pool> rw_holder;

} // namespace

UNIT(data_pool_basic, "data_pool reuse и лимит poolsize")
{
  using namespace fas::testing;
  data_pool pool;
  data_pool::options_type opt;
  opt.poolsize = 2;
  pool.set_options(opt);

  auto a = pool.create(4, 16);
  auto b = pool.create(8, 16);
  t << equal<assert, size_t>(pool.allocs(), 2) << FAS_TESTING_FILE_LINE;
  t << equal<assert, size_t>(pool.reuses(), 0) << FAS_TESTING_FILE_LINE;
  t << stop;

  pool.free(std::move(a));
  pool.free(std::move(b));
  t << equal<assert, size_t>(pool.pooled(), 2) << FAS_TESTING_FILE_LINE;
  t << stop;

  auto c = pool.create(4, 16);
  t << equal<assert, size_t>(pool.reuses(), 1) << FAS_TESTING_FILE_LINE;
  t << equal<assert, size_t>(pool.allocs(), 2) << FAS_TESTING_FILE_LINE;
  t << equal<assert, size_t>(c->size(), 4) << FAS_TESTING_FILE_LINE;
  t << is_true<assert>(c->capacity() >= 4) << FAS_TESTING_FILE_LINE;
  t << stop;

  // сверх poolsize — дропаем, не копим
  pool.free(std::move(c));
  auto d = pool.create(1, 1);
  auto e = pool.create(1, 1);
  auto f = pool.create(1, 1);
  pool.free(std::move(d));
  pool.free(std::move(e));
  pool.free(std::move(f));
  t << equal<assert, size_t>(pool.pooled(), 2) << FAS_TESTING_FILE_LINE;
}

UNIT(read_buffer_with_pool, "read_buffer ходит через create/free пула")
{
  using namespace fas::testing;
  data_pool pool;
  data_pool::options_type pool_opt;
  pool_opt.poolsize = 16;
  pool.set_options(pool_opt);

  read_buffer buf;
  read_buffer::options_type opt;
  buf.get_options(opt);
  opt.bufsize = 16;
  opt.minbuf = 16;
  opt.maxbuf = 32;
  opt.sep = "\n";
  opt.trimsep = true;
  bind_pool(opt, pool);
  buf.set_options(opt);

  auto feed = [&](const char* s)
  {
    auto n = buf.next();
    t << is_true<assert>(n.first != nullptr) << FAS_TESTING_FILE_LINE;
    t << stop;
    const size_t len = std::strlen(s);
    t << is_true<assert>(n.second >= len) << FAS_TESTING_FILE_LINE;
    t << stop;
    std::memcpy(n.first, s, len);
    n.second = len;
    t << is_true<assert>(buf.confirm(n)) << FAS_TESTING_FILE_LINE;
    t << stop;
  };

  feed("one\n");
  feed("two\n");
  feed("three\n");

  size_t msgs = 0;
  for (;;)
  {
    auto d = buf.detach();
    if ( d == nullptr )
      break;
    ++msgs;
    pool.free(std::move(d));
  }

  t << equal<assert, size_t>(msgs, 3) << FAS_TESTING_FILE_LINE;
  t << is_true<assert>(pool.creates() > 0) << FAS_TESTING_FILE_LINE;
  t << is_true<assert>(pool.frees() > 0) << FAS_TESTING_FILE_LINE;
  t << is_true<assert>(pool.pooled() > 0) << FAS_TESTING_FILE_LINE;
  t << stop;

  const auto reuses_before = pool.reuses();
  feed("four\n");
  auto d = buf.detach();
  t << is_true<assert>(d != nullptr) << FAS_TESTING_FILE_LINE;
  t << equal_str<assert>(std::string("four"), std::string(d->begin(), d->end())) << FAS_TESTING_FILE_LINE;
  pool.free(std::move(d));
  t << is_true<assert>(pool.reuses() > reuses_before) << FAS_TESTING_FILE_LINE;
}

UNIT(write_buffer_with_pool, "write_buffer возвращает чанки в пул через free")
{
  using namespace fas::testing;
  data_pool pool;
  data_pool::options_type pool_opt;
  pool_opt.poolsize = 16;
  pool.set_options(pool_opt);

  write_buffer buf;
  write_buffer::options_type opt;
  buf.get_options(opt);
  opt.bufsize = 32;
  opt.minbuf = 0;
  opt.maxbuf = 32;
  opt.sep = "";
  opt.first_as_is = true;
  bind_pool(opt, pool);
  buf.set_options(opt);

  auto d = pool.create(4, 32);
  (*d)[0] = 'a'; (*d)[1] = 'b'; (*d)[2] = 'c'; (*d)[3] = 'd';
  const auto allocs_after_create = pool.allocs();
  buf.attach(std::move(d));

  auto n = buf.next();
  t << is_true<assert>(n.first != nullptr) << FAS_TESTING_FILE_LINE;
  t << equal<assert, size_t>(n.second, 4) << FAS_TESTING_FILE_LINE;
  t << stop;
  t << is_true<assert>(buf.confirm(n)) << FAS_TESTING_FILE_LINE;
  t << stop;

  t << equal<assert, size_t>(buf.count(), 0) << FAS_TESTING_FILE_LINE;
  t << is_true<assert>(pool.frees() >= 1) << FAS_TESTING_FILE_LINE;
  t << is_true<assert>(pool.pooled() >= 1) << FAS_TESTING_FILE_LINE;
  t << equal<assert, size_t>(pool.allocs(), allocs_after_create) << FAS_TESTING_FILE_LINE;
}

UNIT(rw_initialize_keeps_pool_hooks, "rw::ad_initialize не затирает create/free")
{
  using namespace fas::testing;
  data_pool pool;
  rw_holder holder;

  ::iow::io::rw::options opt;
  opt.reader.bufsize = 16;
  opt.reader.sep = "\n";
  opt.writer.bufsize = 16;
  bind_pool(opt.reader, pool);
  bind_pool(opt.writer, pool);

  holder.get_aspect().template get< ::iow::io::rw::_initialize_ >()(holder, opt);

  read_buffer::options_type ro;
  write_buffer::options_type wo;
  holder.get_aspect().template get< ::iow::io::reader::data::_read_buffer_ >().get_options(ro);
  holder.get_aspect().template get< ::iow::io::writer::data::_write_buffer_ >().get_options(wo);

  t << is_true<assert>(static_cast<bool>(ro.create)) << FAS_TESTING_FILE_LINE;
  t << is_true<assert>(static_cast<bool>(ro.free)) << FAS_TESTING_FILE_LINE;
  t << is_true<assert>(static_cast<bool>(wo.create)) << FAS_TESTING_FILE_LINE;
  t << is_true<assert>(static_cast<bool>(wo.free)) << FAS_TESTING_FILE_LINE;
  t << stop;

  // хуки рабочие: next() должен увеличить creates пула
  const auto creates_before = pool.creates();
  auto n = holder.get_aspect().template get< ::iow::io::reader::data::_read_buffer_ >().next();
  t << is_true<assert>(n.first != nullptr) << FAS_TESTING_FILE_LINE;
  t << is_true<assert>(pool.creates() > creates_before) << FAS_TESTING_FILE_LINE;
}

BEGIN_SUITE(data_pool_suite, "data_pool + buffer hooks")
  ADD_UNIT(data_pool_basic)
  ADD_UNIT(read_buffer_with_pool)
  ADD_UNIT(write_buffer_with_pool)
  ADD_UNIT(rw_initialize_keeps_pool_hooks)
END_SUITE(data_pool_suite)

BEGIN_TEST
  RUN_SUITE(data_pool_suite)
END_TEST
