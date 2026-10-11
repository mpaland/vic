///////////////////////////////////////////////////////////////////////////////
// \brief Tests for the widgets: progress bar and bitmap
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "helpers.h"
#include "widget/progress.h"
#include "widget/bitmap.h"
#include "widget/label.h"
#include "widget/groupbox.h"
#include "widget/bargauge.h"
#include "widget/gauge.h"
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
#include "font/LCD_6x8.h"
#include <string.h>
#include "head/memory.h"

using namespace vic;

namespace {

typedef head::memory<24, 10> head_type;

// B = bar, G = background, R = border
const vic_test::palette PAL_BAR = { { { 'B', color::red }, { 'G', color::darkgray }, { 'R', color::white } }, 3U, color::none };

widget::progress::config_type make_config(widget::progress::orientation_type o, bool border)
{
  const widget::progress::config_type cfg = { { 2, 1 }, 20U, 5U, o, 0, 100, color::red, color::darkgray, border ? color::white : color::none };
  return cfg;
}

} // namespace


TEST_CASE(widget_progress_left_to_right)
{
  head_type h;
  widget::progress p(h, make_config(widget::progress::left_to_right, false));
  p.init();
  CHECK_EQ(p.get_pos(), 0);
  // empty bar: everything background, all 5 rows and 20 columns
  CHECK_RASTER(h, PAL_BAR,
    "........................",
    "..GGGGGGGGGGGGGGGGGGGG..",
    "..GGGGGGGGGGGGGGGGGGGG..",
    "..GGGGGGGGGGGGGGGGGGGG..",
    "..GGGGGGGGGGGGGGGGGGGG..",
    "..GGGGGGGGGGGGGGGGGGGG..",
    "........................",
    "........................",
    "........................",
    "........................");
  CHECK_EQ(h.present_count(), 1U);

  p.set_pos(50);
  CHECK_EQ(p.get_pos(), 50);
  CHECK_RASTER(h, PAL_BAR,
    "........................",
    "..BBBBBBBBBBGGGGGGGGGG..",
    "..BBBBBBBBBBGGGGGGGGGG..",
    "..BBBBBBBBBBGGGGGGGGGG..",
    "..BBBBBBBBBBGGGGGGGGGG..",
    "..BBBBBBBBBBGGGGGGGGGG..",
    "........................",
    "........................",
    "........................",
    "........................");
  CHECK_EQ(h.present_count(), 2U);

  // full and empty
  p.set_pos(100);
  CHECK_EQ(vic_test::count_pixels(h, color::red), 100U);
  CHECK_EQ(vic_test::count_pixels(h, color::darkgray), 0U);
  p.set_pos(0);
  CHECK_EQ(vic_test::count_pixels(h, color::red), 0U);
  CHECK_EQ(vic_test::count_pixels(h, color::darkgray), 100U);

  // out of range positions are clamped
  p.set_pos(150);
  CHECK_EQ(p.get_pos(), 100);
  CHECK_EQ(vic_test::count_pixels(h, color::red), 100U);
  p.set_pos(-5);
  CHECK_EQ(p.get_pos(), 0);

  // no present without change
  h.reset_counters();
  p.set_pos(0);
  CHECK_EQ(h.present_count(), 1U);
  CHECK_EQ(h.pixel_set_count(), 0U);
}


TEST_CASE(widget_progress_delta_equals_redraw)
{
  // the delta drawing of set_pos must produce the same pixels as a full redraw
  for (int o = 0; o < 4; ++o) {
    for (int border = 0; border < 2; ++border) {
      head_type a, b;
      widget::progress pa(a, make_config(static_cast<widget::progress::orientation_type>(o), border != 0));
      widget::progress pb(b, make_config(static_cast<widget::progress::orientation_type>(o), border != 0));
      pa.init();
      pb.init();
      pa.set_pos(80);
      pa.set_pos(30);
      pa.set_pos(55);
      pb.set_pos(55);
      pb.redraw();
      CHECK_SAME_BUFFER(a, b);
      // step_it clamps at the range
      pa.set_step(30);
      pa.step_it();
      CHECK_EQ(pa.get_pos(), 85);
      pa.step_it();
      CHECK_EQ(pa.get_pos(), 100);
      pa.set_step(-200);
      pa.step_it();
      CHECK_EQ(pa.get_pos(), 0);
      pb.set_pos(0);
      CHECK_SAME_BUFFER(a, b);
    }
  }
}


TEST_CASE(widget_progress_orientations_and_border)
{
  head_type h;
  widget::progress p(h, make_config(widget::progress::right_to_left, true));
  p.init();
  p.set_pos(50);
  // border is one pixel, the inner bar is 18 x 3, half of it (9) is filled from the right
  CHECK_RASTER(h, PAL_BAR,
    "........................",
    "..RRRRRRRRRRRRRRRRRRRR..",
    "..RGGGGGGGGGBBBBBBBBBR..",
    "..RGGGGGGGGGBBBBBBBBBR..",
    "..RGGGGGGGGGBBBBBBBBBR..",
    "..RRRRRRRRRRRRRRRRRRRR..",
    "........................",
    "........................",
    "........................",
    "........................");

  head_type v;
  widget::progress::config_type cfg = { { 1, 1 }, 4U, 8U, widget::progress::bottom_to_top, 0, 100, color::red, color::darkgray, color::none };
  widget::progress pv(v, cfg);
  pv.init();
  pv.set_pos(25);
  CHECK_RASTER_PART(v, PAL_BAR, (vertex_type{ 1, 1 }),
    "GGGG",
    "GGGG",
    "GGGG",
    "GGGG",
    "GGGG",
    "GGGG",
    "BBBB",
    "BBBB");
  cfg.orientation = widget::progress::top_to_bottom;
  head_type t;
  widget::progress pt(t, cfg);
  pt.init();
  pt.set_pos(75);
  CHECK_RASTER_PART(t, PAL_BAR, (vertex_type{ 1, 1 }),
    "BBBB",
    "BBBB",
    "BBBB",
    "BBBB",
    "BBBB",
    "BBBB",
    "GGGG",
    "GGGG");
  CHECK_EQ(vic_test::count_pixels_not(t, color::none), 32U);
}


TEST_CASE(widget_progress_edge_cases)
{
  // zero range must not divide by zero
  head_type h;
  widget::progress::config_type cfg = { { 0, 0 }, 10U, 2U, widget::progress::left_to_right, 5, 5, color::red, color::darkgray, color::none };
  widget::progress p(h, cfg);
  p.init();
  p.set_pos(5);
  p.step_it();
  CHECK_EQ(vic_test::count_pixels(h, color::darkgray), 20U);
  // set_range repairs it
  p.set_range(0, 10);
  p.set_pos(5);
  CHECK_EQ(vic_test::count_pixels(h, color::red), 10U);
  // an invalid range is ignored
  p.set_range(10, 0);
  CHECK_EQ(p.get_pos(), 5);
  // no background color: the background is left untouched
  head_type n;
  widget::progress::config_type cfg2 = { { 0, 0 }, 10U, 2U, widget::progress::left_to_right, 0, 10, color::red, color::none, color::none };
  widget::progress pn(n, cfg2);
  pn.init();
  pn.set_pos(5);
  CHECK_EQ(vic_test::count_pixels(n, color::red), 10U);
  CHECK_EQ(vic_test::count_pixels(n, color::none), 24U * 10U - 10U);
  pn.set_pos(2);
  CHECK_EQ(vic_test::count_pixels(n, color::red), 10U);   // nothing is erased without a background color
  // a tiny bar with border
  head_type s;
  widget::progress::config_type cfg3 = { { 0, 0 }, 2U, 2U, widget::progress::left_to_right, 0, 10, color::red, color::darkgray, color::white };
  widget::progress ps(s, cfg3);
  ps.init();
  ps.set_pos(10);
  CHECK_EQ(vic_test::count_pixels(s, color::white), 4U);
  // set_color redraws
  p.set_color(color::green);
  CHECK_EQ(vic_test::count_pixels(h, color::green), 10U);
}


