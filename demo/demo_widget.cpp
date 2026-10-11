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
// \brief Widget demo: every widget with its specific features on one screen
//
// Six group boxes, one per widget:
// - progress: all four orientations, with and without border, bouncing with step_it(),
//   the top bar changes its color with set_color()
// - bitmap:   an XPM3 icon, a monochrome L1 pattern with a cycling color, a spinning coin
//   out of a RGB565 sprite sheet rendered as raw bitmap rows and an ARGB8888 alpha ramp
// - label:    left / center / right alignment, border and background, a fixed point value
//   readout without floating point and inline color escapes
// - gauge:    a 270 degree gauge with bands, ticks, hub, background disc and anti aliasing
//   next to a plain 180 degree gauge without anti aliasing
// - bargauge: all four orientations with bands, ticks, fill mode and one or two markers
// - groupbox: the frames around all of them
// The second page (the pages alternate every 15 s) shows:
// - arc:      a full ring and a 270 degree arc filling with the value, anti aliased
// - led:      round and square LEDs, running light and bit patterns
// - spinner:  three rotating ring segments with different sweep, speed and direction
// - textbox:  word wrapped multi line text with a color escape, scrolled line by line
// - input:    the event routing: a simulated touch cursor taps on LED "buttons" and on empty
//             space, base::event() routes the event to the widget and its handler reacts
// The third page shows the input widgets and the compass:
// - compass:  a heading up compass rose with a turning heading
// - imagebutton: soft keys with icon and caption, the tapped one is the active one
// - checkbox, switch, button, slider, spinbox operated by the simulated touch cursor
//
// Link this file together with windows/main.cpp (see demo/CMakeLists.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>

#include "dc.h"
#include "widget/progress.h"
#include "widget/bitmap.h"
#include "widget/label.h"
#include "widget/groupbox.h"
#include "widget/gauge.h"
#include "widget/bargauge.h"
#include "widget/arc.h"
#include "widget/led.h"
#include "widget/spinner.h"
#include "widget/textbox.h"
#include "widget/button.h"
#include "widget/checkbox.h"
#include "widget/switch.h"
#include "widget/slider.h"
#include "widget/spinbox.h"
#include "widget/imagebutton.h"
#include "widget/compass.h"
#include "sprite.h"

// fonts
#include "font/LCD_6x10.h"
#include "font/Montserrat_16_aa4.h"

// the coin sprite sheet is used as raw RGB565 bitmap
#include "sprite_sheets/coin.h"


// the head is created in main.cpp
extern vic::drv& get_singlehead(void);

// output head
static vic::drv& head()
{
  return get_singlehead();
}


