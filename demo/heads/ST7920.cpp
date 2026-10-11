///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief ST7920 demo, 128 x 64 graphic LCD module (12864B) on the serial interface
// io layer: option 0 = instruction (RS = 0), 1 = data (RS = 1); on the serial bus
// (PSB low) every byte is sent as the sync byte F8h / FAh followed by the high and
// the low nibble in the upper four bits of two bytes. Every byte needs 72 us, the io
// layer has to pace the transfers. The modules are upright with orientation_0.
//
///////////////////////////////////////////////////////////////////////////////

#include "head/ST7920.h"
#include "scene.h"


static vic::head::ST7920<128, 64> lcd(reinterpret_cast<vic::io::handle_type>(0));


int main()
{
  demo_banner(lcd, "ST7920 128 x 64 serial");
  lcd.init();
  demo_scene(lcd);
  return 0;
}
