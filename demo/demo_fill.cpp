///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2025-2026, PALANDesign Hannover, Germany
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
// \brief Flood fill demo
// dc::fill() in slow motion: the drawing context presents the head every few
// spans, so the scanline flood fill can be watched while it works. fill() draws
// the region as horizontal spans with line_horz(), the status bar shows the
// span and pixel count of every fill.
// Every round
// - replaces the background around the shapes (background mode: all connected
//   pixels of the background color are filled)
// - fills the 7 regions of a Venn diagram one after the other (bounded mode:
//   the fill stops at the outline color)
// - fills a square spiral, one fill runs through the whole corridor
// The next round refills everything with the next colors.
// Everything is drawn on a RGB565 framebuffer in front of the head, which
// stores the colors with less bits (fill() compares them as stored).
//
// Link this file together with windows/main.cpp (see demo/CMakeLists.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <cstdio>

#include "dc.h"
#include "tc.h"
#include "head/framebuffer.h"

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
const std::int16_t  SCREEN_H   = 200;
const std::int16_t  TITLE_H    = 16;          // title bar
const std::int16_t  STATUS_TOP = 188;         // status bar
const std::uint32_t FRAME_MS   = 15U;         // delay after every slow motion frame
const std::uint32_t HOLD_MS    = 1500U;       // delay at the end of a round

// colors
const vic::color::value_type COL_BAR     = vic::color::argb(0x10U, 0x10U, 0x18U);
const vic::color::value_type COL_TITLE   = vic::color::white;
const vic::color::value_type COL_STATUS  = vic::color::argb(0xA0U, 0xA0U, 0xB0U);
const vic::color::value_type COL_OUTLINE = vic::color::argb(0xF0U, 0xF0U, 0xF0U);
const vic::color::value_type COL_OUTSIDE[2] = { vic::color::argb(0x1DU, 0x2BU, 0x53U), vic::color::argb(0x2BU, 0x1DU, 0x3FU) };
const vic::color::value_type COL_FILL[8] = {
  vic::color::argb(0xE6U, 0x39U, 0x46U), vic::color::argb(0xF4U, 0xA2U, 0x61U), vic::color::argb(0xE9U, 0xC4U, 0x6AU), vic::color::argb(0x2AU, 0x9DU, 0x8FU),
  vic::color::argb(0x52U, 0xB7U, 0x88U), vic::color::argb(0x4EU, 0xA8U, 0xDEU), vic::color::argb(0x9DU, 0x4EU, 0xDDU), vic::color::argb(0xFFU, 0x70U, 0xA6U)
};

// Venn diagram: three circles, a start vertex in each of the 7 regions
const std::uint16_t     VENN_R          = 34U;
const vic::vertex_type  VENN_CENTER[3]  = { { 56, 76 }, { 104, 76 }, { 80, 118 } };
const vic::vertex_type  VENN_SEED[7]    = { { 34, 70 }, { 126, 70 }, { 80, 140 },     // A, B, C only
                                            { 80, 62 }, { 62, 104 }, { 98, 104 },     // A and B, A and C, B and C
                                            { 80, 90 } };                             // all three
const char* const       VENN_NAME[7]    = { "A", "B", "C", "A and B", "A and C", "B and C", "A, B and C" };

// square spiral: outer frame and corridor width
const std::int16_t      SPIRAL_LEFT     = 176;
const std::int16_t      SPIRAL_TOP      = 38;
const std::int16_t      SPIRAL_RIGHT    = 304;
const std::int16_t      SPIRAL_BOTTOM   = 166;
const std::int16_t      SPIRAL_D        = 10;
const vic::vertex_type  SPIRAL_SEED     = { 240, 102 };   // in the center pocket

// start vertex of the background fill
const vic::vertex_type  OUTSIDE_SEED    = { 2, TITLE_H + 2 };


/**
 * Drawing context which shows the progress of fill() in slow motion
 * fill() draws its spans with line_horz(), every few spans the head is presented
 * and the demo waits a frame.
 */
class slowmo_dc : public vic::dc
{
public:
  slowmo_dc(vic::drv& _head)
    : dc(_head)
    , spans_per_frame_(0U)
    , spans_(0U)
    , pixels_(0U)
  { }

  /**
   * Set the slow motion speed and reset the counters
   * \param spans_per_frame Spans drawn per frame, 0 to draw at full speed
   */
  void slowmo(std::uint32_t spans_per_frame)
  {
    spans_per_frame_ = spans_per_frame;
    spans_  = 0U;
    pixels_ = 0U;
  }

  std::uint32_t spans() const  { return spans_; }
  std::uint32_t pixels() const { return pixels_; }

  virtual void line_horz(vic::vertex_type v0, vic::vertex_type v1) override
  {
    dc::line_horz(v0, v1);
    ++spans_;
    pixels_ += static_cast<std::uint32_t>(v1.x > v0.x ? v1.x - v0.x + 1 : v0.x - v1.x + 1);
    if ((spans_per_frame_ != 0U) && ((spans_ % spans_per_frame_) == 0U)) {
      // release the present() lock of fill() for a moment, this presents the head
      present_lock(false);
      present_lock();
      vic::io::delay(FRAME_MS);
    }
  }

private:
  std::uint32_t spans_per_frame_;
  std::uint32_t spans_;
  std::uint32_t pixels_;
};