TEST_CASE(widget_find)
{
  head_type h;
  widget::progress p(h, make_config(widget::progress::left_to_right, false));
  p.init();
  // inside is exclusive at right/bottom
  CHECK_EQ(static_cast<const void*>(p.find({ 2, 1 })), static_cast<const void*>(&p));
  CHECK_EQ(static_cast<const void*>(p.find({ 21, 5 })), static_cast<const void*>(&p));
  CHECK_EQ(static_cast<const void*>(p.find({ 22, 5 })), static_cast<const void*>(nullptr));
  CHECK_EQ(static_cast<const void*>(p.find({ 2, 6 })), static_cast<const void*>(nullptr));
  CHECK_EQ(static_cast<const void*>(p.find({ 1, 1 })), static_cast<const void*>(nullptr));
}


///////////////////////////////////////////////////////////////////////////////
// B I T M A P

TEST_CASE(widget_bitmap_raw)
{
  head_type h;
  dc d(h);
  widget::bitmap b(h);
  b.init();
  b.set_color(color::white);

  // L1: 10 x 2 pixel, rows are byte aligned, LSB first
  static const std::uint8_t l1[] = { 0x05U, 0x02U, 0xF0U, 0x01U };
  b.render_raw({ 1, 1 }, 10U, 2U, color::format_L1, l1);
  CHECK_RASTER_PART(h, vic_test::PAL_X, (vertex_type{ 1, 1 }),
    "X.X......X",
    "....XXXXX.");
  CHECK_EQ(h.present_count(), 1U);
  CHECK_EQ(vic_test::count_pixels(h, color::white), 8U);
  CHECK_EQ(static_cast<const void*>(b.find({ 10, 2 })), static_cast<const void*>(&b));
  CHECK_EQ(static_cast<const void*>(b.find({ 11, 2 })), static_cast<const void*>(nullptr));

  // RGB332
  d.cls();
  static const std::uint8_t rgb332[] = { 0xE0U, 0x03U };
  b.render_raw({ 0, 0 }, 2U, 1U, color::format_RGB332, rgb332);
  CHECK_EQ(h.pixel({ 0, 0 }), 0xFFE00000UL);
  CHECK_EQ(h.pixel({ 1, 0 }), 0xFF0000C0UL);

  // RGB565 and RGB555, little endian
  static const std::uint8_t rgb565[] = { 0x00U, 0xF8U, 0xE0U, 0x07U };
  b.render_raw({ 0, 1 }, 2U, 1U, color::format_RGB565, rgb565);
  CHECK_EQ(h.pixel({ 0, 1 }), 0xFFF80000UL);
  CHECK_EQ(h.pixel({ 1, 1 }), 0xFF00FC00UL);
  static const std::uint8_t rgb555[] = { 0x00U, 0x7CU, 0x1FU, 0x00U };
  b.render_raw({ 0, 2 }, 2U, 1U, color::format_RGB555, rgb555);
  CHECK_EQ(h.pixel({ 0, 2 }), 0xFFF80000UL);
  CHECK_EQ(h.pixel({ 1, 2 }), 0xFF0000F8UL);

  // RGB888 (B G R), exactly 6 bytes for two pixels
  static const std::uint8_t rgb888[6] = { 0x00U, 0x00U, 0xFFU, 0x12U, 0x34U, 0x56U };
  b.render_raw({ 0, 3 }, 2U, 1U, color::format_RGB888, rgb888);
  CHECK_EQ(h.pixel({ 0, 3 }), color::red);
  CHECK_EQ(h.pixel({ 1, 3 }), 0xFF563412UL);

  // ARGB8888 (B G R A): the half transparent pixel is blended with the background
  d.cls(color::blue);
  static const std::uint8_t argb[8] = { 0x00U, 0x00U, 0xFFU, 0xFFU, 0x00U, 0x00U, 0xFFU, 0x80U };
  b.render_raw({ 0, 4 }, 2U, 1U, color::format_ARGB8888, argb);
  CHECK_EQ(h.pixel({ 0, 4 }), color::red);
  const color::value_type m = h.pixel({ 1, 4 });
  CHECK(color::is_opaque(m));
  CHECK(color::get_red(m) >= 127U && color::get_red(m) <= 128U);
  CHECK(color::get_blue(m) >= 127U && color::get_blue(m) <= 128U);

  // unsupported format renders nothing
  d.cls();
  b.render_raw({ 0, 0 }, 2U, 1U, color::format_L4, rgb332);
  CHECK_EQ(vic_test::count_pixels_not(h, color::none), 0U);
}


TEST_CASE(widget_bitmap_xpm)
{
  head_type h;
  widget::bitmap b(h);
  // 4 x 2 image, 3 colors, one char per pixel, lower and upper case hex, transparent "None"
  static const char* xpm[] = {
    "4 2 3 1",
    "a c #ff0000",
    "b\tc\t#00FF00",
    "  c None",
    "aabb",
    "b  a"
  };
  b.render_xpm({ 2, 1 }, xpm);
  CHECK_RASTER_AT(h, vic_test::PAL_RG, (vertex_type{ 2, 1 }),
    "RRGG",
    "G..R");
  CHECK_EQ(h.present_count(), 1U);
  CHECK_EQ(static_cast<const void*>(b.find({ 5, 2 })), static_cast<const void*>(&b));
  CHECK_EQ(static_cast<const void*>(b.find({ 6, 2 })), static_cast<const void*>(nullptr));

  // two chars per pixel
  static const char* xpm2[] = {
    "2 1 2 2",
    "a. c #0000FF",
    ".. c None",
    "a..."
  };
  head_type h2;
  widget::bitmap b2(h2);
  b2.render_xpm({ 0, 0 }, xpm2);
  CHECK_EQ(h2.pixel({ 0, 0 }), color::blue);
  CHECK_EQ(h2.pixel({ 1, 0 }), color::none);

  // more chars per pixel than the CLUT can hold still works
  static const char* xpm3[] = {
    "2 1 2 5",
    "aaaaa c #FFFFFF",
    "bbbbb c #000000",
    "aaaaabbbbb"
  };
  head_type h3;
  widget::bitmap b3(h3);
  b3.render_xpm({ 0, 0 }, xpm3);
  CHECK_EQ(h3.pixel({ 0, 0 }), color::white);
  CHECK_EQ(h3.pixel({ 1, 0 }), color::black);
}


///////////////////////////////////////////////////////////////////////////////
// L A B E L
//

namespace {

vic::font::LCD_6x8 test_font;

widget::label::config_type label_config(widget::label::align_type align, color::value_type bg, color::value_type border)
{
  const widget::label::config_type cfg = { { 0, 0 }, 24U, 10U, &test_font, align, color::white, bg, border };
  return cfg;
}

} // namespace


