///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief SSD1331 RGB OLED demo, 0.96" 96 x 64 SPI module
// io layer: option 0 = command, 1 = data (D/C# line), SPI mode 3, 6 MHz. The common
// modules are upright with orientation_180 (Adafruit remap 72h).
//
///////////////////////////////////////////////////////////////////////////////

#include "head/SSD1331.h"
#include "scene.h"


static vic::head::SSD1331<96, 64, vic::drv::orientation_180> oled(reinterpret_cast<vic::io::handle_type>(0));


int main()
{
  demo_banner(oled, "SSD1331 0.96\" SPI");
  oled.init();
  demo_scene(oled);
  // the graphic accelerator: move the scene 10 pixels to the right (copy command)
  vic::dc d(oled);
  d.move({ 0, 0, 86, 64 }, { 10, 0 });
  oled.display_brightness(255U);
  return 0;
}
