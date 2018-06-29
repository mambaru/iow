cmake_minimum_required(VERSION 3.1)

project(iow)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(mambaru)

get_faslib()
if (NOT IOW_DISABLE_JSON)
  get_mambaru(wjson WJSON_DIR "")
else()
  add_definitions(-DIOW_DISABLE_JSON)
endif()

if (NOT IOW_DISABLE_LOG)
  get_mambaru(wlog WLOG_DIR WLOG_LIB)
else()
  add_definitions(-DIOW_DISABLE_LOG)
endif(NOT IOW_DISABLE_LOG)

get_mambaru(wflow WFLOW_DIR WFLOW_LIB)


include(FindThreads)
find_package(Boost COMPONENTS system date_time REQUIRED)

add_library(
  iow
  iow/io/aux/read_buffer.cpp
  iow/io/aux/write_buffer.cpp
  iow/io/aux/global_pool.cpp
)

if (NOT IOW_DISABLE_LOG)
  target_link_libraries(iow ${WFLOW_LIB} ${WLOG_LIB} ${CMAKE_THREAD_LIBS_INIT}  ${Boost_LIBRARIES} )
else()
  target_link_libraries(iow ${WFLOW_LIB} ${CMAKE_THREAD_LIBS_INIT}  ${Boost_LIBRARIES} )
  add_definitions(-DIOW_DISABLE_LOG=ON)
endif()

if ( BUILD_TESTING )
  enable_testing()
  add_subdirectory(tests)
  #add_subdirectory(examples)
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
