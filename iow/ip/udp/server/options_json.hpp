#pragma once

#include <iow/ip/udp/server/options.hpp>
#include <iow/io/socket/dgram/options_json.hpp>
#include <wjson/json.hpp>
#include <wjson/name.hpp>

namespace iow{ namespace ip{ namespace udp{ namespace server{
  
struct options_json
{
  JSON_NAME(addr)
  JSON_NAME(port)
  JSON_NAME(threads)
  JSON_NAME(chunk_stat)
  typedef ::wjson::object<
    options,
    wjson::member_list<
      wjson::member<n_threads, options, int, &options::threads>,
      wjson::member< n_addr, options, std::string, &options::addr>,
      wjson::member< n_port, options, std::string, &options::port>,
      wjson::member<n_chunk_stat, options, bool, &options::chunk_stat>,
      wjson::base< ::iow::io::socket::dgram::options_json >
    >,
    ::wjson::strict_mode
  > type;
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};


}}}}

