#pragma once

#include <iow/io/types.hpp>
#include <cstddef>
#include <vector>

namespace iow{ namespace io{

// Простой пул data_ptr для create/free хуков read_buffer / write_buffer.
// Не потокобезопасен: один пул на io-поток / соединение, либо внешняя синхронизация.
class data_pool
{
public:
  struct options_type
  {
    size_t poolsize = 32;
  };

  void set_options(const options_type& opt) noexcept
  {
    _poolsize = opt.poolsize == 0 ? 1 : opt.poolsize;
  }

  options_type get_options() const noexcept
  {
    options_type opt;
    opt.poolsize = _poolsize;
    return opt;
  }

  data_ptr create(size_t bufsize, size_t maxbuf)
  {
    ++_creates;
    data_ptr d;
    if ( !_free.empty() )
    {
      d = std::move(_free.back());
      _free.pop_back();
      ++_reuses;
    }
    else
    {
      d = std::make_unique<data_type>();
      ++_allocs;
    }

    const size_t reserve_for = maxbuf > bufsize ? maxbuf : bufsize;
    if ( reserve_for > d->capacity() )
      d->reserve(reserve_for);
    d->resize(bufsize);
    return d;
  }

  void free(data_ptr d)
  {
    if ( d == nullptr )
      return;

    ++_frees;
    if ( _free.size() >= _poolsize )
      return;

    d->clear();
    _free.push_back(std::move(d));
  }

  size_t pooled() const noexcept { return _free.size(); }
  size_t creates() const noexcept { return _creates; }
  size_t frees() const noexcept { return _frees; }
  size_t allocs() const noexcept { return _allocs; }
  size_t reuses() const noexcept { return _reuses; }

  void clear() noexcept
  {
    _free.clear();
    _creates = 0;
    _frees = 0;
    _allocs = 0;
    _reuses = 0;
  }

private:
  size_t _poolsize = 32;
  std::vector<data_ptr> _free;
  size_t _creates = 0;
  size_t _frees = 0;
  size_t _allocs = 0;
  size_t _reuses = 0;
};

}}
