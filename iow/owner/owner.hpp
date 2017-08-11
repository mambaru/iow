//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <iow/owner/owner_handler.hpp>
#include <memory>
#include <atomic>

namespace iow{

class owner
{
public:

  typedef std::shared_ptr<int> alive_type;
  typedef std::weak_ptr<int>   weak_type;

  owner() 
  : _alive( std::make_shared<int>(1) ) 
  {
  }

  owner(const owner& ) = delete;
  owner& operator = (const owner& ) = delete;

  owner(owner&& ) = default;
  owner& operator = (owner&& ) = default;

  alive_type& alive() { return _alive; }
  const alive_type& alive() const { return _alive; }
  
  void reset()
  {
    _alive = std::make_shared<int>(*_alive + 1);
  }
  

  template<typename Handler, typename AltHandler>
  owner_handler< 
    typename std::remove_reference<Handler>::type, 
    typename std::remove_reference<AltHandler>::type
  >
  wrap(Handler&& h, AltHandler&& nh) const
  {
    return 
      owner_handler<
        typename std::remove_reference<Handler>::type, 
        typename std::remove_reference<AltHandler>::type
      >(
          std::forward<Handler>(h),
          std::forward<AltHandler>(nh),
          std::weak_ptr<int>(_alive)
       )
    ;
  }
  
  static void enable_callback_check(bool )
  {
  };

  // TODO: wrap_strong и wrap_strong2 строго на один вызов
  // Если обработчик уничтожаеться без вызова или вызываеться повторно то ошибка (abort)
  // В _alive можно сделать счетчик еще невызванных (только для strong)
  // TODO:enable_callback_check(true) в конфиге core, флаг в глобале, устанавливаеться в domain_object

private:
  mutable alive_type _alive;
};

}
