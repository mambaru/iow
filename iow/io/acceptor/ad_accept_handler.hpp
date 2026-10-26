#pragma once

#include <iow/io/reader/tags.hpp>
#include <iow/io/acceptor/tags.hpp>
#include <iow/system.hpp>
#include <iow/asio.hpp>
#include <iow/logger.hpp>
#include <atomic>
#include <chrono>
#include <cstdint>

namespace iow{ namespace io{ namespace acceptor{

// Временные/ресурсные ошибки accept: listen должен продолжить работу
inline bool is_recoverable_accept_error(const boost::system::error_code& ec)
{
  return ec == boost::asio::error::connection_aborted
      || ec == boost::asio::error::no_descriptors
      || ec == boost::asio::error::no_buffer_space
      || ec == boost::asio::error::no_memory
      || ec == boost::asio::error::interrupted
      || ec == boost::system::errc::too_many_files_open_in_system
      || ec == boost::system::errc::resource_unavailable_try_again;
}

// Не чаще раза в секунду; в сообщении — сколько раз словили с прошлого WARNING
inline void log_recoverable_accept_warning(const boost::system::error_code& ec)
{
  using clock = std::chrono::steady_clock;
  static std::atomic<std::uint64_t> count{0};
  static std::atomic<std::int64_t> last_log_ns{0};

  const auto n = count.fetch_add(1, std::memory_order_relaxed) + 1;
  const auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                        clock::now().time_since_epoch()).count();
  auto last = last_log_ns.load(std::memory_order_relaxed);
  if ( now_ns - last < 1000000000LL )
    return;
  if ( !last_log_ns.compare_exchange_strong(last, now_ns, std::memory_order_relaxed) )
    return;

  const auto logged = count.exchange(0, std::memory_order_relaxed);
  IOW_LOG_WARNING("ad_accept_handler recoverable ("
                  << ec.value() << ") " << ec.message()
                  << " x" << (logged != 0 ? logged : n)
                  << " — retry accept");
}

struct ad_accept_handler
{
  template<typename T, typename P>
  void operator()(T& t, P p, const boost::system::error_code& ec) const
  {

    if ( !ec )
    {
      t.get_aspect().template get< ::iow::io::reader::_complete_ >()(t, std::move(p));
    }
    else if ( ec.value() == boost::system::errc::operation_would_block )
    {
      t.get_aspect().template get< ::iow::io::reader::_more_ >()(t);
    }
    else if ( ec.value() == boost::system::errc::operation_canceled )
    {
      // штатная остановка acceptor
    }
    else if ( is_recoverable_accept_error(ec) )
    {
      log_recoverable_accept_warning(ec);
      t.get_aspect().template get< ::iow::io::reader::_more_ >()(t);
    }
    else
    {
      IOW_LOG_FATAL("ad_accept_handler " << ec.message())
      const auto& context = t.get_aspect().template get<_context_>();
      context.fatal_handler(ec.value(), ec.message());
    }
  }
};


}}}
