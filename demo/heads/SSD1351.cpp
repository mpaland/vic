///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief SSD1351 RGB OLED demo, 1.5" 128 x 128 SPI module
// io layer: option 0 = command, 1 = data (D/C# line), SPI mode 0, 20 MHz. The
// Adafruit modules are upright with orientation_0m (remap 74h).
//
///////////////////////////////////////////////////////////////////////////////

#include "head/SSD1351.h"
#include "scene.h"


static vic::head::SSD1351<128, 128, vic::drv::orientation_0m> oled(reinterpret_cast<vic::io::handle_type>(0));


int main()
{
  demo_banner(oled, "SSD1351 1.5\" SPI");
  oled.init();
  demo_scene(oled);
  oled.display_contrast(200U);
  return 0;
}
