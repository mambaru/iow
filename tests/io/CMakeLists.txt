update_ogenc(
  TARGETS io_test
  WARNINGS -Wnull-dereference OFF)

set( SUITE_LIST
  io_base_suite.cpp
  flow_suite.cpp
  pipe_suite.cpp
  stream_suite.cpp
  factory_suite.cpp
  stream_holder_suite.cpp
  dgram_holder_suite.cpp
)

add_executable( io_test io_test.cpp ${SUITE_LIST} )
target_link_libraries(io_test PUBLIC iow fas::testing)
wci_test( TARGET io_test COMMAND io_test )

if ( NOT CODE_COVERAGE )

  update_ogenc(
    TARGETS udp_test tcp_acceptor tcp_client
    WARNINGS -Wnull-dereference OFF)

  add_executable( tcp_acceptor tcp_acceptor.cpp)
  target_link_libraries(tcp_acceptor PUBLIC iow)

  add_executable( tcp_client tcp_client.cpp)
  target_link_libraries( tcp_client PUBLIC iow )

  add_executable( udp_test udp_test.cpp)
  target_link_libraries( udp_test PUBLIC iow)

  wci_utils(tcp_acceptor tcp_client udp_test)

endif()
