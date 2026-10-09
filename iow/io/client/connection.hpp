#pragma once

#include <iow/io/socket/stream/socket.hpp>
#include <iow/io/reader/data/tags.hpp>
#include <iow/io/writer/data/tags.hpp>
#include <iow/io/stat.hpp>
#include <iow/ip/endpoint.hpp>
#include <iow/asio.hpp>
#include <iow/logger.hpp>
#include <iow/system.hpp>

#include <memory>

namespace iow{ namespace io{ namespace client{

struct _connect_;
struct _sync_resolver_;

struct ad_connect
{
  template<typename T, typename Opt>
  void operator()(T& t, const Opt& opt) const
  {
    auto popt = std::make_shared<Opt>(opt);
    std::weak_ptr<T> wthis = t.shared_from_this();

    if ( popt->show_connect_log )
    {
      IOW_LOG_BEGIN("Client connect to '" << opt.addr << ":" << opt.port << "' ...")
    }

    typedef boost::asio::ip::tcp protocol_type;
    typedef typename protocol_type::endpoint endpoint_type;

    boost::system::error_code ip_ec;
    endpoint_type endpoint;
    if ( ::iow::ip::try_make_ip_endpoint<protocol_type>(opt.addr, opt.port, endpoint, ip_ec) )
    {
      connect_to_(t, wthis, popt, endpoint, popt->async_connect);
      return;
    }

    // addr уже IP, но порт битый — в DNS не ходим
    boost::system::error_code addr_ec;
    boost::asio::ip::make_address(opt.addr, addr_ec);
    if ( !addr_ec )
    {
      IOW_LOG_END("Client FAIL connect to '" << opt.addr << ":" << opt.port
                  << "'. " << ip_ec.value() << " " << ip_ec.message())
      if ( popt->args.error_handler )
        popt->args.error_handler(ip_ec);
      return;
    }

    // hostname: async_resolve → async_connect по results (не блокируем io-поток)
    auto resolver = std::make_shared<protocol_type::resolver>(t.descriptor().get_executor());
    resolver->async_resolve(
      opt.addr,
      opt.port,
      t.wrap(
        [wthis, popt, resolver](const boost::system::error_code& resolve_ec,
                                const protocol_type::resolver::results_type& results) mutable
        {
          auto p = wthis.lock();
          if ( p == nullptr )
            return;

          if ( resolve_ec || results.empty() )
          {
            const auto fail_ec = resolve_ec ? resolve_ec : boost::asio::error::host_not_found;
            IOW_LOG_END("Client FAIL resolve '" << popt->addr << ":" << popt->port
                        << "'. " << fail_ec.value() << " " << fail_ec.message())
            if ( popt->args.error_handler )
              popt->args.error_handler(fail_ec);
            return;
          }

          auto handler = p->wrap(
            [wthis, popt](const boost::system::error_code& connect_ec, const endpoint_type&)
            {
              auto locked = wthis.lock();
              if ( locked == nullptr )
                return;

              if ( !connect_ec )
              {
                if ( popt->show_connect_log )
                {
                  IOW_LOG_END("Client connected to '" << popt->addr << ":" << popt->port
                              << "' " << connect_ec.message())
                }
                if ( popt->args.connect_handler )
                  popt->args.connect_handler();
              }
              else
              {
                IOW_LOG_END("Client FAIL connected to " << popt->addr << ":"
                            << popt->port << ". " << connect_ec.value() << " "
                            << connect_ec.message())
                if ( popt->args.error_handler )
                  popt->args.error_handler(connect_ec);
              }
            },
            nullptr);

          boost::asio::async_connect(p->descriptor(), results, std::move(handler));
        },
        nullptr));
  }

private:
  template<typename T, typename Opt>
  static void connect_to_(
    T& t,
    std::weak_ptr<T> wthis,
    std::shared_ptr<Opt> popt,
    const boost::asio::ip::tcp::endpoint& endpoint,
    bool async_connect)
  {
    auto handler = t.wrap(
      [wthis, popt](const boost::system::error_code& ec)
      {
        auto p = wthis.lock();
        if ( p == nullptr )
          return;

        if ( !ec )
        {
          if ( popt->show_connect_log )
          {
            IOW_LOG_END("Client connected to '" << popt->addr << ":" << popt->port
                        << "' " << ec.message())
          }
          if ( popt->args.connect_handler )
            popt->args.connect_handler();
        }
        else
        {
          IOW_LOG_END("Client FAIL connected to " << popt->addr << ":"
                      << popt->port << ". " << ec.value() << " " << ec.message())
          if ( popt->args.error_handler )
            popt->args.error_handler(ec);
        }
      },
      nullptr);

    if ( async_connect )
    {
      t.descriptor().async_connect(endpoint, std::move(handler));
    }
    else
    {
      boost::system::error_code ec;
      t.descriptor().connect(endpoint, ec);
      handler(ec);
    }
  }
};

typedef fas::aspect<
  fas::advice<_connect_, ad_connect>
> aspect;

template<typename A = fas::aspect<> >
class connection_base
  : public ::iow::io::socket::stream::socket_base< typename fas::merge_aspect<A, aspect>::type >
{
public:
  typedef connection_base<A> self;
  typedef ::iow::io::socket::stream::socket_base< typename fas::merge_aspect<A, aspect>::type > super;
  typedef typename super::descriptor_type descriptor_type;
  typedef typename super::data_ptr data_ptr;

  explicit connection_base(descriptor_type&& desc)
    : super( std::move( desc ) )
  {}

  template<typename T, typename Opt>
  void connect_(T& t, Opt&& opt)
  {
    this->get_aspect().template get<_connect_>()(t, std::forward<Opt>(opt) );
  }

  ::iow::io::connection_stat get_stat_(bool chunk_stat) const
  {
    ::iow::io::connection_stat stat;
    stat.reader = this->get_aspect().template get< iow::io::reader::data::_read_buffer_>().get_stat(chunk_stat);
    stat.writer = this->get_aspect().template get< iow::io::writer::data::_write_buffer_>().get_stat(chunk_stat);
    return stat;
  }
};

template<typename A = fas::aspect<> >
class connection
  : public connection_base<A>
  , public std::enable_shared_from_this< connection<A> >
{
public:
   typedef connection_base<A> super;
   typedef typename super::descriptor_type descriptor_type;
   typedef typename super::mutex_type mutex_type;

  explicit connection(descriptor_type&& desc)
    : super( std::move( desc ) )
  {}

  template<typename Opt>
  void connect(Opt&& opt)
  {
    std::lock_guard<mutex_type> lk( super::mutex() );
    super::connect_( *this, std::forward<Opt>(opt) );
  }


  template<typename Opt>
  void start(Opt&& opt)
  {
    std::lock_guard<mutex_type> lk( super::mutex() );
    super::start_(*this, opt.connection);
  }

  void close()
  {
    std::lock_guard<mutex_type> lk( super::mutex() );
    super::close_(*this);
  }

  void stop()
  {
    std::lock_guard<mutex_type> lk( super::mutex() );
    super::close_(*this);
  }

  ::iow::io::connection_stat get_stat(bool chunk_stat) const
  {
    std::lock_guard<mutex_type> lk( super::mutex() );
    return super::get_stat_(chunk_stat);
  }

};


}}}