namespace {

const std::uint32_t FRAME_MS = 40U;     // 25 frames per second

// colors
const vic::color::value_type COL_BG     = vic::color::black;
const vic::color::value_type COL_RING   = vic::color::gray04;
const vic::color::value_type COL_TEXT   = vic::color::white;
const vic::color::value_type COL_NAME   = vic::color::white;
const vic::color::value_type COL_FRAME  = vic::color::gray08;
const vic::color::value_type COL_GREEN  = vic::color::argb(0x20U, 0xD0U, 0x20U);
const vic::color::value_type COL_YELLOW = vic::color::argb(0xF0U, 0xD0U, 0x20U);
const vic::color::value_type COL_RED    = vic::color::argb(0xF0U, 0x20U, 0x20U);
const vic::color::value_type COL_CYAN   = vic::color::argb(0x40U, 0xE0U, 0xFFU);
const vic::color::value_type COL_ORANGE = vic::color::argb(0xFFU, 0x90U, 0x20U);
const vic::color::value_type COL_BLUE   = vic::color::argb(0x40U, 0x80U, 0xFFU);

// fonts
vic::font::LCD_6x10          font_lcd;    // 1 bpp, crisp captions
vic::font::Montserrat_16_aa4 font_16;     // 4 bpp readouts

typedef vic::widget::gauge<>    gauge_type;
typedef vic::widget::bargauge<> bar_type;
typedef vic::widget::label      label_type;
typedef vic::widget::groupbox   group_type;
typedef vic::widget::progress   progress_type;


// B A N D S   A N D   M A R K E R S
const vic::widget::band_type bands_gauge[] = { { 10, 70, COL_GREEN }, { 70, 85, COL_YELLOW }, { 85, 100, COL_RED } };
const vic::widget::band_type bands_v[]     = { {  0, 15, COL_RED }, { 15, 30, COL_YELLOW }, { 30, 100, COL_GREEN } };
const vic::widget::band_type bands_h[]     = { {  0, 60, COL_GREEN }, { 60, 80, COL_YELLOW }, { 80, 100, COL_RED } };
const vic::widget::band_type bands_fill[]  = { { 90, 100, COL_RED } };

const bar_type::marker_type marker_dual[]  = { { COL_TEXT, bar_type::side_before, 4U }, { COL_CYAN, bar_type::side_after, 4U } };
const bar_type::marker_type marker_after[] = { { COL_TEXT, bar_type::side_after,  4U } };
const bar_type::marker_type marker_before[]= { { COL_TEXT, bar_type::side_before, 4U } };


// B I T M A P S

// XPM3 icon, 16 x 16, 5 colors, transparent corners
const char* xpm_icon[] = {
  "16 16 5 1",
  "  c None",
  ". c #F0D020",
  "o c #A08010",
  "x c #202020",
  "r c #F02020",
  "     oooooo     ",
  "   oo......oo   ",
  "  o..........o  ",
  " o............o ",
  " o...xx..xx...o ",
  "o....xx..xx....o",
  "o..............o",
  "o..............o",
  "o..r........r..o",
  "o..rr......rr..o",
  " o..rrrrrrrr..o ",
  " o....rrrr....o ",
  "  o..........o  ",
  "   oo......oo   ",
  "     oooooo     ",
  "                "
};

// L1 pattern, 16 x 16, as ASCII art, converted to a LSB first bitmap at startup
const char* l1_art[16] = {
  "................",
  "..XXXX....XXXX..",
  ".XXXXXX..XXXXXX.",
  "XXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXX",
  ".XXXXXXXXXXXXXX.",
  "..XXXXXXXXXXXX..",
  "...XXXXXXXXXX...",
  "....XXXXXXXX....",
  ".....XXXXXX.....",
  "......XXXX......",
  ".......XX.......",
  "................",
  "................"
};
std::uint8_t l1_data[32];

// ARGB8888 alpha ramp, 48 x 12, yellow with alpha 0 .. 255 from left to right (B G R A)
const std::uint16_t ARGB_W = 48U;
const std::uint16_t ARGB_H = 12U;
std::uint8_t argb_data[ARGB_W * ARGB_H * 4U];

void build_bitmaps()
{
  for (std::size_t y = 0U; y < 16U; ++y) {
    std::uint16_t row = 0U;
    for (std::size_t x = 0U; x < 16U; ++x) {
      if (l1_art[y][x] == 'X') {
        row = static_cast<std::uint16_t>(row | (1U << x));
      }
    }
    l1_data[y * 2U]      = static_cast<std::uint8_t>(row & 0xFFU);
    l1_data[y * 2U + 1U] = static_cast<std::uint8_t>(row >> 8U);
  }
  for (std::size_t y = 0U; y < ARGB_H; ++y) {
    for (std::size_t x = 0U; x < ARGB_W; ++x) {
      std::uint8_t* p = &argb_data[(y * ARGB_W + x) * 4U];
      p[0] = 0x00U;                                                   // B
      p[1] = 0xD0U;                                                   // G
      p[2] = 0xF0U;                                                   // R
      p[3] = static_cast<std::uint8_t>(x * 255U / (ARGB_W - 1U));     // A
    }
  }
}


// C O N F I G   H E L P E R S
progress_type::config_type progress_cfg(std::int16_t x, std::int16_t y, std::uint16_t w, std::uint16_t h, progress_type::orientation_type o,
                                        vic::color::value_type bar, vic::color::value_type border)
{
  const progress_type::config_type cfg = { { x, y }, w, h, o, 0, 100, bar, vic::color::gray03, border };
  return cfg;
}

label_type::config_type label_cfg(std::int16_t x, std::int16_t y, std::uint16_t w, std::uint16_t h, const vic::font::font_type& font,
                                  label_type::align_type align, vic::color::value_type color,
                                  vic::color::value_type bg = COL_BG, vic::color::value_type border = vic::color::none)
{
  const label_type::config_type cfg = { { x, y }, w, h, &font, align, color, bg, border };
  return cfg;
}

group_type::config_type group_cfg(std::int16_t left, std::int16_t top, std::int16_t right, std::int16_t bottom, const char* title)
{
  const group_type::config_type cfg = { { left, top, right, bottom }, 4U, &font_lcd, title, COL_FRAME, COL_TEXT, COL_BG };
  return cfg;
}


/**
 * Slow sine wave between lo and hi
 * \param t Frame counter
 * \param period Period in frames
 * \param phase Phase in degree
 */
std::int16_t wave(std::uint32_t t, std::uint32_t period, std::int16_t phase, std::int16_t lo, std::int16_t hi)
{
  const std::int16_t a = static_cast<std::int16_t>((t * 360U / period + static_cast<std::uint32_t>(phase)) % 360U);
  const std::int32_t s = vic::util::sin(a) + 16384;   // 0 .. 32768
  return static_cast<std::int16_t>(lo + (static_cast<std::int32_t>(hi - lo) * s) / 32768);
}


/**
 * Advance a progress bar and bounce at the range ends
 */
void bounce(progress_type& p, std::int16_t& step)
{
  p.step_it();
  if ((p.get_pos() >= 100) || (p.get_pos() <= 0)) {
    step = static_cast<std::int16_t>(-step);
    p.set_step(step);
  }
}

} // namespace


/**
 * Page 1: progress, bitmap, label, gauge and bargauge
 * \param frames Frames to show the page
 */
