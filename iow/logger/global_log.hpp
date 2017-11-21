//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <iow/logger/logstream.hpp>
#include <iow/logger/log_writer.hpp>

#include <string>
#include <memory>


namespace iow{
  
#ifndef IOW_DISABLE_GLOBAL_LOG
  
void init_log(const log_writer& writer);
bool log_status();
logstream global_log(const std::string& name, const std::string& type);
  
#else
  
inline void init_log(log_writer)
{
}

inline bool log_status()
{
  return true;
}
  
inline logstream global_log(const std::string& name, const std::string& type )
{
  return logstream(name, type, nullptr);
}

#endif
}
