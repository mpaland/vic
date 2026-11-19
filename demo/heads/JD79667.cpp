///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief JD79667 e-paper demo, 200 x 384 black/white/yellow/red panel in landscape
// io layer: option 0 = command, 1 = data (D/C line), option 2 on io::read() returns
// bit 0 = 1 while the controller is busy (BUSY_N pin low); the hardware reset is
// part of io::init(). The scene is drawn with a present lock, so the panel refreshes
// once (12 .. 25 s on hardware).
// Drawing colors: red and yellow = pigment, other dark colors = black, light / color::none = paper.
//
///////////////////////////////////////////////////////////////////////////////

#include "head/JD79667.h"
#include "scene.h"


static vic::head::JD79667<384, 200, vic::drv::orientation_90> epd(reinterpret_cast<vic::io::handle_type>(0));


int main()
{
  demo_banner(epd, "JD79667 four color e-paper, landscape");
  epd.init();

  // black frame, three discs and text on white paper, one refresh at the end
  vic::dc d(epd);
  d.present_lock();
  d.cls();                                  // white
  d.set_color(vic::color::black);
  d.rectangle({ 0, 0, 384, 200 });
  d.rectangle({ 4, 4, 380, 196 });
  d.disc({ 70, 80 }, 40U);
  d.set_color(vic::color::red);
  d.disc({ 190, 80 }, 40U);
  d.set_color(vic::color::yellow);
  d.disc({ 310, 80 }, 40U);
  d.set_color(vic::color::black);
  vic::tc t(d);
  vic::font::LCD_6x8 lcd_6x8;
  t.set_font(lcd_6x8);
  t.set_pos({ 130, 160 });
  t.out(reinterpret_cast<const std::uint8_t*>("vic four color e-paper"));
  d.present_lock(false);                    // upload and refresh

  epd.shutdown();                           // power off and deep sleep
  return 0;
}
