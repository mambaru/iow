#pragma once

#include <mutex>
#include <memory>
#include <vector>
#include <iostream>
#include <iow/logger.hpp>
#include <iow/io/stat.hpp>

namespace iow{ namespace io{ namespace client{

template<typename Client, bool IsThreadWrapper>
class multi_base
{
public:
  typedef Client client_type;
  typedef typename client_type::data_ptr data_ptr;
  typedef std::shared_ptr<client_type> client_ptr;
  typedef std::vector<client_ptr> client_list;
  typedef typename client_type::io_context_type io_context_type;
  typedef std::recursive_mutex mutex_type;

  explicit multi_base(io_context_type& io)
    : _current(0)
    , _io_context(io)
  {}

  template<typename Opt>
  void start(Opt opt, int count)
  {
    std::lock_guard<mutex_type> lk(_mutex);

    if ( !_clients.empty() )
      return;

    _max_clients = static_cast<size_t>(count > 0 ? count : 0);
    _clients.reserve( static_cast<size_t>(count) );
    for (int i = 0; i < count; ++i)
    {
      _clients.push_back( std::make_shared<client_type>(_io_context) );
      _clients.back()->start( opt );
    }
  }

  template<typename Opt>
  void reconfigure(Opt opt, int count)
  {
    std::lock_guard<mutex_type> lk(_mutex);
    _max_clients = count;
    while ( _clients.size() > count )
    {
      _clients.back()->stop();
      _clients.pop_back();
    }

    while ( int( _clients.size() ) < count )
    {
      _clients.push_back( std::make_shared<client_type>(_io_context) );
      _clients.back()->start( opt );
    }
  }

  void stop()
  {
    client_list clients;
    {
      std::lock_guard<mutex_type> lk(_mutex);
      _clients.swap(clients);
    }
    for ( auto& conn : clients )
    {
      conn->stop();
      conn.reset();
    }
  }

  bool ready_for_write() const
  {
    std::lock_guard<mutex_type> lk(_mutex);
    if ( IsThreadWrapper )
      return true;
    for (auto &cli : _clients )
      if ( cli->ready_for_write() )
        return true;
    return false;
  }
  
  data_ptr send(data_ptr d)
  {
    std::lock_guard<mutex_type> lk(_mutex);

    return this->send_(std::move(d));
  }

  template<typename Opt>
  data_ptr send(data_ptr d, const Opt& opt)
  {
    std::lock_guard<mutex_type> lk(_mutex);
 
    if ( _clients.size() < _max_clients )
    {
      _clients.push_back( std::make_shared<client_type>(_io_context) );
      _clients.back()->start( opt );
      // Not working ws async_connect
      return _clients.back()->send( std::move(d) );
    }
    
    if ( auto cli = next_() )
      return cli->send( std::move(d), opt );

    return d;
  }

  ::iow::io::connection_stat get_stat() const
  {
    std::lock_guard<mutex_type> lk(_mutex);
    ::iow::io::connection_stat stat;
    for (const auto &cli : _clients )
    {
      stat+=cli->stat();
    }
    return stat;
  }

  
private:
  data_ptr send_(data_ptr d)
  {
    if ( auto cli = this->next_() )
      return cli->send( std::move(d) );
    else
      return d;
  }
  
  client_ptr next_()
  {
    client_ptr cli = nullptr;
      
    for ( size_t i = 0; i < _clients.size(); ++i )
    {
      if ( _current >= _clients.size() ) _current = 0;
      cli = _clients[ _current++ ];
      if ( IsThreadWrapper || cli->ready_for_write() )
        break;
      cli = nullptr;
    }
    return cli;
  }

private:
  size_t _current;
  size_t _max_clients = 0;
  io_context_type& _io_context;
  client_list _clients;
  mutable mutex_type _mutex;
};

}}}
