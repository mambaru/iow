#pragma once

#include <iow/ip/udp/client/options.hpp>
#include <iow/io/socket/dgram/options_json.hpp>
#include <wjson/json.hpp>
#include <wjson/name.hpp>

namespace iow{ namespace ip{ namespace udp{ namespace client{
  
struct options_json
{
  JSON_NAME(addr)
  JSON_NAME(port)
  typedef ::wjson::object<
    options,
    wjson::member_list<
      wjson::base< ::iow::io::socket::dgram::options_json >,
      wjson::member< n_addr, options, std::string, &options::addr>,
      wjson::member< n_port, options, std::string, &options::port>
    >,
    ::wjson::strict_mode
  > type;
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};


}}}}