TEST_CASE(widget_label_text_and_alignment)
{
  head_type h;
  widget::label l(h, label_config(widget::label::align_left, color::black, color::none));
  l.init();
  // empty label: the cell is cleared
  CHECK_EQ(vic_test::count_pixels(h, color::black), 24U * 10U);
  CHECK_EQ(h.present_count(), 1U);

  l.set_text("A");
  CHECK(vic_test::count_pixels(h, color::white) > 0U);
  CHECK_EQ(h.present_count(), 2U);
  // left aligned: the glyph is in the first 6 columns, vertically centered (rows 1 - 8)
  for (std::int16_t y = 0; y < 10; ++y) {
    for (std::int16_t x = 6; x < 24; ++x) {
      CHECK_EQ(h.pixel({ x, y }), color::black);
    }
  }
  CHECK_EQ(h.pixel({ 0, 0 }), color::black);
  CHECK_EQ(h.pixel({ 0, 9 }), color::black);

  // same text again: nothing is drawn
  h.reset_counters();
  l.set_text("A");
  CHECK_EQ(h.present_count(), 0U);
  CHECK_EQ(h.pixel_set_count(), 0U);

  // right aligned: the glyph is in the last 6 columns, the old text is erased
  widget::label r(h, label_config(widget::label::align_right, color::black, color::none));
  r.init();
  r.set_text("A");
  for (std::int16_t y = 0; y < 10; ++y) {
    for (std::int16_t x = 0; x < 18; ++x) {
      CHECK_EQ(h.pixel({ x, y }), color::black);
    }
  }
  CHECK(vic_test::count_pixels(h, color::white) > 0U);

  // centered: columns 9 - 14
  widget::label c(h, label_config(widget::label::align_center, color::black, color::none));
  c.init();
  c.set_text("A");
  for (std::int16_t y = 0; y < 10; ++y) {
    for (std::int16_t x = 0; x < 9; ++x) {
      CHECK_EQ(h.pixel({ x, y }), color::black);
    }
    for (std::int16_t x = 15; x < 24; ++x) {
      CHECK_EQ(h.pixel({ x, y }), color::black);
    }
  }

  // border
  widget::label b(h, label_config(widget::label::align_left, color::black, color::red));
  b.init();
  CHECK_EQ(vic_test::count_pixels(h, color::red), 24U * 2U + 8U * 2U);
  CHECK(b.find({ 5, 5 }) != nullptr);
}


TEST_CASE(widget_label_value)
{
  head_type h;
  widget::label l(h, label_config(widget::label::align_left, color::black, color::none));
  l.init();

  l.set_value(0);
  CHECK_EQ(strcmp(l.get_text(), "0"), 0);
  l.set_value(2100);
  CHECK_EQ(strcmp(l.get_text(), "2100"), 0);
  l.set_value(-5);
  CHECK_EQ(strcmp(l.get_text(), "-5"), 0);
  l.set_value(270, 1U);
  CHECK_EQ(strcmp(l.get_text(), "27.0"), 0);
  l.set_value(-4, 1U);
  CHECK_EQ(strcmp(l.get_text(), "-0.4"), 0);
  l.set_value(0, 2U);
  CHECK_EQ(strcmp(l.get_text(), "0.00"), 0);
  l.set_value(12345, 3U);
  CHECK_EQ(strcmp(l.get_text(), "12.345"), 0);
  l.set_value(140, 0U, "KT");
  CHECK_EQ(strcmp(l.get_text(), "140KT"), 0);
  l.set_value(-2147483647 - 1);
  CHECK_EQ(strcmp(l.get_text(), "-2147483648"), 0);

  // texts are truncated to the maximum length
  l.set_text("0123456789012345678901234567890123456789012345678901234567890123456789");
  CHECK_EQ(strlen(l.get_text()), static_cast<std::size_t>(VIC_WIDGET_LABEL_MAX_LEN));
}


///////////////////////////////////////////////////////////////////////////////
// G R O U P B O X
//

TEST_CASE(widget_groupbox)
{
  head_type h;
  widget::groupbox::config_type cfg = { { 0, 0, 24, 10 }, 0U, &test_font, nullptr, color::white, color::red, color::black };
  widget::groupbox g(h, cfg);
  g.init();
  // plain frame without title
  CHECK_EQ(vic_test::count_pixels(h, color::white), 24U * 2U + 8U * 2U);
  CHECK_EQ(vic_test::count_pixels(h, color::black), 22U * 8U);
  CHECK_EQ(h.present_count(), 1U);
  CHECK(g.find({ 5, 5 }) == &g);
  CHECK(g.find({ 24, 5 }) == nullptr);

  // title: cut into the top border at x = left + radius + 4 - 2
  g.set_title("A");
  CHECK_EQ(h.pixel({ 1, 0 }), color::white);
  CHECK_EQ(h.pixel({ 2, 0 }), color::black);
  CHECK_EQ(h.pixel({ 11, 0 }), color::black);
  CHECK_EQ(h.pixel({ 12, 0 }), color::white);
  CHECK(vic_test::count_pixels(h, color::red) > 0U);

  // rounded frame: the corners are not drawn
  head_type r;
  cfg.radius = 3U;
  cfg.title  = nullptr;
  widget::groupbox gr(r, cfg);
  gr.init();
  CHECK_EQ(r.pixel({ 0, 0 }), color::none);
  CHECK_EQ(r.pixel({ 23, 9 }), color::none);
  CHECK_EQ(r.pixel({ 10, 0 }), color::white);
  CHECK_EQ(r.pixel({ 0, 5 }), color::white);
}


///////////////////////////////////////////////////////////////////////////////
// B A R G A U G E
//

namespace {

typedef widget::bargauge<> bar_type;

// R = red band, G = green band, W = marker, F = fill
const vic_test::palette PAL_GAUGE = { { { 'R', color::red }, { 'G', color::green }, { 'W', color::white }, { 'F', color::blue } }, 4U, color::none };

const widget::band_type   bands_rg[]  = { { 0, 50, color::red }, { 50, 100, color::green } };
const bar_type::marker_type marker_b[] = { { color::white, bar_type::side_after,  3U } };
const bar_type::marker_type marker_2[] = { { color::white, bar_type::side_before, 3U }, { color::white, bar_type::side_after, 3U } };

bar_type::config_type bar_config(bar_type::orientation_type o, const bar_type::marker_type* markers, std::uint8_t marker_count, color::value_type fill)
{
  const bar_type::config_type cfg = { { 0, 0 }, 20U, 10U, o, 4U, 0, 100, bands_rg, 2U, 0U, markers, marker_count, fill, color::gray, color::black, color::none };
  return cfg;
}

} // namespace


TEST_CASE(widget_bargauge_bands_and_marker)
{
  head_type h;
  bar_type b(h, bar_config(bar_type::left_to_right, marker_b, 1U, color::none));
  b.init();
  CHECK_EQ(b.get_value(0U), 0);
  // strip rows 3 - 6, marker below the strip at the left end (clipped at the screen edge)
  CHECK_RASTER(h, PAL_GAUGE,
    "........................",
    "........................",
    "........................",
    "RRRRRRRRRRGGGGGGGGGG....",
    "RRRRRRRRRRGGGGGGGGGG....",
    "RRRRRRRRRRGGGGGGGGGG....",
    "RRRRRRRRRRGGGGGGGGGG....",
    "W.......................",
    "WW......................",
    "WWW.....................");

  b.set_value(0U, 50);
  CHECK_EQ(b.get_value(0U), 50);
  CHECK_RASTER(h, PAL_GAUGE,
    "........................",
    "........................",
    "........................",
    "RRRRRRRRRRGGGGGGGGGG....",
    "RRRRRRRRRRGGGGGGGGGG....",
    "RRRRRRRRRRGGGGGGGGGG....",
    "RRRRRRRRRRGGGGGGGGGG....",
    "..........W.............",
    ".........WWW............",
    "........WWWWW...........");

  // full scale: the tip points at the last strip column
  b.set_value(0U, 100);
  CHECK_EQ(h.pixel({ 19, 7 }), color::white);
  CHECK_EQ(h.pixel({ 10, 7 }), color::none);
  // clamping
  b.set_value(0U, 150);
  CHECK_EQ(b.get_value(0U), 100);
  b.set_value(0U, -5);
  CHECK_EQ(b.get_value(0U), 0);
  CHECK_EQ(h.pixel({ 0, 7 }), color::white);
  // invalid marker is ignored
  b.set_value(1U, 50);
  CHECK_EQ(h.pixel({ 10, 7 }), color::none);
  // no present without change
  h.reset_counters();
  b.set_value(0U, 0);
  CHECK_EQ(h.present_count(), 0U);
}


