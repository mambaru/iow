#include <iostream>
#include <iow/io/aux/read_buffer.hpp>
#include <fas/testing.hpp>
#include <cstring>
#include <mutex>
//#include <ucommon/socket.h>

typedef std::vector<char> data_type;
typedef std::string sep_type;
typedef ::iow::io::read_buffer read_buffer;
typedef read_buffer::options_type options;

UNIT(basic_test, "")
{
  using namespace fas::testing;
  read_buffer buf;
  options opt;
  buf.get_options(opt);
  opt.bufsize = 3;
  buf.set_options(opt);
  
  //auto tmp = std::make_unique<data_type>(3);
  //! auto att = buf.attach(std::move(tmp));
   
  //t << is_true<assert>( att == nullptr) << FAS_TESTING_FILE_LINE;
  
  // Еще один attach() без next() запрещен
  //tmp = std::make_unique<data_type>(3);
  //! att = buf.attach(std::move(tmp));
  //t << is_true<assert>( att != nullptr) << FAS_TESTING_FILE_LINE;
  
  auto nxt1 = buf.next();
  t << is_true<assert>( nxt1.first != nullptr) << FAS_TESTING_FILE_LINE;
  t << is_true<assert>( nxt1.second == 3) << FAS_TESTING_FILE_LINE;
  std::strcpy(nxt1.first, "ab");
  // Еще один next() без confirm() запрещен
  auto nxt2 = buf.next();
  t << is_true<assert>( nxt2.first == nullptr) << FAS_TESTING_FILE_LINE;
  t << is_true<assert>( nxt2.second == 0) << FAS_TESTING_FILE_LINE;
  
  // После next() разрешен attach()
  //tmp = std::make_unique<data_type>(3);
  //! att = buf.attach(std::move(tmp));
  //t << is_true<assert>( att == nullptr) << FAS_TESTING_FILE_LINE;

  // Но еще один attach() без next() все еще запрещен
  //tmp = std::make_unique<data_type>(3);
  //! att = buf.attach(std::move(tmp));
  //t << is_true<assert>( att != nullptr) << FAS_TESTING_FILE_LINE;
  
  nxt1.second = 2;
  nxt1.first -= 1;
  bool res = buf.confirm(nxt1);
  t << is_false<assert>( res ) << FAS_TESTING_FILE_LINE;
  nxt1.first += 2;
  res = buf.confirm(nxt1);
  t << is_false<assert>( res ) << FAS_TESTING_FILE_LINE;
  nxt1.first -= 1;
  nxt1.second = 4096 + 1;
  res = buf.confirm(nxt1);
  t << is_false<assert>( res ) << FAS_TESTING_FILE_LINE;
  nxt1.second = 2;
  res = buf.confirm(nxt1);
  t << is_true<assert>( res ) << FAS_TESTING_FILE_LINE;
  
  auto d = buf.detach();
  t << is_true<assert>( d != nullptr) << FAS_TESTING_FILE_LINE;
  t << stop;
  
  auto str = std::string(d->begin(), d->end());
  t << equal_str<assert>( std::string("ab"), str  ) << FAS_TESTING_FILE_LINE;
}

template<typename T>
void test_buff1(T& t, read_buffer& buf, std::vector<std::string> reads, std::vector<std::string> chk)
{
  using namespace fas::testing;
  t << message("test_buff1");
  std::string incoming;
  std::string result;
  std::vector<std::string> vectres;
  for (auto& s: reads)
  {
    incoming+= s;
    auto p = buf.next();
    t << is_true<assert>(s.size() <= p.second) << FAS_TESTING_FILE_LINE;
    t << stop;
    std::strcpy( p.first, s.c_str());
    p.second = s.size();
    bool confirm = buf.confirm(p);
    t << is_true<assert>( confirm ) << FAS_TESTING_FILE_LINE;
    auto d = buf.detach();
    while ( d!=nullptr )
    {
      t << is_true<assert>( !d->empty() ) << FAS_TESTING_FILE_LINE;
      t << stop;
      auto str = std::string(d->begin(), d->end());
      vectres.push_back(str);
      result += vectres.back();
      d = buf.detach();
    }
  }
  t << equal<expect>(incoming, result) << incoming << "!=" << result << FAS_TESTING_FILE_LINE;
  if ( !chk.empty() )
    t << equal<expect>(chk, vectres) << FAS_TESTING_FILE_LINE;
}

