///////////////////////////////////////////////////////////////////////////////
// \brief Tests for gpr.h: box and rectangle, with and without border radius
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "helpers.h"
#include "dc.h"
#include "head/memory.h"

using namespace vic;
using vic_test::PAL_X;

namespace {
  // dc on the gpr path (passthrough shader registered)
  template<std::uint16_t X, std::uint16_t Y>
  struct gpr_dc {
    head::memory<X, Y>          head;
    dc                          d;
    vic_test::passthrough_shader pass;
    gpr_dc() : d(head) { d.shader_register(&pass); d.set_color(color::white); }
  };
}


TEST_CASE(gpr_box)
{
  gpr_dc<8, 6> g;
  g.d.box({ 2, 1, 5, 4 });
  CHECK_RASTER(g.head, PAL_X,
    "........",
    "..XXX...",
    "..XXX...",
    "..XXX...",
    "........",
    "........");
  CHECK_EQ(g.head.present_count(), 1U);
  CHECK_EQ(g.pass.count, 9U);

  // empty rects
  gpr_dc<8, 6> e;
  e.d.box({ 3, 3, 3, 5 });
  e.d.box({ 3, 3, 6, 3 });
  CHECK_EQ(e.pass.count, 0U);

  // vertex wrapper: both vertices inside
  gpr_dc<8, 6> v;
  v.d.box(vertex_type{ 4, 3 }, vertex_type{ 2, 1 });
  CHECK_RASTER(v.head, PAL_X,
    "........",
    "..XXX...",
    "..XXX...",
    "..XXX...",
    "........",
    "........");

  // clipping
  gpr_dc<8, 6> c;
  c.d.box({ -3, -3, 20, 20 });
  CHECK_EQ(vic_test::count_pixels(c.head, color::white), 48U);
}


TEST_CASE(gpr_box_alpha)
{
  // each pixel is written exactly once
  gpr_dc<8, 6> g;
  g.d.set_color(0x80FFFFFFUL);
  g.d.box({ 1, 1, 7, 5 });
  CHECK_EQ(vic_test::count_pixels(g.head, 0x80FFFFFFUL), 24U);
  CHECK_EQ(vic_test::count_pixels(g.head, color::none), 24U);
}


TEST_CASE(gpr_rectangle)
{
  gpr_dc<10, 6> g;
  g.d.rectangle({ 0, 0, 10, 6 });
  CHECK_RASTER(g.head, PAL_X,
    "XXXXXXXXXX",
    "X........X",
    "X........X",
    "X........X",
    "X........X",
    "XXXXXXXXXX");
  CHECK_EQ(g.head.present_count(), 1U);

  // vertex wrapper, both vertices are on the frame
  gpr_dc<10, 6> v;
  v.d.rectangle(vertex_type{ 9, 5 }, vertex_type{ 0, 0 });
  CHECK_SAME_BUFFER(g.head, v.head);

  // degenerated rects
  gpr_dc<10, 6> p;
  p.d.rectangle({ 3, 3, 4, 4 });          // one pixel
  CHECK_EQ(vic_test::count_pixels(p.head, color::white), 1U);
  CHECK_EQ(p.head.pixel({ 3, 3 }), color::white);
  gpr_dc<10, 6> l;
  l.d.rectangle({ 3, 1, 4, 5 });          // vertical line
  CHECK_RASTER(l.head, PAL_X,
    "..........",
    "...X......",
    "...X......",
    "...X......",
    "...X......",
    "..........");
  gpr_dc<10, 6> m;
  m.d.rectangle({ 1, 2, 8, 3 });          // horizontal line
  CHECK_EQ(vic_test::count_pixels(m.head, color::white), 7U);
  gpr_dc<10, 6> e;
  e.d.rectangle({ 3, 3, 3, 7 });          // empty
  CHECK_EQ(e.pass.count, 0U);
}


TEST_CASE(gpr_rectangle_alpha)
{
  // the corners must not be drawn twice
  gpr_dc<10, 6> g;
  g.d.set_color(0x80FFFFFFUL);
  g.d.rectangle({ 0, 0, 10, 6 });
  CHECK_EQ(vic_test::count_pixels(g.head, 0x80FFFFFFUL), 28U);
  CHECK_EQ(g.pass.count, 28U);
}


