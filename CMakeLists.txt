cmake_minimum_required(VERSION 3.1)

project(iow)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(ci)

if ( STANDALONE )
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/wfc/cmake)
  getlibs(faslib wjson wlog wflow)
endif()

include_directories(.)
add_library(
  iow
  iow/io/aux/read_buffer.cpp
  iow/io/aux/write_buffer.cpp
)

if ( STANDALONE )
  target_link_libraries(iow wlog wflow)
endif()

if ( BUILD_TESTING )
  enable_testing()
  add_subdirectory(tests)
endif()

set(CPACK_PACKAGE_NAME "iow")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "7")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/Readme.md")
set(CPACK_PACKAGE_CONTACT "migashko@gmail.com")

install(DIRECTORY iow DESTINATION include FILES_MATCHING PATTERN "*.hpp" )
install(TARGETS iow LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
include(CPack)
