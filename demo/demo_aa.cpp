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
// \brief Anti aliasing demo
// The screen is split in two halves which show exactly the same scene: on the
// left side anti aliasing is off, on the right side it is on. Both halves are
// rendered with the very same code, the only difference is one call to
// dc::anti_aliasing_enable().
// Shown are:
// - a fan of lines with all slopes (Wu lines)
// - circle, disc, rounded box, ring sectors (pie chart)
// - a rotating clock hand, a rotating triangle and a sweeping spinner
// - a translucent disc, anti aliasing and alpha blending work together
// - a 3x magnifier at the bottom which shows the pixels of the moving hand
// The background is a gradient, so the edge pixels are blended with different
// colors.
//
// Link this file together with windows/main.cpp (see demo/CMakeLists.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>

#include "dc.h"
#include "tc.h"
#include "shader/gradient.h"

// fonts
#include "font/LCD_6x8.h"
#include "font/Montserrat_12_aa2.h"


// the head is created in main.cpp
extern vic::drv& get_singlehead(void);

// output head
static vic::drv& head()
{
  return get_singlehead();
}


namespace {

// screen layout (the head is 320 x 200 pixel)
const std::int16_t  SCREEN_W   = 320;
const std::int16_t  PANEL_W    = 160;         // width of one half
const std::int16_t  PANEL_TOP  = 18;          // first row below the title
const std::int16_t  ANIM_TOP   = 82;          // animated zone (redrawn every frame)
const std::int16_t  ANIM_BOT   = 150;
const std::int16_t  MAG_TOP    = 152;         // magnifier zone (152 .. 188)
const std::int16_t  MAG_ZOOM   = 3;
const std::int16_t  MAG_W      = 40;          // magnified source area in pixel
const std::int16_t  MAG_H      = 12;
const std::uint32_t FRAME_MS   = 40U;         // 25 frames per second

// colors
const vic::color::value_type COL_LINE   = vic::color::argb(0xF0U, 0xF0U, 0xF0U);
const vic::color::value_type COL_CIRCLE = vic::color::argb(0x60U, 0xC0U, 0xFFU);
const vic::color::value_type COL_DISC   = vic::color::argb(0xFFU, 0x90U, 0x30U, 0xA0U);   // translucent
const vic::color::value_type COL_BOX    = vic::color::argb(0x80U, 0xE0U, 0x60U);
const vic::color::value_type COL_PIE0   = vic::color::argb(0xE0U, 0x40U, 0x50U);
const vic::color::value_type COL_PIE1   = vic::color::argb(0xF0U, 0xC0U, 0x30U);
const vic::color::value_type COL_PIE2   = vic::color::argb(0x40U, 0xA0U, 0xE0U);
const vic::color::value_type COL_HAND   = vic::color::argb(0xFFU, 0xE0U, 0x40U);
const vic::color::value_type COL_TRI    = vic::color::argb(0xD0U, 0x70U, 0xF0U);
const vic::color::value_type COL_SPIN   = vic::color::argb(0x40U, 0xE0U, 0xB0U);


// vertex in panel coordinates
inline vic::vertex_type P(std::int16_t xo, std::int32_t x, std::int32_t y)
{
  return { static_cast<std::int16_t>(xo + x), static_cast<std::int16_t>(y) };
}

// rect in panel coordinates
inline vic::rect_type R(std::int16_t xo, std::int32_t left, std::int32_t top, std::int32_t right, std::int32_t bottom)
{
  return { static_cast<std::int16_t>(xo + left), static_cast<std::int16_t>(top), static_cast<std::int16_t>(xo + right), static_cast<std::int16_t>(bottom) };
}


// the panel background: a vertical gradient, used for the initial background and to clear the animated zone
vic::shader::gradient_vertical<3>& background()
{
  static vic::shader::gradient_vertical<3> gradient = { { 0, PANEL_TOP,    vic::color::argb(0x18U, 0x20U, 0x38U) },
                                                        { 0, ANIM_TOP,     vic::color::argb(0x30U, 0x28U, 0x50U) },
                                                        { 0, ANIM_BOT - 1, vic::color::argb(0x60U, 0x30U, 0x40U) } };
  return gradient;
}


/**
 * Fill a rect with the background gradient
 */
void clear_background(vic::dc& _dc, const vic::rect_type& rect)
{
  _dc.shader_register(&background());
  _dc.set_color(vic::color::white);
  _dc.box(rect);
  _dc.shader_remove(&background());
}


/**
 * Render the static part of one panel
 * \param _dc Drawing context (anti aliasing on or off)
 * \param xo X origin of the panel
 */
void render_static(vic::dc& _dc, std::int16_t xo)
{
  // L I N E   F A N  -  lines of all slopes out of one point
  _dc.set_color(COL_LINE);
  for (std::int32_t i = 0; i <= 8; ++i) {
    _dc.line(P(xo, 6, ANIM_TOP - 6), P(xo, 8 + i * 9, PANEL_TOP + 4));
  }
  for (std::int32_t i = 1; i <= 4; ++i) {
    _dc.line(P(xo, 6, ANIM_TOP - 6), P(xo, 80, PANEL_TOP + 4 + i * 13));
  }

  // C I R C L E  with a translucent disc on top
  _dc.set_color(COL_CIRCLE);
  _dc.circle(P(xo, 108, 50), 22U);
  _dc.set_color(COL_DISC);
  _dc.disc(P(xo, 118, 58), 14U);

  // R O U N D E D   B O X
  _dc.set_color(COL_BOX);
  _dc.box(R(xo, 136, 24, 158, 54), 7U);
  _dc.rectangle(R(xo, 136, 58, 158, 78), 5U);
}


/**
 * Render the animated part of one panel
 * \param _dc Drawing context (anti aliasing on or off)
 * \param xo X origin of the panel
 * \param t Frame counter
 */
void render_animated(vic::dc& _dc, std::int16_t xo, std::uint32_t t)
{
  const std::int16_t hand_angle = static_cast<std::int16_t>((t * 3U) % 360U);
  const std::int16_t tri_angle  = static_cast<std::int16_t>((t * 2U) % 360U);
  const std::int16_t spin_angle = static_cast<std::int16_t>((t * 6U) % 360U);

  // C L O C K  -  ring, ticks and a rotating hand
  {
    const vic::vertex_type c = P(xo, 32, 116);
    _dc.set_color(COL_CIRCLE);
    _dc.circle(c, 28U);
    _dc.set_color(COL_LINE);
    for (std::int16_t a = 0; a < 360; a = static_cast<std::int16_t>(a + 30)) {
      _dc.line(vic::util::vertex_rotate(P(xo, 32, 116 - 24), c, a), vic::util::vertex_rotate(P(xo, 32, 116 - 27), c, a));
    }
    _dc.set_color(COL_HAND);
    _dc.line(c, vic::util::vertex_rotate(P(xo, 32, 116 - 25), c, hand_angle));
    _dc.line(c, vic::util::vertex_rotate(P(xo, 32, 116 + 6), c, hand_angle));
    _dc.disc(c, 2U);
  }

  // T R I A N G L E  -  solid, rotating around its center
  {
    const vic::vertex_type c  = P(xo, 88, 116);
    const vic::vertex_type v0 = vic::util::vertex_rotate(P(xo, 88, 116 - 24), c, tri_angle);
    const vic::vertex_type v1 = vic::util::vertex_rotate(P(xo, 88, 116 - 24), c, static_cast<std::int16_t>((tri_angle + 120) % 360));
    const vic::vertex_type v2 = vic::util::vertex_rotate(P(xo, 88, 116 - 24), c, static_cast<std::int16_t>((tri_angle + 240) % 360));
    _dc.set_color(COL_TRI);
    _dc.triangle_solid(v0, v1, v2);
  }

  // P I E   S P I N N E R  -  three ring sectors which rotate, plus a sweeping arc inside
  {
    const vic::vertex_type c = P(xo, 136, 116);
    const std::uint16_t a0 = static_cast<std::uint16_t>(spin_angle);
    _dc.set_color(COL_PIE0);
    _dc.sector(c, 14U, 24U, a0, static_cast<std::uint16_t>(a0 + 150U));
    _dc.set_color(COL_PIE1);
    _dc.sector(c, 14U, 24U, static_cast<std::uint16_t>(a0 + 150U), static_cast<std::uint16_t>(a0 + 250U));
    _dc.set_color(COL_PIE2);
    _dc.sector(c, 14U, 24U, static_cast<std::uint16_t>(a0 + 250U), static_cast<std::uint16_t>(a0 + 360U));
    _dc.set_color(COL_SPIN);
    const std::uint16_t s = static_cast<std::uint16_t>((360U - spin_angle) % 360U);
    _dc.sector(c, 6U, 10U, s, static_cast<std::uint16_t>(s + 90U));
  }
}


/**
 * Render the magnifier: the pixels around the clock center are shown 3 times enlarged
 */
void render_magnifier(vic::dc& _dc, std::int16_t xo)
{
  const std::int16_t src_x = static_cast<std::int16_t>(xo + 32 - MAG_W / 2);
  const std::int16_t src_y = static_cast<std::int16_t>(116 - 27);
  const std::int16_t dst_x = static_cast<std::int16_t>(xo + (PANEL_W - MAG_W * MAG_ZOOM) / 2);

  for (std::int16_t y = 0; y < MAG_H; ++y) {
    for (std::int16_t x = 0; x < MAG_W; ++x) {
      const vic::color::value_type c = _dc.pixel_get({ static_cast<std::int16_t>(src_x + x), static_cast<std::int16_t>(src_y + y) });
      _dc.set_color(c);
      _dc.box({ static_cast<std::int16_t>(dst_x + x * MAG_ZOOM),       static_cast<std::int16_t>(MAG_TOP + y * MAG_ZOOM),
                static_cast<std::int16_t>(dst_x + (x + 1) * MAG_ZOOM), static_cast<std::int16_t>(MAG_TOP + (y + 1) * MAG_ZOOM) });
    }
  }
}


/**
 * Render the title bar and the panel labels
 */
void render_title()
{
  vic::dc _dc(head());
  _dc.set_color(vic::color::argb(0x10U, 0x10U, 0x18U));
  _dc.box({ 0, 0, SCREEN_W, PANEL_TOP });
  _dc.box({ 0, ANIM_BOT, SCREEN_W, 200 });
  // divider
  _dc.set_color(vic::color::argb(0x80U, 0x80U, 0x90U));
  _dc.line({ PANEL_W, 0 }, { PANEL_W, 199 });

  vic::tc _tc(_dc);
  vic::font::Montserrat_12_aa2 montserrat;
  _tc.set_font(montserrat);
  _tc.set_color(vic::color::white);
  _tc.out({ 6, 3 },           (const std::uint8_t*)"anti aliasing \aFF6060off");
  _tc.out({ PANEL_W + 6, 3 }, (const std::uint8_t*)"anti aliasing \a60FF60on");

  vic::font::LCD_6x8 lcd;
  _tc.set_font(lcd);
  _tc.set_color(vic::color::argb(0xA0U, 0xA0U, 0xB0U));
  _tc.out({ 6, MAG_TOP + 2 },           (const std::uint8_t*)"3x");
  _tc.out({ PANEL_W + 6, MAG_TOP + 2 }, (const std::uint8_t*)"3x");
  _tc.out({ 6, 191 },                   (const std::uint8_t*)"aa off: hard pixel steps");
  _tc.out({ PANEL_W + 6, 191 },         (const std::uint8_t*)"aa on: coverage -> alpha");
}

} // namespace