void page1(std::uint32_t frames)
{
  vic::dc _dc(head());

  // G R O U P   B O X E S
  group_type grp_progress(head(), group_cfg(  2,   8, 108,  92, "progress"));
  group_type grp_bitmap  (head(), group_cfg(112,   8, 208,  92, "bitmap"));
  group_type grp_label   (head(), group_cfg(212,   8, 318,  92, "label"));
  group_type grp_gauge   (head(), group_cfg(  2, 100, 160, 196, "gauge"));
  group_type grp_bar     (head(), group_cfg(164, 100, 318, 196, "bargauge"));

  // P R O G R E S S  -  four orientations
  progress_type p_ltr(head(), progress_cfg(10, 16, 90U,  8U, progress_type::left_to_right, COL_GREEN,  COL_TEXT));
  progress_type p_rtl(head(), progress_cfg(10, 28, 90U,  8U, progress_type::right_to_left, COL_ORANGE, vic::color::none));
  progress_type p_btt(head(), progress_cfg(10, 40,  8U, 44U, progress_type::bottom_to_top, COL_CYAN,   COL_TEXT));
  progress_type p_ttb(head(), progress_cfg(22, 40,  8U, 44U, progress_type::top_to_bottom, COL_BLUE,   vic::color::none));
  label_type    p_val(head(), label_cfg(36, 52, 64U, 18U, font_16, label_type::align_center, COL_TEXT));
  label_type    p_txt(head(), label_cfg(36, 72, 64U, 12U, font_lcd, label_type::align_center, COL_NAME));

  // B I T M A P  -  XPM, L1, RGB565 and ARGB8888
  vic::widget::bitmap bmp_xpm (head());
  vic::widget::bitmap bmp_l1  (head());
  vic::widget::bitmap bmp_coin(head());
  vic::widget::bitmap bmp_argb(head());
  // static captions leave the background untouched (color::none), so they may overlap the group box frame
  label_type bmp_txt1(head(), label_cfg(108, 38, 36U, 12U, font_lcd, label_type::align_center, COL_NAME, vic::color::none));
  label_type bmp_txt2(head(), label_cfg(138, 38, 36U, 12U, font_lcd, label_type::align_center, COL_NAME, vic::color::none));
  label_type bmp_txt3(head(), label_cfg(168, 38, 36U, 12U, font_lcd, label_type::align_center, COL_NAME, vic::color::none));
  label_type bmp_txt4(head(), label_cfg(120, 70, 80U, 12U, font_lcd, label_type::align_center, COL_NAME, vic::color::none));

  // L A B E L  -  alignment, border, value, color escapes
  label_type l_left  (head(), label_cfg(220, 14, 90U, 13U, font_lcd, label_type::align_left,   COL_TEXT, COL_BG, COL_FRAME));
  label_type l_center(head(), label_cfg(220, 29, 90U, 13U, font_lcd, label_type::align_center, COL_TEXT, vic::color::gray03));
  label_type l_right (head(), label_cfg(220, 44, 90U, 13U, font_lcd, label_type::align_right,  COL_TEXT));
  label_type l_value (head(), label_cfg(220, 59, 90U, 16U, font_16, label_type::align_center, COL_YELLOW));
  label_type l_color (head(), label_cfg(220, 77, 90U, 13U, font_lcd, label_type::align_center, COL_TEXT));

  // G A U G E  -  full featured with anti aliasing and a plain one without
  const gauge_type::config_type g_full_cfg  = { { 44, 148 }, 36U, 7U, -135, 135, 0, 100, bands_gauge, 3U, 7U, 24U, 5U,
                                                COL_RING, COL_TEXT, COL_TEXT, vic::color::gray12, vic::color::gray02 };
  const gauge_type::config_type g_plain_cfg = { { 116, 150 }, 30U, 5U, -90, 90, 0, 100, nullptr, 0U, 5U, 22U, 3U,
                                                COL_BLUE, COL_TEXT, COL_YELLOW, vic::color::none, vic::color::none };
  gauge_type g_full (head(), g_full_cfg);
  gauge_type g_plain(head(), g_plain_cfg);
  g_full.anti_aliasing_enable();
  label_type g_full_val (head(), label_cfg(24, 164, 40U, 16U, font_16, label_type::align_center, COL_TEXT, vic::color::gray02));   // same bg as the gauge disc
  label_type g_full_txt (head(), label_cfg(14, 182, 60U, 12U, font_lcd, label_type::align_center, COL_NAME));
  label_type g_plain_val(head(), label_cfg(96, 156, 40U, 16U, font_16, label_type::align_center, COL_TEXT));
  label_type g_plain_txt(head(), label_cfg(86, 182, 60U, 12U, font_lcd, label_type::align_center, COL_NAME));

  // B A R G A U G E  -  four orientations
  const bar_type::config_type b_btt_cfg = { { 172, 108 },  14U, 80U, bar_type::bottom_to_top, 6U, 0, 100, bands_v, 3U, 0U,
                                            marker_dual, 2U, vic::color::none, COL_RING, COL_BG, COL_BG };
  const bar_type::config_type b_ttb_cfg = { { 192, 108 },  14U, 80U, bar_type::top_to_bottom, 6U, 0, 100, bands_fill, 1U, 5U,
                                            marker_after, 1U, COL_GREEN, COL_RING, COL_BG, COL_BG };
  const bar_type::config_type b_ltr_cfg = { { 214, 110 },  96U, 14U, bar_type::left_to_right, 5U, 0, 100, bands_h, 3U, 6U,
                                            marker_before, 1U, vic::color::none, COL_RING, COL_BG, COL_BG };
  const bar_type::config_type b_rtl_cfg = { { 214, 130 },  96U, 14U, bar_type::right_to_left, 5U, 0, 100, bands_fill, 1U, 0U,
                                            marker_after, 1U, COL_CYAN, COL_RING, COL_BG, COL_BG };
  bar_type b_btt(head(), b_btt_cfg);
  bar_type b_ttb(head(), b_ttb_cfg);
  bar_type b_ltr(head(), b_ltr_cfg);
  bar_type b_rtl(head(), b_rtl_cfg);
  label_type b_val1(head(), label_cfg(214, 150, 48U, 16U, font_16, label_type::align_center, COL_TEXT));
  label_type b_val2(head(), label_cfg(262, 150, 48U, 16U, font_16, label_type::align_center, COL_CYAN));
  label_type b_txt (head(), label_cfg(214, 168, 96U, 12U, font_lcd, label_type::align_center, COL_NAME));

  // I N I T  -  group boxes first, then the widgets inside
  grp_progress.init(); grp_bitmap.init(); grp_label.init(); grp_gauge.init(); grp_bar.init();
  p_ltr.init(); p_rtl.init(); p_btt.init(); p_ttb.init(); p_val.init(); p_txt.init();
  bmp_txt1.init(); bmp_txt2.init(); bmp_txt3.init(); bmp_txt4.init();
  l_left.init(); l_center.init(); l_right.init(); l_value.init(); l_color.init();
  g_full.init(); g_plain.init(); g_full_val.init(); g_full_txt.init(); g_plain_val.init(); g_plain_txt.init();
  b_btt.init(); b_ttb.init(); b_ltr.init(); b_rtl.init(); b_val1.init(); b_val2.init(); b_txt.init();

  // static bitmaps: the XPM icon and the ARGB8888 ramp over a blue box (shows the alpha blending)
  bmp_xpm.render_xpm({ 118, 20 }, xpm_icon);
  _dc.set_color(vic::color::navy);
  _dc.box({ 136, 54, 184, 66 });
  bmp_argb.render_raw({ 136, 54 }, ARGB_W, ARGB_H, vic::color::format_ARGB8888, argb_data);

  // static texts
  p_txt.set_text("step_it()");
  bmp_txt1.set_text("XPM");
  bmp_txt2.set_text("L1");
  bmp_txt3.set_text("RGB565");
  bmp_txt4.set_text("ARGB8888");
  l_left.set_text("left");
  l_center.set_text("center");
  l_right.set_text("right");
  g_full_txt.set_text("270deg AA");   // the LCD font has no degree sign
  g_plain_txt.set_text("180deg");
  b_txt.set_text("dual markers");

  // A N I M A T I O N
  std::int16_t step_ltr = 2, step_rtl = -3, step_btt = 3, step_ttb = 5;
  p_ltr.set_step(step_ltr); p_rtl.set_step(step_rtl);
  p_btt.set_step(step_btt); p_ttb.set_step(step_ttb);
  p_rtl.set_pos(50);
  p_ttb.set_pos(70);
  bool ltr_red = false;

  for (std::uint32_t t = 0U; t < frames; ++t) {
    // progress: bounce at different speeds, the top bar turns red above 80
    bounce(p_ltr, step_ltr);
    bounce(p_rtl, step_rtl);
    bounce(p_btt, step_btt);
    bounce(p_ttb, step_ttb);
    if (ltr_red != (p_ltr.get_pos() > 80)) {
      ltr_red = !ltr_red;
      p_ltr.set_color(ltr_red ? COL_RED : COL_GREEN);
    }
    p_val.set_value(p_ltr.get_pos(), 0U, " %");

    // bitmap: the L1 pattern cycles its hue, the coin spins (one RGB565 sheet frame, drawn row by row)
    bmp_l1.set_color(vic::color::hsv(static_cast<std::uint16_t>((t * 3U) % 360U), 100U, 100U));
    bmp_l1.render_raw({ 148, 20 }, 16U, 16U, vic::color::format_L1, l1_data);
    if ((t % 4U) == 0U) {
      const std::uint32_t frame = (t / 4U) % 8U;
      _dc.present_lock();
      for (std::int16_t row = 0; row < 16; ++row) {
        bmp_coin.render_raw({ 178, static_cast<std::int16_t>(20 + row) }, 16U, 1U, vic::color::format_RGB565,
                            &sprite_sheet_coin_data[static_cast<std::uint32_t>(row) * 256U + frame * 32U]);
      }
      _dc.present_lock(false);
    }

    // label: a fixed point readout and alternating color escape text
    l_value.set_value(wave(t, 300U, 0, 0, 1500), 2U, " V");
    l_color.set_text(((t / 50U) & 1U) ? "\aFF4040red \a40FF40green \a4080FFblue" : "inline colors");

    // gauge
    const std::int16_t v_full  = wave(t, 400U,   0, 0, 100);
    const std::int16_t v_plain = wave(t, 250U, 120, 0, 100);
    g_full.set_value(v_full);   g_full_val.set_value(v_full);
    g_plain.set_value(v_plain); g_plain_val.set_value(v_plain);

    // bargauge
    const std::int16_t v_b1 = wave(t, 500U,  30, 0, 100);
    const std::int16_t v_b2 = wave(t, 500U,  80, 0, 100);
    b_btt.set_value(0U, v_b1); b_val1.set_value(v_b1);
    b_btt.set_value(1U, v_b2); b_val2.set_value(v_b2);
    b_ttb.set_value(0U, wave(t, 350U, 200, 0, 100));
    b_ltr.set_value(0U, wave(t, 450U, 300, 0, 100));
    b_rtl.set_value(0U, wave(t, 280U,  60, 0, 100));

    vic::io::delay(FRAME_MS);
  }
}


