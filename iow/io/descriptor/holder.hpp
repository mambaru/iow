#pragma once
#include <iow/io/io_base.hpp>
#include <iow/io/descriptor/tags.hpp>
#include <unistd.h>
#include <functional>

namespace iow{ namespace io{ namespace descriptor{

template<typename A>
class holder_base
  : public io_base<A>
{
public:
  typedef holder_base<A> self;
  typedef io_base<A> super;

public:
  typedef typename super::mutex_type mutex_type;
  typedef typename super::aspect::template advice_cast<_descriptor_type_>::type descriptor_type;
  typedef typename super::aspect::template advice_cast<_context_>::type context_type;
  typedef typename context_type::data_ptr data_ptr;
  typedef typename context_type::output_handler_type output_handler_type;
  typedef typename context_type::input_handler_type input_handler_type;
  typedef typename context_type::startup_handler_type  startup_handler_type;
  typedef typename context_type::shutdown_handler_type shutdown_handler_type;

public:

  holder_base(const holder_base& ) = delete;
  void operator=(const holder_base& ) = delete;
  
  explicit holder_base(descriptor_type&& desc)
    : super()
    , _descriptor(std::move(desc))
  {}

  void attach(descriptor_type&& desc)
  {
    std::lock_guard< mutex_type > lk( super::mutex() );
    _descriptor = std::move(desc);
  }

  descriptor_type&& detach()
  {
    std::lock_guard< mutex_type > lk( super::mutex() );
    return std::move(_descriptor);
  }

  const descriptor_type& descriptor() const { return _descriptor;}

  descriptor_type& descriptor() { return _descriptor;}

  template<typename Descriptor, typename IOServiceType, typename ProtocolType>
  Descriptor dup(IOServiceType& io, const ProtocolType& protocol)
  {
    std::lock_guard< mutex_type > lk( super::mutex() );
    return std::move(this->dup_<Descriptor>(io, protocol));
  }

  template<typename Descriptor, typename IOServiceType>
  Descriptor dup(IOServiceType& io)
  {
    typedef Descriptor dup_descriptor_type;
    typedef typename dup_descriptor_type::protocol_type dup_protocol_type;
    return std::move( this->dup<Descriptor>(io, dup_protocol_type::v4()) );
  }

  template<typename Descriptor>
  Descriptor dup()
  {
    return std::move( this->dup<Descriptor>(this->get_io_service()) );
  }

protected:
  
  template<typename T>
  void close_(T& t)
  {
    t.get_aspect().template get<_close_>()(t);
  }
  
  template<typename T, typename Opt>
  void reconfigure_(T& t, Opt&& opt)
  {
    super::reconfigure_(t, opt);
    this->initialize_(t, std::forward<Opt>(opt) );
  }

  template<typename T, typename Opt>
  void initialize_(T& t, Opt&& opt)
  {
    _start_with_opt = [&t, opt](){
      t.start(std::move(opt));
    };
  }


  template<typename Descriptor, typename IOServiceType, typename ProtocolType>
  Descriptor dup_(IOServiceType& io, const ProtocolType& protocol)
  {
    typedef Descriptor dup_descriptor_type;
    typedef typename dup_descriptor_type::native_handle_type dup_native_type;
    dup_native_type d = ::dup( this->descriptor().native_handle() );
    dup_descriptor_type dup_descriptor(io, protocol, d);
    return std::move(dup_descriptor);
  }

  using super::start_;
  template<typename T>
  void start_(T& /*t*/)
  {
    if ( _start_with_opt!= nullptr)
      _start_with_opt();  
  }
  
private:
  descriptor_type _descriptor;
  std::function<void()> _start_with_opt;
};


template<typename A>
class holder
  : public holder_base<A>
  , public std::enable_shared_from_this< holder<A> >
{
  typedef holder_base<A> super;
public:
  typedef typename super::mutex_type mutex_type;
  typedef typename super::descriptor_type descriptor_type;
  
  explicit holder(descriptor_type&& desc)
    : super(std::move(desc) )
  {}

  void reset()
  {
    std::lock_guard< mutex_type > lk( super::mutex() );
    super::reset_(*this);
  }

  template<typename O>
  void start(O&& opt)
  {
    std::lock_guard< mutex_type > lk( super::mutex() );
    super::start_(*this, std::forward<O>(opt));
  }

  void start()
  {
    std::lock_guard< mutex_type > lk( super::mutex() );
    super::start_(*this);
  }

  template<typename O>
  void initialize(O&& opt)
  {
    std::lock_guard< mutex_type > lk( super::mutex() );
    super::initialize_(*this, std::forward<O>(opt));
  }

  template<typename O>
  void reconfigure(O&& opt)
  {
    std::lock_guard< mutex_type > lk( super::mutex() );
    super::reconfigure_(*this, std::forward<O>(opt));
  }

  void close()
  {
    std::lock_guard< mutex_type > lk( super::mutex() );
    this->close_(*this);
  }
  
  void stop()
  {
    std::lock_guard< mutex_type > lk( super::mutex() );
    super::stop_(*this);
  }

  template<typename Handler>
  void shutdown(Handler&& handler)
  {
    std::lock_guard< mutex_type > lk( super::mutex() );
    super::shutdown_(*this, std::forward<Handler>(handler));
  }

  
};

}}}

