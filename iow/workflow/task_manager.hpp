#pragma once

#include <iow/workflow/timer_manager.hpp>
//#include <iow/workflow/queue_options.hpp>
#include <iow/asio.hpp>

namespace iow{

class bique;

class task_manager
{
  class pool_impl;
public:
  typedef bique queue_type;

  typedef std::function<void()>                               function_t;
  typedef std::chrono::time_point<std::chrono::system_clock>  time_point_t;
  typedef time_point_t::duration                              duration_t;
  typedef ::iow::asio::io_service io_service_type;

  typedef timer_manager<queue_type> timer_type;
  typedef pool_impl pool_type;

  task_manager( io_service_type& io, size_t queue_maxsize );
  
  task_manager( size_t queue_maxsize, int threads );
  
  task_manager( io_service_type& io, size_t queue_maxsize, int threads, bool use_asio /*= false*/  );

  void reconfigure(size_t queue_maxsize, int threads, bool use_asio /*= false*/ );
  
  void rate_limit(size_t rps);
  
  void start();

  void stop();

  void run();
  
  bool run_one();
  
  bool poll_one();
  
  bool post( function_t&& f );
  
  bool post_at(time_point_t tp, function_t&& f);

  bool delayed_post(duration_t duration, function_t&& f);
  
  std::size_t size() const;
  std::size_t dropped() const;
  std::shared_ptr<timer_type> timer();
  
  size_t get_threads( ) const;
  size_t get_counter( size_t thread ) const;


private:
  std::atomic<int> _threads;
  std::shared_ptr<queue_type> _queue;
  std::shared_ptr<timer_type> _timer;
  std::shared_ptr<pool_type>  _pool;
};

}
