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
// \brief Vector font demo
// One font header, every size: a waterfall of Montserrat_vec and Michroma_vec
// from 9 to 44 pixel line height, then a word zooming smoothly from 8 to the
// largest size that fits the screen width, one pixel size per frame.
//
// Link this file together with windows/main.cpp (see demo/CMakeLists.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <cstdio>

#include "dc.h"
#include "tc.h"
#include "shader/gradient.h"

#include "font/Montserrat_vec.h"
#include "font/Michroma_vec.h"


// the head is created in main.cpp
extern vic::drv& get_singlehead(void);

// output head
static vic::drv& head()
{
  return get_singlehead();
}


namespace {

const std::int16_t  SCREEN_W  = 320;
const std::int16_t  SCREEN_H  = 200;
const std::int16_t  TEXT_TOP  = 22;
const std::int16_t  TEXT_LEFT = 8;
const std::uint32_t PAGE_MS   = 3500U;

vic::font::Montserrat_vec montserrat;
vic::font::Michroma_vec   michroma;


// title band and gradient text area
void background(vic::dc& _dc, vic::tc& _tc, const char* title)
{
  _dc.set_color(vic::color::argb(0x10U, 0x10U, 0x18U));
  _dc.box({ 0, 0, SCREEN_W, TEXT_TOP });
  vic::shader::gradient_vertical<3> gradient = { { 0, TEXT_TOP,     vic::color::argb(0x18U, 0x20U, 0x38U) },
                                                 { 0, 120,          vic::color::argb(0x30U, 0x28U, 0x50U) },
                                                 { 0, SCREEN_H - 1, vic::color::argb(0x60U, 0x30U, 0x40U) } };
  _dc.shader_register(&gradient);
  _dc.set_color(vic::color::white);
  _dc.box({ 0, TEXT_TOP, SCREEN_W, SCREEN_H });
  _dc.shader_remove(&gradient);

  _tc.set_font(montserrat, 14U);
  _tc.set_color(vic::color::white);
  _tc.out({ TEXT_LEFT, 4 }, reinterpret_cast<const std::uint8_t*>(title));
}


// one font from 9 to 44 pixel
void waterfall(vic::dc& _dc, const vic::font::font_type& font, const char* name)
{
  _dc.present_lock();
  vic::tc _tc(_dc);
  char title[64];
  std::snprintf(title, sizeof(title), "vic \aFFD040%s\aFFFFFF  one font, every size", name);
  background(_dc, _tc, title);

  static const std::uint8_t sizes[] = { 9U, 12U, 16U, 22U, 30U, 44U };
  std::int16_t y = static_cast<std::int16_t>(TEXT_TOP + 4);
  for (std::size_t n = 0U; n < sizeof(sizes); ++n) {
    char line[64];
    std::snprintf(line, sizeof(line), "\aFFD040%u\aFFFFFF The quick brown fox jumps over the lazy dog", static_cast<unsigned>(sizes[n]));
    _tc.set_font(font, sizes[n]);
    _tc.out({ TEXT_LEFT, y }, reinterpret_cast<const std::uint8_t*>(line));
    y = static_cast<std::int16_t>(y + sizes[n] + 2);
  }
  _dc.present_lock(false);
}


// a word zooming from 8 pixel to the largest size that fits and back
void zoom(vic::dc& _dc)
{
  const std::uint8_t* word = reinterpret_cast<const std::uint8_t*>("Scal\aFFD040able");
  vic::tc _tc(_dc);
  std::uint8_t max_size = 8U;
  for (std::uint8_t s = 8U; s < 160U; ++s) {
    vic::rect_type r;
    _tc.set_font(montserrat, s);
    _tc.get_extend(r, word);
    if ((r.right > SCREEN_W - 2 * TEXT_LEFT) || (s > SCREEN_H - TEXT_TOP - 8)) {
      break;
    }
    max_size = s;
  }
  for (int step = 0; step <= 2 * (max_size - 8); ++step) {
    const std::uint8_t size = static_cast<std::uint8_t>(step <= max_size - 8 ? 8 + step : 8 + 2 * (max_size - 8) - step);
    _dc.present_lock();
    char title[64];
    std::snprintf(title, sizeof(title), "vic vector font  \aFFD040%u\aFFFFFF px", static_cast<unsigned>(size));
    background(_dc, _tc, title);
    vic::rect_type r;
    _tc.set_font(montserrat, size);
    _tc.get_extend(r, word);
    _tc.set_color(vic::color::white);
    _tc.out({ static_cast<std::int16_t>((SCREEN_W - r.right) / 2), static_cast<std::int16_t>(TEXT_TOP + (SCREEN_H - TEXT_TOP - size) / 2) }, word);
    _dc.present_lock(false);
    vic::io::delay(25U);
  }
}

} // namespace


/**
 * The demo
 */
void demo()
{
  head().init();
  vic::dc _dc(head());

  for (;;) {
    waterfall(_dc, montserrat, "Montserrat_vec");
    vic::io::delay(PAGE_MS);
    waterfall(_dc, michroma, "Michroma_vec");
    vic::io::delay(PAGE_MS);
    zoom(_dc);
  }
}
