cmake_minimum_required(VERSION 3.10)

project(iow VERSION "0.9.8")

include(cmake/ci.cmake)

wci_submodule(NAME faslib)
wci_submodule(NAME wjson)
wci_submodule(NAME wlog)
wci_submodule(NAME wflow)

ogenc_warning(-Wno-pre-c++14-compat "" ON)


add_library(
  iow
  iow/io/aux/read_buffer.cpp
  iow/io/aux/write_buffer.cpp
)
wci_targets(iow)
target_include_directories(iow PUBLIC $<BUILD_INTERFACE:${iow_SOURCE_DIR}>)

target_link_libraries(iow PUBLIC faslib wjson wlog wflow)
target_compile_definitions(iow PRIVATE -DBOOST_ASIO_NO_DEPRECATED)

if ( BUILD_TESTING )
  enable_testing()
  # Valgrind memcheck does not support AVX-512 from -march=native
  set(IOW_VALGRIND_COMPILE_OPTIONS)
  if ( NOT MSVC AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64|i[3-6]86)$" )
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-mno-avx512f" IOW_COMPILER_HAS_MNO_AVX512F)
    if ( IOW_COMPILER_HAS_MNO_AVX512F )
      list(APPEND IOW_VALGRIND_COMPILE_OPTIONS
        $<$<CONFIG:Release>:-mno-avx512f>
        $<$<CONFIG:RelWithDebInfo>:-mno-avx512f>)
      target_compile_options(iow PRIVATE ${IOW_VALGRIND_COMPILE_OPTIONS})
    endif()
  endif()
  add_subdirectory(tests)
endif()

if ( WITH_SAMPLES )
  add_subdirectory(examples)
endif()

include(cmake/install.cmake)
