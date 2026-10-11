///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief Dummy head demo: a head that swallows everything
// Useful as placeholder while the hardware is not there yet, every call compiles
// and runs, nothing is output.
//
///////////////////////////////////////////////////////////////////////////////

#include "head/dummy.h"
#include "scene.h"


static vic::head::dummy<128, 64> nothing;


int main()
{
  demo_banner(nothing, "dummy 128 x 64");
  nothing.init();
  demo_scene(nothing);
  std::printf("nothing happened, as expected\n");
  return 0;
}
