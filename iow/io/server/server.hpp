#pragma once

#include <iow/io/descriptor/mtdup.hpp>
#include <iow/io/descriptor/tags.hpp>
#include <iow/io/reader/data/tags.hpp>
#include <iow/io/writer/data/tags.hpp>
#include <iow/io/stat.hpp>
#include <iow/logger.hpp>

namespace iow{ namespace io{ namespace server{

template<typename Acceptor>
using server_base = ::iow::io::descriptor::mtdup<Acceptor>;

template<typename Acceptor>
class server
  : private server_base<Acceptor>
{
  typedef server_base<Acceptor> super;

public:
  typedef typename super::io_context_type io_context_type;
  typedef Acceptor acceptor_type;
  typedef std::shared_ptr<acceptor_type> acceptor_ptr;
  typedef typename acceptor_type::descriptor_type descriptor_type;

  explicit server(io_context_type& io)
    : super( std::move( descriptor_type(io/*, nullptr*/) ) )
  {}

  template<typename Opt>
  void start(Opt opt)
  {
    super::origin()->listen(opt);
    super::start(opt);
    _chunk_stat = opt.chunk_stat;
  }

  // threads ↓ или 0↔N — возможный сброс клиентов на остановленных acceptor'ах.
  // threads ↑ / прочие options — без сброса; listen не переоткрываем.
  template<typename Opt>
  void reconfigure(Opt opt)
  {
    const int cur = super::thread_count();
    const int next = opt.threads;
    _chunk_stat = opt.chunk_stat;

    if ( (cur == 0) != (next == 0) )
    {
      IOW_LOG_WARNING("iow::io::server::reconfigure: mode switch threads "
                      << cur << " -> " << next
                      << ", hard restart (connections will be dropped)");
      super::stop();
      super::origin()->listen(opt);
      super::start(std::move(opt));
      return;
    }

    if ( next < cur )
    {
      IOW_LOG_WARNING("iow::io::server::reconfigure: decrease threads "
                      << cur << " -> " << next
                      << " (connections on stopped acceptors will be dropped)");
    }

    super::reconfigure(std::move(opt));
  }

  void stop()
  {
    super::stop();
  }

  ::iow::io::connection_stat get_stat() const
  {
    typedef ::iow::io::descriptor::_context_ _context_;
    typedef ::iow::io::reader::data::_read_buffer_ _read_buffer_;
    typedef ::iow::io::writer::data::_write_buffer_ _write_buffer_;

    ::iow::io::connection_stat stat;
    stat.connection_count = 0;

    super::for_each_holder([&](const acceptor_ptr& holder)
    {
      if ( holder == nullptr )
        return;

      std::lock_guard<typename acceptor_type::mutex_type> lk(holder->mutex());
      auto pmanager = holder->get_aspect().template get<_context_>().manager;
      if ( pmanager == nullptr )
        return;

      stat.connection_count += pmanager->size();
      pmanager->template stat<_read_buffer_>(
        [&](const auto& read_buffer){
          stat.reader += read_buffer.get_stat(_chunk_stat);
        }
      );
      pmanager->template stat<_write_buffer_>(
        [&](const auto& write_buffer){
          stat.writer += write_buffer.get_stat(_chunk_stat);
        }
      );
    });

    return stat;
  }
private:
  bool _chunk_stat = false;

};

}}}
