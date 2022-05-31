#pragma once

#include <chrono>
#include <string>
#include <functional>
#include <iow/system.hpp>

namespace iow{ namespace io{ namespace client{

struct basic_options
{
  bool async_connect = false;
  time_t reconnect_timeout_ms = 1000;
  // до получения ответа, клиент становится недоступен
  bool sequence_duplex_mode = false;
  bool show_connect_log = true;
  bool connect_by_request = false;
  
  std::string addr;
  std::string port;

  typedef std::function<void(std::chrono::milliseconds, std::function<void()>) > delayed_handler_f;
  struct {
    std::function<void()> connect_handler;
    std::function<void( boost::system::error_code )> error_handler;
    // Чтобы не тащить сюда wflow, пусть пользователь сам реализует
    delayed_handler_f delayed_handler;
  } args;
};

template<typename ConnectionOptions>
struct client_options: basic_options
{
  ConnectionOptions connection;
};

template<typename ConnectionOptions>
struct multi_client_options: client_options<ConnectionOptions>
{
  int connect_count = 1;
};

template<typename ConnectionOptions>
struct multi_thread_options: client_options<ConnectionOptions>
{
  int threads = 0;
};

}}}
