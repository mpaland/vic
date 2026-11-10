cmake_minimum_required(VERSION 3.20)

project(vic VERSION 2.2.0 LANGUAGES CXX   # x-release-please-version
        DESCRIPTION "Video Interface Controller - header only graphics library for embedded displays")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# header only interface library
add_library(vic INTERFACE)
add_library(vic::vic ALIAS vic)
target_include_directories(vic INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/vic>
)
target_compile_features(vic INTERFACE cxx_std_11)

# install rules and the package config for find_package(vic), on by default if vic is the top level project
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" VIC_TOP_LEVEL)
option(VIC_INSTALL "Generate the install rules and the package config of vic" ${VIC_TOP_LEVEL})
if(VIC_INSTALL)
  set(VIC_CMAKE_DIR ${CMAKE_INSTALL_DATADIR}/cmake/vic)
  install(TARGETS vic EXPORT vicTargets)
  install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vic
          FILES_MATCHING PATTERN "*.h"
          PATTERN "doc" EXCLUDE
          PATTERN "ttf" EXCLUDE)
  install(EXPORT vicTargets NAMESPACE vic:: DESTINATION ${VIC_CMAKE_DIR})
  configure_package_config_file(cmake/vicConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/vicConfig.cmake
                                INSTALL_DESTINATION ${VIC_CMAKE_DIR})
  write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/vicConfigVersion.cmake
                                   COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/vicConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/vicConfigVersion.cmake
          DESTINATION ${VIC_CMAKE_DIR})
endif()

# tests are only built when vic is the top level project
if(VIC_TOP_LEVEL)
  option(VIC_BUILD_TESTS        "Build the vic unit tests"            ON)
  option(VIC_WARNINGS_AS_ERRORS "Treat compiler warnings as errors"   ON)
  option(VIC_TEST_ASAN          "Build the tests with sanitizers"     OFF)
  option(VIC_BUILD_DEMOS        "Build the demos (graphic demos need Windows or X11)" ${WIN32})

  if(VIC_BUILD_TESTS)
    include(CTest)
    add_subdirectory(test)
  endif()
  if(VIC_BUILD_DEMOS)
    add_subdirectory(demo)
  endif()
endif()
