#pragma once

#include <iow/io/descriptor/mtdup.hpp>
#include <iow/io/descriptor/tags.hpp>
#include <iow/io/reader/data/tags.hpp>
#include <iow/io/writer/data/tags.hpp>
#include <iow/io/stat.hpp>

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

    stat.connection_count = super::origin()->get_aspect().template get<_context_>().manager->size();
    super::origin()->get_aspect().template get<_context_>().manager->template stat<_read_buffer_>(
      [&](const auto& read_buffer){
        stat.reader += read_buffer.get_stat(_chunk_stat);
      }
    );
    super::origin()->get_aspect().template get<_context_>().manager->template stat<_write_buffer_>(
      [&](const auto& write_buffer){
        stat.writer += write_buffer.get_stat(_chunk_stat);
      }
    );
    return stat;
  }
private:
  bool _chunk_stat = false;

};

}}}
