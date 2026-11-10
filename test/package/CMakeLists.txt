# Consumer of the installed vic package, checks the install rules and the package config:
#   cmake --install build --prefix <prefix>
#   cmake -S test/package -B build-package -DCMAKE_PREFIX_PATH=<prefix>
#   cmake --build build-package && ctest --test-dir build-package

cmake_minimum_required(VERSION 3.20)

project(vic_package_test LANGUAGES CXX)

find_package(vic CONFIG REQUIRED)

add_executable(vic_package_test main.cpp)
target_link_libraries(vic_package_test PRIVATE vic::vic)

enable_testing()
add_test(NAME vic.package COMMAND vic_package_test)
