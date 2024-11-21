cmake_minimum_required(VERSION 3.10)

project(iow VERSION "0.9.8")

include(cmake/ci.cmake)

wci_submodule(NAME faslib)
wci_submodule(NAME wjson)
wci_submodule(NAME wlog)
wci_submodule(NAME wflow)

ogenc_warning(-Wno-pre-c++14-compat "" ON)


add_library(
  iow
  iow/io/aux/read_buffer.cpp
  iow/io/aux/write_buffer.cpp
)
wci_targets(iow)
target_include_directories(iow PUBLIC $<BUILD_INTERFACE:${iow_SOURCE_DIR}>)

target_link_libraries(iow PUBLIC faslib wjson wlog wflow)
target_compile_definitions(iow PRIVATE -DBOOST_ASIO_NO_DEPRECATED)

if ( BUILD_TESTING )
  enable_testing()
  add_subdirectory(tests)
endif()

include(cmake/install.cmake)
