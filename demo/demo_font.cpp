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
// \brief Font demo
// Cycles through all fonts of the library, one page per font: the pangram
// "The quick brown fox jumps over the lazy dog" word wrapped to the screen
// width, digits and punctuation, the Latin-1 characters (if the font has them)
// and a line with inline color escapes. The text area has a gradient
// background, so the anti aliased glyph edges are blended against changing
// colors. 2.5 seconds per font, 12 fonts, 30 seconds per cycle.
//
// Link this file together with windows/main.cpp (see demo/CMakeLists.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <cstring>

#include "dc.h"
#include "tc.h"
#include "shader/gradient.h"

// fonts
#include "font/LCD_6x8.h"
#include "font/LCD_6x10.h"
#include "font/LCD_8x8.h"
#include "font/Arial_std_12_aa2.h"
#include "font/Montserrat_10_aa2.h"
#include "font/Montserrat_12_aa2.h"
#include "font/Montserrat_16_aa4.h"
#include "font/Montserrat_24_aa4.h"
#include "font/Michroma_10_aa2.h"
#include "font/Michroma_12_aa2.h"
#include "font/Michroma_16_aa4.h"
#include "font/Michroma_24_aa4.h"


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
const std::int16_t  TEXT_TOP  = 40;       // text area below the title, the font name and the font info
const std::int16_t  TEXT_LEFT = 8;
const std::int16_t  TEXT_W    = SCREEN_W - 2 * TEXT_LEFT;
const std::uint32_t PAGE_MS   = 2500U;

// a font page
struct page_type {
  const vic::font::font_type* font;
  const char*                 name;
  const char*                 info;
  bool                        latin1;     // font has the Latin-1 block
};

vic::font::LCD_6x8            lcd_6x8;
vic::font::LCD_6x10           lcd_6x10;
vic::font::LCD_8x8            lcd_8x8;
vic::font::Arial_std_12_aa2   arial_12;
vic::font::Montserrat_10_aa2  montserrat_10;
vic::font::Montserrat_12_aa2  montserrat_12;
vic::font::Montserrat_16_aa4  montserrat_16;
vic::font::Montserrat_24_aa4  montserrat_24;
vic::font::Michroma_10_aa2 michroma_10;
vic::font::Michroma_12_aa2 michroma_12;
vic::font::Michroma_16_aa4 michroma_16;
vic::font::Michroma_24_aa4 michroma_24;

const page_type pages[] = {
  { &lcd_6x8,        "LCD_6x8",            "monospace, 1 bpp, 0x20 - 0xFE",            false },
  { &lcd_6x10,       "LCD_6x10",           "monospace, 1 bpp, 0x20 - 0xFE",            false },
  { &lcd_8x8,        "LCD_8x8",            "monospace, 1 bpp, 0x20 - 0xFE",            false },
  { &montserrat_10,  "Montserrat_10_aa2",  "proportional, 2 bpp, ASCII + Latin-1",     true  },
  { &montserrat_12,  "Montserrat_12_aa2",  "proportional, 2 bpp, ASCII + Latin-1",     true  },
  { &montserrat_16,  "Montserrat_16_aa4",  "proportional, 4 bpp, ASCII + Latin-1",     true  },
  { &montserrat_24,  "Montserrat_24_aa4",  "proportional, 4 bpp, ASCII + Latin-1",     true  },
  { &michroma_10, "Michroma_10_aa2", "proportional, 2 bpp, ASCII + Latin-1",     true  },
  { &michroma_12, "Michroma_12_aa2", "proportional, 2 bpp, ASCII + Latin-1",     true  },
  { &michroma_16, "Michroma_16_aa4", "proportional, 4 bpp, ASCII + Latin-1",     true  },
  { &michroma_24, "Michroma_24_aa4", "proportional, 4 bpp, ASCII + Latin-1",     true  },
  { &arial_12,       "Arial_std_12_aa2",   "proportional, 2 bpp, ASCII + Latin-1",     true  },
};
const std::size_t page_count = sizeof(pages) / sizeof(pages[0]);


/**
 * Output a text word wrapped to the text width, returns the y position below the text
 * The text is split at spaces, every line is measured with get_extend()
 */