/**
 * Show a text in the status bar
 */
void status(slowmo_dc& _dc, vic::tc& _tc, const char* text)
{
  _dc.set_color(COL_BAR);
  _dc.box({ 0, STATUS_TOP, SCREEN_W, SCREEN_H });
  _tc.set_color(COL_STATUS);
  _tc.out({ 4, static_cast<std::int16_t>(STATUS_TOP + 2) }, reinterpret_cast<const std::uint8_t*>(text));
}


/**
 * Fill in slow motion and show the result in the status bar
 */
void slow_fill(slowmo_dc& _dc, vic::tc& _tc, const char* name, vic::vertex_type start, vic::color::value_type color, bool use_color_as_bounding,
               vic::color::value_type pen, std::uint32_t spans_per_frame)
{
  char text[64];
  std::snprintf(text, sizeof(text), "%s ...", name);
  status(_dc, _tc, text);

  _dc.set_color(pen);
  _dc.slowmo(spans_per_frame);
  _dc.fill(start, color, use_color_as_bounding);
  const unsigned int spans = static_cast<unsigned int>(_dc.spans()), pixels = static_cast<unsigned int>(_dc.pixels());
  _dc.slowmo(0U);

  std::snprintf(text, sizeof(text), "%s: %u spans, %u pixels", name, spans, pixels);
  status(_dc, _tc, text);
}


/**
 * Render the title, the background and the outlines
 */
void render_scene(slowmo_dc& _dc, vic::tc& _tc, vic::font::Montserrat_12_aa2& montserrat, vic::font::LCD_6x8& lcd)
{
  _dc.cls(COL_OUTSIDE[0]);

  // title bar
  _dc.set_color(COL_BAR);
  _dc.box({ 0, 0, SCREEN_W, TITLE_H });
  _tc.set_font(montserrat);
  _tc.set_color(COL_TITLE);
  _tc.out({ 6, 1 }, reinterpret_cast<const std::uint8_t*>("flood fill"));
  _tc.set_font(lcd);
  _tc.set_color(COL_STATUS);
  _tc.out({ SCREEN_W - 6 - 18 * 6, 5 }, reinterpret_cast<const std::uint8_t*>("RGB565 framebuffer"));

  // Venn diagram
  _dc.set_color(COL_OUTLINE);
  for (std::size_t i = 0U; i < 3U; ++i) {
    _dc.circle(VENN_CENTER[i], VENN_R);
  }

  // square spiral: a closed frame and a wall which winds inwards, the corridor between is one region
  _dc.rectangle({ SPIRAL_LEFT, SPIRAL_TOP, SPIRAL_RIGHT + 1, SPIRAL_BOTTOM + 1 });
  std::int16_t l = SPIRAL_LEFT, t = SPIRAL_TOP + SPIRAL_D, r = SPIRAL_RIGHT - SPIRAL_D, b = SPIRAL_BOTTOM - SPIRAL_D;
  while ((r - l > SPIRAL_D) && (b - t > SPIRAL_D)) {
    const std::int16_t l1 = static_cast<std::int16_t>(l + SPIRAL_D);
    _dc.line({ l, t },  { r, t });
    _dc.line({ r, t },  { r, b });
    _dc.line({ r, b },  { l1, b });
    _dc.line({ l1, b }, { l1, static_cast<std::int16_t>(t + SPIRAL_D) });
    l = l1;
    t = static_cast<std::int16_t>(t + SPIRAL_D);
    r = static_cast<std::int16_t>(r - SPIRAL_D);
    b = static_cast<std::int16_t>(b - SPIRAL_D);
  }
}


/**
 * One round: background, Venn regions, spiral
 * \param round Round number, selects the colors
 */
void render_round(slowmo_dc& _dc, vic::tc& _tc, std::uint32_t round)
{
  // background mode: replace all connected pixels of the actual background color
  slow_fill(_dc, _tc, "background", OUTSIDE_SEED, COL_OUTSIDE[round % 2U], false, COL_OUTSIDE[(round + 1U) % 2U], 6U);

  // bounded mode: fill up to the outline
  for (std::size_t i = 0U; i < 7U; ++i) {
    char name[32];
    std::snprintf(name, sizeof(name), "Venn %s", VENN_NAME[i]);
    slow_fill(_dc, _tc, name, VENN_SEED[i], COL_OUTLINE, true, COL_FILL[(i + round) % 8U], 2U);
  }

  slow_fill(_dc, _tc, "spiral", SPIRAL_SEED, COL_OUTLINE, true, COL_FILL[(7U + round) % 8U], 6U);
}

} // namespace


/**
 * The demo
 */
void demo()
{
  head().init();

  // all drawing goes to a RGB565 framebuffer, present() transfers the changed rect to the head
  static vic::head::framebuffer<SCREEN_W, SCREEN_H, 1U, vic::color::format_RGB565> fb(head());
  slowmo_dc _dc(fb);
  vic::tc   _tc(_dc);
  vic::font::Montserrat_12_aa2 montserrat;
  vic::font::LCD_6x8           lcd;

  _dc.present_lock();
  render_scene(_dc, _tc, montserrat, lcd);
  _dc.present_lock(false);

  for (std::uint32_t round = 0U; ; ++round) {
    render_round(_dc, _tc, round);
    vic::io::delay(HOLD_MS);
  }
}
