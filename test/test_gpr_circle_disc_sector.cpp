///////////////////////////////////////////////////////////////////////////////
// \brief Tests for gpr.h: circle, disc, disc_sector and sector
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "helpers.h"
#include "dc.h"
#include "head/memory.h"

using namespace vic;
using vic_test::PAL_X;

namespace {
  template<std::uint16_t X, std::uint16_t Y>
  struct gpr_dc {
    head::memory<X, Y>           head;
    dc                           d;
    vic_test::passthrough_shader pass;
    gpr_dc() : d(head) { d.shader_register(&pass); d.set_color(color::white); }
  };
}


TEST_CASE(gpr_circle)
{
  gpr_dc<5, 5> g;
  g.d.circle({ 2, 2 }, 2U);
  CHECK_RASTER(g.head, PAL_X,
    ".XXX.",
    "X...X",
    "X...X",
    "X...X",
    ".XXX.");
  CHECK_EQ(g.head.present_count(), 1U);

  gpr_dc<3, 3> r1;
  r1.d.circle({ 1, 1 }, 1U);
  CHECK_RASTER(r1.head, PAL_X,
    ".X.",
    "X.X",
    ".X.");

  gpr_dc<3, 3> r0;
  r0.d.circle({ 1, 1 }, 0U);
  CHECK_RASTER(r0.head, PAL_X,
    "...",
    ".X.",
    "...");

  gpr_dc<9, 9> r4;
  r4.d.circle({ 4, 4 }, 4U);
  CHECK_RASTER(r4.head, PAL_X,
    "...XXX...",
    "..X...X..",
    ".X.....X.",
    "X.......X",
    "X.......X",
    "X.......X",
    ".X.....X.",
    "..X...X..",
    "...XXX...");

  // clipping
  gpr_dc<5, 5> c;
  c.d.circle({ 0, 0 }, 4U);
  CHECK_EQ(c.head.pixel({ 4, 0 }), color::white);
  CHECK_EQ(c.head.pixel({ 0, 4 }), color::white);
  CHECK(c.head.oob_count() > 0U);
}


TEST_CASE(gpr_circle_arcs)
{
  // arcs are half open: the start angle point is drawn, the end angle point is not,
  // so the four quadrant arcs tile the full circle without overlaps
  gpr_dc<7, 7> q;
  q.d.circle({ 3, 3 }, 3U, 0U, 90U);
  CHECK_RASTER(q.head, PAL_X,
    "....X..",
    ".....X.",
    "......X",
    "......X",
    ".......",
    ".......",
    ".......");

  gpr_dc<7, 7> q2;
  q2.d.circle({ 3, 3 }, 3U, 90U, 180U);
  CHECK_RASTER(q2.head, PAL_X,
    "..XX...",
    ".X.....",
    "X......",
    ".......",
    ".......",
    ".......",
    ".......");

  gpr_dc<7, 7> q3;
  q3.d.circle({ 3, 3 }, 3U, 180U, 270U);
  CHECK_RASTER(q3.head, PAL_X,
    ".......",
    ".......",
    ".......",
    "X......",
    "X......",
    ".X.....",
    "..X....");

  gpr_dc<7, 7> q4;
  q4.d.circle({ 3, 3 }, 3U, 270U, 360U);
  CHECK_RASTER(q4.head, PAL_X,
    ".......",
    ".......",
    ".......",
    ".......",
    "......X",
    ".....X.",
    "...XX..");

  // half circle
  gpr_dc<7, 7> hc;
  hc.d.circle({ 3, 3 }, 3U, 0U, 180U);
  CHECK_RASTER(hc.head, PAL_X,
    "..XXX..",
    ".X...X.",
    "X.....X",
    "......X",
    ".......",
    ".......",
    ".......");

  // the quadrants tile the full circle, every pixel is written exactly once
  gpr_dc<7, 7> tiled;
  tiled.d.set_color(0x80FFFFFFUL);
  tiled.d.circle({ 3, 3 }, 3U, 0U, 90U);
  tiled.d.circle({ 3, 3 }, 3U, 90U, 180U);
  tiled.d.circle({ 3, 3 }, 3U, 180U, 270U);
  tiled.d.circle({ 3, 3 }, 3U, 270U, 360U);
  CHECK_EQ(vic_test::count_pixels(tiled.head, 0x80FFFFFFUL), 16U);
  CHECK_EQ(tiled.pass.count, 16U);
  gpr_dc<7, 7> full;
  full.d.set_color(0x80FFFFFFUL);
  full.d.circle({ 3, 3 }, 3U);
  CHECK_SAME_BUFFER(tiled.head, full.head);
}


