///////////////////////////////////////////////////////////////////////////////
// \brief Tests for gpr.h: pixel, plot, line, line_horz, line_vert, anti aliasing
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "helpers.h"
#include "dc.h"
#include "head/memory.h"

using namespace vic;
using vic_test::PAL_X;


TEST_CASE(gpr_pixel_plot)
{
  head::memory<8, 4> h;
  dc d(h);
  d.set_color(color::white);

  d.pixel_set({ 1, 1 });
  d.pixel_set({ 2, 1 }, color::red);
  CHECK_EQ(h.present_count(), 0U);
  d.plot({ 3, 1 });
  CHECK_EQ(h.present_count(), 1U);
  d.plot({ 4, 1 }, color::red);
  CHECK_EQ(h.present_count(), 2U);
  CHECK_RASTER(h, vic_test::PAL_RG,
    "........",
    ".WRWR...",
    "........",
    "........");
  CHECK_EQ(d.pixel_get({ 2, 1 }), color::red);
  CHECK_EQ(d.pixel_get({ 0, 0 }), color::none);
  CHECK_EQ(d.pixel_get({ 100, 100 }), color::none);
}


TEST_CASE(gpr_line_axis_aligned)
{
  head::memory<8, 6> h;
  dc d(h);
  d.set_color(color::white);
  d.line({ 1, 1 }, { 6, 1 });
  d.line({ 6, 4 }, { 1, 4 });
  d.line({ 0, 0 }, { 0, 5 });
  d.line({ 7, 5 }, { 7, 0 });
  CHECK_RASTER(h, PAL_X,
    "X......X",
    "XXXXXXXX",
    "X......X",
    "X......X",
    "XXXXXXXX",
    "X......X");
  CHECK_EQ(h.present_count(), 4U);
}


TEST_CASE(gpr_line_diagonal)
{
  head::memory<7, 7> h;
  dc d(h);
  d.set_color(color::white);
  d.line({ 0, 0 }, { 6, 6 });
  CHECK_RASTER(h, PAL_X,
    "X......",
    ".X.....",
    "..X....",
    "...X...",
    "....X..",
    ".....X.",
    "......X");

  head::memory<7, 7> h2;
  dc d2(h2);
  d2.set_color(color::white);
  d2.line({ 6, 0 }, { 0, 6 });
  CHECK_RASTER(h2, PAL_X,
    "......X",
    ".....X.",
    "....X..",
    "...X...",
    "..X....",
    ".X.....",
    "X......");
}


TEST_CASE(gpr_line_shallow)
{
  head::memory<7, 3> h;
  dc d(h);
  d.set_color(color::white);
  d.line({ 0, 0 }, { 6, 2 });
  CHECK_RASTER(h, PAL_X,
    "XX.....",
    "..XXX..",
    ".....XX");

  // same line in reverse direction
  head::memory<7, 3> r;
  dc dr(r);
  dr.set_color(color::white);
  dr.line({ 6, 2 }, { 0, 0 });
  CHECK_SAME_BUFFER(h, r);
}


TEST_CASE(gpr_line_properties)
{
  // for any line: both end points are set, pixel count is max(dx, dy) + 1
  // and for shallow lines every column has exactly one pixel
  static const vertex_type ends[][2] = {
    { { 1, 1 }, { 13, 4 } }, { { 13, 4 }, { 1, 1 } }, { { 2, 9 }, { 12, 3 } }, { { 12, 3 }, { 2, 9 } },
    { { 3, 1 }, { 5, 9 } },  { { 5, 9 }, { 3, 1 } },  { { 0, 0 }, { 15, 11 } }, { { 15, 0 }, { 0, 11 } },
    { { 4, 4 }, { 4, 4 } }
  };
  for (std::size_t i = 0U; i < sizeof(ends) / sizeof(ends[0]); ++i) {
    head::memory<16, 12> h;
    dc d(h);
    d.set_color(color::white);
    const vertex_type v0 = ends[i][0], v1 = ends[i][1];
    d.line(v0, v1);
    const std::size_t dx = static_cast<std::size_t>(util::abs<std::int16_t>(static_cast<std::int16_t>(v1.x - v0.x)));
    const std::size_t dy = static_cast<std::size_t>(util::abs<std::int16_t>(static_cast<std::int16_t>(v1.y - v0.y)));
    CHECK_EQ(h.pixel({ v0.x, v0.y }), color::white);
    CHECK_EQ(h.pixel({ v1.x, v1.y }), color::white);
    CHECK_EQ(vic_test::count_pixels(h, color::white), (dx > dy ? dx : dy) + 1U);
    CHECK_EQ(h.oob_count(), 0U);
    if (dx >= dy) {
      for (std::int16_t x = util::min2(v0.x, v1.x); x <= (v0.x > v1.x ? v0.x : v1.x); ++x) {
        std::size_t cnt = 0U;
        for (std::int16_t y = 0; y < 12; ++y) {
          if (h.pixel({ x, y }) == color::white) { ++cnt; }
        }
        CHECK_EQ(cnt, 1U);
      }
    }
  }
}


