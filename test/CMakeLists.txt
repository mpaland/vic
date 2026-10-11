# vic unit tests
# All tests are compiled into one executable, test cases are registered by name.
# Test case names are prefixed with the module name (util_, color_, gpr_, ...)
# so that ctest can run them per module via --filter.

add_executable(vic_tests
  test_main.cpp
  io_stub.cpp
  test_util.cpp
  test_color.cpp
  test_drv.cpp
  test_gpr_lines.cpp
  test_gpr_box_rect.cpp
  test_gpr_circle_disc_sector.cpp
  test_gpr_triangle_polyline.cpp
  test_gpr_fill.cpp
  test_gpr_aa.cpp
  test_dc.cpp
  test_shader.cpp
  test_txr_tc.cpp
  test_sprite.cpp
  test_multihead.cpp
  test_heads_compile.cpp
  test_avl_array.cpp
  test_perf.cpp
  test_widget.cpp
  test_framebuffer.cpp
  test_fonts.cpp
)

target_link_libraries(vic_tests PRIVATE vic::vic)

set_target_properties(vic_tests PROPERTIES
  CXX_STANDARD          11
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS        OFF
)

if(MSVC)
  # MSVC has no /std:c++11, C++14 is the lowest available standard.
  # /permissive- enforces two-phase name lookup like GCC/Clang do.
  target_compile_options(vic_tests PRIVATE
    /W4 /permissive- /utf-8 /EHsc /std:c++14 /Zc:__cplusplus
    $<$<BOOL:${VIC_WARNINGS_AS_ERRORS}>:/WX>
  )
  if(VIC_TEST_ASAN)
    target_compile_options(vic_tests PRIVATE /fsanitize=address /Zi)
    target_link_options(vic_tests PRIVATE /INCREMENTAL:NO)
  endif()
else()
  target_compile_options(vic_tests PRIVATE
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
    $<$<BOOL:${VIC_WARNINGS_AS_ERRORS}>:-Werror>
  )
  if(VIC_TEST_ASAN)
    target_compile_options(vic_tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer -g)
    target_link_options(vic_tests PRIVATE -fsanitize=address,undefined)
  endif()
endif()

# one ctest entry per module
foreach(mod util color drv gpr dc shader txr tc sprite multihead heads avl perf widget framebuffer fonts)
  add_test(NAME vic.${mod} COMMAND vic_tests --filter ${mod}_)
endforeach()

# micro benchmark (not a test)
add_executable(vic_bench bench.cpp)
target_link_libraries(vic_bench PRIVATE vic::vic)
set_target_properties(vic_bench PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
if(MSVC)
  target_compile_options(vic_bench PRIVATE /W4 /permissive- /utf-8 /EHsc /std:c++14)
else()
  target_compile_options(vic_bench PRIVATE -Wall -Wextra)
endif()