TEST_CASE(widget_bargauge_fill_delta_equals_redraw)
{
  // all sprites share one chain, so the widgets of the two heads must not live at the same time
  for (int o = 0; o < 4; ++o) {
    head_type a, b;
    {
      bar_type pa(a, bar_config(static_cast<bar_type::orientation_type>(o), marker_2, 2U, color::blue));
      pa.init();
      pa.set_value(0U, 80);
      pa.set_value(1U, 30);
      pa.set_value(0U, 30);
      pa.set_value(0U, 55);
      pa.set_value(1U, 70);
      // the fill covers the bands up to the value, the bands show in the rest
      CHECK(vic_test::count_pixels(a, color::blue) > 0U);
      CHECK(vic_test::count_pixels(a, color::green) > 0U);
      CHECK_EQ(vic_test::count_pixels(a, color::red), 0U);
    }
    {
      bar_type pb(b, bar_config(static_cast<bar_type::orientation_type>(o), marker_2, 2U, color::blue));
      pb.init();
      pb.set_value(0U, 55);
      pb.set_value(1U, 70);
      pb.redraw();
    }
    CHECK_SAME_BUFFER(a, b);

    // back to zero and a new range
    {
      bar_type pa(a, bar_config(static_cast<bar_type::orientation_type>(o), marker_2, 2U, color::blue));
      pa.init();
      pa.set_value(0U, 90);
      pa.set_value(1U, 70);
      pa.set_value(0U, 0);
      CHECK_EQ(vic_test::count_pixels(a, color::blue), 0U);
      pa.set_range(0, 10);
      pa.set_value(0U, 5);
    }
    {
      bar_type pb(b, bar_config(static_cast<bar_type::orientation_type>(o), marker_2, 2U, color::blue));
      pb.init();
      pb.set_range(0, 10);
      pb.set_value(0U, 5);
    }
    CHECK_SAME_BUFFER(a, b);
  }
}


TEST_CASE(widget_bargauge_vertical_fill)
{
  head_type h;
  bar_type::config_type cfg = { { 1, 0 }, 10U, 10U, bar_type::bottom_to_top, 4U, 0, 100, nullptr, 0U, 0U, marker_b, 1U, color::blue, color::gray, color::black, color::none };
  bar_type b(h, cfg);
  b.init();
  b.set_value(0U, 50);
  // strip columns 4 - 7, filled from the bottom to the middle, marker right of the strip (tip at x = 8, row 4)
  const vic_test::palette pal = { { { 'S', color::gray }, { 'F', color::blue }, { 'W', color::white } }, 3U, color::none };
  CHECK_RASTER(h, pal,
    "....SSSS................",
    "....SSSS................",
    "....SSSS..W.............",
    "....SSSS.WW.............",
    "....SSSSWWW.............",
    "....FFFF.WW.............",
    "....FFFF..W.............",
    "....FFFF................",
    "....FFFF................",
    "....FFFF................");
}


///////////////////////////////////////////////////////////////////////////////
// G A U G E
//

TEST_CASE(widget_gauge_scale_and_needle)
{
  head::memory<40, 40> h;
  const widget::band_type bands[] = { { 0, 50, color::red }, { 50, 100, color::green } };
  // half circle from 9 o'clock over the top to 3 o'clock
  widget::gauge<>::config_type cfg = { { 20, 20 }, 16U, 4U, -90, 90, 0, 100, bands, 2U, 3U, 12U, 2U,
                                       color::gray, color::white, color::white, color::none, color::none };
  head::memory<40, 40> r;
  {
    widget::gauge<> g(h, cfg);
    g.init();
    CHECK_EQ(g.get_value(), 0);

    // bands: just left of 12 o'clock is red (the tick is at 12 o'clock), 45 degree is green, -45 degree is red
    CHECK_EQ(h.pixel({ 18, 6 }), color::red);
    CHECK_EQ(h.pixel({ 30, 10 }), color::green);
    CHECK_EQ(h.pixel({ 10, 10 }), color::red);
    // the lower half is untouched
    CHECK_EQ(h.pixel({ 20, 34 }), color::none);
    CHECK_EQ(h.pixel({ 10, 30 }), color::none);
    // ticks cross the ring at both ends and the middle (outside the ring)
    CHECK_EQ(h.pixel({ 20, 3 }), color::white);
    CHECK_EQ(h.pixel({ 37, 20 }), color::white);
    CHECK_EQ(h.pixel({ 3, 20 }), color::white);

    // needle at value 0 points to 9 o'clock
    CHECK(h.pixel({ 12, 20 }) != color::none);
    CHECK(h.pixel({ 28, 20 }) == color::none);
    CHECK(g.find({ 20, 20 }) == &g);
    CHECK(g.find({ 38, 38 }) == nullptr);

    // move to full scale: the old needle is restored
    g.set_value(100);
    CHECK(h.pixel({ 12, 20 }) == color::none);
    CHECK(h.pixel({ 28, 20 }) != color::none);
    g.set_value(50);
    CHECK(h.pixel({ 28, 20 }) == color::none);
    CHECK(h.pixel({ 20, 12 }) != color::none);
    CHECK(h.pixel({ 20, 12 }) != color::red);

    // no present without change
    h.reset_counters();
    g.set_value(50);
    CHECK_EQ(h.present_count(), 0U);

    // clamping
    g.set_value(200);
    CHECK_EQ(g.get_value(), 100);
    g.set_value(50);
    g.redraw();

  }
  // a fresh gauge reproduces the same picture (sprites share one chain, so the first gauge is gone first)
  {
    widget::gauge<> gr(r, cfg);
    gr.init();
    gr.set_value(50);
  }
  CHECK_SAME_BUFFER(h, r);
}


