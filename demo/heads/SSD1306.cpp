///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief SSD1306 / SH1106 OLED demo, 0.96" 128 x 64 I2C module and 1.3" SH1106 module
// io layer (I2C): every io::write() is one I2C transfer to address 3Ch, the control
// byte (00h commands, 40h data) is the first byte of the block; option is 0 / 1 too.
// The reference libraries show these modules upright with orientation_180.
//
///////////////////////////////////////////////////////////////////////////////

#include "head/SSD1306.h"
#include "scene.h"


static vic::head::SSD1306<128, 64, vic::drv::orientation_180> oled(reinterpret_cast<vic::io::handle_type>(0x3C));
static vic::head::SSD1306<128, 64, vic::drv::orientation_180, vic::head::SSD1306_I2C, true, true, vic::head::SSD1306_CTRL_SH1106> oled_sh1106(reinterpret_cast<vic::io::handle_type>(0x3C));


int main()
{
  demo_banner(oled, "SSD1306 0.96\" I2C");
  oled.init();
  demo_scene(oled);
  oled.display_contrast(0x40U);

  demo_banner(oled_sh1106, "SH1106 1.3\" I2C (page addressing, column offset 2)");
  oled_sh1106.init();
  demo_scene(oled_sh1106);
  return 0;
}
