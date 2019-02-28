cmake_minimum_required(VERSION 3.1)

project(iow)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(ci)

rootlibs(faslib wjson wlog wflow)

include_directories(.)
add_library(
  iow
  iow/io/aux/read_buffer.cpp
  iow/io/aux/write_buffer.cpp
)

if ( BUILD_TESTING )
  enable_testing()
  include_directories(.)
  include(FindThreads)
  find_package(Boost REQUIRED system )
  target_link_libraries(iow wlog wflow ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
  add_subdirectory(tests)
endif()

set(CPACK_PACKAGE_NAME "iow")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "7")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/Readme.md")
set(CPACK_PACKAGE_CONTACT "migashko@gmail.com")

install(DIRECTORY iow DESTINATION include FILES_MATCHING PATTERN "*.hpp" )
install(TARGETS iow LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
include(CPack)
