///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief SSD1322 gray scale OLED demo, 3.12" 256 x 64 module (NHD-3.12-25664)
// io layer: option 0 = command, 1 = data (D/C# line), 4 wire SPI, 10 MHz. The NHD
// modules are upright with orientation_0m, the panel sits at RAM column 1Ch.
//
///////////////////////////////////////////////////////////////////////////////

#include "head/SSD1322.h"
#include "scene.h"


static vic::head::SSD1322<256, 64, vic::drv::orientation_0m> oled(reinterpret_cast<vic::io::handle_type>(0));


int main()
{
  demo_banner(oled, "SSD1322 3.12\" SPI, 16 gray levels");
  oled.init();
  demo_scene(oled);
  // gray ramp along the bottom line
  vic::dc d(oled);
  for (std::int16_t x = 0; x < 256; ++x) {
    const std::uint8_t gs = static_cast<std::uint8_t>(x);
    d.plot({ x, 62 }, vic::color::argb(gs, gs, gs));
  }
  return 0;
}
