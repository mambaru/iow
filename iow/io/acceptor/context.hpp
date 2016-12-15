#pragma once

#include <iow/io/descriptor/manager.hpp>
#include <iow/io/descriptor/context.hpp>
#include <memory>

namespace iow{ namespace io{ namespace acceptor{

template<typename ConnectionType>
struct context
  : public ::iow::io::descriptor::context<
      size_t,
      ConnectionType,
      std::shared_ptr<ConnectionType>
    >
{
  typedef ConnectionType connection_type;
  //typedef typename connection_type::options_type connection_options_type;
  typedef std::shared_ptr<connection_type> connection_ptr;
  typedef ::iow::io::descriptor::manager<connection_type> manager_type;
  typedef std::shared_ptr<manager_type> manager_ptr;
  
  int backlog = 0;
  int max_connections = 0;
  std::string addr;
  std::string port;

  //connection_options_type connection_options;
  manager_ptr manager;
};
  
}}}
