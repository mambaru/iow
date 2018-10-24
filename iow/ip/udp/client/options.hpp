#pragma once

#include <iow/io/socket/dgram/options.hpp>
#include <iow/io/client/options.hpp>
#include <thread>
#include <string>
#include <functional>

namespace iow{ namespace ip{ namespace udp{ namespace client{
  
struct basic_options:
  ::iow::io::socket::dgram::options
{
  std::string addr;
  std::string port;
};

struct options:
  public ::iow::io::client::options< basic_options >
{
  
};

}}}}

