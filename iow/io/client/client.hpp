#pragma once

#include <fas/aop.hpp>

#include <iow/logger.hpp>
#include <iow/io/client/connection.hpp>

#include <wjson/wjson.hpp>

#include <memory>
#include <cassert>
#include <iow/asio.hpp>

namespace iow{ namespace io{ namespace client{

template<typename Connection >
class client
  : public Connection
  , public std::enable_shared_from_this< client<Connection> >
{
public:
  typedef Connection super;
  typedef client<Connection> self;
  typedef typename super::data_ptr data_ptr;
  typedef typename super::descriptor_type descriptor_type;
  typedef boost::asio::io_context io_context_type;
  typedef typename super::mutex_type mutex_type;
  typedef typename super::output_handler_type output_handler_t;
  typedef std::vector< data_ptr > wait_data_t;

  ~client()
  {
  }

  explicit client( io_context_type& io)
    :  super(std::move(descriptor_type(io)) )
    , _io(io)
  {
  }

  client( io_context_type& io, descriptor_type&& desc)
    : super(std::move(desc) )
    , _io(io)
  {
  }

  template<typename Opt>
  void start(Opt opt)
  {
    std::lock_guard<mutex_type> lk( super::mutex() );
    if ( _started ) return;
    if ( opt.args.delayed_handler != nullptr )
    {
        _delayed_handler = opt.args.delayed_handler;
    }
    else
    {
      IOW_LOG_WARNING("iow::io::client delayed_handler not set")
    }
    _started = true;
    _connected = false;
    _ready_for_write = opt.connect_by_request;
    _connect_by_request = opt.connect_by_request;
    _reconnect_timeout_ms = opt.reconnect_timeout_ms;
    _sequence_duplex_mode = opt.sequence_duplex_mode;

    _ping_timeout_ms = opt.ping_timeout_ms;
    _ping_data = opt.ping_data;

    if ( !_ping_data.empty() )
    {
      if ( wjson::parser::is_string( _ping_data.begin(), _ping_data.end() ) )
      {
        std::string str;
        typedef wjson::string<>::serializer serializer;
        serializer()( str, _ping_data.begin(), _ping_data.end(), nullptr);
        _ping_data.swap( str );
      }
    }

    if ( !_connect_by_request )
    {
      this->upgrate_options_(opt);
      super::connect_( *this, opt );
    }
  }

  template<typename Opt>
  void connect(Opt opt)
  {
    std::lock_guard<mutex_type> lk( super::mutex() );
    this->upgrate_options_(opt);
    super::connect_( *this, opt );
  }

  void stop()
  {
    std::lock_guard<mutex_type> lk( super::mutex() );
    if ( !_started )
      return;
    _started = false;
    _connected = false;
    _ready_for_write = false;
    super::stop_(*this);
  }

  template<typename Handler>
  void shutdown(Handler&& handler)
  {
    std::lock_guard<mutex_type> lk( super::mutex() );
    _ready_for_write = false;
    _connected = false;
    super::shutdown_( *this, std::forward<Handler>(handler) );
  }

  void close()
  {
    std::lock_guard<mutex_type> lk( super::mutex() );
    _ready_for_write = false;
    _connected = false;
    super::close_(*this);
  }
  
  bool ready_for_write() const
  {
    std::lock_guard<mutex_type> lk( super::mutex() );
    return _ready_for_write;
  }

  data_ptr send(data_ptr d)
  {
    std::lock_guard<mutex_type> lk( super::mutex() );

    if ( !_connected )
      return d;

    return this->send_( std::move(d) );
  }
  
  template<typename Opt>
  data_ptr send(data_ptr d, Opt opt )
  {
    std::lock_guard<mutex_type> lk( super::mutex() );

    if ( !_connected && _connect_by_request )
    {
      this->upgrate_options_(opt);
      super::connect_( *this, opt );
    }

    if ( !_connected )
      return d;

    return this->send_(std::move(d));
  }


  void send( data_ptr d, io_id_t , output_handler_t handler)
  {
    auto dd = this->send( std::move(d) ) ;
    if ( dd!=nullptr && handler!=nullptr )
    {
      handler(nullptr);
    }
  }

private:
  
  data_ptr send_(data_ptr d)
  {
    if ( d==nullptr )
      return nullptr;

    if ( _ready_for_write && _output_handler!=nullptr )
    {
      if ( _sequence_duplex_mode )
        _ready_for_write = false;
      _output_handler( std::move(d) );
    }
    return d;
  }


  template<typename Opt>
  void client_start_(Opt opt)
  {
    this->upgrate_options_(opt);
    super::start_(*this, opt.connection);
  }


  void client_stop_()
  {
    super::stop_(*this);
    _connected = false;
    _ready_for_write = false;
    _output_handler = nullptr;
  }

  void startup_handler_(io_id_t, output_handler_t handler)
  {
    _connected = true;
    _ready_for_write = true;
    _output_handler = handler;

    if ( _ping_timeout_ms!=0 && _delayed_handler!=nullptr )
    {
      _delayed_handler(
        std::chrono::milliseconds(_ping_timeout_ms),
        this->wrap_(*this, std::bind(&self::ping, this), nullptr)
      );
    }
  }

  void ping()
  {
    std::lock_guard<mutex_type> lk( super::mutex() );
    self::ping_();
  }

  void ping_()
  {
    if ( _ping_timeout_ms==0 || _ready_for_write == false )
      return;
    this->send_( std::make_unique<data_type>( _ping_data.begin(), _ping_data.end()) );
    if ( _delayed_handler!=nullptr )
    {
      _delayed_handler(
        std::chrono::milliseconds(_ping_timeout_ms),
        this->wrap_(*this, std::bind(&self::ping, this), nullptr)
      );
    }
  }

  template<typename Opt>
  void delayed_reconnect_(Opt opt)
  {
    if (!this->_started)
      return;

    super::close_(*this);

    if ( opt.connect_by_request )
      return;

    std::weak_ptr<self> wthis = this->shared_from_this();
    auto reconnect_handler = [opt, wthis]() mutable
    {
      if (auto pthis = wthis.lock() )
      {
        std::lock_guard<mutex_type> lk( pthis->mutex() );
        if ( pthis->_started )
        {
          pthis->upgrate_options_(opt);
          pthis->connect_( *pthis, opt );
        }
      }
      else
      {
        IOW_LOG_ERROR("Client Reconnect ERROR. Owner is destroyed.");
      }
    };
    
    if ( _delayed_handler!= nullptr )
    {
      _delayed_handler( std::chrono::milliseconds( this->_reconnect_timeout_ms ), reconnect_handler); // safe_post
    }
    else
    {
      if ( this->_reconnect_timeout_ms != 0 )
      { IOW_LOG_WARNING("Delayed reconnect is not available. Required initialize 'delayed_handler'.") }
      boost::asio::post(_io, reconnect_handler);
    }
  }

  template<typename Opt>
  void upgrate_options_(Opt& opt)
  {
    Opt opt2 = opt;
    std::weak_ptr<self> wthis = this->shared_from_this();
    opt.args.connect_handler = this->wrap_(*this, [wthis, opt2]()
    {
      if ( opt2.args.connect_handler!=nullptr )
        opt2.args.connect_handler();

      if ( auto pthis = wthis.lock() )
      {
        std::lock_guard<mutex_type> lk( pthis->mutex() );
        pthis->client_start_(opt2);
      }
    }, nullptr);

    opt.args.error_handler = [wthis, opt2](boost::system::error_code ec)
    {
      IOW_LOG_MESSAGE("iow::io::client error handler: " << ec.message() )

      if ( opt2.args.error_handler!=nullptr )
        opt2.args.error_handler(ec);
      if ( auto pthis = wthis.lock() )
      {
        std::lock_guard<mutex_type> lk( pthis->mutex() );
        pthis->_connected = false;
        pthis->_ready_for_write = opt2.connect_by_request;
        pthis->delayed_reconnect_(opt2);
      }
      
      if ( opt2.connection.input_handler!=nullptr)
        opt2.connection.input_handler(nullptr, 0, nullptr);
    };

    opt.connection.shutdown_handler = [wthis, opt2]( io_id_t io_id)
    {
      IOW_LOG_DEBUG("iow::io::client connection shutdown handler" )

      if ( opt2.connection.shutdown_handler!=nullptr )
        opt2.connection.shutdown_handler(io_id);

      if ( auto pthis = wthis.lock() )
      {
        std::lock_guard<mutex_type> lk( pthis->mutex() );
        // connect_by_request: после закрытия оставляем _ready_for_write=true, 
        // чтобы иницировать подключение при следующем запросе 
        pthis->_ready_for_write = opt2.connect_by_request;
        pthis->_connected = false;
        pthis->delayed_reconnect_(opt2);
      }
    };

    opt.connection.startup_handler = this->wrap_(*this, [wthis, opt2]( io_id_t io_id, output_handler_t output)
    {
      if ( auto pthis = wthis.lock() )
      {
        std::lock_guard<mutex_type> lk( pthis->mutex() );
        pthis->startup_handler_(io_id, output);
      }

      if ( opt2.connection.startup_handler != nullptr )
      {
        opt2.connection.startup_handler( io_id, output);
      }
    }, nullptr);

    if ( opt.connection.input_handler == nullptr )
    {
      opt.connection.input_handler
        = []( data_ptr d, io_id_t , output_handler_t )
      {
        only_for_log(d);
        IOW_LOG_ERROR("Client input_handler not set [" << d << "]" )
      };
    }
    else if ( _sequence_duplex_mode )
    {
      auto input_handler = opt.connection.input_handler;
      opt.connection.input_handler
        = [input_handler, wthis]( data_ptr d, io_id_t id, output_handler_t output)
      {
        if ( auto pthis = wthis.lock() )
        {
          std::lock_guard<mutex_type> lk( pthis->mutex() );
          if (pthis->_connected) 
            pthis->_ready_for_write = true;
        }
        input_handler( std::move(d), id, output);
      };
    }
    
    
 }
private:
  io_context_type& _io;
  bool _started = false;
  bool _connected = false;
  bool _ready_for_write = false;
  bool _sequence_duplex_mode = false;
  bool _connect_by_request = false;
  time_t _reconnect_timeout_ms = 0;
  time_t _ping_timeout_ms = 0;
  std::string _ping_data;
  output_handler_t _output_handler;
  typedef std::function<void(std::chrono::milliseconds, std::function<void()>) > delayed_handler_f;
  delayed_handler_f _delayed_handler;
};

}}}