std::int16_t out_wrapped(vic::tc& _tc, std::int16_t y, const char* text)
{
  const std::uint16_t line_height = _tc.char_height();
  char line[128];
  std::size_t line_len = 0U;
  const char* p = text;
  while (*p) {
    // next word incl. the trailing space
    const char* w = p;
    while (*p && (*p != ' ')) { ++p; }
    while (*p == ' ') { ++p; }
    const std::size_t word_len = static_cast<std::size_t>(p - w);
    if (line_len + word_len >= sizeof(line)) {
      break;
    }
    // does the line still fit with the word?
    std::memcpy(&line[line_len], w, word_len);
    line[line_len + word_len] = 0;
    vic::rect_type r;
    _tc.get_extend(r, reinterpret_cast<const std::uint8_t*>(line));
    if ((r.right > TEXT_W) && (line_len > 0U)) {
      // no: output the line without the word and start a new line with it
      line[line_len] = 0;
      _tc.out({ TEXT_LEFT, y }, reinterpret_cast<const std::uint8_t*>(line));
      y = static_cast<std::int16_t>(y + line_height);
      std::memcpy(line, w, word_len);
      line_len = word_len;
      line[line_len] = 0;
    }
    else {
      line_len += word_len;
    }
  }
  if (line_len) {
    _tc.out({ TEXT_LEFT, y }, reinterpret_cast<const std::uint8_t*>(line));
    y = static_cast<std::int16_t>(y + line_height);
  }
  return y;
}


/**
 * Render one font page
 */
void render_page(vic::dc& _dc, const page_type& page, std::size_t index)
{
  _dc.present_lock();

  // background: dark title band, gradient text area
  _dc.set_color(vic::color::argb(0x10U, 0x10U, 0x18U));
  _dc.box({ 0, 0, SCREEN_W, TEXT_TOP });
  vic::shader::gradient_vertical<3> gradient = { { 0, TEXT_TOP,     vic::color::argb(0x18U, 0x20U, 0x38U) },
                                                 { 0, 120,          vic::color::argb(0x30U, 0x28U, 0x50U) },
                                                 { 0, SCREEN_H - 1, vic::color::argb(0x60U, 0x30U, 0x40U) } };
  _dc.shader_register(&gradient);
  _dc.set_color(vic::color::white);
  _dc.box({ 0, TEXT_TOP, SCREEN_W, SCREEN_H });
  _dc.shader_remove(&gradient);

  vic::tc _tc(_dc);

  // title and page counter
  _tc.set_font(montserrat_12);
  _tc.set_color(vic::color::white);
  _tc.out({ TEXT_LEFT, 3 }, reinterpret_cast<const std::uint8_t*>("vic \aFFD040font\aFFFFFF demo"));
  char counter[16];
  counter[0] = static_cast<char>('0' + (index + 1U) / 10U);
  counter[1] = static_cast<char>('0' + (index + 1U) % 10U);
  counter[2] = '/';
  counter[3] = static_cast<char>('0' + page_count / 10U);
  counter[4] = static_cast<char>('0' + page_count % 10U);
  counter[5] = 0;
  vic::rect_type r;
  _tc.get_extend(r, reinterpret_cast<const std::uint8_t*>(counter));
  _tc.set_color(vic::color::argb(0xA0U, 0xA0U, 0xB0U));
  _tc.out({ static_cast<std::int16_t>(SCREEN_W - TEXT_LEFT - r.right), 3 }, reinterpret_cast<const std::uint8_t*>(counter));

  // font name and info
  _tc.set_font(lcd_6x8);
  _tc.set_color(vic::color::argb(0xFFU, 0xD0U, 0x40U));
  _tc.out({ TEXT_LEFT, 20 }, reinterpret_cast<const std::uint8_t*>(page.name));
  _tc.set_color(vic::color::argb(0xA0U, 0xA0U, 0xB0U));
  _tc.out({ TEXT_LEFT, 30 }, reinterpret_cast<const std::uint8_t*>(page.info));

  // the samples
  _tc.set_font(*page.font);
  _tc.set_color(vic::color::white);
  std::int16_t y = static_cast<std::int16_t>(TEXT_TOP + 5);
  y = out_wrapped(_tc, y, "The quick brown fox jumps over the lazy dog");
  y = static_cast<std::int16_t>(y + 4);
  y = out_wrapped(_tc, y, "0123456789 !?%&()[]{}+-*/=<>#@");
  if (page.latin1) {
    y = static_cast<std::int16_t>(y + 4);
    y = out_wrapped(_tc, y, "\xC3\x84rger \xC3\x96l \xC3\x9C" "bung Stra\xC3\x9F" "e \xC3\xA9\xC3\xA8\xC3\xAA \xC3\xB1 \xC2\xBD \xC2\xB0" "C \xC2\xB5m");   // Ärger Öl Übung Straße éèê ñ ½ °C µm
  }
  y = static_cast<std::int16_t>(y + 4);
  if (y + _tc.char_height() <= SCREEN_H) {
    out_wrapped(_tc, y, "\aFF6060red \a60FF60green \a6080FFblue");
  }

  _dc.present_lock(false);
}

} // namespace


/**
 * The demo
 */
void demo()
{
  head().init();
  vic::dc _dc(head());

  for (std::size_t n = 0U; ; ++n) {
    render_page(_dc, pages[n % page_count], n % page_count);
    vic::io::delay(PAGE_MS);
  }
}
