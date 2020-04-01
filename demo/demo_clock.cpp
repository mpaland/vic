///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2020, PALANDesign Hannover, Germany
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
// \brief Clock demo
//
///////////////////////////////////////////////////////////////////////////////

#include <ctime>
#include <cstdlib>

#include "dc.h"
#include "tc.h"

#include "sprite.h"
#include "shader/gradient.h"
#include "shader/brush.h"

// fonts
#include "font/LCD_6x10.h"
#include "font/LCD_8x8.h"
#include "font/Arial_num_16x24_AA4.h"

// sprite sheets
#include "sprite_sheets/bina.h"


extern vic::drv& get_singlehead(void);

// output head
vic::drv& head()
{
  return get_singlehead();
}


// clock size in pixel
#define CLOCK_SIZE    180


// get the actual time as h - m - s
void get_time(std::uint8_t& h, std::uint8_t& m, std::uint8_t& s)
{
  std::time_t t = std::time(nullptr);
  struct tm lc;
  localtime_s(&lc, &t);
  h = static_cast<std::uint8_t>(lc.tm_hour);
  m = static_cast<std::uint8_t>(lc.tm_min);
  s = static_cast<std::uint8_t>(lc.tm_sec);
}


// calculate the endpoint of the hand
inline vic::vertex_type get_hand(std::uint16_t radius, std::uint16_t degree)
{
  return {  static_cast<std::int16_t>(static_cast<std::int32_t>(radius) * vic::util::sin(degree) / 16384U),
           -static_cast<std::int16_t>(static_cast<std::int32_t>(radius) * vic::util::cos(degree) / 16384U)
         };
}


// calculate the startpoint of the text
inline vic::vertex_type get_text(const vic::rect_type& rect, std::uint16_t degree)
{
  return { static_cast<std::int16_t>((2 - rect.width()  - rect.width()  * vic::util::sin(degree) / 16384U) / 2),
           static_cast<std::int16_t>((2 - rect.height() + rect.height() * vic::util::cos(degree) / 16384U) / 2)
         };
}


void render_face(const vic::vertex_type& center)
{
  const std::int16_t radius = CLOCK_SIZE / 2U;

  vic::dc dc_(head());  // create a drawing context
  vic::tc tc_(dc_);     // create a text context

  // draw bounding
  dc_.set_color(vic::color::gray);
  dc_.circle(center, radius);

  // background fill
  vic::shader::gradient<4> gr = { { center.x, center.y - radius, vic::color::darkred },
                                  { center.x + radius, center.y, vic::color::darkgreen },
                                  { center.x, center.y + radius, vic::color::darkblue },
                                  { center.x - radius, center.y, vic::color::darkyellow }
                                };
  dc_.shader_register(&gr);
  dc_.fill(center, vic::color::gray);
  dc_.shader_remove(&gr);
  dc_.present();

  // select text font
  tc_.set_color(vic::color::gray12);
  // vic::font::LCD_6x10 _lcd_6x10;
  vic::font::LCD_8x8 _lcd_8x8;
  tc_.set_font(_lcd_8x8);

  // draw tickmarks and text
  dc_.set_color(vic::color::white);
  for (std::uint16_t n = 0U; n < 12U; ++n) {
    dc_.line(center + get_hand(radius * 5U / 6U, (360U * n / 12U)),
             center + get_hand(radius,           (360U * n / 12U))
            );

    char str[4];
    _itoa_s(n, str, 4, 10);
    vic::rect_type rect;
    tc_.get_extend(rect, (std::uint8_t*)str);
    tc_.set_pos(center + get_hand(radius * 6U / 8U, (360U * n / 12U)) + get_text(rect, (360U * n / 12U)));
    tc_.out((std::uint8_t*)str);
  }
}


void render_time(const vic::vertex_type& center, std::uint16_t radius, std::uint8_t hour, std::uint8_t min, std::uint8_t sec)
{
  // create the hand sprites
  static vic::sprite::canvas<CLOCK_SIZE * 2U, 1> h(head(), 0);   // create a sprite object with one frame, z-index 0
  static vic::sprite::canvas<CLOCK_SIZE * 2U, 1> m(head(), 1);   // create a sprite object with one frame, z-index 1
  static vic::sprite::canvas<CLOCK_SIZE * 2U, 1> s(head(), 2);   // create a sprite object with one frame, z-index 2

  // create a shared brush
  static vic::shader::brush br;
  static vic::shader::brush::shape_type shape;
  br.get_stock_shape(shape, vic::shader::brush::style_solid, 2);
  br.set_shape(shape);

  // render the hour pointer
  h.shader_register(&br);
  h.cls();                                                                            // clear the canvas
  h.set_color(vic::color::blue);                                                      // set the line drawing color
  h.line({ 0, 0 }, get_hand(radius / 2U, (360U * hour / 12U) + (30U * min / 60U)));   // draw the line
  h.render(center);                                                                   // render to head with 'center' as position
  h.shader_remove(&br);

  // render the minute pointer
  m.shader_register(&br);
  m.cls();
  m.set_color(vic::color::red);
  m.line({ 0, 0 }, get_hand(radius * 3U / 4U, (360U * min  / 60U) + ( 6U * sec / 60U)));
  m.render(center);
  m.shader_remove(&br);

  // render the seconds pointer
  s.shader_register(&br);
  s.cls();
  s.set_color(vic::color::green);
  s.line({ 0, 0 }, get_hand(radius * 20U / 21U, 360U * sec / 60U));
  s.render(center);
  s.shader_remove(&br);
}


void demo()
{
  // the init head
  head().init();

  // set face center
  const vic::vertex_type center = { static_cast<std::int16_t>(head().screen_width()  / 2U),
                                    static_cast<std::int16_t>(head().screen_height() / 2U) };

  // create the face background
  render_face(center);

  // draw the hands every 200 ms
  for(;;) {
    std::uint8_t h, m, s;
    get_time(h, m, s);
    render_time(center, CLOCK_SIZE / 2U, h, m, s);
    vic::io::delay(100U);
  }
}

