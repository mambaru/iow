
add_executable(write_buffer_test $ENV{FAS_TESTING_CPP} write_buffer_test.cpp)
target_link_libraries(write_buffer_test PUBLIC iow)
wci_test(TARGET write_buffer_test COMMAND write_buffer_test )

add_executable(read_buffer_test $ENV{FAS_TESTING_CPP} read_buffer_test.cpp)
target_link_libraries(read_buffer_test PUBLIC iow)
wci_test(TARGET read_buffer_test COMMAND read_buffer_test )

add_executable(write_buffer_bench write_buffer_bench.cpp)
target_link_libraries(write_buffer_bench PUBLIC iow)

add_executable(read_buffer_bench read_buffer_bench.cpp)
target_link_libraries(read_buffer_bench PUBLIC iow)

add_executable(read_buffer_bench_big read_buffer_bench_big.cpp)
target_link_libraries(read_buffer_bench_big PUBLIC iow)

wci_utils(write_buffer_bench read_buffer_bench read_buffer_bench_big)


