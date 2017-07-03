#pragma once

#include <iow/io/aux/write_buffer_options.hpp>
#include <iow/io/types.hpp>
#include <wjson/json.hpp>
#include <wjson/name.hpp>

namespace iow{ namespace io{
    
struct write_buffer_options_json
{
  typedef ::iow::io::write_buffer_options  options_type;

  JSON_NAME(sep)
  JSON_NAME(bufsize)
  JSON_NAME(maxbuf)
  JSON_NAME(minbuf)
  JSON_NAME(maxsize)
  JSON_NAME(first_as_is)

  typedef ::wjson::object<
    options_type,
    ::wjson::member_list<
      ::wjson::member< n_sep,      options_type, std::string, &options_type::sep     >,
      ::wjson::member< n_bufsize,  options_type, size_t,      &options_type::bufsize >,
      ::wjson::member< n_maxbuf,   options_type, size_t,      &options_type::maxbuf  >,
      ::wjson::member< n_minbuf,   options_type, size_t,      &options_type::minbuf  >,
      ::wjson::member< n_maxsize,   options_type, size_t,      &options_type::maxsize  >,
      ::wjson::member< n_first_as_is,  options_type, bool,        &options_type::first_as_is >
    >,
    ::wjson::strict_mode
  > type;
  
  typedef typename type::target target;
  typedef typename type::serializer serializer;
  typedef typename type::member_list member_list;
};

}}