template<typename T>
void test_buff2(T& t, read_buffer& buf, std::vector<std::string> reads, std::vector<std::string> chk)
{
  using namespace fas::testing;
  t << message("test_buff2");
  std::string incoming;
  std::string result;
  for (auto& s: reads)
  {
    incoming+= s;
    auto p = buf.next();
    t << is_true<assert>(s.size() <= p.second) << FAS_TESTING_FILE_LINE;
    t << stop;
    std::strcpy( p.first, s.c_str());
    p.second = s.size();
    bool confirm = buf.confirm(p);
    t << is_true<assert>( confirm ) << FAS_TESTING_FILE_LINE;
  }
  
  std::vector<std::string> vectres;
  size_t count = 0;
  auto d = buf.detach();
  while ( d!=nullptr )
  {
    ++count;
    t << is_true<assert>( !d->empty() ) << FAS_TESTING_FILE_LINE;
    t << stop;
    auto str = std::string(d->begin(), d->end());
    vectres.push_back(str);
    result += str;
    d = buf.detach();
  }

  t << equal<expect>(incoming, result) << incoming << "!=" << result << FAS_TESTING_FILE_LINE;
  if ( !chk.empty() )
  {
    options opt;
    buf.get_options(opt);
    if ( !opt.sep.empty() )
    {
      t << equal<expect>(chk.size(), vectres.size()) << FAS_FL;
      for (size_t i = 0 ; i != chk.size(); i++)
        if ( i < vectres.size() )
        {
          t << equal<expect>(chk[i], vectres[i]) << FAS_TESTING_FILE_LINE;
        }
    }
  }
}


template<typename T>
void do_detach(T& t, read_buffer& buf, std::vector<std::string>& vectres, std::string& result)
{
  using namespace fas::testing;
  auto d = buf.detach();
  while ( d!=nullptr )
  {
    t << is_true<assert>( !d->empty() ) << FAS_TESTING_FILE_LINE;
    t << stop;
    auto str = std::string(d->begin(), d->end());
    //t << message("detach: ") << str;
    //t << message("-2-");
    vectres.push_back(str);
    result += vectres.back();
    d = buf.detach();
  }
}

template<typename T>
void test_buff3(T& t, read_buffer& buf, std::vector<std::string> reads, std::vector<std::string> chk)
{
  using namespace fas::testing;
  t << message("test_buff3");
  std::string incoming;
  std::string result;
  std::vector<std::string> vectres;
  for (auto& s: reads)
  {
    // t << message("read:   ") << s ;
    // t << message("-1-");
    incoming+= s;
    auto p = buf.next();
    t << is_true<assert>(s.size() <= p.second) << s.size() << " > " << p.second << FAS_TESTING_FILE_LINE;
    t << stop;
    std::strcpy( p.first, s.c_str());
    p.second = s.size();
    do_detach(t, buf, vectres, result);
    bool confirm = buf.confirm(p);
    t << is_true<assert>( confirm ) << FAS_TESTING_FILE_LINE;
    t << stop;
  }
  do_detach(t, buf, vectres, result);
  t << equal<expect>(incoming, result) << incoming << "!=" << result << FAS_TESTING_FILE_LINE;
  if ( !chk.empty() )
    t << equal<expect>(chk, vectres) << FAS_TESTING_FILE_LINE;
}

template<typename T>
void test_buff4(T& t, read_buffer& buf, std::vector<std::string> reads, std::vector<std::string> chk)
{
  using namespace fas::testing;
  t << message("test_buff4");
  std::string incoming;
  std::string result;
  std::vector<std::string> vectres;
  auto p = buf.next();
  for (auto& s: reads)
  {
    incoming += s;
    t << is_true<assert>(s.size() <= p.second) << s.size() << " > " << p.second << FAS_TESTING_FILE_LINE;
    t << stop;
    std::strcpy( p.first, s.c_str());
    p.second = s.size();
    
    bool confirm = buf.confirm(p);
    t << is_true<assert>( confirm ) << FAS_TESTING_FILE_LINE;
    t << stop;
    p = buf.next();
    do_detach(t, buf, vectres, result);
  }
  
  p.second = 0;
  bool confirm = buf.confirm(p);
  t << is_true<assert>( confirm ) << FAS_TESTING_FILE_LINE;
  t << stop;
  
  do_detach(t, buf, vectres, result);
  t << equal<expect>(incoming, result) << incoming << "!=" << result << FAS_TESTING_FILE_LINE;
  if ( !chk.empty() )
    t << equal<expect>(chk, vectres) << FAS_TESTING_FILE_LINE;
  t << equal<assert, size_t>(buf.size(), 0) << "buf.size()==" << buf.size() << FAS_TESTING_FILE_LINE;
  t << equal<assert, size_t>(buf.count(), 0) << "buf.count()==" << buf.count() << FAS_TESTING_FILE_LINE;
}


