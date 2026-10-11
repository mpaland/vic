///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief Memory head demo: a pure RAM head for off screen rendering and tests
// The head needs no io layer, the scene is rendered into RAM and printed as ASCII art.
//
///////////////////////////////////////////////////////////////////////////////

#include "head/memory.h"
#include "scene.h"


static vic::head::memory<48, 16> mem;


int main()
{
  demo_banner(mem, "memory head, printed as ASCII art");
  mem.init();
  demo_scene(mem);
  for (std::int16_t y = 0; y < 16; ++y) {
    for (std::int16_t x = 0; x < 48; ++x) {
      std::putchar(mem.pixel({ x, y }) != vic::color::none ? '#' : '.');
    }
    std::putchar('\n');
  }
  return 0;
}
