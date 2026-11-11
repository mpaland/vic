---
title: Installation
description: Add vic to a CMake or PlatformIO project, or simply put src/ on the include path.
sidebar:
  order: 2
---

vic is header only, so the `src` folder on the include path is all it needs. Your platform provides the I/O
functions declared in `io.h` (bus access and delay), see [Platform I/O](#platform-io) below.

## CMake

The CMake project provides the `vic::vic` interface target. There are three ways to get it:

```cmake
# installed package: cmake --install build --prefix <prefix>, then
find_package(vic CONFIG REQUIRED)

# or fetched at configure time
include(FetchContent)
FetchContent_Declare(vic GIT_REPOSITORY https://github.com/mpaland/vic.git GIT_TAG v2.2.0)
FetchContent_MakeAvailable(vic)

# or as a sub folder / git submodule
add_subdirectory(vic)

target_link_libraries(my_app PRIVATE vic::vic)
```

Tests, demos and install rules are only generated if vic is the top level project (`-DVIC_INSTALL=ON` forces the
install rules).

## PlatformIO

Add the repository to the `lib_deps` of your `platformio.ini`, `library.json` is the manifest:

```ini
[env:my_board]
lib_deps = https://github.com/mpaland/vic.git
```

## Plain include path

Copy or submodule the repository and add `vic/src` to the include path of your compiler. Nothing has to be compiled.

## Configuration

Compile time options are set in `src/vic_cfg.h`, for example `VIC_FONT_VECTOR_ASCII_ONLY` to keep only the ASCII
glyphs of the vector fonts. See [Configuration](/vic/reference/config/).

## Platform I/O

The head drivers talk to the hardware through the functions declared in `src/io.h`. On a new platform you implement
them once for your bus (SPI, I²C, UART, parallel or memory mapped) and every head runs on it. The head demos in
`demo/heads/` link `io_trace.cpp`, which dumps the command / data stream to stdout: copy a head demo and replace the
`vic::io` functions with your bus code to bring up a display. See [Write your own head](/vic/heads/skeleton/#platform-io)
for the details.

## Building the tests and demos

vic itself needs no build. The repository contains a CMake project which builds the unit tests (`test/`) and, on
Windows and Linux (X11), the demos (`demo/`):

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

On Windows `build.cmd [Debug|Release]` locates Visual Studio and does the above. On Linux the demos need the Xlib
headers (`sudo apt install libx11-dev`) and `-DVIC_BUILD_DEMOS=ON`.

The tests are compiled with `/W4 /WX /permissive-` (MSVC) and `-Wall -Wextra -Wpedantic -Wshadow -Werror` in strict
C++11 mode (GCC, Clang). `-DVIC_TEST_ASAN=ON` builds them with address and undefined behaviour sanitizers. The test
framework (`test/vic_test.h`) is self contained: the tests render on the `memory` head and compare the result with
ASCII art rasters.
