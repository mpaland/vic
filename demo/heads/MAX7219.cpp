///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief MAX7219 LED matrix demo, one 8 x 8 module
// io layer: every io::write() is one 16 bit SPI frame (register, data), latched with
// the LOAD / CS line, option 0.
//
///////////////////////////////////////////////////////////////////////////////

#include "head/MAX7219.h"
#include "scene.h"


static vic::head::MAX7219<8, 8> matrix(reinterpret_cast<vic::io::handle_type>(0));


int main()
{
  demo_banner(matrix, "MAX7219 8 x 8");
  matrix.init();
  demo_scene(matrix);
  matrix.display_brightness(64U);
  return 0;
}
