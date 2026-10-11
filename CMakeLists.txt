cmake_minimum_required(VERSION 3.20)

project(vic VERSION 0.3.0 LANGUAGES CXX
        DESCRIPTION "Video Interface Controller - header only graphics library for embedded displays")

# header only interface library
add_library(vic INTERFACE)
add_library(vic::vic ALIAS vic)
target_include_directories(vic INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include/vic>
)
target_compile_features(vic INTERFACE cxx_std_11)

# tests are only built when vic is the top level project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  option(VIC_BUILD_TESTS        "Build the vic unit tests"            ON)
  option(VIC_WARNINGS_AS_ERRORS "Treat compiler warnings as errors"   ON)
  option(VIC_TEST_ASAN          "Build the tests with sanitizers"     OFF)
  option(VIC_BUILD_DEMOS        "Build the (Windows) demos"           ${WIN32})

  if(VIC_BUILD_TESTS)
    include(CTest)
    add_subdirectory(test)
  endif()
  if(VIC_BUILD_DEMOS)
    add_subdirectory(demo)
  endif()
endif()