TEST_CASE(widget_gauge_anti_aliasing)
{
  head::memory<40, 40> h;
  dc d(h);
  d.cls(color::black);
  // plain white ring without bands, short needle so the ring region (radius 12 - 16) only holds scale pixels
  widget::gauge<>::config_type cfg = { { 20, 20 }, 16U, 4U, -90, 90, 0, 100, nullptr, 0U, 3U, 6U, 2U,
                                       color::white, color::white, color::white, color::none, color::none };
  widget::gauge<> g(h, cfg);
  g.init();

  // count the blended (neither black nor white) pixels outside radius 10
  std::size_t blended = 0U;
  for (std::int16_t y = 0; y < 40; ++y) {
    for (std::int16_t x = 0; x < 40; ++x) {
      const color::value_type c = h.pixel({ x, y });
      if ((util::distance_squared({ x, y }, { 20, 20 }) > 100U) && (c != color::black) && (c != color::white)) {
        ++blended;
      }
    }
  }
  // anti aliasing is off by default
  CHECK_EQ(blended, 0U);
  CHECK_EQ(h.pixel({ 30, 10 }), color::white);

  // switch it on: the scale is redrawn with blended edge pixels, interior ring pixels stay white
  g.anti_aliasing_enable();
  blended = 0U;
  for (std::int16_t y = 0; y < 40; ++y) {
    for (std::int16_t x = 0; x < 40; ++x) {
      const color::value_type c = h.pixel({ x, y });
      if ((util::distance_squared({ x, y }, { 20, 20 }) > 100U) && (c != color::black) && (c != color::white)) {
        ++blended;
      }
    }
  }
  CHECK(blended > 0U);
  CHECK_EQ(h.pixel({ 30, 10 }), color::white);
  CHECK_EQ(h.pixel({ 10, 10 }), color::white);
}


///////////////////////////////////////////////////////////////////////////////
// A R C
//

TEST_CASE(widget_arc_delta_equals_redraw)
{
  // a full ring from 0 to 100, delta drawing must give the same pixels as a redraw
  head::memory<40, 40> a, b;
  const widget::arc::config_type cfg = { { 20, 20 }, 16U, 5U, -180, 180, 0, 100, color::green, color::gray, color::none };
  widget::arc pa(a, cfg);
  widget::arc pb(b, cfg);
  pa.init();
  pb.init();
  CHECK_EQ(pa.get_value(), 0);
  // empty: only track pixels
  CHECK_EQ(vic_test::count_pixels(a, color::green), 0U);
  CHECK(vic_test::count_pixels(a, color::gray) > 0U);
  const std::size_t ring = vic_test::count_pixels(a, color::gray);

  pa.set_value(80);
  pa.set_value(30);
  pa.set_value(55);
  pb.set_value(55);
  CHECK_SAME_BUFFER(a, b);
  // the ring pixel count stays constant, arc + track
  CHECK_EQ(vic_test::count_pixels(a, color::green) + vic_test::count_pixels(a, color::gray), ring);
  // -180 degree (6 o'clock) is the start, clockwise over 9 o'clock, so 55 % covers the left half and a bit past 12
  CHECK_EQ(a.pixel({ 7, 20 }), color::green);
  CHECK_EQ(a.pixel({ 33, 20 }), color::gray);
  CHECK_EQ(a.pixel({ 20, 7 }), color::green);

  // full and empty
  pa.set_value(100);
  CHECK_EQ(vic_test::count_pixels(a, color::gray), 0U);
  CHECK_EQ(vic_test::count_pixels(a, color::green), ring);
  pa.set_value(0);
  CHECK_EQ(vic_test::count_pixels(a, color::green), 0U);
  CHECK_EQ(vic_test::count_pixels(a, color::gray), ring);

  // clamping and no present without change
  pa.set_value(200);
  CHECK_EQ(pa.get_value(), 100);
  a.reset_counters();
  pa.set_value(100);
  CHECK_EQ(a.present_count(), 0U);
  CHECK(pa.find({ 20, 20 }) == &pb);   // both arcs contain the vertex, the one created last wins
}


TEST_CASE(widget_arc_partial_and_aa)
{
  head::memory<40, 40> h;
  dc d(h);
  d.cls(color::black);
  // 270 degree arc from the lower left over the top to the lower right
  const widget::arc::config_type cfg = { { 20, 20 }, 16U, 5U, -135, 135, 0, 100, color::green, color::gray, color::black };
  widget::arc a(h, cfg);
  a.init();
  // the gap at the bottom stays black
  CHECK_EQ(h.pixel({ 20, 34 }), color::black);
  CHECK_EQ(h.pixel({ 20, 6 }), color::gray);
  a.set_value(50);
  CHECK_EQ(h.pixel({ 20, 6 }), color::green);    // 12 o'clock is the middle of the scale, start ray is part of the arc
  CHECK_EQ(h.pixel({ 10, 10 }), color::green);
  CHECK_EQ(h.pixel({ 30, 10 }), color::gray);

  // anti aliasing: blended edge pixels appear, interior pixels stay
  a.anti_aliasing_enable();
  std::size_t blended = 0U;
  for (std::int16_t y = 0; y < 40; ++y) {
    for (std::int16_t x = 0; x < 40; ++x) {
      const color::value_type c = h.pixel({ x, y });
      if ((c != color::black) && (c != color::gray) && (c != color::green)) {
        ++blended;
      }
    }
  }
  CHECK(blended > 0U);
  CHECK_EQ(h.pixel({ 10, 10 }), color::green);
  CHECK_EQ(h.pixel({ 30, 10 }), color::gray);
  a.set_value(80);   // 81 degree, covers the 45 degree pixel
  CHECK_EQ(h.pixel({ 30, 10 }), color::green);

  // lowering the value must leave no green remnants, the anti aliased edges are erased before the redraw
  a.set_value(100);
  a.set_value(0);
  std::size_t greenish = 0U;
  for (std::int16_t y = 0; y < 40; ++y) {
    for (std::int16_t x = 0; x < 40; ++x) {
      const color::value_type c = h.pixel({ x, y });
      if (color::get_green(c) > color::get_red(c) + 8U) {
        ++greenish;
      }
    }
  }
  CHECK_EQ(greenish, 0U);
}


///////////////////////////////////////////////////////////////////////////////
// L E D
//

TEST_CASE(widget_led)
{
  head_type h;
  widget::led::config_type cfg = { { 5, 5 }, 3U, widget::led::shape_round, color::green, color::darkgreen, color::none };
  widget::led l(h, cfg);
  l.init();
  CHECK(!l.is_on());
  CHECK_EQ(vic_test::count_pixels(h, color::green), 0U);
  const std::size_t disc = vic_test::count_pixels(h, color::darkgreen);
  CHECK(disc > 0U);
  CHECK_EQ(h.present_count(), 1U);

  l.set_state(true);
  CHECK(l.is_on());
  CHECK_EQ(vic_test::count_pixels(h, color::green), disc);
  CHECK_EQ(vic_test::count_pixels(h, color::darkgreen), 0U);
  // no redraw without change
  h.reset_counters();
  l.set_state(true);
  CHECK_EQ(h.present_count(), 0U);
  l.toggle();
  CHECK(!l.is_on());
  CHECK_EQ(vic_test::count_pixels(h, color::darkgreen), disc);
  CHECK(l.find({ 5, 5 }) == &l);
  CHECK(l.find({ 9, 9 }) == nullptr);

  // square LED with border: 7 x 7 box with a white frame
  head_type s;
  cfg.shape        = widget::led::shape_square;
  cfg.color_border = color::white;
  widget::led ls(s, cfg);
  ls.init();
  ls.set_state(true);
  CHECK_EQ(s.pixel({ 5, 5 }), color::green);
  CHECK_EQ(s.pixel({ 2, 5 }), color::white);
  CHECK_EQ(s.pixel({ 8, 5 }), color::white);
  CHECK_EQ(s.pixel({ 5, 2 }), color::white);
  CHECK_EQ(s.pixel({ 5, 8 }), color::white);
  CHECK_EQ(s.pixel({ 1, 5 }), color::none);
  CHECK_EQ(s.pixel({ 9, 5 }), color::none);
}


///////////////////////////////////////////////////////////////////////////////
// S P I N N E R
//

