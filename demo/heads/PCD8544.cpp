///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief PCD8544 demo, Nokia 5110 84 x 48 module
// io layer: option 0 = command, 1 = data (D/C line), SPI mode 0, 4 MHz. The modules
// are upright with orientation_0.
//
///////////////////////////////////////////////////////////////////////////////

#include "head/PCD8544.h"
#include "scene.h"


static vic::head::PCD8544<84, 48> lcd(reinterpret_cast<vic::io::handle_type>(0));


int main()
{
  demo_banner(lcd, "PCD8544 Nokia 5110");
  lcd.init();
  demo_scene(lcd);
  lcd.display_contrast(0x70U);
  return 0;
}
