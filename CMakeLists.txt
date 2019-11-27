cmake_minimum_required(VERSION 3.1)

project(iow)

include(cmake/ci.cmake)

wci_getlib(NAME faslib)
wci_getlib(NAME wjson)
wci_getlib(NAME wlog)
wci_getlib(NAME wflow)

add_library(
  iow
  iow/io/aux/read_buffer.cpp
  iow/io/aux/write_buffer.cpp
)
wci_targets(iow)
target_include_directories(iow PUBLIC ${iow_SOURCE_DIR})

include(FindThreads)
find_package(Boost REQUIRED system )
target_link_libraries(iow PUBLIC faslib wjson wlog wflow ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

if ( BUILD_TESTING )
  enable_testing()
  add_subdirectory(tests)
endif()

set(CPACK_PACKAGE_NAME "iow")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "9")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/Readme.md")
set(CPACK_PACKAGE_CONTACT "migashko@gmail.com")

install(DIRECTORY iow DESTINATION include FILES_MATCHING PATTERN "*.hpp" )
install(TARGETS iow LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
include(CPack)
