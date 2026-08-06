add_definitions(-DBOOST_ASIO_NO_DEPRECATED)

if ( IOW_VALGRIND_COMPILE_OPTIONS )
  add_compile_options(${IOW_VALGRIND_COMPILE_OPTIONS})
endif()

add_subdirectory(aux)
add_subdirectory(io)

