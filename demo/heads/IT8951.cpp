///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief IT8951 e-paper demo, 6" 800 x 600 panel with 16 gray levels
// io layer: every io::write() is one SPI packet (CS low period) which starts with the
// preamble word, option 0 = command packet, 1 = data packet; io::read() option 1 reads
// data words (the io layer sends the read preamble and drops the dummy word), option 2
// returns bit 0 = 1 while the controller is busy (HRDY pin low); the hardware reset is
// part of io::init().
// The head has no framebuffer: every primitive is loaded into the image buffer of the
// controller, the scene is drawn with a present lock, so the changed area is refreshed once.
//
///////////////////////////////////////////////////////////////////////////////

#include "head/IT8951.h"
#include "scene.h"


static vic::head::IT8951<800, 600> epd(reinterpret_cast<vic::io::handle_type>(0));


int main()
{
  demo_banner(epd, "IT8951 16 gray level e-paper");
  epd.init();                               // clears the panel to white (INIT waveform, about 2 s)

  // frame, a gray scale of 16 boxes and text, one GC16 refresh of the changed area at the end
  vic::dc d(epd);
  d.present_lock();
  d.set_color(vic::color::black);
  d.rectangle({ 100, 100, 700, 500 });
  for (std::int16_t i = 0; i < 16; ++i) {
    const std::uint8_t g = static_cast<std::uint8_t>(i * 17);
    d.set_color(vic::color::argb(g, g, g));
    d.box({ static_cast<std::int16_t>(144 + i * 32), 200, static_cast<std::int16_t>(176 + i * 32), 300 });
  }
  d.set_color(vic::color::black);
  vic::tc t(d);
  vic::font::LCD_6x8 lcd_6x8;
  t.set_font(lcd_6x8);
  t.set_pos({ 340, 340 });
  t.out(reinterpret_cast<const std::uint8_t*>("vic 16 gray levels"));
  d.present_lock(false);                    // refresh the changed area

  epd.shutdown();                           // sleep
  return 0;
}
