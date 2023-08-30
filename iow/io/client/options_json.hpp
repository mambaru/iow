#pragma once

#include <string>
#include <iow/io/client/options.hpp>
#include <wjson/json.hpp>
#include <wjson/name.hpp>

namespace iow{ namespace io{ namespace client{

struct basic_options_json
{
  typedef basic_options option_type;
  
  JSON_NAME(async_connect)
  JSON_NAME(addr)
  JSON_NAME(port)
  JSON_NAME(reconnect_timeout_ms)
  JSON_NAME(sequence_duplex_mode)
  JSON_NAME(show_connect_log)
  JSON_NAME(connect_by_request)

  JSON_NAME(ping_timeout_ms)
  JSON_NAME(ping_data)

  
  typedef wjson::object<
    option_type,
    wjson::member_list<
      wjson::member< n_addr, option_type, std::string, &option_type::addr>,
      wjson::member< n_port, option_type, std::string, &option_type::port>,
      wjson::member< n_async_connect, option_type, bool, &option_type::async_connect>,
      wjson::member< n_sequence_duplex_mode, option_type, bool, &option_type::sequence_duplex_mode>,
      wjson::member< n_show_connect_log, option_type, bool, &option_type::show_connect_log>,
      wjson::member< n_connect_by_request, option_type, bool, &option_type::connect_by_request>,
      wjson::member< n_reconnect_timeout_ms, option_type, time_t, &option_type::reconnect_timeout_ms, wjson::time_interval_ms<time_t> >,
      wjson::member< n_ping_timeout_ms, option_type, time_t, &option_type::ping_timeout_ms, wjson::time_interval_ms<time_t> >,
      wjson::member< n_ping_data, option_type, std::string, &option_type::ping_data, wjson::raw_value<> >
    >,
    ::wjson::strict_mode
  > meta;
  typedef typename meta::target target;
  typedef typename meta::serializer serializer;
  typedef typename meta::member_list member_list;
};

template< typename ConnectionOptionsJson >
struct client_options_json
{
  typedef ConnectionOptionsJson connection_json;
  typedef typename connection_json::target connection_options;
  typedef client_options<connection_options> option_type;
  
  JSON_NAME(connection)
  
  typedef wjson::object<
    option_type,
    wjson::member_list<
      wjson::member< n_connection, option_type, connection_options, &option_type::connection, connection_json>,
      wjson::base< basic_options_json >
    >,
    ::wjson::strict_mode
  > meta;
  typedef typename meta::target target;
  typedef typename meta::serializer serializer;
  typedef typename meta::member_list member_list;
};


template< typename ConnectionOptionsJson >
struct multi_client_options_json
{
  typedef ConnectionOptionsJson connection_json;
  typedef typename connection_json::target connection_options;
  typedef multi_client_options<connection_options> option_type;

  JSON_NAME(connect_count)
  
  typedef wjson::object<
    option_type,
    wjson::member_list<
      wjson::base< client_options_json<connection_json> >,
      wjson::member< n_connect_count, option_type, int, &option_type::connect_count>
    >,
    wjson::strict_mode
  > meta;
  typedef typename meta::target target;
  typedef typename meta::serializer serializer;
  typedef typename meta::member_list member_list;
};

template< typename ConnectionOptionsJson >
struct multi_thread_options_json
{
  typedef ConnectionOptionsJson connection_json;
  typedef typename connection_json::target connection_options;
  typedef multi_thread_options<connection_options> option_type;

  JSON_NAME(threads)
  
  typedef wjson::object<
    option_type,
    wjson::member_list<
      wjson::base< multi_client_options_json<connection_json> >,
      wjson::member< n_threads, option_type, int, &option_type::threads>
    >,
    wjson::strict_mode
  > meta;
  typedef typename meta::target target;
  typedef typename meta::serializer serializer;
  typedef typename meta::member_list member_list;
};




}}}
