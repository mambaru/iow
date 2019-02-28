enable_testing()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${UNIT_TEST_BIN_OUTPUT_DIR})

add_subdirectory(owner)
add_subdirectory(aux)
add_subdirectory(io)

