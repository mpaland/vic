///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief ST7565 / ST7567 / UC1701 demo, 128 x 64 COG module (EA DOGM128 style)
// io layer: option 0 = command, 1 = data (A0 line), SPI mode 3, 20 MHz. The DOGM
// and ERC modules are upright with orientation_180 or orientation_0m, some glasses
// start at RAM column 4 (Column_Offset).
//
///////////////////////////////////////////////////////////////////////////////

#include "head/ST7565.h"
#include "scene.h"


static vic::head::ST7565<vic::head::ST7565_ST7565, 128, 64, vic::drv::orientation_180> lcd(reinterpret_cast<vic::io::handle_type>(0));
// variants:
//   vic::head::ST7565<vic::head::ST7565_ST7567, 128, 64>                                lcd_st7567;
//   vic::head::ST7565<vic::head::ST7565_UC1701, 128, 64, vic::drv::orientation_0, 4U>   lcd_uc1701;   // column offset 4


int main()
{
  demo_banner(lcd, "ST7565 128 x 64 COG");
  lcd.init();
  demo_scene(lcd);
  lcd.display_contrast(0x90U);
  return 0;
}
