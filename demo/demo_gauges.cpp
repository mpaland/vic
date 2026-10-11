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
// \brief Gauge widget demo: an aircraft engine page
//
// Shows all gauge style widgets on one 320 x 200 screen:
// - three round gauges (widget::gauge) with color bands, ticks and a sprite needle
// - vertical bar gauges (widget::bargauge) with bands and one or two markers, one in
//   fill (bar graph) mode, and a horizontal strip with a marker below
// - labels (widget::label) for names and numeric readouts without floating point
// - group boxes (widget::groupbox) around the fuel and the electrical section
// All values are animated with slow sine waves.
//
// Link this file together with windows/main.cpp (see demo/CMakeLists.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>

#include "dc.h"
#include "widget/gauge.h"
#include "widget/bargauge.h"
#include "widget/label.h"
#include "widget/groupbox.h"

// fonts
#include "font/Montserrat_10_aa2.h"
#include "font/Montserrat_12_aa2.h"
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
const vic::color::value_type COL_BG     = vic::color::black;
const vic::color::value_type COL_RING   = vic::color::gray04;
const vic::color::value_type COL_TEXT   = vic::color::white;
const vic::color::value_type COL_NAME   = vic::color::gray10;
const vic::color::value_type COL_FRAME  = vic::color::gray08;
const vic::color::value_type COL_GREEN  = vic::color::argb(0x20U, 0xD0U, 0x20U);
const vic::color::value_type COL_YELLOW = vic::color::argb(0xF0U, 0xD0U, 0x20U);
const vic::color::value_type COL_RED    = vic::color::argb(0xF0U, 0x20U, 0x20U);
const vic::color::value_type COL_CYAN   = vic::color::argb(0x40U, 0xE0U, 0xFFU);

// fonts
vic::font::Montserrat_10_aa2 font_10;
vic::font::Montserrat_12_aa2 font_12;
vic::font::Montserrat_16_aa4 font_16;

typedef vic::widget::gauge<>    gauge_type;
typedef vic::widget::bargauge<> bar_type;
typedef vic::widget::label      label_type;
typedef vic::widget::groupbox   group_type;


// B A N D S  (values with one decimal are stored times 10)
const vic::widget::band_type bands_rpm[]   = { {  800, 2500, COL_GREEN }, { 2500, 2700, COL_YELLOW }, { 2700, 3000, COL_RED } };
const vic::widget::band_type bands_man[]   = { {  100,  150, COL_YELLOW }, { 150, 300, COL_GREEN }, { 300, 350, COL_RED } };
const vic::widget::band_type bands_oilp[]  = { {    0,   25, COL_RED }, { 25, 50, COL_YELLOW }, { 50, 85, COL_GREEN }, { 85, 100, COL_RED } };
const vic::widget::band_type bands_qty[]   = { {    0,    5, COL_RED }, { 5, 10, COL_YELLOW }, { 10, 50, COL_GREEN } };
const vic::widget::band_type bands_gph[]   = { {  180,  200, COL_RED } };
const vic::widget::band_type bands_fpsi[]  = { {    0,   10, COL_RED }, { 10, 30, COL_YELLOW }, { 30, 80, COL_GREEN }, { 80, 100, COL_RED } };
const vic::widget::band_type bands_volt[]  = { {  100,  110, COL_RED }, { 110, 125, COL_YELLOW }, { 125, 150, COL_GREEN }, { 150, 160, COL_RED } };
const vic::widget::band_type bands_amp[]   = { {  -30,   -5, COL_RED }, { -5, 0, COL_YELLOW }, { 0, 60, COL_GREEN } };
const vic::widget::band_type bands_oilt[]  = { {   50,  100, COL_YELLOW }, { 100, 220, COL_GREEN }, { 220, 250, COL_RED } };

// M A R K E R S
const bar_type::marker_type marker_left[]  = { { COL_TEXT, bar_type::side_before, 4U } };
const bar_type::marker_type marker_right[] = { { COL_TEXT, bar_type::side_after,  4U } };
const bar_type::marker_type marker_dual[]  = { { COL_TEXT, bar_type::side_before, 4U }, { COL_CYAN, bar_type::side_after, 4U } };
const bar_type::marker_type marker_below[] = { { COL_TEXT, bar_type::side_after,  4U } };


// config helpers
gauge_type::config_type gauge_cfg(std::int16_t cx, std::int16_t lower, std::int16_t upper, const vic::widget::band_type* bands, std::uint8_t band_count, std::uint8_t ticks)
{
  const gauge_type::config_type cfg = { { cx, 38 }, 34U, 6U, -120, 120, lower, upper, bands, band_count, ticks, 26U, 5U,
                                        COL_RING, COL_TEXT, COL_TEXT, vic::color::gray12, vic::color::none };
  return cfg;
}

