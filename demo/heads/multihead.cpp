///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief Multihead demo: four MAX7219 8 x 8 modules side by side as one 32 x 8 screen
// Every module is its own head (own chip select handle), the multihead maps the
// screen coordinates onto them.
//
///////////////////////////////////////////////////////////////////////////////

#include "head/MAX7219.h"
#include "head/multihead.h"
#include "scene.h"


static vic::head::MAX7219<8, 8> m0(reinterpret_cast<vic::io::handle_type>(0));
static vic::head::MAX7219<8, 8> m1(reinterpret_cast<vic::io::handle_type>(1));
static vic::head::MAX7219<8, 8> m2(reinterpret_cast<vic::io::handle_type>(2));
static vic::head::MAX7219<8, 8> m3(reinterpret_cast<vic::io::handle_type>(3));
static vic::head::multihead<32, 8, 4> wall = { { m0, { 0, 0 } }, { m1, { 8, 0 } }, { m2, { 16, 0 } }, { m3, { 24, 0 } } };


int main()
{
  demo_banner(wall, "4 x MAX7219 as 32 x 8");
  wall.init();
  demo_scene(wall);
  return 0;
}
