set( SUITE_LIST
  io_base_suite.cpp
  flow_suite.cpp
  pipe_suite.cpp
  stream_suite.cpp
  factory_suite.cpp
  stream_holder_suite.cpp
)

add_executable( io_test ${FAS_TESTING_CPP} io_test.cpp ${SUITE_LIST} )
target_link_libraries( io_test ${CMAKE_THREAD_LIBS_INIT}  ${Boost_LIBRARIES} )

add_executable( tcp_acceptor tcp_acceptor.cpp)
target_link_libraries( tcp_acceptor ${CMAKE_THREAD_LIBS_INIT}  ${Boost_LIBRARIES} )

add_test( NAME io_test COMMAND io_test WORKING_DIRECTORY ${IOW_TEST_DIR} )