TEST_CASE(gpr_circle_angles_robust)
{
  // odd angle inputs must terminate quickly and not draw outside the circle
  gpr_dc<9, 9> g;
  g.d.circle({ 4, 4 }, 3U, 0U, 0U);
  g.d.circle({ 4, 4 }, 3U, 90U, 90U);
  g.d.circle({ 4, 4 }, 3U, 200U, 100U);
  CHECK_EQ(vic_test::count_pixels(g.head, color::white), 0U);
  g.d.circle({ 4, 4 }, 3U, 0U, 720U);
  g.d.circle({ 4, 4 }, 3U, 400U, 500U);
  CHECK_EQ(g.head.pixel({ 4, 4 }), color::none);
  CHECK_EQ(g.head.pixel({ 0, 0 }), color::none);
  CHECK(vic_test::count_pixels(g.head, color::white) > 0U);
  // the full circle, however it is requested, is the same
  gpr_dc<9, 9> a, b;
  a.d.circle({ 4, 4 }, 3U);
  b.d.circle({ 4, 4 }, 3U, 0U, 360U);
  CHECK_SAME_BUFFER(a.head, b.head);
}


TEST_CASE(gpr_disc)
{
  // the disc is the filled circle: same outline as circle()
  gpr_dc<5, 5> g;
  g.d.disc({ 2, 2 }, 2U);
  CHECK_RASTER(g.head, PAL_X,
    ".XXX.",
    "XXXXX",
    "XXXXX",
    "XXXXX",
    ".XXX.");
  CHECK_EQ(g.head.present_count(), 1U);

  gpr_dc<3, 3> r1;
  r1.d.disc({ 1, 1 }, 1U);
  CHECK_RASTER(r1.head, PAL_X,
    ".X.",
    "XXX",
    ".X.");

  gpr_dc<3, 3> r0;
  r0.d.disc({ 1, 1 }, 0U);
  CHECK_EQ(vic_test::count_pixels(r0.head, color::white), 1U);
  CHECK_EQ(r0.head.pixel({ 1, 1 }), color::white);

  gpr_dc<9, 9> r4;
  r4.d.disc({ 4, 4 }, 4U);
  CHECK_RASTER(r4.head, PAL_X,
    "...XXX...",
    "..XXXXX..",
    ".XXXXXXX.",
    "XXXXXXXXX",
    "XXXXXXXXX",
    "XXXXXXXXX",
    ".XXXXXXX.",
    "..XXXXX..",
    "...XXX...");

  // every circle pixel is a disc pixel and the disc has no pixel outside the circle
  for (std::uint16_t r = 0U; r < 7U; ++r) {
    gpr_dc<16, 16> c, d;
    c.d.circle({ 8, 8 }, r);
    d.d.disc({ 8, 8 }, r);
    for (std::int16_t y = 0; y < 16; ++y) {
      std::int16_t first = -1, last = -1;
      for (std::int16_t x = 0; x < 16; ++x) {
        if (c.head.pixel({ x, y }) == color::white) {
          if (first < 0) { first = x; }
          last = x;
        }
      }
      for (std::int16_t x = 0; x < 16; ++x) {
        const bool in_disc = d.head.pixel({ x, y }) == color::white;
        const bool in_circle_span = (first >= 0) && (x >= first) && (x <= last);
        CHECK_EQ(in_disc, in_circle_span);
      }
    }
  }
}


TEST_CASE(gpr_disc_alpha)
{
  gpr_dc<9, 9> g;
  g.d.set_color(0x80FFFFFFUL);
  g.d.disc({ 4, 4 }, 4U);
  CHECK_EQ(vic_test::count_pixels(g.head, 0x80FFFFFFUL), 57U);
  CHECK_EQ(g.pass.count, 57U);
}


