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
// \brief Input widget demo: a settings page with a compass, operated by a simulated touch
//
// Left: checkboxes, a switch, a slider, a spinbox and two buttons. Right: a heading up compass
// with its readout and a soft key bar of image buttons. A simulated touch cursor walks through a
// script of taps and long taps, every event goes through widget::base::event() exactly as a touch
// driver would send it (event_down, event_up, event_tap / event_long_tap), the widgets react and
// their event handlers report into the status line.
//
// Link this file together with windows/main.cpp (see demo/CMakeLists.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>

#include "dc.h"
#include "sprite.h"
#include "widget/label.h"
#include "widget/groupbox.h"
#include "widget/checkbox.h"
#include "widget/switch.h"
#include "widget/slider.h"
#include "widget/spinbox.h"
#include "widget/button.h"
#include "widget/imagebutton.h"
#include "widget/compass.h"

// fonts
#include "font/LCD_6x10.h"
#include "font/Montserrat_16_aa4.h"


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
const vic::color::value_type COL_BG      = vic::color::black;
const vic::color::value_type COL_TEXT    = vic::color::white;
const vic::color::value_type COL_FRAME   = vic::color::gray08;
const vic::color::value_type COL_CTRL    = vic::color::gray04;     // control background
const vic::color::value_type COL_PRESSED = vic::color::gray08;
const vic::color::value_type COL_ACTIVE  = vic::color::argb(0x20U, 0x50U, 0xA0U);
const vic::color::value_type COL_GREEN   = vic::color::argb(0x20U, 0xD0U, 0x20U);
const vic::color::value_type COL_CYAN    = vic::color::argb(0x40U, 0xE0U, 0xFFU);
const vic::color::value_type COL_MAGENTA = vic::color::argb(0xFFU, 0x40U, 0xFFU);
const vic::color::value_type COL_RED     = vic::color::argb(0xF0U, 0x20U, 0x20U);

vic::font::LCD_6x10          font_lcd;
vic::font::Montserrat_16_aa4 font_16;

typedef vic::widget::label     label_type;
typedef vic::widget::groupbox  group_type;

// soft key icons, 12 x 10 XPM
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

// one step of the touch script
struct step_type {
  vic::vertex_type          pos;
  vic::widget::event_type   ev;       // event_tap or event_long_tap
};

// the event report: a label and the name of the widget
struct report_type {
  label_type* line;
  const char* name;
};

char report_text[32];

void set_report(report_type* r, const char* what)
{
  std::size_t n = 0U;
  for (const char* p = what;    *p && (n < 30U); ++p) { report_text[n++] = *p; }
  report_text[n++] = ' ';
  for (const char* p = r->name; *p && (n < 31U); ++p) { report_text[n++] = *p; }
  report_text[n] = 0;
  r->line->set_text(report_text);
}

// generic handler: reports taps and long taps
void on_widget(vic::widget::base& w, vic::vertex_type v, vic::widget::event_type ev, void* arg)
{
  (void)w; (void)v;
  if (ev == vic::widget::event_tap)      { set_report(static_cast<report_type*>(arg), "tap"); }
  if (ev == vic::widget::event_long_tap) { set_report(static_cast<report_type*>(arg), "long tap"); }
}

// soft keys: the tapped key becomes the active one
vic::widget::imagebutton* softkeys[4] = { nullptr, nullptr, nullptr, nullptr };

void on_softkey(vic::widget::base& w, vic::vertex_type v, vic::widget::event_type ev, void* arg)
{
  on_widget(w, v, ev, arg);
  if (ev == vic::widget::event_tap) {
    for (std::size_t i = 0U; i < 4U; ++i) {
      softkeys[i]->set_active(softkeys[i] == &w);
    }
  }
}

label_type::config_type label_cfg(std::int16_t x, std::int16_t y, std::uint16_t w, std::uint16_t h, const vic::font::font_type& font,
                                  label_type::align_type align, vic::color::value_type color, vic::color::value_type bg = COL_BG)
{
  const label_type::config_type cfg = { { x, y }, w, h, &font, align, color, bg, vic::color::none };
  return cfg;
}

