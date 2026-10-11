///////////////////////////////////////////////////////////////////////////////
// \brief Tests for gpr.h: polyline, triangle and triangle_solid
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


TEST_CASE(gpr_polyline)
{
  // the polyline is drawn from vertex to vertex, not to the last one
  static const vertex_type l_shape[] = { { 0, 0 }, { 4, 0 }, { 4, 3 } };
  gpr_dc<5, 4> g;
  g.d.polyline(l_shape, 3U);
  CHECK_RASTER(g.head, PAL_X,
    "XXXXX",
    "....X",
    "....X",
    "....X");
  CHECK_EQ(g.head.present_count(), 1U);

  // the array is exactly as long as the count (the 4th vertex must not be read)
  static const vertex_type zigzag[] = { { 0, 3 }, { 2, 0 }, { 4, 3 }, { 6, 0 }, { 99, 99 } };
  gpr_dc<7, 4> z;
  z.d.polyline(zigzag, 4U);
  CHECK_RASTER(z.head, PAL_X,
    "..X...X",
    ".X.X.X.",
    ".X.X.X.",
    "X...X..");
  CHECK_EQ(z.head.oob_count(), 0U);

  // less than two vertices: nothing
  gpr_dc<5, 4> n;
  n.d.polyline(l_shape, 1U);
  n.d.polyline(l_shape, 0U);
  CHECK_EQ(n.pass.count, 0U);
  CHECK_EQ(n.head.present_count(), 0U);

  // two vertices is a line
  gpr_dc<5, 4> two;
  two.d.polyline(l_shape, 2U);
  CHECK_RASTER(two.head, PAL_X,
    "XXXXX",
    ".....",
    ".....",
    ".....");
}


TEST_CASE(gpr_triangle)
{
  gpr_dc<5, 5> g;
  g.d.triangle({ 0, 0 }, { 4, 0 }, { 0, 4 });
  CHECK_RASTER(g.head, PAL_X,
    "XXXXX",
    "X..X.",
    "X.X..",
    "XX...",
    "X....");
  CHECK_EQ(g.head.present_count(), 1U);
}


TEST_CASE(gpr_triangle_solid)
{
  gpr_dc<5, 5> g;
  g.d.triangle_solid({ 0, 0 }, { 4, 0 }, { 0, 4 });
  CHECK_RASTER(g.head, PAL_X,
    "XXXXX",
    "XXXX.",
    "XXX..",
    "XX...",
    "X....");
  CHECK_EQ(g.head.present_count(), 1U);

  // vertex order does not matter
  gpr_dc<5, 5> a, b, c;
  a.d.triangle_solid({ 0, 0 }, { 0, 4 }, { 4, 0 });
  b.d.triangle_solid({ 4, 0 }, { 0, 0 }, { 0, 4 });
  c.d.triangle_solid({ 0, 4 }, { 4, 0 }, { 0, 0 });
  CHECK_SAME_BUFFER(g.head, a.head);
  CHECK_SAME_BUFFER(g.head, b.head);
  CHECK_SAME_BUFFER(g.head, c.head);

  // the solid triangle contains its vertices and every pixel of the (Bresenham) outline
  // is part of the solid triangle or a direct neighbor of it
  gpr_dc<16, 12> s, o;
  s.d.triangle_solid({ 1, 1 }, { 14, 4 }, { 5, 10 });
  o.d.triangle({ 1, 1 }, { 14, 4 }, { 5, 10 });
  CHECK_EQ(s.head.pixel({ 1, 1 }), color::white);
  CHECK_EQ(s.head.pixel({ 14, 4 }), color::white);
  CHECK_EQ(s.head.pixel({ 5, 10 }), color::white);
  std::size_t far_away = 0U;
  for (std::int16_t y = 0; y < 12; ++y) {
    for (std::int16_t x = 0; x < 16; ++x) {
      if (o.head.pixel({ x, y }) == color::white) {
        const bool near = (s.head.pixel({ x, y }) == color::white) ||
                          (s.head.pixel({ static_cast<std::int16_t>(x - 1), y }) == color::white) ||
                          (s.head.pixel({ static_cast<std::int16_t>(x + 1), y }) == color::white) ||
                          (s.head.pixel({ x, static_cast<std::int16_t>(y - 1) }) == color::white) ||
                          (s.head.pixel({ x, static_cast<std::int16_t>(y + 1) }) == color::white);
        if (!near) {
          ++far_away;
        }
      }
    }
  }
  CHECK_EQ(far_away, 0U);
  // no solid pixel is outside the outline's bounding box
  for (std::int16_t y = 0; y < 12; ++y) {
    for (std::int16_t x = 0; x < 16; ++x) {
      if (s.head.pixel({ x, y }) == color::white) {
        CHECK(x >= 1 && x <= 14 && y >= 1 && y <= 10);
      }
    }
  }
  CHECK(vic_test::count_pixels(s.head, color::white) > vic_test::count_pixels(o.head, color::white));
  CHECK_EQ(s.head.oob_count(), 0U);
}


TEST_CASE(gpr_triangle_solid_degenerated)
{
  // collinear vertices: a line
  gpr_dc<5, 5> l;
  l.d.triangle_solid({ 0, 0 }, { 2, 2 }, { 4, 4 });
  CHECK_RASTER(l.head, PAL_X,
    "X....",
    ".X...",
    "..X..",
    "...X.",
    "....X");

  // all vertices equal: one pixel
  gpr_dc<5, 5> p;
  p.d.triangle_solid({ 2, 2 }, { 2, 2 }, { 2, 2 });
  CHECK_EQ(vic_test::count_pixels(p.head, color::white), 1U);
  CHECK_EQ(p.head.pixel({ 2, 2 }), color::white);

  // partially outside the screen
  gpr_dc<5, 5> c;
  c.d.triangle_solid({ -3, -3 }, { 8, 2 }, { 2, 8 });
  CHECK(vic_test::count_pixels(c.head, color::white) > 10U);
}


TEST_CASE(gpr_triangle_solid_alpha)
{
  // each pixel is written exactly once
  gpr_dc<8, 8> g;
  g.d.set_color(0x80FFFFFFUL);
  g.d.triangle_solid({ 0, 0 }, { 7, 0 }, { 0, 7 });
  CHECK_EQ(vic_test::count_pixels(g.head, 0x80FFFFFFUL), 36U);
  CHECK_EQ(g.pass.count, 36U);
}
