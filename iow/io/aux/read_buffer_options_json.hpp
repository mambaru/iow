#pragma once

#include <iow/io/aux/read_buffer_options.hpp>
#include <wjson/json.hpp>
#include <wjson/name.hpp>

namespace iow{ namespace io{
    
struct read_buffer_options_json
{
  typedef ::iow::io::read_buffer_options  options_type;
  
  JSON_NAME(sep)
  JSON_NAME(bufsize)
  JSON_NAME(maxbuf)
  JSON_NAME(minbuf)
  JSON_NAME(maxsize)
  JSON_NAME(trimsep)
 // JSON_NAME(chunk_stats)

  typedef ::wjson::object<
    options_type,
    ::wjson::member_list<
      ::wjson::member< n_sep,      options_type, std::string, &options_type::sep     >,
      ::wjson::member< n_bufsize,  options_type, size_t,      &options_type::bufsize, wjson::size_value<size_t> >,
      ::wjson::member< n_maxbuf,   options_type, size_t,      &options_type::maxbuf, wjson::size_value<size_t>  >,
      ::wjson::member< n_minbuf,   options_type, size_t,      &options_type::minbuf, wjson::size_value<size_t>  >,
      ::wjson::member< n_maxsize,   options_type, size_t,      &options_type::maxsize, wjson::size_value<size_t>  >,
      ::wjson::member< n_trimsep,  options_type, bool,        &options_type::trimsep >/*,
      ::wjson::member< n_chunk_stats,  options_type, bool,        &options_type::chunk_stats >*/
    >,
    ::wjson::strict_mode
  > type;
  
  typedef typename type::target target;
  typedef typename type::serializer serializer;
  typedef typename type::member_list member_list;
};

}}
