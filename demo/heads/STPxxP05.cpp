///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief STP16CP05 / STP08DP05 LED driver demo, two chained 16 bit drivers as a 32 x 1 bar
// io layer: io::write() shifts the bytes out (SPI), the latch (LE) pulse follows the
// transfer, option 0.
//
///////////////////////////////////////////////////////////////////////////////

#include "head/STPxxP05.h"
#include "scene.h"


static vic::head::STPxxP05<32, 1, 4U> bar(reinterpret_cast<vic::io::handle_type>(0));


int main()
{
  demo_banner(bar, "STP16CP05 x 2, 32 LEDs");
  bar.init();
  demo_scene(bar);
  return 0;
}