namespace {

// the text of the textbox, the widget keeps only the pointer
const char* textbox_text =
  "The textbox wraps a long text at spaces to the cell width and honors explicit line breaks.\n"
  "\aF0D020Color escapes\aFFFFFF work across lines. The text is not copied, lines which don't fit are "
  "scrolled in with scroll(1).";

// event handler of the input demo: toggles the LED and reports the event in a label
struct input_report_type {
  vic::widget::label* report;
  const char*         name;
};

void on_led_event(vic::widget::base& w, vic::vertex_type v, vic::widget::event_type ev, void* arg)
{
  (void)v;
  input_report_type* r = static_cast<input_report_type*>(arg);
  if (ev == vic::widget::event_tap) {
    static_cast<vic::widget::led&>(w).toggle();
    char txt[16] = "tap ";
    txt[4] = r->name[0]; txt[5] = 0;
    r->report->set_text(txt);
  }
  else if (ev == vic::widget::event_long_tap) {
    static_cast<vic::widget::led&>(w).set_state(false);
    char txt[16] = "long tap ";
    txt[9] = r->name[0]; txt[10] = 0;
    r->report->set_text(txt);
  }
}

} // namespace


/**
 * Page 2: arc, led, spinner, textbox and the input routing
 * \param frames Frames to show the page
 */
