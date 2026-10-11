///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief SSD1680 e-paper demo, 2.13" 250 x 122 black/white module in landscape
// io layer: option 0 = command, 1 = data (D/C line), option 2 on io::read() returns
// the BUSY pin (bit 0); the hardware reset is part of io::init(). The scene is
// drawn with a present lock, so the panel refreshes once (2 .. 4 s on hardware).
// Drawing colors: dark = ink, light / color::none = paper, red = red plane (tri color).
//
///////////////////////////////////////////////////////////////////////////////

#include "head/SSD1680.h"
#include "scene.h"


static vic::head::SSD1680<250, 122, vic::drv::orientation_90> epd(reinterpret_cast<vic::io::handle_type>(0));


int main()
{
  demo_banner(epd, "SSD1680 2.13\" e-paper, landscape");
  epd.init();

  // black frame and text on white paper, one refresh at the end
  vic::dc d(epd);
  d.present_lock();
  d.cls();                                  // white
  d.set_color(vic::color::black);
  d.rectangle({ 0, 0, 250, 122 });
  d.rectangle({ 4, 4, 246, 118 });
  d.disc({ 60, 61 }, 40U);
  vic::tc t(d);
  vic::font::LCD_6x8 lcd_6x8;
  t.set_font(lcd_6x8);
  t.set_pos({ 120, 50 });
  t.out(reinterpret_cast<const std::uint8_t*>("vic e-paper"));
  d.present_lock(false);                    // upload and refresh

  epd.shutdown();                           // deep sleep
  return 0;
}
