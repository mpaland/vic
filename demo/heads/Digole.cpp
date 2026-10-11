///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief Digole serial graphic adapter demo, 128 x 64 module on UART (9600 baud)
// io layer: io::write() sends the Digole command strings, option 0; I2C and SPI
// variants use the interface template parameter.
//
///////////////////////////////////////////////////////////////////////////////

#include "head/Digole.h"
#include "scene.h"


static vic::head::digole<128, 64> lcd(reinterpret_cast<vic::io::handle_type>(0), 9600U);


int main()
{
  demo_banner(lcd, "Digole 128 x 64 UART");
  lcd.init();
  demo_scene(lcd);
  return 0;
}
