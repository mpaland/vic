///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief HD44780 demo, 16 x 2 character LCD on a 4 bit bus (or an I2C PCF8574 backpack)
// io layer: option 0 = instruction (RS = 0), 1 = data (RS = 1), 2 = instruction high
// nibble only (the four single nibble transfers of the 4 bit init). The io layer
// drives E, R/W and RS and splits the bytes into nibbles. With HD44780_WAIT_DELAY
// the head waits 1 ms per byte, HD44780_WAIT_BUSY polls the busy flag through
// io::read() option 0, HD44780_WAIT_IO leaves the 37 us to the io layer.
//
///////////////////////////////////////////////////////////////////////////////

#include "head/HD44780.h"
#include "scene.h"


static vic::head::HD44780<16, 2, vic::head::HD44780_BUS4, vic::head::HD44780_WAIT_DELAY> lcd(reinterpret_cast<vic::io::handle_type>(0));
// 20 x 4 module on an 8 bit bus with busy flag polling:
//   vic::head::HD44780<20, 4, vic::head::HD44780_BUS8, vic::head::HD44780_WAIT_BUSY> lcd_20x4;


int main()
{
  demo_banner(lcd, "HD44780 16 x 2, 4 bit bus");
  lcd.init();

  // a user defined character (code 0) and the text scene
  static const std::uint8_t heart[8] = { 0x00U, 0x0AU, 0x1FU, 0x1FU, 0x0EU, 0x04U, 0x00U, 0x00U };
  lcd.char_define(0U, heart);
  demo_text(lcd);
  vic::tc t(lcd);
  t.set_pos({ 12, 1 });
  t.out(static_cast<std::uint16_t>(0U));    // the heart
  lcd.cursor_set(true, true);
  return 0;
}
