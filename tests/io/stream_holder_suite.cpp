#include <iostream>
#include <iow/io/descriptor/holder.hpp>
#include <iow/io/socket/stream/aspect.hpp>
#include <iow/io/socket/stream/options.hpp>
#include <iow/io/reader/asio/aspect.hpp>
#include <iow/io/writer/asio/aspect.hpp>
#include <iow/io/rw/aspect.hpp>
#include <iow/io/basic/aspect.hpp>

#include <fas/testing.hpp>

typedef std::vector<char> data_type;
typedef iow::asio::posix::stream_descriptor descriptor_type;
typedef iow::io::socket::stream::options options_type;

/*
struct ad_initialize
{
  template<typename T, typename O>
  void operator()(T& t, O&& opt)  const
  {
    t.get_aspect().template get< ::iow::io::rw::_initialize_ >()(t, opt);
  }
};
*/

struct aspect_stream : fas::aspect<
    fas::type< ::iow::io::descriptor::_descriptor_type_, descriptor_type >,
    fas::type< ::iow::io::_options_type_, options_type >,
    fas::alias< ::iow::io::socket::stream::_initialize_, ::iow::io::rw::_initialize_>,
    ::iow::io::socket::stream::aspect,
    // Заглужка descriptor_type не поддерживает set_options
    
    ::iow::io::reader::asio::aspect,
    ::iow::io::writer::asio::aspect,
    ::iow::io::rw::aspect,
    ::iow::io::basic::aspect< std::recursive_mutex >::advice_list
>{};

typedef ::iow::io::descriptor::holder<aspect_stream> stream_holder_t;


UNIT(stream_holder_unit, "")
{
  using namespace fas::testing;
  iow::asio::io_service service;
  int f1[2]={-1, -1};
  int f2[2]={-1, -1};
  int res1 = ::pipe(f1);
  int res2 = ::pipe(f2);
  t << message("pipe1: ") << res1 << " " << f1[0] << " " << f1[1] << std::endl;
  t << message("pipe2: ") << res2 << " " << f2[0] << " " << f2[1] << std::endl;
  descriptor_type d1(service, f1[0]);
  descriptor_type d2(service, f2[1]);
  
  auto h1 = std::make_shared<stream_holder_t>(std::move(d1));
  auto h2 = std::make_shared<stream_holder_t>(std::move(d2));
  const char* instr = "Hello world!\r\nBuy!";
  t << message("write...");
  res1 = int(write(f1[1], instr, std::strlen(instr) ));
  t << message("...write:") << res1;
  
  options_type opt;
  opt.input_handler = [&]( iow::io::data_ptr d, size_t, options_type::output_handler_type /*callback*/){
    t << message("data: ") << d ;
    h2->get_aspect().get< ::iow::io::writer::_output_>()( *h2, std::move(d) );
  };
  opt.reader.sep = "\r\n";
  opt.reader.trimsep = true;
  opt.writer.sep = "";
  t << message("start1");
  h1->start(opt);
  t << message("start2");
  h2->reconfigure(opt);
  t << message("service.run()...");
  service.run_one();
  t << message("...service.run()");
  
  
  char outstr[128];
  ssize_t size = read(f2[0], outstr, 128);
  outstr[size]='\0';
  t << message("outstr: ") << outstr << std::endl;
  t << nothing;
  t << equal<expect, std::string>(outstr, "Hello world!") << FAS_FL;
}

BEGIN_SUITE(stream_holder,"")
  ADD_UNIT(stream_holder_unit)
END_SUITE(stream_holder)