TEST_CASE(gpr_disc_sector)
{
  // the four quadrants together are the disc, quadrant 0 is top/right
  gpr_dc<5, 5> q0;
  q0.d.disc_sector({ 2, 2 }, 2U, 0U);
  CHECK_RASTER(q0.head, PAL_X,
    "..XX.",
    "..XXX",
    "..XXX",
    ".....",
    ".....");
  gpr_dc<5, 5> q1;
  q1.d.disc_sector({ 2, 2 }, 2U, 1U);
  CHECK_RASTER(q1.head, PAL_X,
    ".XX..",
    "XXX..",
    "XXX..",
    ".....",
    ".....");
  gpr_dc<5, 5> q2;
  q2.d.disc_sector({ 2, 2 }, 2U, 2U);
  CHECK_RASTER(q2.head, PAL_X,
    ".....",
    ".....",
    "XXX..",
    "XXX..",
    ".XX..");
  gpr_dc<5, 5> q3;
  q3.d.disc_sector({ 2, 2 }, 2U, 3U);
  CHECK_RASTER(q3.head, PAL_X,
    ".....",
    ".....",
    "..XXX",
    "..XXX",
    "..XX.");
  CHECK_EQ(q3.head.present_count(), 1U);

  // invalid quadrant draws nothing
  gpr_dc<5, 5> qi;
  qi.d.disc_sector({ 2, 2 }, 2U, 4U);
  CHECK_EQ(qi.pass.count, 0U);
}


TEST_CASE(gpr_sector_quadrant)
{
  // 0 degree is 3 o'clock, angles count counter clockwise
  // start angle is inside, end angle is outside, outer radius is outside
  gpr_dc<11, 11> g;
  g.d.sector({ 5, 5 }, 0U, 5U, 270U, 360U);
  CHECK_RASTER(g.head, PAL_X,
    "...........",
    "...........",
    "...........",
    "...........",
    "...........",
    "...........",
    ".....XXXXX.",
    ".....XXXXX.",
    ".....XXXX..",
    ".....XXX...",
    "...........");
  CHECK_EQ(g.head.present_count(), 1U);

  // the center pixel counts as angle 0, so it is part of the sector containing the 0 degree ray
  gpr_dc<11, 11> q1;
  q1.d.sector({ 5, 5 }, 0U, 5U, 0U, 90U);
  CHECK_RASTER(q1.head, PAL_X,
    "...........",
    "......XX...",
    "......XXX..",
    "......XXXX.",
    "......XXXX.",
    ".....XXXXX.",
    "...........",
    "...........",
    "...........",
    "...........",
    "...........");

  gpr_dc<11, 11> q2;
  q2.d.sector({ 5, 5 }, 0U, 5U, 90U, 180U);
  CHECK_RASTER(q2.head, PAL_X,
    "...........",
    "...XXX.....",
    "..XXXX.....",
    ".XXXXX.....",
    ".XXXXX.....",
    "...........",
    "...........",
    "...........",
    "...........",
    "...........",
    "...........");

  gpr_dc<11, 11> q3;
  q3.d.sector({ 5, 5 }, 0U, 5U, 180U, 270U);
  CHECK_RASTER(q3.head, PAL_X,
    "...........",
    "...........",
    "...........",
    "...........",
    "...........",
    ".XXXX......",
    ".XXXX......",
    ".XXXX......",
    "..XXX......",
    "...XX......",
    "...........");
}