TEST_CASE(widget_spinner_step_equals_redraw)
{
  const std::int16_t steps[] = { 10, -10, 100, -100, 45 };
  for (std::size_t n = 0U; n < sizeof(steps) / sizeof(steps[0]); ++n) {
    head::memory<40, 40> a, b;
    const widget::spinner::config_type cfg = { { 20, 20 }, 16U, 5U, 90U, steps[n], color::white, color::gray, color::none };
    widget::spinner sa(a, cfg);
    widget::spinner sb(b, cfg);
    sa.init();
    CHECK_EQ(sa.get_pos(), 0);
    const std::size_t ring = vic_test::count_pixels(a, color::white) + vic_test::count_pixels(a, color::gray);
    CHECK(vic_test::count_pixels(a, color::white) > 0U);
    for (int i = 0; i < 7; ++i) {
      sa.step();
      sb.set_pos(sa.get_pos());
      CHECK_SAME_BUFFER(a, b);
      CHECK_EQ(vic_test::count_pixels(a, color::white) + vic_test::count_pixels(a, color::gray), ring);
    }
    CHECK_EQ(sa.get_pos(), static_cast<std::int16_t>(((7 * steps[n]) % 360 + 360) % 360));
  }
}


TEST_CASE(widget_spinner_segment_position)
{
  head::memory<40, 40> h;
  const widget::spinner::config_type cfg = { { 20, 20 }, 16U, 5U, 90U, 90, color::white, color::gray, color::none };
  widget::spinner s(h, cfg);
  s.init();
  // segment from 12 to 3 o'clock
  CHECK_EQ(h.pixel({ 30, 10 }), color::white);
  CHECK_EQ(h.pixel({ 10, 10 }), color::gray);
  CHECK_EQ(h.pixel({ 30, 30 }), color::gray);
  s.step();
  // segment from 3 to 6 o'clock
  CHECK_EQ(h.pixel({ 30, 10 }), color::gray);
  CHECK_EQ(h.pixel({ 30, 30 }), color::white);
  s.set_pos(270);
  CHECK_EQ(h.pixel({ 10, 10 }), color::white);
  CHECK_EQ(h.pixel({ 30, 30 }), color::gray);
  CHECK(s.find({ 20, 20 }) == &s);
}


///////////////////////////////////////////////////////////////////////////////
// T E X T B O X
//

TEST_CASE(widget_textbox_wrap_and_scroll)
{
  // 24 x 20 head, LCD_6x8 font: 4 characters per line, 2 lines of 8 pixel fit with 1 pixel spacing
  head::memory<24, 20> h;
  widget::textbox::config_type cfg = { { 0, 0 }, 24U, 20U, &test_font, widget::textbox::align_left, 1U, color::white, color::black, color::none };
  widget::textbox tb(h, cfg);
  tb.init();
  CHECK_EQ(tb.get_line_count(), 0U);
  CHECK_EQ(tb.get_visible_lines(), 2U);
  CHECK_EQ(vic_test::count_pixels(h, color::black), 24U * 20U);

  // "AB CD" fits on one line (5 chars = 30 px? no: 24 px cell holds 4 chars), so it wraps into "AB" / "CD"
  CHECK_EQ(tb.set_text("AB CD"), 2U);
  CHECK_EQ(tb.get_line_count(), 2U);
  // first line in rows 0 - 7, second line in rows 9 - 16, glyph columns 0 - 11
  CHECK(vic_test::count_pixels(h, color::white) > 0U);
  for (std::int16_t x = 12; x < 24; ++x) {
    for (std::int16_t y = 0; y < 20; ++y) {
      CHECK_EQ(h.pixel({ x, y }), color::black);
    }
  }
  for (std::int16_t x = 0; x < 24; ++x) {
    CHECK_EQ(h.pixel({ x, 8 }), color::black);    // spacing row
    CHECK_EQ(h.pixel({ x, 17 }), color::black);   // below the second line
  }

  // explicit line breaks, three lines, only two visible
  CHECK_EQ(tb.set_text("A\nB\nC"), 3U);
  head::memory<24, 20> r;
  widget::textbox tr(r, cfg);
  tr.init();
  tr.set_text("A\nB");
  CHECK_SAME_BUFFER(h, r);

  // scroll down: "B" and "C" are shown
  tb.scroll(1);
  CHECK_EQ(tb.get_first_line(), 1U);
  tr.set_text("B\nC");
  CHECK_SAME_BUFFER(h, r);
  // scrolling is clamped to the last line
  tb.scroll(5);
  CHECK_EQ(tb.get_first_line(), 2U);
  tb.scroll(-10);
  CHECK_EQ(tb.get_first_line(), 0U);
  tr.set_text("A\nB");
  CHECK_SAME_BUFFER(h, r);

  // a long word is wrapped on its own line, the text in front of it stays
  CHECK_EQ(tb.set_text("A ABCDEFGH B"), 3U);
  // right alignment: the single char line "A" is in the last 6 columns
  cfg.align = widget::textbox::align_right;
  widget::textbox ta(r, cfg);
  ta.init();
  ta.set_text("A");
  for (std::int16_t x = 0; x < 18; ++x) {
    for (std::int16_t y = 0; y < 20; ++y) {
      CHECK_EQ(r.pixel({ x, y }), color::black);
    }
  }
  CHECK(vic_test::count_pixels(r, color::white) > 0U);
  // nullptr clears the box
  CHECK_EQ(ta.set_text(nullptr), 0U);
  CHECK_EQ(vic_test::count_pixels(r, color::white), 0U);
  CHECK(tb.find({ 3, 3 }) == &ta);     // all boxes contain the vertex, the one created last wins
}


///////////////////////////////////////////////////////////////////////////////
// E V E N T   R O U T I N G
//

namespace {

struct event_log_type {
  widget::base*       widget;
  vertex_type         vertex;
  widget::event_type  ev;
  int                 count;
};

void log_event(widget::base& w, vertex_type v, widget::event_type ev, void* arg)
{
  event_log_type* log = static_cast<event_log_type*>(arg);
  log->widget = &w;
  log->vertex = v;
  log->ev     = ev;
  log->count++;
}

} // namespace


TEST_CASE(widget_event_routing)
{
  head_type h;
  // a group box around everything (created first) and two LEDs, the second one with a handler
  const widget::groupbox::config_type gcfg = { { 0, 0, 24, 10 }, 0U, nullptr, nullptr, color::white, color::white, color::none };
  widget::groupbox g(h, gcfg);
  g.init();
  const widget::led::config_type cfg1 = { { 4, 5 }, 3U, widget::led::shape_round, color::green, color::darkgreen, color::none };
  const widget::led::config_type cfg2 = { { 16, 5 }, 3U, widget::led::shape_round, color::red, color::darkred, color::none };
  widget::led l1(h, cfg1);
  widget::led l2(h, cfg2);
  l1.init();
  l2.init();
  event_log_type log = { nullptr, { 0, 0 }, widget::event_down, 0 };
  l2.set_event_handler(log_event, &log);

  // outside of everything: no widget; between the LEDs: the group box
  CHECK(widget::base::event({ 10, 15 }, widget::event_tap) == nullptr);
  CHECK(widget::base::event({ 10, 5 }, widget::event_tap) == &g);
  CHECK_EQ(log.count, 0);
  // l1 lies inside the group box but was created later, so it gets the event; it has no handler
  CHECK(widget::base::event({ 4, 5 }, widget::event_tap) == &l1);
  CHECK_EQ(log.count, 0);
  // l2: handler called with the widget, position and event
  CHECK(widget::base::event({ 17, 6 }, widget::event_long_tap) == &l2);
  CHECK_EQ(log.count, 1);
  CHECK(log.widget == &l2);
  CHECK((log.vertex == vertex_type{ 17, 6 }));
  CHECK(log.ev == widget::event_long_tap);
  // the handler may change the widget
  l2.set_event_handler(nullptr);
  CHECK(widget::base::event({ 16, 5 }, widget::event_down) == &l2);
  CHECK_EQ(log.count, 1);
}


