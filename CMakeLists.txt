cmake_minimum_required(VERSION 3.1)

project(iow)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(mambaru)
include(FindThreads)
find_package(Boost COMPONENTS system date_time REQUIRED)

add_library(
  iow
  iow/io/aux/read_buffer.cpp
  iow/io/aux/write_buffer.cpp
)

message(STATUS "${WFLOW_LIB}")
message(STATUS "${WLOG_LIB}")

target_link_libraries(iow ${WFLOW_LIB} ${WLOG_LIB} ${CMAKE_THREAD_LIBS_INIT}  ${Boost_LIBRARIES} )

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
