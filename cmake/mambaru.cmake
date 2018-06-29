if ( NOT "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}" )
  message(STATUS "${PROJECT_NAME} is not top level project")
  return()
endif()

include(mambaopt)
include(mambalibs)

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

include_directories(${CMAKE_CURRENT_SOURCE_DIR})