void page2(std::uint32_t frames)
{
  // G R O U P   B O X E S
  group_type grp_arc    (head(), group_cfg(  2,   8, 108,  92, "arc"));
  group_type grp_led    (head(), group_cfg(112,   8, 208,  92, "led"));
  group_type grp_spinner(head(), group_cfg(212,   8, 318,  92, "spinner"));
  group_type grp_text   (head(), group_cfg(  2, 100, 160, 196, "textbox"));
  group_type grp_input  (head(), group_cfg(164, 100, 318, 196, "input"));

  // A R C  -  a full ring (anti aliased) and a 270 degree arc
  const vic::widget::arc::config_type a_full_cfg = { { 30, 52 }, 27U, 8U, -180, 180, 0, 100, COL_GREEN, COL_RING, COL_BG };
  const vic::widget::arc::config_type a_part_cfg = { { 80, 52 }, 22U, 6U, -135, 135, 0, 100, COL_CYAN,  COL_RING, COL_BG };
  vic::widget::arc a_full(head(), a_full_cfg);
  vic::widget::arc a_part(head(), a_part_cfg);
  a_full.anti_aliasing_enable();
  a_part.anti_aliasing_enable();
  // the value labels must fit into the ring holes (radius 19 and 16) with two pixel clearance for the erased edges
  label_type a_full_val(head(), label_cfg(16, 44, 28U, 16U, font_16, label_type::align_center, COL_TEXT));
  label_type a_part_val(head(), label_cfg(67, 44, 26U, 16U, font_16, label_type::align_center, COL_TEXT));
  label_type a_txt     (head(), label_cfg( 8, 80, 96U, 12U, font_lcd, label_type::align_center, COL_NAME, vic::color::none));

  // L E D  -  four round LEDs as running light, three square LEDs showing a bit pattern
  const vic::color::value_type led_colors[4] = { COL_RED, COL_YELLOW, COL_GREEN, COL_BLUE };
  vic::widget::led::config_type led_cfg = { { 0, 30 }, 6U, vic::widget::led::shape_round, COL_RED, vic::color::gray03, vic::color::gray08 };
  led_cfg.center.x = 128; led_cfg.color_on = led_colors[0]; vic::widget::led led_r0(head(), led_cfg);
  led_cfg.center.x = 149; led_cfg.color_on = led_colors[1]; vic::widget::led led_r1(head(), led_cfg);
  led_cfg.center.x = 170; led_cfg.color_on = led_colors[2]; vic::widget::led led_r2(head(), led_cfg);
  led_cfg.center.x = 191; led_cfg.color_on = led_colors[3]; vic::widget::led led_r3(head(), led_cfg);
  led_cfg.shape = vic::widget::led::shape_square; led_cfg.radius = 7U; led_cfg.center.y = 58; led_cfg.color_on = COL_ORANGE;
  led_cfg.center.x = 138; vic::widget::led led_s0(head(), led_cfg);
  led_cfg.center.x = 160; vic::widget::led led_s1(head(), led_cfg);
  led_cfg.center.x = 182; vic::widget::led led_s2(head(), led_cfg);
  vic::widget::led* leds_round[4]  = { &led_r0, &led_r1, &led_r2, &led_r3 };
  vic::widget::led* leds_square[3] = { &led_s0, &led_s1, &led_s2 };
  label_type led_txt(head(), label_cfg(112, 76, 96U, 12U, font_lcd, label_type::align_center, COL_NAME, vic::color::none));

  // S P I N N E R  -  three ring segments, different sweep, speed and direction
  const vic::widget::spinner::config_type sp_big_cfg = { { 240, 50 }, 22U, 6U,  90U,   6, COL_TEXT,   COL_RING, COL_BG };
  const vic::widget::spinner::config_type sp_mid_cfg = { { 286, 38 }, 14U, 5U, 120U,  -8, COL_YELLOW, COL_RING, COL_BG };
  const vic::widget::spinner::config_type sp_sml_cfg = { { 286, 70 },  8U, 3U, 180U,  15, COL_CYAN,   COL_RING, COL_BG };
  vic::widget::spinner sp_big(head(), sp_big_cfg);
  vic::widget::spinner sp_mid(head(), sp_mid_cfg);
  vic::widget::spinner sp_sml(head(), sp_sml_cfg);
  sp_big.anti_aliasing_enable();
  label_type sp_txt(head(), label_cfg(212, 78, 56U, 12U, font_lcd, label_type::align_center, COL_NAME, vic::color::none));

  // T E X T B O X  -  word wrapped text, scrolled line by line
  const vic::widget::textbox::config_type tb_cfg = { { 8, 108 }, 146U, 62U, &font_lcd, vic::widget::textbox::align_left, 1U,
                                                     COL_TEXT, COL_BG, vic::color::none };
  vic::widget::textbox tb(head(), tb_cfg);
  label_type tb_txt(head(), label_cfg(8, 180, 146U, 12U, font_lcd, label_type::align_center, COL_NAME, vic::color::none));

  // I N P U T  -  three LED "buttons", a report label and a simulated touch cursor
  led_cfg.radius = 10U; led_cfg.center.y = 124; led_cfg.color_on = COL_GREEN;
  led_cfg.center.x = 192; vic::widget::led btn_a(head(), led_cfg);
  led_cfg.center.x = 241; vic::widget::led btn_b(head(), led_cfg);
  led_cfg.center.x = 290; vic::widget::led btn_c(head(), led_cfg);
  label_type btn_txt_a(head(), label_cfg(180, 140, 24U, 12U, font_lcd, label_type::align_center, COL_NAME));
  label_type btn_txt_b(head(), label_cfg(229, 140, 24U, 12U, font_lcd, label_type::align_center, COL_NAME));
  label_type btn_txt_c(head(), label_cfg(278, 140, 24U, 12U, font_lcd, label_type::align_center, COL_NAME));
  label_type report   (head(), label_cfg(170, 156, 142U, 16U, font_16, label_type::align_center, COL_YELLOW));
  label_type in_txt   (head(), label_cfg(170, 180, 142U, 12U, font_lcd, label_type::align_center, COL_NAME));
  input_report_type rep_a = { &report, "A" };
  input_report_type rep_b = { &report, "B" };
  input_report_type rep_c = { &report, "C" };
  btn_a.set_event_handler(on_led_event, &rep_a);
  btn_b.set_event_handler(on_led_event, &rep_b);
  btn_c.set_event_handler(on_led_event, &rep_c);
  // the cursor: a crosshair canvas sprite
  vic::sprite::canvas<64, 1> cursor(head(), 10);
  cursor.set_color(COL_TEXT);
  cursor.line({ -6, 0 }, { 6, 0 });
  cursor.line({ 0, -6 }, { 0, 6 });
  cursor.set_color(COL_RED);
  cursor.circle({ 0, 0 }, 3U);

  // I N I T  -  group boxes first, then the widgets inside
  grp_arc.init(); grp_led.init(); grp_spinner.init(); grp_text.init(); grp_input.init();
  a_full.init(); a_part.init(); a_full_val.init(); a_part_val.init(); a_txt.init();
  for (std::size_t i = 0U; i < 4U; ++i) { leds_round[i]->init(); }
  for (std::size_t i = 0U; i < 3U; ++i) { leds_square[i]->init(); }
  led_txt.init();
  sp_big.init(); sp_mid.init(); sp_sml.init(); sp_txt.init();
  tb.init(); tb_txt.init();
  btn_a.init(); btn_b.init(); btn_c.init(); btn_txt_a.init(); btn_txt_b.init(); btn_txt_c.init(); report.init(); in_txt.init();

  // static texts
  a_txt.set_text("ring / 270deg");
  led_txt.set_text("round / square");
  sp_txt.set_text("step()");
  tb.set_text(textbox_text);
  tb_txt.set_text("scroll(1)");
  btn_txt_a.set_text("A");
  btn_txt_b.set_text("B");
  btn_txt_c.set_text("C");
  in_txt.set_text("base::event()");

  // A N I M A T I O N
  // the cursor visits the three buttons and an empty spot, 100 frames per stop: 40 frames move, tap at frame 60
  const vic::vertex_type targets[4] = { { 192, 124 }, { 241, 124 }, { 290, 124 }, { 241, 184 } };
  vic::vertex_type cursor_pos = targets[3];
  for (std::uint32_t t = 0U; t < frames; ++t) {
    // arc
    const std::int16_t v_full = wave(t, 300U,   0, 0, 99);
    const std::int16_t v_part = wave(t, 220U, 180, 0, 99);
    a_full.set_value(v_full); a_full_val.set_value(v_full);
    a_part.set_value(v_part); a_part_val.set_value(v_part);

    // led: running light and a 3 bit counter
    for (std::size_t i = 0U; i < 4U; ++i) { leds_round[i]->set_state(((t / 10U) % 4U) == i); }
    for (std::size_t i = 0U; i < 3U; ++i) { leds_square[i]->set_state((((t / 25U) >> i) & 1U) != 0U); }

    // spinner
    sp_big.step();
    sp_mid.step();
    if ((t % 2U) == 0U) { sp_sml.step(); }

    // textbox: scroll one line every 50 frames, start over at the end
    if ((t % 50U) == 49U) {
      if (tb.get_first_line() + tb.get_visible_lines() >= tb.get_line_count()) { tb.set_first_line(0U); }
      else                                                                      { tb.scroll(1); }
    }

    // input: move the cursor to the next target and tap, every 4th stop is a long tap
    {
      const std::uint32_t stop  = t / 100U;
      const std::uint32_t phase = t % 100U;
      const vic::vertex_type from = targets[(stop + 3U) % 4U];
      const vic::vertex_type to   = targets[stop % 4U];
      if (phase < 40U) {
        cursor_pos = { static_cast<std::int16_t>(from.x + (to.x - from.x) * static_cast<std::int32_t>(phase) / 40),
                       static_cast<std::int16_t>(from.y + (to.y - from.y) * static_cast<std::int32_t>(phase) / 40) };
      }
      else {
        cursor_pos = to;
      }
      if (phase == 60U) {
        // the touch driver would call base::event() like this; the cursor is hidden while the widget redraws below it
        cursor.render(cursor_pos, 0xFFFFU);
        vic::widget::base::event(cursor_pos, vic::widget::event_down);
        vic::widget::base::event(cursor_pos, vic::widget::event_up);
        if (vic::widget::base::event(cursor_pos, (stop % 8U) == 5U ? vic::widget::event_long_tap : vic::widget::event_tap) == nullptr) {
          report.set_text("no widget");
        }
      }
      cursor.render(cursor_pos, 0U);
    }

    vic::io::delay(FRAME_MS);
  }
}