template<typename T>
void test_buff(T& t, read_buffer& buf, std::vector<std::string> reads, std::vector<std::string> chk)
{
  test_buff1(t, buf, reads, chk);
  test_buff2(t, buf, reads, chk);
  test_buff3(t, buf, reads, chk);
  test_buff4(t, buf, reads, chk);
}

/*
template<typename T>
void test_buff0(T& t, read_buffer& buf, std::vector<std::string> reads, std::vector<std::string> chk)
{
  using namespace fas::testing;
  t << message("test_buff0");
  std::string result;
  std::vector<std::string> vectres;
  auto p = buf.next();
  for (auto& s: reads)
  {
    t << is_true<assert>(s.size() <= p.second) << s.size() << " > " << p.second << FAS_FL;
    t << stop;
    std::strcpy( p.first, s.c_str());
    p.second = s.size();
    
    bool confirm = buf.confirm(p);
    t << is_true<assert>( confirm ) << FAS_TESTING_FILE_LINE;
    t << stop;
    p = buf.next();
    do_detach(t, buf, vectres, result);
  }
  
  p.second = 0;
  bool confirm = buf.confirm(p);
  t << is_true<assert>( confirm ) << FAS_TESTING_FILE_LINE;
  t << stop;
  
  do_detach(t, buf, vectres, result);
  t << equal<expect>(incoming, result) << incoming << "!=" << result << FAS_TESTING_FILE_LINE;
  if ( !chk.empty() )
    t << equal<expect>(chk, vectres) << FAS_TESTING_FILE_LINE;
  t << equal<assert, size_t>(buf.size(), 0) << "buf.size()==" << buf.size() << FAS_TESTING_FILE_LINE;
  t << equal<assert, size_t>(buf.count(), 0) << "buf.count()==" << buf.count() << FAS_TESTING_FILE_LINE;
}
*/

UNIT(basic_sep0, "")
{
  using namespace fas::testing;
  read_buffer buf;
  options opt;
  buf.get_options(opt);
  opt.sep="";
  opt.bufsize = 6;
  opt.minbuf = 6;
  buf.set_options(opt);
  test_buff(t, buf, 
            {"aa|", "|", "bb|", "|cc|", "|dd", "||ee||"}, 
            {"aa|", "|", "bb|", "|cc|", "|dd", "||ee||"}
           );
  
  opt.bufsize = 1;
  opt.minbuf = 1;
  buf.set_options(opt);
  test_buff(t, buf, 
            {"a", "a", "|", "|", "b", "b", "|", "|", "c", "c", "|", "|", "d", "d", "|", "|", "e", "e", "|", "|"},
            {"a", "a", "|", "|", "b", "b", "|", "|", "c", "c", "|", "|", "d", "d", "|", "|", "e", "e", "|", "|"}
           );
  
  opt.bufsize = 2;
  opt.minbuf = 2;
  buf.set_options(opt);
  test_buff(t, buf, 
            {"aa", "||", "bb", "||", "cc", "||", "dd", "||", "ee", "||"},
            {"aa", "||", "bb", "||", "cc", "||", "dd", "||", "ee", "||"}
           );
  
}

