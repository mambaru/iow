#include <memory>
#include <iostream>
#include <vector>
#include <iow/io/aux/write_buffer.hpp>


int main(int, char **)
{
  std::cout << "press ani key 0";
  std::cin.get();

  typedef std::vector<char> data_type;
  {
    iow::io::write_buffer wb;
    iow::io::write_buffer_options opt;
    wb.set_options(opt);
    for (int i=0; i < 1000; ++i)
    {
        wb.attach( std::make_unique<data_type>(100000) );
        wb.attach( std::make_unique<data_type>( data_type{'\r', '\n'}) );
    }
    std::cout << "press ani key 1";
    std::cin.get();
    auto now = time(nullptr);
    for (;;)
    {
      auto next = wb.next();
      if (next.second == 0) break;
      if ( now != time(nullptr) ){
        std::cout << "cunk_size:" << next.second << " write_buffer size: " << wb.size() << " write_buffer count: " << wb.count() << std::endl;
        now = time(nullptr);
      }
      wb.confirm(iow::io::write_buffer::data_pair(next.first, 1));
      wb.confirm(iow::io::write_buffer::data_pair(next.first+1, next.second-1));
      /*auto end = next.first;
      std::advance(end, next.second);
      for ( ;next.first!=end; ++next.first)
      {
        //std::cout << "\t" << std::distance(next.first,end) << std::endl;
        wb.confirm( iow::io::write_buffer::data_pair(next.first, 1) );
      }*/
    }
    auto stat = wb.get_stat(true);
    std::cout << "total_size:" << stat.total_size << std::endl;
    std::cout << "total_capacity:" << stat.total_capacity << std::endl;
    std::cout << "stat.chunk_count:" << stat.chunk_count << std::endl;
    std::cout << "stat.chunk_count_capacity:" << stat.chunk_count_capacity << std::endl;

    std::cout << "press ani key 3";
    std::cin.get();

    wb.clear();
    std::cout << "press ani key 4";
    std::cin.get();
  }
  std::cout << "press ani key 5";
  std::cin.get();
  return 0;
}