namespace {

// soft key icons of page 3, 12 x 10 XPM
const char* xpm_map[] = {
  "12 10 2 1", "  c None", ". c #40E0FF",
  "   ..       ", "  .  .      ", " .    .     ", ".      .    ", ".       .   ",
  " .       .  ", "  .       . ", "   .     .  ", "    .   .   ", "     ...    "
};
const char* xpm_eng[] = {
  "12 10 2 1", "  c None", ". c #F0D020",
  "    ....    ", "   ......   ", "  ..    ..  ", " ..      .. ", " .        . ",
  " .        . ", " ..      .. ", "  ..    ..  ", "   ......   ", "    ....    "
};

// a generic event report: label and widget name
struct report3_type {
  label_type* line;
  const char* name;
};
char report3_text[32];

void on_input3(vic::widget::base& w, vic::vertex_type v, vic::widget::event_type ev, void* arg)
{
  (void)w; (void)v;
  if ((ev != vic::widget::event_tap) && (ev != vic::widget::event_long_tap)) {
    return;
  }
  report3_type* r = static_cast<report3_type*>(arg);
  const char* what = (ev == vic::widget::event_tap) ? "tap " : "long ";
  std::size_t n = 0U;
  for (const char* p = what;    *p && (n < 30U); ++p) { report3_text[n++] = *p; }
  for (const char* p = r->name; *p && (n < 31U); ++p) { report3_text[n++] = *p; }
  report3_text[n] = 0;
  r->line->set_text(report3_text);
}

vic::widget::imagebutton* softkeys3[2] = { nullptr, nullptr };

void on_softkey3(vic::widget::base& w, vic::vertex_type v, vic::widget::event_type ev, void* arg)
{
  on_input3(w, v, ev, arg);
  if (ev == vic::widget::event_tap) {
    softkeys3[0]->set_active(softkeys3[0] == &w);
    softkeys3[1]->set_active(softkeys3[1] == &w);
  }
}

} // namespace