bar_type::config_type vbar_cfg(std::int16_t x, std::int16_t y, std::uint16_t h, std::int16_t lower, std::int16_t upper,
                               const vic::widget::band_type* bands, std::uint8_t band_count,
                               const bar_type::marker_type* markers, std::uint8_t marker_count, vic::color::value_type fill)
{
  const bar_type::config_type cfg = { { x, y }, 14U, h, bar_type::bottom_to_top, 6U, lower, upper, bands, band_count, 0U,
                                      markers, marker_count, fill, COL_RING, COL_BG, COL_BG };
  return cfg;
}

label_type::config_type label_cfg(std::int16_t x, std::int16_t y, std::uint16_t w, std::uint16_t h, const vic::font::font_type& font,
                                  label_type::align_type align, vic::color::value_type color)
{
  const label_type::config_type cfg = { { x, y }, w, h, &font, align, color, COL_BG, vic::color::none };
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

} // namespace


void demo()
{
  head().init();

  vic::dc _dc(head());
  _dc.cls(COL_BG);

  // R O W   1  -  round gauges
  gauge_type rpm (head(), gauge_cfg( 54,   0, 3000, bands_rpm,  3U, 7U));
  gauge_type man (head(), gauge_cfg(160, 100,  350, bands_man,  3U, 6U));
  gauge_type oilp(head(), gauge_cfg(266,   0,  100, bands_oilp, 4U, 5U));
  label_type rpm_name (head(), label_cfg( 34, 46, 40U, 10U, font_10, label_type::align_center, COL_NAME));
  label_type man_name (head(), label_cfg(140, 46, 40U, 10U, font_10, label_type::align_center, COL_NAME));
  label_type oilp_name(head(), label_cfg(246, 46, 40U, 10U, font_10, label_type::align_center, COL_NAME));
  label_type rpm_val  (head(), label_cfg( 24, 58, 60U, 18U, font_16, label_type::align_center, COL_TEXT));
  label_type man_val  (head(), label_cfg(130, 58, 60U, 18U, font_16, label_type::align_center, COL_TEXT));
  label_type oilp_val (head(), label_cfg(236, 58, 60U, 18U, font_16, label_type::align_center, COL_TEXT));

  // R O W   2  -  fuel group: three vertical bars
  const group_type::config_type fuel_cfg = { { 2, 90, 150, 198 }, 4U, &font_10, "FUEL", COL_FRAME, COL_TEXT, COL_BG };
  group_type fuel(head(), fuel_cfg);
  label_type qty_name (head(), label_cfg( 10,  93, 44U, 10U, font_10, label_type::align_center, COL_NAME));
  label_type gph_name (head(), label_cfg( 54,  93, 44U, 10U, font_10, label_type::align_center, COL_NAME));
  label_type fpsi_name(head(), label_cfg( 98,  93, 44U, 10U, font_10, label_type::align_center, COL_NAME));
  label_type qty_val  (head(), label_cfg( 10, 104, 44U, 12U, font_12, label_type::align_center, COL_TEXT));
  label_type gph_val  (head(), label_cfg( 54, 104, 44U, 12U, font_12, label_type::align_center, COL_TEXT));
  label_type fpsi_val (head(), label_cfg( 98, 104, 44U, 12U, font_12, label_type::align_center, COL_TEXT));
  bar_type   qty (head(), vbar_cfg( 25, 118, 74U,   0,  50, bands_qty,  3U, marker_left,  1U, vic::color::none));
  bar_type   gph (head(), vbar_cfg( 69, 118, 74U,   0, 200, bands_gph,  1U, marker_right, 1U, COL_GREEN));
  bar_type   fpsi(head(), vbar_cfg(113, 118, 74U,   0, 100, bands_fpsi, 4U, marker_left,  1U, vic::color::none));

  // R O W   2  -  electrical group: two dual marker bars and a horizontal strip
  const group_type::config_type elec_cfg = { { 158, 90, 318, 198 }, 4U, &font_10, "ELECTRICAL", COL_FRAME, COL_TEXT, COL_BG };
  group_type elec(head(), elec_cfg);
  label_type volt_name(head(), label_cfg(166,  93, 64U, 10U, font_10, label_type::align_center, COL_NAME));
  label_type amp_name (head(), label_cfg(242,  93, 64U, 10U, font_10, label_type::align_center, COL_NAME));
  // dual readouts: left value in white, right value in the color of the second marker
  label_type volt_val (head(), label_cfg(166, 104, 32U, 12U, font_10, label_type::align_center, COL_TEXT));
  label_type volt_val2(head(), label_cfg(198, 104, 32U, 12U, font_10, label_type::align_center, COL_CYAN));
  label_type amp_val  (head(), label_cfg(242, 104, 32U, 12U, font_10, label_type::align_center, COL_TEXT));
  label_type amp_val2 (head(), label_cfg(274, 104, 32U, 12U, font_10, label_type::align_center, COL_CYAN));
  bar_type   volt(head(), vbar_cfg(191, 118, 42U, 100, 160, bands_volt, 4U, marker_dual, 2U, vic::color::none));
  bar_type   amp (head(), vbar_cfg(267, 118, 42U, -30,  60, bands_amp,  3U, marker_dual, 2U, vic::color::none));
  label_type oilt_name(head(), label_cfg(170, 162, 60U, 10U, font_10, label_type::align_left,  COL_NAME));
  label_type oilt_val (head(), label_cfg(246, 162, 60U, 10U, font_10, label_type::align_right, COL_TEXT));
  const bar_type::config_type oilt_cfg = { { 170, 174 }, 136U, 16U, bar_type::left_to_right, 5U, 50, 250, bands_oilt, 3U, 5U,
                                           marker_below, 1U, vic::color::none, COL_RING, COL_BG, COL_BG };
  bar_type   oilt(head(), oilt_cfg);

  // anti aliased scales for the round gauges
  rpm.anti_aliasing_enable();
  man.anti_aliasing_enable();
  oilp.anti_aliasing_enable();

  // init all widgets, group boxes first
  fuel.init();
  elec.init();
  rpm.init();      man.init();      oilp.init();
  rpm_name.init(); man_name.init(); oilp_name.init();
  rpm_val.init();  man_val.init();  oilp_val.init();
  qty_name.init(); gph_name.init(); fpsi_name.init();
  qty_val.init();  gph_val.init();  fpsi_val.init();
  qty.init();      gph.init();      fpsi.init();
  volt_name.init(); amp_name.init();
  volt_val.init();  volt_val2.init(); amp_val.init(); amp_val2.init();
  volt.init();      amp.init();
  oilt_name.init(); oilt_val.init(); oilt.init();

  // static texts
  rpm_name.set_text("RPM");
  man_name.set_text("MAN IN");
  oilp_name.set_text("OIL PSI");
  qty_name.set_text("QTY");
  gph_name.set_text("GPH");
  fpsi_name.set_text("PSI");
  volt_name.set_text("VOLTS");
  amp_name.set_text("AMPS");
  oilt_name.set_text("OIL \xC2\xB0" "F");

  // A N I M A T I O N
  for (std::uint32_t t = 0U; ; ++t) {
    const std::int16_t v_rpm  = wave(t, 400U,   0,  600, 2900);
    const std::int16_t v_man  = wave(t, 300U,  90,  120,  330);
    const std::int16_t v_oilp = wave(t, 500U, 200,   20,   95);
    const std::int16_t v_qty  = wave(t, 900U,  45,    2,   48);
    const std::int16_t v_gph  = wave(t, 250U, 180,   30,  195);
    const std::int16_t v_fpsi = wave(t, 350U, 300,    5,   95);
    const std::int16_t v_volt = wave(t, 600U,  20,  105,  155);
    const std::int16_t v_vol2 = wave(t, 600U,  60,  105,  155);
    const std::int16_t v_amp  = wave(t, 450U, 120,  -25,   55);
    const std::int16_t v_amp2 = wave(t, 450U, 150,  -25,   55);
    const std::int16_t v_oilt = wave(t, 700U, 270,   60,  240);

    rpm.set_value(v_rpm);     rpm_val.set_value(v_rpm);
    man.set_value(v_man);     man_val.set_value(v_man, 1U);
    oilp.set_value(v_oilp);   oilp_val.set_value(v_oilp);
    qty.set_value(0U, v_qty); qty_val.set_value(v_qty);
    gph.set_value(0U, v_gph); gph_val.set_value(v_gph, 1U);
    fpsi.set_value(0U, v_fpsi); fpsi_val.set_value(v_fpsi, 1U);
    volt.set_value(0U, v_volt); volt_val.set_value(v_volt, 1U);
    volt.set_value(1U, v_vol2); volt_val2.set_value(v_vol2, 1U);
    amp.set_value(0U, v_amp);   amp_val.set_value(v_amp);
    amp.set_value(1U, v_amp2);  amp_val2.set_value(v_amp2);
    oilt.set_value(0U, v_oilt); oilt_val.set_value(v_oilt);

    vic::io::delay(FRAME_MS);
  }
}