TEST_CASE(gpr_sector_tiling)
{
  // adjacent sectors tile without gaps and without overlaps
  gpr_dc<11, 11> full;
  full.d.sector({ 5, 5 }, 1U, 5U, 0U, 360U);
  CHECK_EQ(vic_test::count_pixels(full.head, color::white), 68U);
  CHECK_EQ(full.head.pixel({ 5, 5 }), color::none);   // inner radius 1 excludes the center

  gpr_dc<11, 11> tiled;
  tiled.d.set_color(0x80FFFFFFUL);
  tiled.d.sector({ 5, 5 }, 1U, 5U, 0U, 90U);
  tiled.d.sector({ 5, 5 }, 1U, 5U, 90U, 180U);
  tiled.d.sector({ 5, 5 }, 1U, 5U, 180U, 270U);
  tiled.d.sector({ 5, 5 }, 1U, 5U, 270U, 360U);
  CHECK_EQ(vic_test::count_pixels(tiled.head, 0x80FFFFFFUL), 68U);   // no pixel written twice
  CHECK_EQ(tiled.pass.count, 68U);

  // with inner radius 0 the center pixel is drawn by exactly one of the tiles
  gpr_dc<11, 11> pie;
  pie.d.set_color(0x80FFFFFFUL);
  pie.d.sector({ 5, 5 }, 0U, 5U, 0U, 90U);
  pie.d.sector({ 5, 5 }, 0U, 5U, 90U, 180U);
  pie.d.sector({ 5, 5 }, 0U, 5U, 180U, 270U);
  pie.d.sector({ 5, 5 }, 0U, 5U, 270U, 360U);
  CHECK_EQ(vic_test::count_pixels(pie.head, 0x80FFFFFFUL), 69U);
  CHECK_EQ(pie.pass.count, 69U);

  // odd split
  gpr_dc<11, 11> odd;
  odd.d.set_color(0x80FFFFFFUL);
  odd.d.sector({ 5, 5 }, 1U, 5U, 30U, 200U);
  odd.d.sector({ 5, 5 }, 1U, 5U, 200U, 330U);
  odd.d.sector({ 5, 5 }, 1U, 5U, 330U, 30U);   // wrap around
  CHECK_EQ(vic_test::count_pixels(odd.head, 0x80FFFFFFUL), 68U);
  CHECK_EQ(odd.pass.count, 68U);

  // full ring requested in different ways
  gpr_dc<11, 11> f2;
  f2.d.sector({ 5, 5 }, 1U, 5U, 90U, 450U);
  CHECK_SAME_BUFFER(full.head, f2.head);
  gpr_dc<11, 11> f3;
  f3.d.sector({ 5, 5 }, 1U, 5U, 45U, 45U + 360U);
  CHECK_SAME_BUFFER(full.head, f3.head);
}


TEST_CASE(gpr_sector_wrap_and_large)
{
  // a sector over the 0 degree line
  gpr_dc<11, 11> w;
  w.d.sector({ 5, 5 }, 0U, 5U, 315U, 45U);
  CHECK_EQ(w.head.pixel({ 9, 5 }), color::white);   // 0 degree ray
  CHECK_EQ(w.head.pixel({ 8, 3 }), color::white);
  CHECK_EQ(w.head.pixel({ 8, 7 }), color::white);
  CHECK_EQ(w.head.pixel({ 2, 5 }), color::none);
  CHECK_EQ(w.head.pixel({ 5, 1 }), color::none);
  CHECK_EQ(w.head.pixel({ 5, 9 }), color::none);
  gpr_dc<11, 11> w2;
  w2.d.sector({ 5, 5 }, 0U, 5U, 315U, 405U);
  CHECK_SAME_BUFFER(w.head, w2.head);

  // bigger than 180 degree
  gpr_dc<11, 11> big;
  big.d.sector({ 5, 5 }, 0U, 5U, 200U, 100U);
  CHECK_EQ(big.head.pixel({ 9, 5 }), color::white);   // 0 degree
  CHECK_EQ(big.head.pixel({ 5, 9 }), color::white);   // 270 degree
  CHECK_EQ(big.head.pixel({ 5, 1 }), color::white);   // 90 degree is inside (end is 100)
  CHECK_EQ(big.head.pixel({ 1, 5 }), color::none);    // 180 degree
  CHECK_EQ(big.head.pixel({ 2, 3 }), color::none);    // ~150 degree

  // zero span draws nothing
  gpr_dc<11, 11> z;
  z.d.sector({ 5, 5 }, 0U, 5U, 100U, 100U);
  CHECK_EQ(z.pass.count, 0U);

  // inner radius >= outer radius draws nothing, radius 0 draws nothing
  gpr_dc<11, 11> e;
  e.d.sector({ 5, 5 }, 5U, 5U, 0U, 360U);
  e.d.sector({ 5, 5 }, 6U, 5U, 0U, 360U);
  e.d.sector({ 5, 5 }, 0U, 0U, 0U, 360U);
  CHECK_EQ(e.pass.count, 0U);
}


TEST_CASE(gpr_sector_ring)
{
  // inner radius is inside
  gpr_dc<11, 11> g;
  g.d.sector({ 5, 5 }, 4U, 5U, 0U, 360U);
  CHECK_RASTER(g.head, PAL_X,
    "...........",
    "...XXXXX...",
    "..X.....X..",
    ".X.......X.",
    ".X.......X.",
    ".X.......X.",
    ".X.......X.",
    ".X.......X.",
    "..X.....X..",
    "...XXXXX...",
    "...........");
}
