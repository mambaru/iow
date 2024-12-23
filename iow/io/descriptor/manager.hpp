#pragma once

#include <memory>
#include <map>
#include <iow/asio.hpp>
#include <iow/logger.hpp>

namespace iow{ namespace io{ namespace descriptor{

template<typename DescriptorHolder>
class manager
{
public:

  typedef DescriptorHolder holder_type;
  typedef std::shared_ptr<holder_type> holder_ptr;
  typedef typename holder_type::descriptor_type descriptor_type;
  typedef typename holder_type::io_id_type io_id_type;
  typedef typename descriptor_type::executor_type executor_type; 
  typedef boost::asio::io_context io_context;

  template<typename Opt>
  explicit manager(Opt&& opt)
    : _initilizer([opt](holder_ptr h){ h->initialize(opt);})
  {

  }

  void attach(io_id_type id, const holder_ptr& h)
  {
    _holders[id] = h;
  }

  holder_ptr create(const executor_type& io)
  {
    auto h =  std::make_shared<holder_type>( descriptor_type(io) );
    this->_initilizer(h);
    return h;
  }

  holder_ptr create(descriptor_type&& desc)
  {
    auto h =  std::make_shared<holder_type>( std::forward<descriptor_type>(desc) );
    this->_initilizer(h);
    return h;
  }

  void erase(io_id_type id)
  {
    auto itr = _holders.find(id);
    if ( itr != _holders.end() )
    {
      _holders.erase(itr);
    }
    else
    {
      IOW_LOG_ERROR("iow::io::descriptor::manager::erase: id=" << id << " not found")
    }
  }

  size_t size() const
  {
    return _holders.size();
  }

  template<typename Tg, typename F>
  void stat(F&& fun) const
  {
    for (const auto &i: _holders)
      fun( i.second->get_aspect().template get<Tg>() );
  }

private:
  std::function<void(holder_ptr)> _initilizer;
  typedef std::map< io_id_type, holder_ptr> holder_map;
  holder_map _holders;
};

}}}