TEST_CASE(gpr_box_rounded)
{
  gpr_dc<10, 6> g;
  g.d.box(rect_type{ 0, 0, 10, 6 }, 2U);
  CHECK_RASTER(g.head, PAL_X,
    ".XXXXXXXX.",
    "XXXXXXXXXX",
    "XXXXXXXXXX",
    "XXXXXXXXXX",
    "XXXXXXXXXX",
    ".XXXXXXXX.");
  CHECK_EQ(g.head.present_count(), 1U);

  gpr_dc<10, 8> r3;
  r3.d.box(rect_type{ 0, 0, 10, 8 }, 3U);
  CHECK_RASTER(r3.head, PAL_X,
    "..XXXXXX..",
    ".XXXXXXXX.",
    "XXXXXXXXXX",
    "XXXXXXXXXX",
    "XXXXXXXXXX",
    "XXXXXXXXXX",
    ".XXXXXXXX.",
    "..XXXXXX..");

  // radius 0 is a plain box
  gpr_dc<10, 6> z;
  z.d.box(rect_type{ 1, 1, 8, 5 }, 0U);
  gpr_dc<10, 6> b;
  b.d.box(rect_type{ 1, 1, 8, 5 });
  CHECK_SAME_BUFFER(z.head, b.head);

  // vertex wrapper
  gpr_dc<10, 6> v;
  v.d.box(vertex_type{ 9, 5 }, vertex_type{ 0, 0 }, 2U);
  CHECK_SAME_BUFFER(v.head, g.head);

  // a too big radius is clamped, nothing is drawn outside
  gpr_dc<10, 6> c;
  c.d.box(rect_type{ 0, 0, 10, 6 }, 100U);
  CHECK_EQ(c.head.oob_count(), 0U);
  CHECK(vic_test::count_pixels(c.head, color::white) >= 50U);
  CHECK_EQ(c.head.pixel({ 0, 0 }), color::none);
  CHECK_EQ(c.head.pixel({ 5, 0 }), color::white);
  CHECK_EQ(c.head.pixel({ 5, 5 }), color::white);
  CHECK_EQ(c.head.pixel({ 0, 3 }), color::white);
  CHECK_EQ(c.head.pixel({ 9, 3 }), color::white);

  // tiny rects fall back to a plain box
  gpr_dc<10, 6> t;
  t.d.box(rect_type{ 2, 2, 3, 3 }, 2U);
  CHECK_EQ(vic_test::count_pixels(t.head, color::white), 1U);
  gpr_dc<10, 6> t2;
  t2.d.box(rect_type{ 2, 2, 4, 4 }, 5U);
  CHECK_EQ(vic_test::count_pixels(t2.head, color::white), 4U);
}


TEST_CASE(gpr_box_rounded_alpha)
{
  // each pixel is written exactly once
  gpr_dc<10, 6> g;
  g.d.set_color(0x80FFFFFFUL);
  g.d.box(rect_type{ 0, 0, 10, 6 }, 2U);
  CHECK_EQ(vic_test::count_pixels(g.head, 0x80FFFFFFUL), 56U);
  CHECK_EQ(g.pass.count, 56U);
}


TEST_CASE(gpr_rectangle_rounded)
{
  gpr_dc<10, 6> g;
  g.d.rectangle(rect_type{ 0, 0, 10, 6 }, 2U);
  CHECK_RASTER(g.head, PAL_X,
    ".XXXXXXXX.",
    "X........X",
    "X........X",
    "X........X",
    "X........X",
    ".XXXXXXXX.");
  CHECK_EQ(g.head.present_count(), 1U);

  // bigger rect with straight segments on all sides
  gpr_dc<12, 10> b;
  b.d.rectangle(rect_type{ 1, 1, 11, 9 }, 2U);
  CHECK_RASTER_AT(b.head, PAL_X, (vertex_type{ 1, 1 }),
    ".XXXXXXXX.",
    "X........X",
    "X........X",
    "X........X",
    "X........X",
    "X........X",
    "X........X",
    ".XXXXXXXX.");

  // radius 0 is a plain rectangle
  gpr_dc<10, 6> z;
  z.d.rectangle(rect_type{ 0, 0, 10, 6 }, 0U);
  gpr_dc<10, 6> p;
  p.d.rectangle(rect_type{ 0, 0, 10, 6 });
  CHECK_SAME_BUFFER(z.head, p.head);

  // vertex wrapper
  gpr_dc<10, 6> v;
  v.d.rectangle(vertex_type{ 9, 5 }, vertex_type{ 0, 0 }, 2U);
  CHECK_SAME_BUFFER(v.head, g.head);

  // clamped radius, nothing outside
  gpr_dc<10, 6> c;
  c.d.rectangle(rect_type{ 0, 0, 10, 6 }, 100U);
  CHECK_EQ(c.head.oob_count(), 0U);
  CHECK_EQ(c.head.pixel({ 0, 0 }), color::none);
  CHECK_EQ(c.head.pixel({ 5, 0 }), color::white);
  CHECK_EQ(c.head.pixel({ 0, 3 }), color::white);
}


TEST_CASE(gpr_rectangle_rounded_alpha)
{
  gpr_dc<10, 6> g;
  g.d.set_color(0x80FFFFFFUL);
  g.d.rectangle(rect_type{ 0, 0, 10, 6 }, 2U);
  CHECK_EQ(vic_test::count_pixels(g.head, 0x80FFFFFFUL), 24U);
  CHECK_EQ(g.pass.count, 24U);
}