///////////////////////////////////////////////////////////////////////////////
// I N P U T   W I D G E T S
//

namespace {

// counts the events a handler receives
struct event_count_type {
  int tap;
  int down;
  int up;
  int long_tap;
};

void count_event(widget::base& w, vertex_type v, widget::event_type ev, void* arg)
{
  (void)w; (void)v;
  event_count_type* c = static_cast<event_count_type*>(arg);
  switch (ev) {
    case widget::event_tap      : c->tap++;      break;
    case widget::event_down     : c->down++;     break;
    case widget::event_up       : c->up++;       break;
    case widget::event_long_tap : c->long_tap++; break;
    default : break;
  }
}

} // namespace


TEST_CASE(widget_button)
{
  head_type h;
  const widget::button::config_type cfg = { { 0, 0, 20, 10 }, 0U, &test_font, "OK", color::white, color::gray, color::red, color::none };
  widget::button b(h, cfg);
  b.init();
  CHECK(!b.is_pressed());
  CHECK(vic_test::count_pixels(h, color::gray) > 0U);
  CHECK(vic_test::count_pixels(h, color::white) > 0U);     // caption
  CHECK_EQ(vic_test::count_pixels(h, color::red), 0U);
  event_count_type cnt = { 0, 0, 0, 0 };
  b.set_event_handler(count_event, &cnt);

  // down: pressed look, up: released, the tap goes to the handler
  CHECK(widget::base::event({ 5, 5 }, widget::event_down) == &b);
  CHECK(b.is_pressed());
  CHECK(vic_test::count_pixels(h, color::red) > 0U);
  CHECK_EQ(vic_test::count_pixels(h, color::gray), 0U);
  CHECK(widget::base::event({ 5, 5 }, widget::event_up) == &b);
  CHECK(!b.is_pressed());
  CHECK_EQ(vic_test::count_pixels(h, color::red), 0U);
  widget::base::event({ 5, 5 }, widget::event_tap);
  CHECK_EQ(cnt.down, 1);
  CHECK_EQ(cnt.up, 1);
  CHECK_EQ(cnt.tap, 1);
  CHECK(widget::base::event({ 21, 5 }, widget::event_tap) == nullptr);
  b.set_caption(nullptr);
  CHECK_EQ(vic_test::count_pixels(h, color::white), 0U);
}


TEST_CASE(widget_checkbox)
{
  head_type h;
  const widget::checkbox::config_type cfg = { { 1, 1 }, 8U, &test_font, "A", color::white, color::green, color::gray, color::none };
  widget::checkbox cb(h, cfg);
  cb.init();
  CHECK(!cb.is_checked());
  CHECK_EQ(vic_test::count_pixels(h, color::gray), 64U);
  CHECK_EQ(vic_test::count_pixels(h, color::green), 0U);
  CHECK(vic_test::count_pixels(h, color::white) > 0U);     // caption
  event_count_type cnt = { 0, 0, 0, 0 };
  cb.set_event_handler(count_event, &cnt);

  // tap on the box toggles, the handler is called after the toggle
  CHECK(widget::base::event({ 4, 4 }, widget::event_tap) == &cb);
  CHECK(cb.is_checked());
  CHECK(vic_test::count_pixels(h, color::green) > 0U);
  CHECK_EQ(cnt.tap, 1);
  // tap on the caption toggles as well, beyond the caption nothing
  CHECK(widget::base::event({ 15, 4 }, widget::event_tap) == &cb);
  CHECK(!cb.is_checked());
  CHECK_EQ(vic_test::count_pixels(h, color::green), 0U);
  CHECK(widget::base::event({ 20, 4 }, widget::event_tap) == nullptr);
  // down / up don't toggle
  widget::base::event({ 4, 4 }, widget::event_down);
  widget::base::event({ 4, 4 }, widget::event_up);
  CHECK(!cb.is_checked());
  cb.set_checked(true);
  cb.toggle();
  CHECK(!cb.is_checked());
}


TEST_CASE(widget_switch)
{
  head_type h;
  const widget::toggle::config_type cfg = { { 0, 0 }, 20U, 10U, color::green, color::gray, color::white, color::none };
  widget::toggle sw(h, cfg);
  sw.init();
  CHECK(!sw.is_on());
  // off: gray track, the knob at the left
  CHECK(vic_test::count_pixels(h, color::gray) > 0U);
  CHECK_EQ(vic_test::count_pixels(h, color::green), 0U);
  CHECK_EQ(h.pixel({ 5, 5 }), color::white);
  CHECK_EQ(h.pixel({ 14, 5 }), color::gray);
  // tap: on, green track, the knob at the right
  CHECK(widget::base::event({ 10, 5 }, widget::event_tap) == &sw);
  CHECK(sw.is_on());
  CHECK_EQ(vic_test::count_pixels(h, color::gray), 0U);
  CHECK_EQ(h.pixel({ 14, 5 }), color::white);
  CHECK_EQ(h.pixel({ 5, 5 }), color::green);
  sw.toggle_state();
  CHECK(!sw.is_on());
  // no redraw without change
  h.reset_counters();
  sw.set_state(false);
  CHECK_EQ(h.present_count(), 0U);
}


TEST_CASE(widget_slider)
{
  head_type h;
  // horizontal: track from x = 3 to 20 (knob radius 3 kept free), knob travel 17 pixel
  const widget::slider::config_type cfg = { { 0, 0 }, 24U, 10U, widget::slider::horizontal, 0, 100, 2U, 3U,
                                            color::gray, color::green, color::white, color::black };
  widget::slider sl(h, cfg);
  sl.init();
  CHECK_EQ(sl.get_value(), 0);
  CHECK_EQ(h.pixel({ 3, 5 }), color::white);    // knob at the left end
  CHECK_EQ(h.pixel({ 12, 5 }), color::gray);    // track
  CHECK_EQ(vic_test::count_pixels(h, color::green), 0U);
  sl.set_value(100);
  CHECK_EQ(h.pixel({ 20, 5 }), color::white);   // knob at the right end
  CHECK_EQ(h.pixel({ 12, 5 }), color::green);   // filled track
  CHECK_EQ(h.pixel({ 3, 5 }), color::green);
  event_count_type cnt = { 0, 0, 0, 0 };
  sl.set_event_handler(count_event, &cnt);

  // a tap sets the value to the position: x = 11 is 8 of 17 pixel travel
  CHECK(widget::base::event({ 11, 5 }, widget::event_tap) == &sl);
  CHECK_EQ(sl.get_value(), 47);
  CHECK_EQ(cnt.tap, 1);
  CHECK_EQ(h.pixel({ 11, 5 }), color::white);
  CHECK_EQ(h.pixel({ 20, 5 }), color::gray);
  // positions outside the travel are clamped, down works like tap
  widget::base::event({ 0, 5 }, widget::event_down);
  CHECK_EQ(sl.get_value(), 0);
  widget::base::event({ 23, 5 }, widget::event_down);
  CHECK_EQ(sl.get_value(), 100);
  // up does nothing
  widget::base::event({ 11, 5 }, widget::event_up);
  CHECK_EQ(sl.get_value(), 100);
  // clamping of set_value
  sl.set_value(-10);
  CHECK_EQ(sl.get_value(), 0);

  // vertical: range_lower at the bottom
  head::memory<10, 24> v;
  const widget::slider::config_type vcfg = { { 0, 0 }, 10U, 24U, widget::slider::vertical, 0, 100, 2U, 3U,
                                             color::gray, color::green, color::white, color::black };
  widget::slider sv(v, vcfg);
  sv.init();
  CHECK_EQ(v.pixel({ 5, 20 }), color::white);
  widget::base::event({ 5, 3 }, widget::event_tap);
  CHECK_EQ(sv.get_value(), 100);
  CHECK_EQ(v.pixel({ 5, 3 }), color::white);
  CHECK_EQ(v.pixel({ 5, 12 }), color::green);
}


