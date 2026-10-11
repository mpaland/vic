///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief Seven segment demo, 4 digits driven by shift registers
// io layer: io::write() shifts one segment byte per digit out (SPI), option 0.
//
///////////////////////////////////////////////////////////////////////////////

#include "head/seven_segment.h"
#include "scene.h"


static vic::head::seven_segment<4> digits(reinterpret_cast<vic::io::handle_type>(0));


int main()
{
  demo_banner(digits, "4 x 7 segment");
  digits.init();
  vic::tc t(digits);
  t.cls();
  t.out({ 0, 0 }, reinterpret_cast<const std::uint8_t*>("12.34"));
  return 0;
}
