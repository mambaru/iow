cmake_minimum_required(VERSION 3.10)

project(iow)

include(cmake/ci.cmake)

wci_submodule(NAME faslib)
wci_submodule(NAME wjson)
wci_submodule(NAME wlog)
wci_submodule(NAME wflow)

add_library(
  iow
  iow/io/aux/read_buffer.cpp
  iow/io/aux/write_buffer.cpp
)
wci_targets(iow)
target_include_directories(iow PUBLIC ${iow_SOURCE_DIR})

target_link_libraries(iow PUBLIC faslib wjson wlog wflow)
target_compile_definitions(iow PRIVATE -DBOOST_ASIO_NO_DEPRECATED)

if ( BUILD_TESTING )
  enable_testing()
  add_subdirectory(tests)
endif()

set(CPACK_PACKAGE_NAME "iow")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "9")
set(CPACK_PACKAGE_VERSION_PATCH "4")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/Readme.md")
set(CPACK_PACKAGE_CONTACT "migashko@gmail.com")

install(DIRECTORY iow DESTINATION include FILES_MATCHING PATTERN "*.hpp" )
install(TARGETS iow LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
include(CPack)
