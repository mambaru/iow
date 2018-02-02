cmake_minimum_required(VERSION 2.6)

project(iow)

if ( "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}" )
  INCLUDE(FindThreads)
  SET(CMAKE_CXX_FLAGS       "${CXX_FLAGS} -std=c++11 -ftemplate-backtrace-limit=0")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -ggdb -DNDEBUG -march=core2 ") 
  SET(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb ")
  if (PARANOID_WARNING)
    if(UNIX)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-align \
                         -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization\
                         -Wformat=2 -Winit-self -Wmissing-declarations \
                         -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual \
                         -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo \
                         -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror")
      # -Wlogical-op -Wnoexcept -Wstrict-null-sentinel 
    endif()
  endif ()

  set(FASLIB_DIR "${CMAKE_SOURCE_DIR}/../faslib")
  set(FAS_TESTING_CPP "${FASLIB_DIR}/fas/testing/testing.cpp")
  find_package(Boost COMPONENTS system date_time REQUIRED)
  include_directories(../faslib)
  include_directories(../wjson)
  include_directories(../wflow)
endif()

include_directories(.)

add_library(
  iow
  iow/io/aux/read_buffer.cpp
  iow/io/aux/write_buffer.cpp
  iow/io/aux/global_pool.cpp
)

target_link_libraries(iow ${CMAKE_THREAD_LIBS_INIT}  ${Boost_LIBRARIES})

if ( BUILD_TESTING )
  enable_testing()
  add_definitions(-DIOW_DISABLE_LOG)
  add_subdirectory(tests)
endif()

