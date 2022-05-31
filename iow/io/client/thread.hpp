#pragma once

/*#include <iow/io/client/multi_client.hpp>*/
#include <iow/io/descriptor/thread.hpp>
#include <iow/logger.hpp>
#include <iow/io/types.hpp>

namespace iow{ namespace io{ namespace client{


template<typename Client  >
class thread
  : public ::iow::io::descriptor::thread< Client >
{
  typedef thread<Client> self;
  typedef ::iow::io::descriptor::thread< Client > super;
public:
  typedef Client client_type;
  typedef typename client_type::io_context_type io_context_type;
  typedef typename client_type::data_ptr data_ptr;

  explicit thread(io_context_type& io)
    : super(io)
  {
  }

  // для простого клиента
  data_ptr send(data_ptr d)
  {
    if ( auto h = super::holder() )
    {
      return h->send(std::move(d));
    }

    IOW_LOG_ERROR( "iow::io::client::thread::send drop [" << d << "]" )
    return nullptr;
  }

  template<typename Opt>
  data_ptr send(data_ptr d, const Opt& opt)
  {
    if ( auto h = super::holder() )
    {
      return h->send(std::move(d), opt);
    }

    IOW_LOG_ERROR( "iow::io::client::thread::send(opt) drop [" << d << "]" )
    return nullptr;
  }

  // для мулти-коннект клиента
  void send( data_ptr d, io_id_t id, output_handler_t handler)
  {
    if ( auto h = super::holder() )
    {
      h->send(std::move(d), id, std::move(handler) );
    }
    else
    {
      IOW_LOG_ERROR( "iow::io::client::thread::send drop [" << d << "]" )
    }
  }
};

}}}