UNIT(basic_sep1, "")
{
  using namespace fas::testing;
  read_buffer buf;
  options opt;
  buf.get_options(opt);
  opt.sep="|";
  opt.bufsize = 6;
  opt.minbuf = 6;
  buf.set_options(opt);
  
  
  test_buff(t, buf, 
            {"aa|", "|", "bb|", "|cc|", "|dd", "||ee||"}, 
            {"aa|", "|", "bb|", "|", "cc|", "|", "dd|", "|", "ee|", "|"}
           );
  
  
  
  opt.bufsize = 1;
  opt.minbuf = 1;
  buf.set_options(opt);
  test_buff(t, buf, 
            {"a", "a", "|", "|", "b", "b", "|", "|", "c", "c", "|", "|", "d", "d", "|", "|", "e", "e", "|", "|"},
            {"aa|", "|", "bb|", "|", "cc|", "|", "dd|", "|", "ee|", "|"}
           );
  
  opt.bufsize = 2;
  opt.minbuf = 2;
  buf.set_options(opt);
  test_buff(t, buf, 
            {"aa", "||", "bb", "||", "cc", "||", "dd", "||", "ee", "||"},
            {"aa|", "|", "bb|", "|", "cc|", "|", "dd|", "|", "ee|", "|"}
           );
  
  
  
}


UNIT(basic_sep2, "")
{
  using namespace fas::testing;
    read_buffer buf;
  options opt;
  buf.get_options(opt);
  opt.sep="##";
  opt.bufsize = 10;
  opt.minbuf = 10;
  buf.set_options(opt);
  
  test_buff(t, buf, 
            {"aa##", "##", "bb##", "##cc##", "##dd", "####ee####"}, 
            {"aa##", "##", "bb##", "##", "cc##", "##", "dd##", "##", "ee##", "##"}
           );
  
  
  
  opt.bufsize = 1;
  opt.minbuf = 1;
  buf.set_options(opt);
  test_buff(t, buf, 
            {"a", "a", "#","#", "#","#", "b", "b", "#","#", "#","#", "c", "c", "#","#", "#","#", "d", "d", "#","#", "#","#", "e", "e", "#","#", "#","#"},
            {"aa##", "##", "bb##", "##", "cc##", "##", "dd##", "##", "ee##", "##"}
           );
           
  opt.bufsize = 4;
  opt.minbuf = 4;
  buf.set_options(opt);
  test_buff(t, buf, 
            {"aa", "####", "bb", "####", "cc", "####", "dd", "####", "ee", "####"},
            {"aa##", "##", "bb##", "##", "cc##", "##", "dd##", "##", "ee##", "##"}
           );
}

UNIT(basic_sep3, "")
{
  using namespace fas::testing;
    read_buffer buf;
  options opt;
  buf.get_options(opt);
  opt.sep="@@@";
  opt.bufsize = 20;
  opt.minbuf = 20;
  buf.set_options(opt);
  
  test_buff(t, buf, 
            {"aa@@@", "@@@", "bb@@@", "@@@cc@@@", "@@@dd", "@@@@@@ee@@@@@@"}, 
            {"aa@@@", "@@@", "bb@@@", "@@@", "cc@@@", "@@@", "dd@@@", "@@@", "ee@@@", "@@@"}
           );
  
  
  
  opt.bufsize = 1;
  opt.minbuf = 1;
  buf.set_options(opt);
  test_buff(t, buf, 
            {"a", "a", "@","@","@", "@","@","@", "b", "b", "@", "@","@", "@", "@","@", "c", "c", "@", "@","@", "@", "@","@", "d", "d", "@", "@","@", "@", "@","@", "e", "e", "@","@","@", "@","@","@"},
            {"aa@@@", "@@@", "bb@@@", "@@@", "cc@@@", "@@@", "dd@@@", "@@@", "ee@@@", "@@@"}
           );
           
  opt.bufsize = 6;
  opt.minbuf = 6;
  buf.set_options(opt);
  test_buff(t, buf, 
            {"aa", "@@@@@@", "bb", "@@@@@@", "cc", "@@@@@@", "dd", "@@@@@@", "ee", "@@@@@@"},
            {"aa@@@", "@@@", "bb@@@", "@@@", "cc@@@", "@@@", "dd@@@", "@@@", "ee@@@", "@@@"}
           );
}


BEGIN_SUITE(read_buffer, "read_buffer suite")
  ADD_UNIT(basic_test)
  ADD_UNIT(basic_sep0)
  ADD_UNIT(basic_sep1)
  ADD_UNIT(basic_sep2)
  ADD_UNIT(basic_sep3)
END_SUITE(read_buffer)

BEGIN_TEST
  RUN_SUITE(read_buffer)
END_TEST
