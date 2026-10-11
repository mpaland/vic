///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief Framebuffer head demo: a RAM framebuffer in front of a write only display
// The SSD1331 can't read its pixels back on SPI; with the framebuffer head in front
// the context can read pixels (anti aliasing, fill) and the display gets the dirty
// rectangle with one blit per present().
//
///////////////////////////////////////////////////////////////////////////////

#include "head/SSD1331.h"
#include "head/framebuffer.h"
#include "scene.h"


static vic::head::SSD1331<96, 64, vic::drv::orientation_180>  oled(reinterpret_cast<vic::io::handle_type>(0));
static vic::head::framebuffer<96, 64>                          fb(oled);


int main()
{
  demo_banner(fb, "framebuffer in front of an SSD1331");
  fb.init();                                // also inits the display
  demo_scene(fb);
  vic::dc d(fb);
  d.anti_aliasing_enable();                 // needs pixel read back, provided by the framebuffer
  d.set_color(vic::color::green);
  d.line({ 0, 63 }, { 95, 0 });
  return 0;
}
