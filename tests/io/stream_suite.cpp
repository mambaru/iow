#include <iostream>
#include <iow/io/io_base.hpp>
#include <iow/io/basic/aspect.hpp>
#include <iow/io/rw/aspect.hpp>
/*
 * #include <iow/io/reader/aspect.hpp>
#include <iow/io/writer/aspect.hpp>
*/
#include <iow/memory.hpp>
#include <iow/asio.hpp>

#include <fas/testing.hpp>

#include <vector>
#include <memory>
#include <list>
#include <cstdlib>
#include <boost/concept_check.hpp>
typedef std::vector<char> data_type;
typedef std::unique_ptr<data_type> data_ptr;

struct ad_read_some
{
  template<typename T, typename D>
  void operator()(T& t, D d) const
  {
    if ( t.input.empty() )
      return;

    boost::asio::post(t.service, [&t, d](){
      auto dd = d;
      auto tmp = std::move(t.input.front());
      t.input.pop_front();
      std::copy(tmp->begin(), tmp->end(), dd.first);
      dd.second = tmp->size();
      t.get_aspect().template get< ::iow::io::reader::_complete_>()(t, std::move(dd));
    }/*, nullptr*/);
  }
};

struct ad_write_some
{
  template<typename T, typename P>
  void operator()(T& t, P p) const
  {
    if ( p.first == nullptr )
      return;

    boost::asio::post(t.service, [&t, p](){
      t.result += std::string(p.first, p.first + p.second);
      t.get_aspect().template get< ::iow::io::writer::_complete_>()(t, std::move(p));
    }/*, nullptr*/);
  }
};

struct stream_options
{
  typedef ::iow::io::write_buffer_options write_buffer_options;
  typedef ::iow::io::read_buffer_options read_buffer_options;

  write_buffer_options writer;
  read_buffer_options reader;
};

class stream
  : public ::iow::io::io_base< fas::aspect<
      fas::advice< ::iow::io::reader::_some_, ad_read_some>,
      fas::advice< ::iow::io::writer::_some_, ad_write_some>,
      ::iow::io::basic::aspect<std::recursive_mutex>::advice_list,
      ::iow::io::rw::aspect::advice_list,
      fas::type< ::iow::io::_options_type_, fas::empty_type >,
      fas::alias< ::iow::io::reader::data::_input_, ::iow::io::writer::_output_>,
      fas::group< ::iow::io::_initialize_, ::iow::io::rw::_initialize_>
    > >
{
public:
  typedef stream_options options_type;
  typedef data_ptr input_t;
  typedef data_ptr output_t;

  explicit stream(boost::asio::io_context& io)
    : service(io)
  {}

  void start()
  {
    stream_options opt;
    opt.reader.sep.clear();
    opt.writer.sep.clear();
    this->start_(*this, opt );
  }

  void add(std::string val)
  {
    input.push_back( std::make_unique<data_type>(val.begin(), val.end()) );
  }

  boost::asio::io_context& service;
  std::list<data_ptr> input;
  std::string result;
};

UNIT(stream_unit, "")
{
  using namespace fas::testing;

  boost::asio::io_context io;
  stream f(io);

  f.add("Hello ");
  f.add("world");
  f.add("!");
  f.start();
  io.run();
  t << equal<expect, std::string>(f.result, "Hello world!") << f.result << FAS_TESTING_FILE_LINE;

}

BEGIN_SUITE(stream,"")
  ADD_UNIT(stream_unit)
END_SUITE(stream)

