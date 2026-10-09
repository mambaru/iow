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

update_ogenc(
  TARGETS max_connections_test accept_error_test server_reconfigure_test resolve_test get_stat_test
  WARNINGS -Wnull-dereference OFF)

add_executable( max_connections_test max_connections_test.cpp )
target_link_libraries(max_connections_test PUBLIC iow fas::testing)
wci_test( TARGET max_connections_test COMMAND max_connections_test )

add_executable( accept_error_test accept_error_test.cpp )
target_link_libraries(accept_error_test PUBLIC iow fas::testing)
wci_test( TARGET accept_error_test COMMAND accept_error_test )

add_executable( server_reconfigure_test server_reconfigure_test.cpp )
target_link_libraries(server_reconfigure_test PUBLIC iow fas::testing)
wci_test( TARGET server_reconfigure_test COMMAND server_reconfigure_test )

add_executable( resolve_test resolve_test.cpp )
target_link_libraries(resolve_test PUBLIC iow fas::testing)
wci_test( TARGET resolve_test COMMAND resolve_test )

add_executable( get_stat_test get_stat_test.cpp )
target_link_libraries(get_stat_test PUBLIC iow fas::testing)
wci_test( TARGET get_stat_test COMMAND get_stat_test )

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