/**
 * The demo
 */
void demo()
{
  head().init();

  // two drawing contexts on the same head, the only difference is the anti aliasing setting
  vic::dc dc_plain(head());
  vic::dc dc_aa(head());
  dc_plain.anti_aliasing_enable(false);
  dc_aa.anti_aliasing_enable(true);

  vic::dc* const    dcs[2] = { &dc_plain, &dc_aa };
  const std::int16_t xos[2] = { 0, PANEL_W };

  // static scene (both contexts are locked, so the head is presented once per frame)
  dc_plain.present_lock();
  dc_aa.present_lock();
  render_title();
  for (std::size_t p = 0U; p < 2U; ++p) {
    clear_background(*dcs[p], R(xos[p], 0, PANEL_TOP, PANEL_W, ANIM_BOT));
    render_static(*dcs[p], xos[p]);
  }
  dc_aa.present_lock(false);
  dc_plain.present_lock(false);

  // animation
  for (std::uint32_t t = 0U; ; ++t) {
    dc_plain.present_lock();
    dc_aa.present_lock();
    for (std::size_t p = 0U; p < 2U; ++p) {
      clear_background(*dcs[p], R(xos[p], 0, ANIM_TOP, PANEL_W, ANIM_BOT));
      render_animated(*dcs[p], xos[p], t);
      render_magnifier(dc_plain, xos[p]);
    }
    dc_aa.present_lock(false);
    dc_plain.present_lock(false);
    vic::io::delay(FRAME_MS);
  }
}