TEST_CASE(gpr_line_clipping)
{
  // lines partially or completely outside the screen must not touch anything inside wrongly
  head::memory<8, 8> h;
  dc d(h);
  d.set_color(color::white);
  d.line({ -5, 3 }, { 12, 3 });
  CHECK_EQ(vic_test::count_pixels(h, color::white), 8U);
  CHECK_EQ(h.oob_count(), 0U);           // axis aligned lines take the clipped head fast path
  d.line({ -10, -10 }, { -1, -1 });
  CHECK_EQ(vic_test::count_pixels(h, color::white), 8U);
  CHECK_EQ(h.oob_count(), 10U);          // the diagonal line is plotted through the pipe

  // the same through the pipe (shader active): identical pixels, the pixels outside are rejected by the head
  head::memory<8, 8> p;
  dc dp(p);
  vic_test::passthrough_shader pass;
  dp.shader_register(&pass);
  dp.set_color(color::white);
  dp.line({ -5, 3 }, { 12, 3 });
  dp.line({ -10, -10 }, { -1, -1 });
  CHECK_SAME_BUFFER(h, p);
  CHECK_EQ(p.oob_count(), 20U);
}


TEST_CASE(gpr_line_long)
{
  // long lines must not overflow the error term (and must terminate)
  head::memory<8, 8> h;
  dc d(h);
  d.set_color(color::white);
  d.line({ 0, 2 }, { 20000, 2 });
  CHECK_EQ(vic_test::count_pixels(h, color::white), 8U);
  CHECK_EQ(h.oob_count(), 0U);           // clipped by the fast path

  head::memory<8, 8> v;
  dc dv(v);
  dv.set_color(color::white);
  dv.line({ 3, -20000 }, { 3, 7 });
  CHECK_EQ(vic_test::count_pixels(v, color::white), 8U);

  head::memory<8, 8> g;
  dc dg(g);
  dg.set_color(color::white);
  dg.line({ 0, 0 }, { 30000, 15000 });
  CHECK_EQ(vic_test::count_pixels(g, color::white), 8U);
  CHECK_EQ(g.pixel({ 0, 0 }), color::white);
  CHECK_EQ(g.pixel({ 2, 1 }), color::white);

  // the pipe path (shader active) walks all pixels and must terminate as well
  head::memory<8, 8> p;
  dc dp(p);
  vic_test::passthrough_shader pass;
  dp.shader_register(&pass);
  dp.set_color(color::white);
  dp.line({ 0, 2 }, { 20000, 2 });
  CHECK_EQ(p.pixel_set_count() + p.oob_count(), 20001U);
  dp.line({ 0, 0 }, { 30000, 15000 });
  CHECK_EQ(p.pixel_set_count() + p.oob_count(), 20001U + 30001U);
  CHECK_EQ(p.pixel({ 2, 1 }), color::white);
}


TEST_CASE(gpr_line_horz_vert)
{
  head::memory<8, 6> h;
  dc d(h);
  vic_test::passthrough_shader pass;
  d.shader_register(&pass);     // use the gpr implementation
  d.set_color(color::white);
  d.line_horz({ 1, 1 }, { 6, 1 });
  d.line_horz({ 6, 4 }, { 1, 2 });   // reverse, y of v1 is ignored
  d.line_vert({ 0, 0 }, { 0, 5 });
  d.line_vert({ 7, 5 }, { 3, 0 });   // reverse, x of v1 is ignored
  CHECK_RASTER(h, PAL_X,
    "X......X",
    "XXXXXXXX",
    "X......X",
    "X......X",
    "XXXXXXXX",
    "X......X");
  CHECK_EQ(h.present_count(), 4U);
}


TEST_CASE(gpr_line_alpha)
{
  // every pixel of a line is written exactly once
  head::memory<8, 8> h;
  dc d(h);
  d.set_color(0x80FFFFFFUL);
  d.line({ 0, 0 }, { 7, 5 });
  CHECK_EQ(vic_test::count_pixels(h, 0x80FFFFFFUL), 8U);
  CHECK_EQ(vic_test::count_pixels(h, color::none), 56U);
}


TEST_CASE(gpr_anti_aliasing_switch)
{
  // anti aliasing only affects the rendering while it is enabled, never with debug colors
  head::memory<16, 16> a, b, c;
  dc da(a), db(b), dcc(c);
  da.set_color(color::white);
  db.set_color(color::white);
  dcc.set_color(color::white);
  db.anti_aliasing_enable(true);
  db.alpha_blending_enable(false);
  dcc.anti_aliasing_enable(true);
  dcc.anti_aliasing_enable(false);

  da.line({ 1, 2 }, { 14, 9 });     db.line({ 1, 2 }, { 14, 9 });     dcc.line({ 1, 2 }, { 14, 9 });
  da.circle({ 8, 8 }, 6);           db.circle({ 8, 8 }, 6);           dcc.circle({ 8, 8 }, 6);
  da.disc({ 12, 3 }, 2);            db.disc({ 12, 3 }, 2);            dcc.disc({ 12, 3 }, 2);
  // switched off again: identical to never enabled
  CHECK_SAME_BUFFER(a, c);
  // enabled: partially covered pixels exist, all in the pen color
  CHECK(vic_test::count_pixels_not(b, color::none) > vic_test::count_pixels(b, color::white));
  CHECK_EQ(vic_test::count_pixels(b, color::yellow), 0U);
  CHECK_EQ(vic_test::count_pixels(b, color::red), 0U);
  CHECK_EQ(vic_test::count_pixels(b, color::green), 0U);
  for (std::int16_t y = 0; y < 16; ++y) {
    for (std::int16_t x = 0; x < 16; ++x) {
      const color::value_type p = b.pixel({ x, y });
      if (p != color::none) {
        CHECK_EQ(p & 0x00FFFFFFUL, 0x00FFFFFFUL);
      }
    }
  }
}
