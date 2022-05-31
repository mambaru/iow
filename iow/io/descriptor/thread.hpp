#pragma once

#include <iow/logger.hpp>
#include <iow/asio.hpp>
#include <iow/system.hpp>
#include <memory>
#include <mutex>
#include <list>
#include <thread>
#include <cstdlib>
#include <iostream>

namespace iow{ namespace io{ namespace descriptor{


template<typename Holder>
class thread
  : public std::enable_shared_from_this< thread<Holder> >

{
public:
  typedef Holder holder_type;
  typedef std::shared_ptr<holder_type> holder_ptr;
  typedef std::thread thread_type;
  typedef std::recursive_mutex mutex_type;
  typedef std::shared_ptr<thread_type> thread_ptr;
  typedef boost::asio::io_context io_context_type;
  typedef std::shared_ptr<io_context_type> io_context_ptr;

  explicit thread(io_context_type& io)
    : _started(false)
    , _io_context(io)
  {
  }

  ~thread()
  {
  }

  template<typename Opt>
  void start(Opt&& opt)
  {
    std::lock_guard<mutex_type> lk(_mutex);
    this->start_( std::forward<Opt>(opt) );
  }

  template<typename Opt>
  void start_(Opt&& opt1)
  {
    typename std::decay<Opt>::type opt = opt1;

    if ( _holder != nullptr ) return;

    if ( opt.threads != 0 )
      _io_context2 = std::make_shared<io_context_type>();

    _holder = std::make_shared<holder_type>( (_io_context2!=nullptr ? (*_io_context2) : _io_context) );
    _holder->start(opt1);
    if ( io_context_ptr io = this->_io_context2 )
    {
      _thread = std::thread([io]()
      {
        typedef boost::asio::executor_work_guard<io_context_type::executor_type> work_type;
        work_type wrk( io->get_executor() );
        io->run();
      });
    }
  }

  template<typename Opt>
  void reconfigure(Opt&& opt)
  {
    std::lock_guard<mutex_type> lk(_mutex);
    if ( _holder == nullptr ) return;
    bool f1 = (_io_context2 == nullptr);
    bool f2 = (opt.threads == 0);

    if ( f1 != f2)
    {
      this->stop_();
      return this->start_( std::forward<Opt>(opt) );
    }
    _holder->reconfigure( std::forward<Opt>(opt) );
  }

  template<typename Handler>
  void shutdown(Handler handler)
  {
    std::lock_guard<mutex_type> lk(_mutex);
    if ( _holder == nullptr )
      return;

    auto pthis = this->shared_from_this();
    _holder->shutdown([handler,pthis]()
    {
      pthis->stop();
      if (handler != nullptr) handler();
    });
  }

  void stop()
  {
    std::lock_guard<mutex_type> lk(_mutex);
    if ( _holder == nullptr ) return;
    this->stop_();
  }

  holder_ptr holder() const
  {
    std::lock_guard<mutex_type> lk(_mutex);
    return _holder;
  }
  
  bool ready_for_write() const 
  {
    std::lock_guard<mutex_type> lk(_mutex);
    if ( _holder == nullptr ) return false;
    return _holder->ready_for_write();
  }

  void stop_()
  {
    _holder->stop();
    _holder = nullptr;
    if ( _io_context2 != nullptr )
    {
      _io_context2->stop();
      _io_context2 = nullptr;
      if ( _thread.joinable() )
      {
        try
        {
          _thread.join();
        }
        catch(const std::exception& e)
        {
          IOW_LOG_FATAL("io::descriptor::thread::stop_" <<  e.what() )
        }
      }
    }
  }

  mutex_type& mutex() const
  {
    return _mutex;
  }

private:
  bool _started;
  io_context_type& _io_context;
  io_context_ptr _io_context2;
  mutable mutex_type _mutex;
  holder_ptr _holder;
  thread_type _thread;
};

}}}
