#include <iow/jsonrpc/incoming/incoming_holder.hpp>
#include <iow/jsonrpc/outgoing/outgoing_holder.hpp>
#include <iow/jsonrpc/method/aspect/send_error.hpp>
#include <iow/jsonrpc/incoming/send_error.hpp>
#include <iow/jsonrpc/errors.hpp>
#include <iow/jsonrpc/types.hpp>
#include <iow/logger/logger.hpp>
#include <wjson/strerror.hpp>

namespace iow{ namespace jsonrpc{

incoming_holder::incoming_holder(data_ptr d, bool timepoint )
  : _parsed(false)
  , _data(std::move(d))
{
  if ( timepoint )
    _time_point = clock_t::now();
}

void incoming_holder::attach(data_ptr d, bool timepoint)
{
  _data = std::move(d);
  _parsed = false;
  if ( timepoint )
    _time_point = clock_t::now();
}


incoming_holder::data_ptr incoming_holder::parse(outgoing_handler_t error_handler)
{
  if ( _data == nullptr )
    return nullptr;

  ::wjson::json_error e;
  _begin = ::wjson::parser::parse_space(_data->begin(), _data->end(), &e);
  if ( !e )
  {
    _end = incoming_json::serializer()( _incoming, _begin, _data->end(), &e);
    _parsed = true;
  }
  else 
  {
    if ( error_handler!=nullptr )
    {
      incoming_holder eh( this->detach() );
      aux::send_error( std::move(eh), std::make_unique<parse_error>(), error_handler );
    }
    return nullptr;
  }

  iterator next = ::wjson::parser::parse_space( _end, _data->end(), nullptr);
  if ( next == _data->end() || *next=='\0')
    return nullptr;

  return std::make_unique<data_type>(next, _data->end());
}

bool incoming_holder::method_equal_to(const char* name)  const
{
  if ( !this->ready_() )
    return false;

  incoming::iterator beg = _incoming.method.first;
  incoming::iterator end = _incoming.method.second;
  if (beg++==end) return false;
  if (beg==end--) return false;
  for (; beg != end && *beg==*name; ++beg, ++name);
  return beg==end && *name=='\0';
}

std::string incoming_holder::method() const
{
  if ( this->ready_() && std::distance(_incoming.method.first, _incoming.method.second ) > 2 )
  {
    return std::string( _incoming.method.first+1, _incoming.method.second-1);
  }
  return std::string();
}

incoming_holder::raw_t incoming_holder::raw_method() const 
{
  return std::make_pair( _incoming.method.first, _incoming.method.second );
}

incoming_holder::raw_t incoming_holder::raw_id() const
{
  return std::make_pair( _incoming.id.first, _incoming.id.second );
}

std::string incoming_holder::str() const
{
  if ( _data != nullptr )
    return std::string(_data->begin(), _data->end());
  return std::string();
}


std::string incoming_holder::error_message(const ::wjson::json_error& e) const
{
  if ( _data != nullptr   )
    return ::wjson::strerror::message_trace( e, _data->begin(), _data->end() );
  return ::wjson::strerror::message(e);
}

std::string incoming_holder::params_error_message(const ::wjson::json_error& e) const
{
  return ::wjson::strerror::message_trace( e, _incoming.params.first, _incoming.params.second);
}

std::string incoming_holder::result_error_message(const ::wjson::json_error& e) const
{
  return ::wjson::strerror::message_trace( e, _incoming.result.first, _incoming.result.second);
}

std::string incoming_holder::error_error_message(const ::wjson::json_error& e) const
{
  return ::wjson::strerror::message_trace( e, _incoming.error.first, _incoming.error.second);
}

std::string incoming_holder::id_error_message(const ::wjson::json_error& e) const
{
  return ::wjson::strerror::message_trace( e, _incoming.id.first, _incoming.id.second);
}

const incoming& incoming_holder::get()  const 
{
  return _incoming;
}

incoming_holder::clock_t::time_point incoming_holder::time_point() const 
{
  return _time_point;
}

incoming_holder::data_ptr incoming_holder::detach()
{
  _incoming = incoming();
  return std::move(_data);
}

incoming_holder::data_ptr incoming_holder::acquire_params()
{
  if ( !this->ready_() )
    return nullptr;

  std::move( _incoming.params.first, _incoming.params.second, _data->begin() );
  _data->resize( std::distance(_incoming.params.first, _incoming.params.second) );
  _incoming = incoming();
  return std::move(_data);
}

/*
void incoming_holder::send_error( incoming_holder holder, std::unique_ptr<error> err, outgoing_handler_t outgoing_handler)
{
  typedef outgoing_error_json< error_json > message_json;
  outgoing_error<error> error_message;
    
  error_message.error = std::move(err);
  auto id_range = holder.raw_id();
  if ( id_range.first != id_range.second )
  {
    error_message.id = std::make_unique<data_type>( id_range.first, id_range.second );
  }

  auto d = holder.detach();
  if ( d == nullptr )
    d = std::make_unique<data_type>();
  d->clear();
  d->reserve(80);
  typename message_json::serializer()(error_message, std::inserter( *d, d->end() ));
  
  outgoing_holder out(std::move(d));
  outgoing_handler( std::move(out) );
}
*/

/*
namespace{

  data_ptr incoming_holder_perform_once(
    data_ptr d, io_id_t io_id, outgoing_handler_t outgoing_handler, 
    std::function<void(incoming_holder, io_id_t, outgoing_handler_t)> incoming_handler 
  )
  {
    incoming_holder holder(std::move(d));
    try
    {
      d = holder.parse();
    }
    catch(const json::json_error& er)
    {
      WJRPC_LOG_WARNING( "jsonrpc::incoming_holder: parse error: " << holder.error_message(er) )
      incoming_holder::send_error( std::move(holder), std::make_unique<parse_error>(), std::move(outgoing_handler));
      return nullptr;
    }

    if ( holder.is_valid() )
    {
      incoming_handler( std::move(holder), io_id, std::move(outgoing_handler));
    }
    else
    {
      incoming_holder::send_error( std::move(holder), std::make_unique<invalid_request>(), std::move(outgoing_handler));
    }
    return std::move(d);
  }
}
*/

/*
void incoming_holder::perform(
    data_ptr d, io_id_t io_id, outgoing_handler_t outgoing_handler, 
    std::function<void(incoming_holder, io_id_t, outgoing_handler_t)> incoming_handler )
{
  try
  {
    while (d != nullptr)
    {
      d = incoming_holder_perform_once(std::move(d), io_id, outgoing_handler, incoming_handler);
    }
  }
  catch(const std::exception& ex)
  {
    WJRPC_LOG_ERROR( "jsonrpc::engine: server error: " << ex.what() )
    incoming_holder::send_error( std::move(incoming_holder(nullptr)), std::make_unique<server_error>(), std::move(outgoing_handler));
    return;
  }
  catch(...)
  {
    WJRPC_LOG_ERROR( "jsonrpc::engine: server error: " << "unhandler exception" )
    incoming_holder::send_error( std::move(incoming_holder(nullptr)), std::make_unique<server_error>(), std::move(outgoing_handler));
    return;
  }
}
*/

}}