TEST_CASE(widget_spinbox)
{
  head::memory<40, 10> h;
  // buttons are 8 x 8 at the ends, the value cell in between
  const widget::spinbox::config_type cfg = { { 0, 0, 40, 8 }, 0U, &test_font, 0, 100, 5, 0, color::white, color::black, color::gray, color::none };
  widget::spinbox sb(h, cfg);
  sb.init();
  CHECK_EQ(sb.get_value(), 0);
  // buttons, value cell and signs cover everything but the two one pixel gaps beside the value cell
  CHECK_EQ(vic_test::count_pixels(h, color::gray) + vic_test::count_pixels(h, color::white) + vic_test::count_pixels(h, color::black), 40U * 8U - 16U);
  CHECK(vic_test::count_pixels(h, color::white) > 0U);     // signs and the "0"
  event_count_type cnt = { 0, 0, 0, 0 };
  sb.set_event_handler(count_event, &cnt);

  // plus / minus taps, long tap is 10 steps, the value cell taps do nothing
  CHECK(widget::base::event({ 36, 4 }, widget::event_tap) == &sb);
  CHECK_EQ(sb.get_value(), 5);
  CHECK_EQ(cnt.tap, 1);
  widget::base::event({ 36, 4 }, widget::event_long_tap);
  CHECK_EQ(sb.get_value(), 55);
  widget::base::event({ 3, 4 }, widget::event_tap);
  CHECK_EQ(sb.get_value(), 50);
  widget::base::event({ 20, 4 }, widget::event_tap);
  CHECK_EQ(sb.get_value(), 50);
  // clamping
  widget::base::event({ 36, 4 }, widget::event_long_tap);
  CHECK_EQ(sb.get_value(), 100);
  widget::base::event({ 36, 4 }, widget::event_tap);
  CHECK_EQ(sb.get_value(), 100);
  sb.set_step(50);
  widget::base::event({ 3, 4 }, widget::event_long_tap);
  CHECK_EQ(sb.get_value(), 0);
  sb.set_range(10, 20);
  CHECK_EQ(sb.get_value(), 10);
  // the value is rendered with decimals
  const widget::spinbox::config_type dcfg = { { 0, 0, 40, 8 }, 0U, &test_font, 0, 1000, 25, 1, color::white, color::black, color::gray, color::none };
  head::memory<40, 10> d;
  widget::spinbox sd(d, dcfg);
  sd.init();
  sd.set_value(275);
  CHECK(vic_test::count_pixels(d, color::white) > vic_test::count_pixels(h, color::white));   // "27.5" is longer than "10"
}


TEST_CASE(widget_imagebutton)
{
  head::memory<24, 16> h;
  static const char* xpm[] = { "4 2 2 1", "a c #FF0000", "  c None", "aaaa", " aa " };
  // 14 pixel high: icon (2) + gap (2) + caption (8) stacked and centered
  const widget::imagebutton::config_type cfg = { { 0, 0, 24, 14 }, 0U, xpm, &test_font, "A",
                                                 color::white, color::gray, color::blue, color::green, color::none };
  widget::imagebutton ib(h, cfg);
  ib.init();
  // icon (6 red pixels) and caption on the gray button
  CHECK_EQ(vic_test::count_pixels(h, color::red), 6U);
  CHECK(vic_test::count_pixels(h, color::white) > 0U);
  CHECK(vic_test::count_pixels(h, color::gray) > 0U);
  event_count_type cnt = { 0, 0, 0, 0 };
  ib.set_event_handler(count_event, &cnt);

  // the inner icon widget is transparent for events, the button receives them (also on the icon pixels)
  CHECK(widget::base::event({ 11, 0 }, widget::event_down) == &ib);
  CHECK(ib.is_pressed());
  CHECK(vic_test::count_pixels(h, color::blue) > 0U);
  CHECK_EQ(vic_test::count_pixels(h, color::red), 6U);      // the icon is drawn again on the pressed button
  CHECK(widget::base::event({ 11, 0 }, widget::event_up) == &ib);
  CHECK(!ib.is_pressed());
  widget::base::event({ 5, 5 }, widget::event_tap);
  CHECK_EQ(cnt.tap, 1);
  // active look
  ib.set_active(true);
  CHECK(ib.is_active());
  CHECK(vic_test::count_pixels(h, color::green) > 0U);
  CHECK_EQ(vic_test::count_pixels(h, color::gray), 0U);
  ib.set_active(false);
  CHECK_EQ(vic_test::count_pixels(h, color::green), 0U);
}


TEST_CASE(widget_compass)
{
  head::memory<40, 40> h;
  const widget::compass::config_type cfg = { { 20, 20 }, 16U, nullptr, color::black, color::gray, color::white, color::yellow, color::red };
  widget::compass cp(h, cfg);
  cp.init();
  CHECK_EQ(cp.get_heading(), 0);
  // disc, ring, pointer and ticks
  CHECK_EQ(h.pixel({ 20, 20 }), color::red);     // center dot
  CHECK_EQ(h.pixel({ 20, 4 }), color::red);      // pointer
  CHECK_EQ(h.pixel({ 36, 20 }), color::gray);    // ring
  CHECK_EQ(h.pixel({ 31, 20 }), color::white);   // the east tick (90 degree, 4 pixel long) reaches radius 11
  CHECK_EQ(h.pixel({ 20, 12 }), color::black);   // inside
  const std::size_t ticks = vic_test::count_pixels(h, color::white);
  CHECK(ticks > 0U);
  CHECK_EQ(h.present_count(), 1U);

  // heading 180: the rose turned by half, the tick pattern is the same (every 10 degree), the picture differs
  head::memory<40, 40> r;
  widget::compass cr(r, cfg);
  cr.init();
  cr.set_heading(180);
  CHECK_EQ(cr.get_heading(), 180);
  CHECK_EQ(vic_test::count_pixels(r, color::white), ticks);
  CHECK_EQ(r.pixel({ 20, 4 }), color::red);
  // heading 60: the 150 degree tick (2 pixel) is at the east, it doesn't reach radius 11
  cr.set_heading(60);
  CHECK_EQ(r.pixel({ 31, 20 }), color::black);
  CHECK_EQ(r.pixel({ 33, 20 }), color::white);
  // normalization
  cr.set_heading(-90);
  CHECK_EQ(cr.get_heading(), 270);
  cr.set_heading(725);
  CHECK_EQ(cr.get_heading(), 5);
  // no redraw without change
  r.reset_counters();
  cr.set_heading(5);
  CHECK_EQ(r.present_count(), 0U);
  CHECK(cr.find({ 20, 30 }) == &cr);
  CHECK(cr.find({ 38, 38 }) == nullptr);
}