std::int16_t wave(std::uint32_t t, std::uint32_t period, std::int16_t phase, std::int16_t lo, std::int16_t hi)
{
  const std::int16_t a = static_cast<std::int16_t>((t * 360U / period + static_cast<std::uint32_t>(phase)) % 360U);
  const std::int32_t s = vic::util::sin(a) + 16384;
  return static_cast<std::int16_t>(lo + (static_cast<std::int32_t>(hi - lo) * s) / 32768);
}

} // namespace


void demo()
{
  head().init();
  vic::dc _dc(head());
  _dc.cls(COL_BG);

  // F R A M E S   A N D   T I T L E S
  const group_type::config_type grp_set_cfg = { { 2, 10, 154, 166 }, 4U, &font_lcd, "settings", COL_FRAME, COL_TEXT, COL_BG };
  const group_type::config_type grp_nav_cfg = { { 160, 10, 318, 166 }, 4U, &font_lcd, "navigation", COL_FRAME, COL_TEXT, COL_BG };
  group_type grp_set(head(), grp_set_cfg);
  group_type grp_nav(head(), grp_nav_cfg);

  // S E T T I N G S
  const vic::widget::checkbox::config_type cb_sound_cfg = { { 12, 22 }, 12U, &font_lcd, "Sound",     COL_TEXT, COL_GREEN, COL_CTRL, COL_FRAME };
  const vic::widget::checkbox::config_type cb_light_cfg = { { 12, 40 }, 12U, &font_lcd, "Backlight", COL_TEXT, COL_GREEN, COL_CTRL, COL_FRAME };
  vic::widget::checkbox cb_sound(head(), cb_sound_cfg);
  vic::widget::checkbox cb_light(head(), cb_light_cfg);
  label_type sw_txt(head(), label_cfg(12, 58, 60U, 14U, font_lcd, label_type::align_left, COL_TEXT));
  const vic::widget::toggle::config_type sw_cfg = { { 100, 58 }, 30U, 14U, COL_GREEN, COL_CTRL, COL_TEXT, COL_FRAME };
  vic::widget::toggle sw_auto(head(), sw_cfg);
  label_type sl_txt(head(), label_cfg(12, 78, 70U, 12U, font_lcd, label_type::align_left, COL_TEXT));
  label_type sl_val(head(), label_cfg(100, 78, 44U, 12U, font_lcd, label_type::align_right, COL_CYAN));
  const vic::widget::slider::config_type sl_cfg = { { 10, 90 }, 136U, 16U, vic::widget::slider::horizontal, 0, 100, 4U, 6U,
                                                    COL_CTRL, COL_CYAN, COL_TEXT, COL_BG };
  vic::widget::slider sl_bright(head(), sl_cfg);
  label_type sb_txt(head(), label_cfg(12, 110, 60U, 12U, font_lcd, label_type::align_left, COL_TEXT));
  const vic::widget::spinbox::config_type sb_cfg = { { 70, 108, 146, 126 }, 3U, &font_lcd, 0, 20000, 100, 0, COL_TEXT, COL_BG, COL_CTRL, COL_FRAME };
  vic::widget::spinbox sb_alt(head(), sb_cfg);
  const vic::widget::button::config_type btn_ok_cfg     = { { 12, 136, 76, 158 },  4U, &font_lcd, "OK",     COL_TEXT, COL_CTRL, COL_PRESSED, COL_FRAME };
  const vic::widget::button::config_type btn_cancel_cfg = { { 82, 136, 146, 158 }, 4U, &font_lcd, "CANCEL", COL_TEXT, COL_CTRL, COL_PRESSED, COL_FRAME };
  vic::widget::button btn_ok(head(), btn_ok_cfg);
  vic::widget::button btn_cancel(head(), btn_cancel_cfg);

  // N A V I G A T I O N  -  compass with readout and the soft key bar
  const vic::widget::compass::config_type cp_cfg = { { 239, 76 }, 52U, &font_lcd, COL_BG, COL_FRAME, COL_TEXT, COL_TEXT, COL_MAGENTA };
  vic::widget::compass cp(head(), cp_cfg);
  cp.anti_aliasing_enable();
  label_type hdg_txt(head(), label_cfg(168, 140, 40U, 16U, font_lcd, label_type::align_left, COL_TEXT));
  label_type hdg_val(head(), label_cfg(200, 136, 60U, 18U, font_16, label_type::align_center, COL_MAGENTA));   // centered: the "1" glyph starts left of the text position
  const vic::widget::imagebutton::config_type sk_cfg[4] = {
    { { 164, 170, 200, 196 }, 3U, xpm_map,  &font_lcd, "MAP",  COL_TEXT, COL_CTRL, COL_PRESSED, COL_ACTIVE, COL_FRAME },
    { { 203, 170, 239, 196 }, 3U, nullptr,  &font_lcd, "TER",  COL_TEXT, COL_CTRL, COL_PRESSED, COL_ACTIVE, COL_FRAME },
    { { 242, 170, 278, 196 }, 3U, xpm_eng,  &font_lcd, "ENG",  COL_TEXT, COL_CTRL, COL_PRESSED, COL_ACTIVE, COL_FRAME },
    { { 281, 170, 317, 196 }, 3U, nullptr,  &font_lcd, "INFO", COL_TEXT, COL_CTRL, COL_PRESSED, COL_ACTIVE, COL_FRAME },
  };
  vic::widget::imagebutton sk0(head(), sk_cfg[0]);
  vic::widget::imagebutton sk1(head(), sk_cfg[1]);
  vic::widget::imagebutton sk2(head(), sk_cfg[2]);
  vic::widget::imagebutton sk3(head(), sk_cfg[3]);
  softkeys[0] = &sk0; softkeys[1] = &sk1; softkeys[2] = &sk2; softkeys[3] = &sk3;

  // S T A T U S   L I N E
  label_type status(head(), label_cfg(4, 172, 156U, 12U, font_lcd, label_type::align_left, COL_CYAN));
  label_type hint  (head(), label_cfg(4, 186, 156U, 12U, font_lcd, label_type::align_left, COL_FRAME));

  // event handlers
  report_type rep_sound  = { &status, "Sound" },  rep_light  = { &status, "Backlight" }, rep_auto = { &status, "Autopilot" };
  report_type rep_slider = { &status, "slider" }, rep_spin   = { &status, "spinbox" };
  report_type rep_ok     = { &status, "OK" },     rep_cancel = { &status, "CANCEL" };
  report_type rep_sk[4]  = { { &status, "MAP" }, { &status, "TER" }, { &status, "ENG" }, { &status, "INFO" } };
  cb_sound.set_event_handler(on_widget, &rep_sound);
  cb_light.set_event_handler(on_widget, &rep_light);
  sw_auto.set_event_handler(on_widget, &rep_auto);
  sl_bright.set_event_handler(on_widget, &rep_slider);
  sb_alt.set_event_handler(on_widget, &rep_spin);
  btn_ok.set_event_handler(on_widget, &rep_ok);
  btn_cancel.set_event_handler(on_widget, &rep_cancel);
  sk0.set_event_handler(on_softkey, &rep_sk[0]);
  sk1.set_event_handler(on_softkey, &rep_sk[1]);
  sk2.set_event_handler(on_softkey, &rep_sk[2]);
  sk3.set_event_handler(on_softkey, &rep_sk[3]);

  // the touch cursor: a crosshair canvas sprite
  vic::sprite::canvas<64, 1> cursor(head(), 10);
  cursor.set_color(COL_TEXT);
  cursor.line({ -6, 0 }, { 6, 0 });
  cursor.line({ 0, -6 }, { 0, 6 });
  cursor.set_color(COL_RED);
  cursor.circle({ 0, 0 }, 3U);

  // I N I T
  grp_set.init(); grp_nav.init();
  cb_sound.init(); cb_light.init(); sw_txt.init(); sw_auto.init();
  sl_txt.init(); sl_val.init(); sl_bright.init(); sb_txt.init(); sb_alt.init();
  btn_ok.init(); btn_cancel.init();
  cp.init(); hdg_txt.init(); hdg_val.init();
  sk0.init(); sk1.init(); sk2.init(); sk3.init();
  status.init(); hint.init();
  sw_txt.set_text("Autopilot");
  sl_txt.set_text("Brightness");
  sb_txt.set_text("Altitude");
  hdg_txt.set_text("HDG");
  hint.set_text("simulated touch");
  cb_sound.set_checked(true);
  sl_bright.set_value(60);
  sb_alt.set_value(3500);
  sk0.set_active(true);

  // T O U C H   S C R I P T  -  one step per 60 frames: 30 frames move, event at frame 40
  const step_type script[] = {
    { {  18,  28 }, vic::widget::event_tap },        // Sound off
    { {  18,  46 }, vic::widget::event_tap },        // Backlight on
    { { 115,  65 }, vic::widget::event_tap },        // Autopilot on
    { {  40,  98 }, vic::widget::event_tap },        // Brightness low
    { { 120,  98 }, vic::widget::event_tap },        // Brightness high
    { { 137, 117 }, vic::widget::event_tap },        // Altitude + 100
    { { 137, 117 }, vic::widget::event_long_tap },   // Altitude + 1000
    { {  79, 117 }, vic::widget::event_tap },        // Altitude - 100
    { { 221, 183 }, vic::widget::event_tap },        // soft key TER
    { { 260, 183 }, vic::widget::event_tap },        // soft key ENG
    { {  44, 147 }, vic::widget::event_tap },        // OK
    { {  18,  28 }, vic::widget::event_tap },        // Sound on
    { { 115,  65 }, vic::widget::event_tap },        // Autopilot off
    { {  80,  98 }, vic::widget::event_tap },        // Brightness middle
    { { 299, 183 }, vic::widget::event_tap },        // soft key INFO
    { { 182, 183 }, vic::widget::event_tap },        // soft key MAP
    { { 114, 147 }, vic::widget::event_long_tap },   // CANCEL, long
  };
  const std::uint32_t steps = sizeof(script) / sizeof(script[0]);

  vic::vertex_type cursor_pos = { 160, 100 };
  vic::vertex_type from       = cursor_pos;
  for (std::uint32_t t = 0U; ; ++t) {
    const std::uint32_t step  = (t / 60U) % steps;
    const std::uint32_t phase = t % 60U;
    const vic::vertex_type to = script[step].pos;
    if (phase == 0U) {
      from = cursor_pos;
    }
    if (phase < 30U) {
      cursor_pos = { static_cast<std::int16_t>(from.x + (to.x - from.x) * static_cast<std::int32_t>(phase) / 30),
                     static_cast<std::int16_t>(from.y + (to.y - from.y) * static_cast<std::int32_t>(phase) / 30) };
    }
    else {
      cursor_pos = to;
    }
    // the touch driver part: down at frame 40, up and tap at frame 48 (a long tap stays down longer)
    const bool long_tap = script[step].ev == vic::widget::event_long_tap;
    if ((phase == 40U) || (phase == (long_tap ? 56U : 48U))) {
      cursor.render(cursor_pos, 0xFFFFU);    // hide the cursor while the widget below redraws
      if (phase == 40U) {
        vic::widget::base::event(cursor_pos, vic::widget::event_down);
      }
      else {
        vic::widget::base::event(cursor_pos, vic::widget::event_up);
        vic::widget::base::event(cursor_pos, script[step].ev);
      }
    }
    cursor.render(cursor_pos, 0U);

    // readouts
    sl_val.set_value(sl_bright.get_value(), 0U, " %");
    const std::int16_t hdg = static_cast<std::int16_t>((t / 3U) % 360U);
    cp.set_heading(hdg);
    hdg_val.set_value(hdg);

    vic::io::delay(FRAME_MS);
  }
}
