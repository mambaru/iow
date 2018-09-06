#pragma once

#include <iow/io/rw/tags.hpp>
#include <iow/io/reader/data/tags.hpp>
#include <iow/io/writer/data/tags.hpp>
namespace iow{ namespace io{ namespace rw{

struct ad_initialize
{
  template<typename T, typename O>
  void operator()(T& t, O opt) const
  {
    opt.reader.create = nullptr;
    opt.reader.free = nullptr;
    opt.writer.create = nullptr;
    opt.writer.free = nullptr;
    t.get_aspect().template get< ::iow::io::reader::data::_initialize_ >()(t, opt.reader);
    t.get_aspect().template get< ::iow::io::writer::data::_initialize_ >()(t, opt.writer);
  }
};

}}}
