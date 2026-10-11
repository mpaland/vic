///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// This file is part of the vic library.
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// \brief Small test scenes shared by the head demos
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_DEMO_HEADS_SCENE_H_
#define _VIC_DEMO_HEADS_SCENE_H_

#include <cstdio>

#include "dc.h"
#include "tc.h"
#include "font/LCD_6x8.h"


/**
 * Graphic test scene: frame, diagonal, circle, disc and a text, scaled to the head. Tiny heads (LED
 * matrices) get a checkerboard. The dc is present locked, so the head gets one present() at the end
 * (one refresh on an e-paper, one framebuffer transfer on the page organized displays).
 */
inline void demo_scene(vic::drv& head)
{
  const std::int16_t w = static_cast<std::int16_t>(head.screen_width());
  const std::int16_t h = static_cast<std::int16_t>(head.screen_height());
  const std::int16_t m = w < h ? w : h;

  vic::dc d(head);
  d.present_lock();
  d.cls();
  d.set_color(vic::color::white);
  if ((w < 8) || (h < 4)) {
    for (std::int16_t y = 0; y < h; ++y) {
      for (std::int16_t x = 0; x < w; ++x) {
        if (((x + y) & 1) == 0) {
          d.plot({ x, y });
        }
      }
    }
  }
  else {
    d.rectangle({ 0, 0, w, h });
    d.line({ 0, 0 }, { static_cast<std::int16_t>(w - 1), static_cast<std::int16_t>(h - 1) });
    d.set_color(vic::color::yellow);
    d.circle({ static_cast<std::int16_t>(w / 2), static_cast<std::int16_t>(h / 2) }, static_cast<std::uint16_t>(m / 3));
    d.set_color(vic::color::red);
    d.disc({ static_cast<std::int16_t>(w / 2), static_cast<std::int16_t>(h / 2) }, static_cast<std::uint16_t>(m / 8));
    if ((w >= 32) && (h >= 16)) {
      vic::tc t(d);
      vic::font::LCD_6x8 lcd_6x8;
      t.set_font(lcd_6x8);
      t.set_pos({ 2, 2 });
      t.out(reinterpret_cast<const std::uint8_t*>("vic"));
    }
  }
  d.present_lock(false);
}


/**
 * Alphanumeric test scene: two lines of text
 */
inline void demo_text(vic::drv& head)
{
  vic::tc t(head);
  t.cls();
  t.out({ 0, 0 }, reinterpret_cast<const std::uint8_t*>("vic head demo"));
  if (head.screen_height() > 1U) {
    t.out({ 0, 1 }, reinterpret_cast<const std::uint8_t*>("Hello World"));
  }
}


/**
 * Print the head name and size
 */
inline void demo_banner(vic::drv& head, const char* module)
{
  std::printf("%s\n%s, %u x %u %s\n", head.version(), module,
              static_cast<unsigned>(head.screen_width()), static_cast<unsigned>(head.screen_height()),
              head.is_graphic() ? "pixel" : "characters");
}

#endif  // _VIC_DEMO_HEADS_SCENE_H_
