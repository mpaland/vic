# vic demos
# The graphic demos use the Windows head (head/Windows.h), so they are Windows only.
# Every demo is one .cpp file in this folder which implements demo(), the head and
# main() are provided by windows/main.cpp

# head demos: one small portable program per head (heads/<head>.cpp with its own main()),
# the io layer heads/io_trace.cpp has no hardware behind it and dumps the command / data
# stream to stdout; it is the template for a real io layer
set(VIC_HEAD_DEMOS BA6x Digole HD44780 ILI9325 MAX7219 MIPI_DCS PCD8544 SSD1306 SSD1322 SSD1331 SSD1351 SSD1680
                   ST7565 ST7920 STPxxP05 seven_segment dummy framebuffer memory multihead)

foreach(demo ${VIC_HEAD_DEMOS})
  add_executable(vic_head_${demo} heads/${demo}.cpp heads/io_trace.cpp)
  target_link_libraries(vic_head_${demo} PRIVATE vic::vic)
  set_target_properties(vic_head_${demo} PROPERTIES
    CXX_STANDARD          14
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS        OFF
  )
  if(MSVC)
    target_compile_options(vic_head_${demo} PRIVATE /W4 /permissive- /utf-8 /EHsc)
  else()
    target_compile_options(vic_head_${demo} PRIVATE -Wall -Wextra)
  endif()
endforeach()

if(NOT WIN32)
  message(STATUS "vic demos need Windows, skipped")
  return()
endif()

set(VIC_DEMOS demo_sprite demo_clock demo_aa demo_font demo_gauges demo_widget demo_input)

foreach(demo ${VIC_DEMOS})
  add_executable(vic_${demo} windows/main.cpp ${demo}.cpp)
  target_link_libraries(vic_${demo} PRIVATE vic::vic user32 gdi32)
  target_include_directories(vic_${demo} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_compile_definitions(vic_${demo} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
  set_target_properties(vic_${demo} PROPERTIES
    CXX_STANDARD          14
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS        OFF
  )
  if(MSVC)
    target_compile_options(vic_${demo} PRIVATE /W4 /permissive- /utf-8 /EHsc)
  endif()
endforeach()
