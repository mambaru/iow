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

  typedef ::wjson::object<
    options_type,
    ::wjson::member_list<
      ::wjson::member< n_sep,      options_type, std::string, &options_type::sep     >,
      ::wjson::member< n_bufsize,  options_type, size_t,      &options_type::bufsize >,
      ::wjson::member< n_maxbuf,   options_type, size_t,      &options_type::maxbuf  >,
      ::wjson::member< n_minbuf,   options_type, size_t,      &options_type::minbuf  >,
      ::wjson::member< n_maxsize,   options_type, size_t,      &options_type::maxsize  >,
      ::wjson::member< n_trimsep,  options_type, bool,        &options_type::trimsep >
    >
  > type;
  
  typedef typename type::target target;
  typedef typename type::serializer serializer;
  typedef typename type::member_list member_list;
};

/*

template<typename DataType>
struct read_buffer_options
{
  typedef std::unique_ptr<DataType> data_ptr;
  typedef std::function< data_ptr(size_t, size_t) > create_fun;
  typedef std::function< void(data_ptr) > free_fun;

  std::string sep;
  size_t bufsize=4096;
  size_t maxbuf=4096*2;
  size_t minbuf=0;
  bool fast_mode = false;
  bool trimsep = false; // Отрезать сепаратор 
  std::function< data_ptr(size_t, size_t) > create;
  std::function< void(data_ptr) > free;
};
*/

}}
