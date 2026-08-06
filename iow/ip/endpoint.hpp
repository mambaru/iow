#pragma once

#include <iow/asio.hpp>
#include <iow/system.hpp>
#include <cstdlib>
#include <string>

namespace iow{ namespace ip{

// Разобрать порт без DNS. При ошибке возвращает false и выставляет ec.
inline bool parse_port(const std::string& port, unsigned short& out, boost::system::error_code& ec)
{
  if ( port.empty() )
  {
    ec = boost::asio::error::invalid_argument;
    return false;
  }

  char* end = nullptr;
  const unsigned long value = std::strtoul(port.c_str(), &end, 10);
  if ( end == port.c_str() || *end != '\0' || value > 65535ul )
  {
    ec = boost::asio::error::invalid_argument;
    return false;
  }

  out = static_cast<unsigned short>(value);
  ec.clear();
  return true;
}

// Если addr — IP (v4/v6), собрать endpoint без DNS. Иначе false (нужен resolve).
template<typename Protocol>
inline bool try_make_ip_endpoint(
  const std::string& addr,
  const std::string& port,
  typename Protocol::endpoint& endpoint,
  boost::system::error_code& ec)
{
  const auto address = boost::asio::ip::make_address(addr, ec);
  if ( ec )
    return false;

  unsigned short port_num = 0;
  if ( !parse_port(port, port_num, ec) )
    return false;

  endpoint = typename Protocol::endpoint(address, port_num);
  ec.clear();
  return true;
}

// Sync resolve с IP-bypass. Не бросает исключений.
template<typename Protocol, typename Executor>
inline typename Protocol::endpoint resolve_endpoint(
  Executor executor,
  const std::string& addr,
  const std::string& port,
  boost::system::error_code& ec)
{
  typedef typename Protocol::endpoint endpoint_type;
  typedef typename Protocol::resolver resolver_type;

  endpoint_type endpoint;
  if ( !try_make_ip_endpoint<Protocol>(addr, port, endpoint, ec) )
  {
    ec.clear();
    resolver_type resolver(executor);
    auto results = resolver.resolve(addr, port, ec);
    if ( ec || results.empty() )
    {
      if ( !ec )
        ec = boost::asio::error::host_not_found;
    }
    else
    {
      endpoint = *results.begin();
    }
  }
  return endpoint;
}

}}