/**
 * Page 3: compass, image buttons and the input widgets
 * \param frames Frames to show the page
 */
void page3(std::uint32_t frames)
{
  const vic::color::value_type COL_CTRL = vic::color::gray04, COL_PRESSED = vic::color::gray08, COL_ACTIVE = vic::color::argb(0x20U, 0x50U, 0xA0U);
  const vic::color::value_type COL_MAGENTA = vic::color::argb(0xFFU, 0x40U, 0xFFU);

  // G R O U P   B O X E S
  group_type grp_compass(head(), group_cfg(  2,   8, 108,  92, "compass"));
  group_type grp_keys   (head(), group_cfg(112,   8, 208,  92, "imagebutton"));
  group_type grp_check  (head(), group_cfg(212,   8, 318,  92, "checkbox/switch"));
  group_type grp_ctrl   (head(), group_cfg(  2, 100, 318, 196, "button/slider/spinbox"));

  // C O M P A S S
  const vic::widget::compass::config_type cp_cfg = { { 42, 50 }, 36U, &font_lcd, COL_BG, COL_FRAME, COL_TEXT, COL_TEXT, COL_MAGENTA };
  vic::widget::compass cp(head(), cp_cfg);
  cp.anti_aliasing_enable();
  label_type cp_val(head(), label_cfg(82, 44, 24U, 12U, font_lcd, label_type::align_right, COL_MAGENTA));

  // I M A G E B U T T O N  -  two soft keys, the tapped one is active
  const vic::widget::imagebutton::config_type sk0_cfg = { { 118, 24, 158, 56 }, 3U, xpm_map, &font_lcd, "MAP", COL_TEXT, COL_CTRL, COL_PRESSED, COL_ACTIVE, COL_FRAME };
  const vic::widget::imagebutton::config_type sk1_cfg = { { 162, 24, 202, 56 }, 3U, xpm_eng, &font_lcd, "ENG", COL_TEXT, COL_CTRL, COL_PRESSED, COL_ACTIVE, COL_FRAME };
  vic::widget::imagebutton sk0(head(), sk0_cfg);
  vic::widget::imagebutton sk1(head(), sk1_cfg);
  softkeys3[0] = &sk0; softkeys3[1] = &sk1;
  label_type sk_txt(head(), label_cfg(112, 64, 96U, 12U, font_lcd, label_type::align_center, COL_NAME, vic::color::none));

  // C H E C K B O X   /   S W I T C H
  const vic::widget::checkbox::config_type cb0_cfg = { { 220, 22 }, 12U, &font_lcd, "Sound", COL_TEXT, COL_GREEN, COL_CTRL, COL_FRAME };
  const vic::widget::checkbox::config_type cb1_cfg = { { 220, 40 }, 12U, &font_lcd, "Light", COL_TEXT, COL_GREEN, COL_CTRL, COL_FRAME };
  vic::widget::checkbox cb0(head(), cb0_cfg);
  vic::widget::checkbox cb1(head(), cb1_cfg);
  const vic::widget::toggle::config_type sw0_cfg = { { 220, 60 }, 30U, 14U, COL_GREEN, COL_CTRL, COL_TEXT, COL_FRAME };
  const vic::widget::toggle::config_type sw1_cfg = { { 270, 60 }, 30U, 14U, COL_CYAN,  COL_CTRL, COL_TEXT, COL_FRAME };
  vic::widget::toggle sw0(head(), sw0_cfg);
  vic::widget::toggle sw1(head(), sw1_cfg);
  sw0.anti_aliasing_enable();
  sw1.anti_aliasing_enable();

  // B U T T O N   /   S L I D E R   /   S P I N B O X
  const vic::widget::button::config_type btn0_cfg = { { 12, 116, 72, 138 }, 4U, &font_lcd, "OK",     COL_TEXT, COL_CTRL, COL_PRESSED, COL_FRAME };
  const vic::widget::button::config_type btn1_cfg = { { 78, 116, 150, 138 }, 4U, &font_lcd, "CANCEL", COL_TEXT, COL_CTRL, COL_PRESSED, COL_FRAME };
  vic::widget::button btn0(head(), btn0_cfg);
  vic::widget::button btn1(head(), btn1_cfg);
  const vic::widget::slider::config_type slh_cfg = { { 10, 146 }, 140U, 16U, vic::widget::slider::horizontal, 0, 100, 4U, 6U, COL_CTRL, COL_CYAN, COL_TEXT, COL_BG };
  const vic::widget::slider::config_type slv_cfg = { { 292, 112 }, 16U, 76U, vic::widget::slider::vertical,   0, 100, 4U, 6U, COL_CTRL, COL_GREEN, COL_TEXT, COL_BG };
  vic::widget::slider slh(head(), slh_cfg);
  vic::widget::slider slv(head(), slv_cfg);
  slh.anti_aliasing_enable();
  slv.anti_aliasing_enable();
  label_type sl_val(head(), label_cfg(100, 166, 50U, 12U, font_lcd, label_type::align_right, COL_CYAN));
  const vic::widget::spinbox::config_type sb_cfg = { { 166, 116, 282, 136 }, 3U, &font_lcd, 0, 20000, 100, 0, COL_TEXT, COL_BG, COL_CTRL, COL_FRAME };
  vic::widget::spinbox sb(head(), sb_cfg);
  label_type sb_txt(head(), label_cfg(166, 140, 116U, 12U, font_lcd, label_type::align_center, COL_NAME, vic::color::none));
  label_type report(head(), label_cfg(166, 166, 120U, 12U, font_lcd, label_type::align_left, COL_CYAN));
  label_type in_txt(head(), label_cfg(10, 182, 300U, 12U, font_lcd, label_type::align_left, COL_NAME, vic::color::none));

  // handlers
  report3_type rep_cb0 = { &report, "Sound" }, rep_cb1 = { &report, "Light" }, rep_sw0 = { &report, "switch 1" }, rep_sw1 = { &report, "switch 2" };
  report3_type rep_b0 = { &report, "OK" }, rep_b1 = { &report, "CANCEL" }, rep_slh = { &report, "slider" }, rep_slv = { &report, "slider" };
  report3_type rep_sb = { &report, "spinbox" }, rep_sk0 = { &report, "MAP" }, rep_sk1 = { &report, "ENG" };
  cb0.set_event_handler(on_input3, &rep_cb0); cb1.set_event_handler(on_input3, &rep_cb1);
  sw0.set_event_handler(on_input3, &rep_sw0); sw1.set_event_handler(on_input3, &rep_sw1);
  btn0.set_event_handler(on_input3, &rep_b0); btn1.set_event_handler(on_input3, &rep_b1);
  slh.set_event_handler(on_input3, &rep_slh); slv.set_event_handler(on_input3, &rep_slv);
  sb.set_event_handler(on_input3, &rep_sb);
  sk0.set_event_handler(on_softkey3, &rep_sk0); sk1.set_event_handler(on_softkey3, &rep_sk1);

  // the touch cursor
  vic::sprite::canvas<64, 1> cursor(head(), 10);
  cursor.set_color(COL_TEXT);
  cursor.line({ -6, 0 }, { 6, 0 });
  cursor.line({ 0, -6 }, { 0, 6 });
  cursor.set_color(COL_RED);
  cursor.circle({ 0, 0 }, 3U);

  // I N I T
  grp_compass.init(); grp_keys.init(); grp_check.init(); grp_ctrl.init();
  cp.init(); cp_val.init();
  sk0.init(); sk1.init(); sk_txt.init();
  cb0.init(); cb1.init(); sw0.init(); sw1.init();
  btn0.init(); btn1.init(); slh.init(); slv.init(); sl_val.init(); sb.init(); sb_txt.init(); report.init(); in_txt.init();
  sk_txt.set_text("soft keys");
  sb_txt.set_text("altitude");
  in_txt.set_text("simulated touch via base::event()");
  sk0.set_active(true);
  cb0.set_checked(true);
  slh.set_value(60);
  slv.set_value(30);
  sb.set_value(3500);

  // T O U C H   S C R I P T  -  one step per 50 frames: 25 frames move, down at 32, up and tap at 40
  struct step_type { vic::vertex_type pos; vic::widget::event_type ev; };
  const step_type script[] = {
    { { 226,  28 }, vic::widget::event_tap },        // Sound off
    { { 226,  46 }, vic::widget::event_tap },        // Light on
    { { 235,  67 }, vic::widget::event_tap },        // switch 1 on
    { { 285,  67 }, vic::widget::event_tap },        // switch 2 on
    { { 182,  40 }, vic::widget::event_tap },        // ENG soft key
    { {  40, 154 }, vic::widget::event_tap },        // slider low
    { { 130, 154 }, vic::widget::event_tap },        // slider high
    { { 300, 125 }, vic::widget::event_tap },        // vertical slider high
    { { 273, 126 }, vic::widget::event_tap },        // spinbox + 100
    { { 273, 126 }, vic::widget::event_long_tap },   // spinbox + 1000
    { { 175, 126 }, vic::widget::event_tap },        // spinbox - 100
    { {  42, 127 }, vic::widget::event_tap },        // OK
    { { 138,  40 }, vic::widget::event_tap },        // MAP soft key
    { { 235,  67 }, vic::widget::event_tap },        // switch 1 off
    { { 300, 175 }, vic::widget::event_tap },        // vertical slider low
    { { 114, 127 }, vic::widget::event_long_tap },   // CANCEL, long
  };
  const std::uint32_t steps = sizeof(script) / sizeof(script[0]);

  vic::vertex_type cursor_pos = { 160, 100 };
  vic::vertex_type from       = cursor_pos;
  for (std::uint32_t t = 0U; t < frames; ++t) {
    const std::uint32_t step  = (t / 50U) % steps;
    const std::uint32_t phase = t % 50U;
    const vic::vertex_type to = script[step].pos;
    if (phase == 0U) {
      from = cursor_pos;
    }
    if (phase < 25U) {
      cursor_pos = { static_cast<std::int16_t>(from.x + (to.x - from.x) * static_cast<std::int32_t>(phase) / 25),
                     static_cast<std::int16_t>(from.y + (to.y - from.y) * static_cast<std::int32_t>(phase) / 25) };
    }
    else {
      cursor_pos = to;
    }
    const bool long_tap = script[step].ev == vic::widget::event_long_tap;
    if ((phase == 32U) || (phase == (long_tap ? 46U : 40U))) {
      cursor.render(cursor_pos, 0xFFFFU);
      if (phase == 32U) {
        vic::widget::base::event(cursor_pos, vic::widget::event_down);
      }
      else {
        vic::widget::base::event(cursor_pos, vic::widget::event_up);
        vic::widget::base::event(cursor_pos, script[step].ev);
      }
    }
    cursor.render(cursor_pos, 0U);

    // compass turns, readouts follow
    const std::int16_t hdg = static_cast<std::int16_t>((t / 2U) % 360U);
    cp.set_heading(hdg);
    cp_val.set_value(hdg);
    sl_val.set_value(slh.get_value(), 0U, " %");

    vic::io::delay(FRAME_MS);
  }
}


void demo()
{
  head().init();
  build_bitmaps();

  vic::dc _dc(head());
  for (;;) {
    _dc.cls(COL_BG);
    page1(250U);     // 10 s
    _dc.cls(COL_BG);
    page2(250U);
    _dc.cls(COL_BG);
    page3(250U);
  }
}
