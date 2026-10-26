#pragma once
#include <memory>
#include <mutex>
#include <list>
#include <thread>
#include <chrono>

#include <iow/io/basic/tags.hpp>
#include <iow/logger.hpp>
#include <iow/asio.hpp>
#include <iow/system.hpp>

namespace iow{ namespace io{ namespace descriptor{

template<typename Holder>
class mtdup
{
public:
  typedef Holder holder_type;
  typedef typename holder_type::descriptor_type descriptor_type;
  typedef std::shared_ptr<holder_type> holder_ptr;
  typedef std::list<holder_ptr> holder_list;
  typedef std::list<std::thread> thread_list;
  typedef std::recursive_mutex mutex_type;
  typedef boost::asio::io_context io_context_type;
  typedef std::shared_ptr<io_context_type> io_context_ptr;
  typedef std::list<io_context_ptr> service_list;

  explicit mtdup(descriptor_type&& desc)
    : _origin( std::make_shared<holder_type>( std::forward<descriptor_type>(desc)))
    , _thread_count(0)
  {
  }

  holder_ptr origin() const
  {
    return _origin;
  }

  int thread_count() const
  {
    std::lock_guard<mutex_type> lk(_mutex);
    return _thread_count;
  }

  // threads==0 → origin; threads>0 → только dup-acceptors (на origin manager нет).
  template<typename Handler>
  void for_each_holder(Handler&& handler) const
  {
    holder_list snapshot;
    {
      std::lock_guard<mutex_type> lk(_mutex);
      if ( _thread_count == 0 )
      {
        if ( _origin != nullptr )
          snapshot.push_back(_origin);
      }
      else
      {
        snapshot = _dup_list;
      }
    }

    for (auto& h : snapshot)
      handler(h);
  }

  template<typename Handler>
  auto wrap(Handler&& h) const
    -> typename holder_type::template result_of<_wrap_, Handler>::type
  {
    return std::move( _origin->wrap( std::forward<Handler>(h)) );
  }


  template<typename Opt>
  void start(Opt&& opt)
  {
    std::lock_guard<mutex_type> lk(_mutex);
    this->start_(std::forward<Opt>(opt));
  }

  // threads ↑  — добавить acceptor-потоки, клиентов не трогать
  // threads ↓  — жёстко остановить лишние (их соединения сбрасываются)
  // threads == — только options на живых holders
  // 0 ↔ N      — смена модели; нужен повторный listen() снаружи (server::reconfigure)
  template<typename Opt>
  void reconfigure(Opt&& opt)
  {
    std::lock_guard<mutex_type> lk(_mutex);
    Opt local = std::forward<Opt>(opt);

    const int next = local.threads;
    const int cur = _thread_count;

    if ( (cur == 0) != (next == 0) )
    {
      IOW_LOG_WARNING("mtdup::reconfigure: mode switch threads "
                      << cur << " -> " << next
                      << " (hard restart; listen must be redone by caller)");
      this->stop_();
      this->start_(std::move(local));
      return;
    }

    if ( cur == 0 && next == 0 )
    {
      _origin->reconfigure(std::move(local));
      return;
    }

    // cur > 0 && next > 0
    for (auto& h : _dup_list)
      h->reconfigure(local);

    if ( next > cur )
    {
      IOW_LOG_MESSAGE("mtdup::reconfigure: increase threads " << cur << " -> " << next);
      this->add_threads_(local, next - cur);
    }
    else if ( next < cur )
    {
      IOW_LOG_WARNING("mtdup::reconfigure: decrease threads " << cur << " -> " << next
                      << " (dropping connections on stopped acceptors)");
      this->stop_excess_(static_cast<size_t>(cur - next));
    }

    _thread_count = next;
  }

  void stop()
  {
    std::lock_guard<mutex_type> lk(_mutex);
    this->stop_();
  }

  mutex_type& mutex() const
  {
    return _mutex;
  }

private:

  template<typename Opt>
  void start_(Opt&& opt)
  {
    if ( opt.threads == 0 )
    {
      _origin->start(opt);
      _thread_count = 0;
      return;
    }

    this->add_threads_(opt, opt.threads);
    _thread_count = opt.threads;
  }

  template<typename Opt>
  void add_threads_(const Opt& opt, int count)
  {
    for (int i = 0; i < count; ++i)
    {
      auto io = std::make_shared<io_context_type>();
      auto desc = _origin->template dup< descriptor_type >( *io );
      auto h = std::make_shared<holder_type>( std::move( desc ) );
      _dup_list.push_back(h);
      _services.push_back(io);

      h->start(opt);

      auto tup = opt.thread_startup;
      auto tdown = opt.thread_shutdown;
      auto tstat = opt.thread_statistics;
      _threads.push_back( std::thread([io, tup, tdown, tstat]()
      {
        auto thread_id = std::this_thread::get_id();

        if (tup) tup(thread_id);
        if ( tstat == nullptr )
          io->run();
        else
        {
          for (;;)
          {
            auto start_ts = std::chrono::steady_clock::now();
            size_t handlers = io->run_one();
            if ( handlers == 0 )
              break;

            auto finish_ts = std::chrono::steady_clock::now();
            auto span = finish_ts - start_ts ;
            if ( tstat != nullptr )
              tstat( thread_id, handlers, span );
          }
        }

        IOW_LOG_MESSAGE("mtdup thread stopped")
        if (tdown) tdown(thread_id);
      }));
    }
  }

  void stop_excess_(size_t count)
  {
    for (size_t i = 0; i < count; ++i)
    {
      if ( _dup_list.empty() )
        break;

      auto h = _dup_list.back();
      auto s = _services.back();
      // сначала закрываем, чтоб реконнект на другой ассептор не прошел
      h->close();
      h->stop();
      s->stop();
      _threads.back().join();

      _dup_list.pop_back();
      _services.pop_back();
      _threads.pop_back();
    }
  }

  void stop_()
  {
    _origin->close();
    for (auto h : _dup_list)
    {
      // сначала закрываем, чтоб реконнект на другой ассептор не прошел
      h->close();
    }


    _origin->stop();
    for (auto h : _dup_list)
    {
      h->stop();
    }


    for (auto s : _services)
    {
      s->stop();
    }

    for (auto& t : _threads)
    {
      t.join();
    }


    _dup_list.clear();
    _threads.clear();
    _services.clear();
    _thread_count = 0;
  }

  mutable mutex_type _mutex;
  holder_ptr _origin;
  holder_list _dup_list;
  thread_list _threads;
  service_list _services;
  int _thread_count;
};

}}}
