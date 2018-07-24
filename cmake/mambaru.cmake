include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if ( NOT "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}" )
  message(STATUS "${PROJECT_NAME} is not top level project")
  return()
endif()

include(mambaopt)
include(mambalibs)

get_faslib()

if (NOT IOW_DISABLE_JSON)
  get_mambaru(wjson WJSON_DIR "" "")
  set(WFLOW_DISABLE_JSON OFF)
  set(WLOG_DISABLE_JSON OFF)
else()
  add_definitions(-DIOW_DISABLE_JSON)
  set(WFLOW_DISABLE_JSON ON)
  set(WLOG_DISABLE_JSON ON)
endif()

if (NOT IOW_DISABLE_LOG)
  get_mambaru(wlog WLOG_DIR WLOG_LIB "-DWLOG_DISABLE_JSON=${WLOG_DISABLE_JSON}")
else()
  add_definitions(-DIOW_DISABLE_LOG)
  set(WLOG_LIB "")
endif()

get_mambaru(wflow WFLOW_DIR WFLOW_LIB "-DWFLOW_DISABLE_LOG=${WFLOW_DISABLE_LOG} -DWFLOW_DISABLE_JSON=${WFLOW_DISABLE_JSON}")

