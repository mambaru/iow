#include <wfc/core/workflow.hpp>
#include <iow/workflow/workflow.hpp>
#include <iow/workflow/bique.hpp>

namespace iow{
  

workflow::~workflow()
{
  this->stop();
  _impl = nullptr;
}

workflow::workflow(workflow_options opt )
{
  _id = opt.id;
  _workflow_ptr = opt.workflow_ptr;
  _impl = std::make_shared<task_manager>(opt.maxsize, opt.threads, opt.use_io_service);
  _impl->rate_limit( opt.rate_limit );
  _impl->set_startup( opt.startup_handler );
  _impl->set_finish( opt.finish_handler );
  _impl->set_statistics( opt.statistics_handler );
  _delay_ms = opt.post_delay_ms;
  this->create_wrn_timer_(opt);
}

workflow::workflow(io_service_type& io, workflow_options opt)
{
  _id = opt.id;
  _workflow_ptr = opt.workflow_ptr;
  _impl = std::make_shared<task_manager>(io, opt.maxsize, opt.threads, opt.use_io_service);
  _impl->rate_limit( opt.rate_limit );
  _impl->set_startup( opt.startup_handler );
  _impl->set_finish( opt.finish_handler );
  _impl->set_statistics( opt.statistics_handler );
  _delay_ms = opt.post_delay_ms;
  this->create_wrn_timer_(opt);
}

void workflow::start()
{
  _impl->start();
}

void workflow::reconfigure(workflow_options opt)
{
  _id = opt.id;
  _workflow_ptr = opt.workflow_ptr;
  _impl->rate_limit( opt.rate_limit );
  _impl->set_startup( opt.startup_handler );
  _impl->set_finish( opt.finish_handler );
  _impl->set_statistics( opt.statistics_handler );
  _impl->reconfigure(opt.maxsize, opt.threads, opt.use_io_service);
  this->create_wrn_timer_(opt);
}

void workflow::stop()
{
  _impl->stop();
}

std::shared_ptr< task_manager > workflow::manager() const
{
  return _impl;
}

std::shared_ptr< workflow::timer_type> workflow::get_timer() const
{
  return _impl->timer();
}

bool workflow::post(post_handler handler, post_handler drop)
{
  if ( _delay_ms == 0)
    return _impl->post( std::move(handler), std::move(drop) );
  else
    return this->post( std::chrono::milliseconds(_delay_ms), std::move(handler), std::move(drop) );
}

bool workflow::post(time_point_t tp, post_handler handler, post_handler drop)
{
  return _impl->post_at( tp, std::move(handler), std::move(drop) );
}

bool workflow::post(duration_t d,   post_handler handler, post_handler drop)
{
  return _impl->delayed_post(d, std::move(handler), std::move(drop) );
}

workflow::timer_id_t workflow::create_timer(duration_t d, timer_handler handler, bool expires_after)
{
  return _impl->timer()->create(d, std::move(handler), expires_after );
}

workflow::timer_id_t workflow::create_async_timer(duration_t d, async_timer_handler handler, bool expires_after)
{
  return _impl->timer()->create(d, std::move(handler), expires_after );
}

workflow::timer_id_t workflow::create_timer(duration_t sd, duration_t d, timer_handler handler, bool expires_after)
{
  return _impl->timer()->create( sd, d, std::move(handler), expires_after );
}

workflow::timer_id_t workflow::create_async_timer(duration_t sd, duration_t d, async_timer_handler handler, bool expires_after)
{
  return _impl->timer()->create( sd, d, std::move(handler), expires_after );
}

workflow::timer_id_t workflow::create_timer(time_point_t tp, duration_t d, timer_handler handler, bool expires_after)
{
  return _impl->timer()->create(tp, d, std::move(handler), expires_after );
}

workflow::timer_id_t workflow::create_async_timer(time_point_t tp, duration_t d, async_timer_handler handler, bool expires_after)
{
  return _impl->timer()->create(tp, d, std::move(handler), expires_after );
}

workflow::timer_id_t workflow::create_timer(std::string tp, duration_t d, timer_handler handler, bool expires_after)
{
  return _impl->timer()->create(tp, d, std::move(handler), expires_after );
}

workflow::timer_id_t workflow::create_async_timer(std::string tp, duration_t d, async_timer_handler handler, bool expires_after)
{
  return _impl->timer()->create(tp, d, std::move(handler), expires_after );
}

std::shared_ptr<bool> workflow::detach_timer(timer_id_t id)
{
  return _impl->timer()->detach(id);
}

bool workflow::release_timer( timer_id_t id )
{
  return _impl->timer()->release(id);
}
  
size_t workflow::timer_count() const
{
  return _impl->timer()->size();
}

size_t workflow::queue_size() const
{
  return _impl->size();
}

size_t workflow::dropped() const
{
  return _impl->dropped();
}

void workflow::create_wrn_timer_(const workflow_options& opt)
{
  workflow& wrkf = _workflow_ptr == 0 ? *this : *_workflow_ptr;
  size_t dropsave = 0;
  auto old_timer = _wrn_timer;
  /*if ( _wrn_timer != 0 )
    wrkf.release_timer(_wrn_timer);
    */

  if ( ( opt.wrnsize == 0 && opt.maxsize==0) || opt.show_wrn_ms==0)
  {
    // заглушка, чтобы не выскакивал
    _wrn_timer = wrkf.create_timer(std::chrono::seconds(3600), []{ return true;} );
  }
  else
  {
    size_t wrnsize = opt.wrnsize;
    std::function<bool()> handler = opt.handler != nullptr
      ? opt.handler
      : [this, wrnsize, dropsave]() mutable ->bool 
      {
        // TODO: Вынести логирование
        auto dropped = this->_impl->dropped();
        auto size = this->_impl->size();
        auto dropdiff = dropped - dropsave;
        if ( dropdiff!=0 )
        {
          IOW_LOG_ERROR("Workflow '" << this->_id << "' queue dropped " << dropdiff << " items (total " << dropped << ", size " << size << ")" )
          dropsave = dropped;
        }
        else if ( size > wrnsize )
        {
          IOW_LOG_WARNING("Workflow '" << this->_id << "' queue size warning. Size " << size << " (wrnsize=" << wrnsize << ")")
        }
        return true;
      };
    _wrn_timer = wrkf.create_timer(std::chrono::milliseconds(opt.show_wrn_ms), handler );
  }
  wrkf.release_timer(old_timer);
}
  
}
