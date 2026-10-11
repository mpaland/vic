///////////////////////////////////////////////////////////////////////////////
// \brief Tests for the shader pipeline: base.h, gradient.h, brush.h, rotate.h
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "helpers.h"
#include "dc.h"
#include "shader/gradient.h"
#include "shader/brush.h"
#include "shader/rotate.h"
#include "sprite.h"
#include "head/memory.h"

using namespace vic;
using vic_test::PAL_X;

typedef head::memory<12, 10> head_type;


TEST_CASE(shader_output_alpha)
{
  head_type h;
  dc d(h);
  d.cls(color::blue);
  // default: alpha blending on
  d.plot({ 0, 0 }, 0x80FF0000UL);
  CHECK(color::is_opaque(h.pixel({ 0, 0 })));
  CHECK_EQ(color::get_green(h.pixel({ 0, 0 })), 0U);
  // off: raw value
  d.alpha_blending_enable(false);
  d.plot({ 1, 0 }, 0x80FF0000UL);
  CHECK_EQ(h.pixel({ 1, 0 }), 0x80FF0000UL);
  // transparent color on raw output
  d.plot({ 2, 0 }, color::none);
  CHECK_EQ(h.pixel({ 2, 0 }), color::none);
  // pixel_get passes through
  CHECK_EQ(d.pixel_get({ 1, 0 }), 0x80FF0000UL);
}


TEST_CASE(shader_alpha_blend)
{
  // explicit alpha blend shader while the output shader doesn't blend
  head_type h;
  dc d(h);
  d.cls(color::blue);
  d.alpha_blending_enable(false);
  shader::alpha_blend ab;
  d.shader_register(&ab);
  d.plot({ 0, 0 }, 0x80FF0000UL);
  const color::value_type c = h.pixel({ 0, 0 });
  CHECK(color::is_opaque(c));
  CHECK(color::get_red(c)  >= 127U && color::get_red(c)  <= 128U);
  CHECK(color::get_blue(c) >= 127U && color::get_blue(c) <= 128U);
  // opaque passes unchanged
  d.plot({ 1, 0 }, color::green);
  CHECK_EQ(h.pixel({ 1, 0 }), color::green);
  // pixel_get goes through the base class default to the next shader
  CHECK_EQ(d.pixel_get({ 1, 0 }), color::green);
}


TEST_CASE(shader_clipping)
{
  head_type h;
  dc d(h);
  d.set_color(color::white);

  // inside clipping: only pixels inside the region are drawn, right/bottom are outside
  shader::clipping clip({ 2, 2, 6, 5 });
  CHECK(clip.is_inside({ 2, 2 }));
  CHECK(clip.is_inside({ 5, 4 }));
  CHECK(!clip.is_inside({ 6, 4 }));
  CHECK(!clip.is_inside({ 5, 5 }));
  CHECK((clip.get() == rect_type{ 2, 2, 6, 5 }));
  d.shader_register(&clip);
  d.box({ 0, 0, 12, 10 });
  CHECK_RASTER(h, PAL_X,
    "............",
    "............",
    "..XXXX......",
    "..XXXX......",
    "..XXXX......",
    "............",
    "............",
    "............",
    "............",
    "............");

  // outside clipping: everything except the region
  d.cls();
  clip.set({ 2, 2, 6, 5 }, false);
  d.box({ 0, 0, 12, 10 });
  CHECK_EQ(vic_test::count_pixels(h, color::white), 120U - 12U);
  CHECK_EQ(h.pixel({ 3, 3 }), color::none);
  CHECK_EQ(h.pixel({ 6, 4 }), color::white);

  // default constructed clipping is disabled (empty region, outside mode): all pixels pass
  d.cls();
  d.shader_remove(&clip);
  shader::clipping none;
  d.shader_register(&none);
  d.box({ 0, 0, 12, 10 });
  CHECK_EQ(vic_test::count_pixels(h, color::white), 120U);
}


TEST_CASE(shader_zoom)
{
  head_type h;
  dc d(h);
  d.set_color(color::white);
  shader::zoom z(2U, 3U);
  d.shader_register(&z);
  d.plot({ 1, 1 });
  CHECK_RASTER(h, PAL_X,
    "............",
    "............",
    "............",
    "..XX........",
    "..XX........",
    "..XX........",
    "............",
    "............",
    "............",
    "............");
  // pixel_get maps back to the zoomed origin
  CHECK_EQ(d.pixel_get({ 1, 1 }), color::white);
  CHECK_EQ(d.pixel_get({ 0, 0 }), color::none);
  // a line of 3 pixels becomes 6 x 3
  d.cls();
  d.line_horz({ 0, 0 }, { 2, 0 });
  CHECK_EQ(vic_test::count_pixels(h, color::white), 18U);
}


TEST_CASE(shader_offset)
{
  head_type h;
  dc d(h);
  d.set_color(color::white);
  shader::offset off({ 3, 2 });
  CHECK((off.get() == vertex_type{ 3, 2 }));
  d.shader_register(&off);
  d.plot({ 1, 1 });
  CHECK_EQ(h.pixel({ 4, 3 }), color::white);
  CHECK_EQ(vic_test::count_pixels(h, color::white), 1U);
  CHECK_EQ(d.pixel_get({ 1, 1 }), color::white);
  off.set({ -1, -1 });
  CHECK((off.get() == vertex_type{ -1, -1 }));
  d.plot({ 1, 1 });
  CHECK_EQ(h.pixel({ 0, 0 }), color::white);
  // negative result is clipped by the head
  d.plot({ 0, 0 });
  CHECK_EQ(h.oob_count(), 1U);
}


TEST_CASE(shader_tint)
{
  head_type h;
  dc d(h);
  shader::tint t(color::red, 0U);
  d.shader_register(&t);

  // weight 0: pass through
  d.set_color(color::white);
  d.plot({ 1, 1 });
  CHECK_EQ(h.pixel({ 1, 1 }), color::white);

  // weight 255: the RGB part is the tint color
  t.set(color::red, 255U);
  d.plot({ 2, 1 });
  CHECK_EQ(h.pixel({ 2, 1 }), color::red);
  d.set_color(color::blue);
  d.plot({ 3, 1 });
  CHECK_EQ(h.pixel({ 3, 1 }), color::red);

  // weight 128: midpoint between the pixel and the tint color
  t.set(color::red, 128U);
  d.set_color(color::blue);
  d.plot({ 4, 1 });
  const color::value_type m = h.pixel({ 4, 1 });
  CHECK(color::is_opaque(m));
  CHECK(color::get_red(m)  >= 127U && color::get_red(m)  <= 128U);
  CHECK(color::get_blue(m) >= 127U && color::get_blue(m) <= 128U);
  CHECK_EQ(color::get_green(m), 0U);

  // the alpha of the pixel is kept: a half transparent white pixel becomes a half transparent red pixel,
  // which the output shader blends with the black background
  d.cls(color::black);
  t.set(color::red, 255U);
  d.set_color(color::argb(0xFFU, 0xFFU, 0xFFU, 0x80U));
  d.plot({ 5, 1 });
  const color::value_type b = h.pixel({ 5, 1 });
  CHECK(color::get_red(b) >= 127U && color::get_red(b) <= 129U);
  CHECK_EQ(color::get_green(b), 0U);
  CHECK_EQ(color::get_blue(b), 0U);

  // a tinted box on a canvas sprite: all pattern pixels are tinted, the alpha is kept
  head_type hs;
  dc ds(hs);
  ds.cls(color::black);
  sprite::canvas<64, 1> s(hs, 0);
  shader::tint st(color::green, 255U);
  s.shader_register(&st);
  s.set_color(color::argb(0xFFU, 0xFFU, 0xFFU, 0x80U));
  s.box({ 0, 0, 2, 2 });
  s.shader_remove(&st);
  s.render({ 3, 3 });
  const color::value_type sp = hs.pixel({ 3, 3 });
  CHECK_EQ(color::get_red(sp), 0U);
  CHECK(color::get_green(sp) >= 127U && color::get_green(sp) <= 129U);
  CHECK_EQ(vic_test::count_pixels_not(hs, color::black), 4U);
}


TEST_CASE(shader_flip)
{
  head_type h;
  dc d(h);
  d.set_color(color::white);
  shader::flip f;
  d.shader_register(&f);

  // disabled: identity
  d.plot({ 2, 1 });
  CHECK_EQ(h.pixel({ 2, 1 }), color::white);

  // horizontal flip around x = 5: 2 -> 8
  d.cls();
  f.set_horizontal(true, { 5, 0 });
  d.plot({ 2, 1 });
  CHECK_EQ(h.pixel({ 8, 1 }), color::white);
  CHECK_EQ(vic_test::count_pixels(h, color::white), 1U);
  CHECK_EQ(d.pixel_get({ 2, 1 }), color::white);

  // vertical flip around y = 4: 1 -> 7
  d.cls();
  f.set_horizontal(false, { 5, 0 });
  f.set_vertical(true, { 0, 4 });
  d.plot({ 2, 1 });
  CHECK_EQ(h.pixel({ 2, 7 }), color::white);
  CHECK_EQ(vic_test::count_pixels(h, color::white), 1U);

  // both
  d.cls();
  f.set_horizontal(true, { 5, 0 });
  d.plot({ 2, 1 });
  CHECK_EQ(h.pixel({ 8, 7 }), color::white);
  CHECK_EQ(vic_test::count_pixels(h, color::white), 1U);

  // a flipped triangle is the mirrored triangle
  head_type a, b;
  dc da(a), db(b);
  da.set_color(color::white);
  db.set_color(color::white);
  shader::flip fh;
  fh.set_horizontal(true, { 5, 0 });
  db.shader_register(&fh);
  da.triangle_solid({ 8, 1 }, { 10, 8 }, { 6, 8 });
  db.triangle_solid({ 2, 1 }, { 0, 8 }, { 4, 8 });
  CHECK_SAME_BUFFER(a, b);
}


///////////////////////////////////////////////////////////////////////////////
// G R A D I E N T

TEST_CASE(shader_gradient_two_points)
{
  head::memory<11, 3> h;
  dc d(h);
  d.alpha_blending_enable(false);
  d.set_color(color::white);
  shader::gradient<2> gr = { { 0, 1, color::red }, { 10, 1, color::blue } };
  d.shader_register(&gr);
  d.line_horz({ 0, 1 }, { 10, 1 });

  // at the gradient vertices (nearly) the vertex color, opaque
  const color::value_type l = h.pixel({ 0, 1 });
  const color::value_type r = h.pixel({ 10, 1 });
  const color::value_type m = h.pixel({ 5, 1 });
  CHECK_EQ(color::get_alpha(l), 255U);
  CHECK_EQ(color::get_alpha(r), 255U);
  CHECK_EQ(color::get_alpha(m), 255U);
  CHECK(color::get_red(l) >= 250U);
  CHECK(color::get_blue(l) <= 5U);
  CHECK(color::get_blue(r) >= 250U);
  CHECK(color::get_red(r) <= 5U);
  CHECK_EQ(color::get_green(l), 0U);
  CHECK_EQ(color::get_green(m), 0U);
  // in the middle both colors have the same weight, no channel overflow
  CHECK(color::get_red(m) >= 120U && color::get_red(m) <= 135U);
  CHECK(color::get_blue(m) >= 120U && color::get_blue(m) <= 135U);
  // monotonic: red decreases, blue increases from left to right
  for (std::int16_t x = 1; x <= 10; ++x) {
    CHECK(color::get_red(h.pixel({ x, 1 }))  <= color::get_red(h.pixel({ static_cast<std::int16_t>(x - 1), 1 })));
    CHECK(color::get_blue(h.pixel({ x, 1 })) >= color::get_blue(h.pixel({ static_cast<std::int16_t>(x - 1), 1 })));
  }
}


TEST_CASE(shader_gradient_same_colors)
{
  // a gradient between identical colors is exactly that color everywhere (no carry between channels)
  head::memory<11, 3> h;
  dc d(h);
  d.alpha_blending_enable(false);
  d.set_color(color::white);
  shader::gradient<2> gr = { { 0, 1, color::white }, { 10, 1, color::white } };
  d.shader_register(&gr);
  d.box({ 0, 0, 11, 3 });
  CHECK_EQ(vic_test::count_pixels(h, color::white), 33U);

  // single point gradient
  head::memory<11, 3> s;
  dc ds(s);
  ds.alpha_blending_enable(false);
  ds.set_color(color::white);
  shader::gradient<1> g1 = { { 5, 1, 0xFF123456UL } };
  ds.shader_register(&g1);
  ds.box({ 0, 0, 11, 3 });
  CHECK_EQ(vic_test::count_pixels(s, 0xFF123456UL), 33U);

  // four points with the same color
  head::memory<11, 3> f;
  dc df(f);
  df.alpha_blending_enable(false);
  df.set_color(color::white);
  shader::gradient<4> g4 = { { 0, 0, color::darkred }, { 10, 0, color::darkred }, { 0, 2, color::darkred }, { 10, 2, color::darkred } };
  df.shader_register(&g4);
  df.box({ 0, 0, 11, 3 });
  CHECK_EQ(vic_test::count_pixels(f, color::darkred), 33U);
}


TEST_CASE(shader_gradient_alpha)
{
  // the pen alpha is multiplied with the gradient alpha
  head::memory<11, 3> h;
  dc d(h);
  d.alpha_blending_enable(false);
  shader::gradient<2> gr = { { 0, 1, color::white }, { 10, 1, color::white } };
  d.shader_register(&gr);
  d.set_color(0x80000000UL);     // the pen color itself is ignored, only its alpha counts
  d.plot({ 5, 1 });
  CHECK_EQ(h.pixel({ 5, 1 }), 0x80FFFFFFUL);
  d.set_color(color::white);
  d.plot({ 6, 1 });
  CHECK_EQ(h.pixel({ 6, 1 }), color::white);
  // transparent gradient points give transparent pixels
  shader::gradient<2> gt = { { 0, 1, 0x00FFFFFFUL }, { 10, 1, 0x00FFFFFFUL } };
  d.shader_remove(&gr);
  d.shader_register(&gt);
  d.plot({ 7, 1 });
  CHECK_EQ(color::get_alpha(h.pixel({ 7, 1 })), 0U);
}


TEST_CASE(shader_gradient_horizontal_vertical)
{
  head::memory<11, 5> h;
  dc d(h);
  d.alpha_blending_enable(false);
  d.set_color(color::white);
  shader::gradient_horizontal<2> gh = { { 0, 0, color::red }, { 10, 0, color::blue } };
  d.shader_register(&gh);
  d.box({ 0, 0, 11, 5 });
  // y is ignored: all rows are equal
  for (std::int16_t x = 0; x < 11; ++x) {
    for (std::int16_t y = 1; y < 5; ++y) {
      CHECK_EQ(h.pixel({ x, y }), h.pixel({ x, 0 }));
    }
  }
  CHECK(color::get_red(h.pixel({ 0, 0 })) >= 250U);
  CHECK(color::get_blue(h.pixel({ 10, 4 })) >= 250U);
  d.shader_remove(&gh);

  shader::gradient_vertical<2> gv = { { 0, 0, color::red }, { 0, 4, color::blue } };
  d.shader_register(&gv);
  d.box({ 0, 0, 11, 5 });
  for (std::int16_t y = 0; y < 5; ++y) {
    for (std::int16_t x = 1; x < 11; ++x) {
      CHECK_EQ(h.pixel({ x, y }), h.pixel({ 0, y }));
    }
  }
  // the two points are only 4 pixels apart, so the far point still has some weight
  CHECK(color::get_red(h.pixel({ 5, 0 })) >= 230U);
  CHECK(color::get_blue(h.pixel({ 5, 4 })) >= 230U);
  CHECK(color::get_red(h.pixel({ 5, 0 })) > color::get_blue(h.pixel({ 5, 0 })));
  CHECK(color::get_blue(h.pixel({ 5, 4 })) > color::get_red(h.pixel({ 5, 4 })));
}


TEST_CASE(shader_gradient_solid)
{
  // the color of the nearest point, exactly
  head::memory<11, 3> h;
  dc d(h);
  d.alpha_blending_enable(false);
  d.set_color(color::white);
  shader::gradient_solid<3> gs = { { 0, 1, color::red }, { 5, 1, color::green }, { 10, 1, color::blue } };
  d.shader_register(&gs);
  d.line_horz({ 0, 1 }, { 10, 1 });
  CHECK_EQ(h.pixel({ 0, 1 }), color::red);
  CHECK_EQ(h.pixel({ 1, 1 }), color::red);
  CHECK_EQ(h.pixel({ 2, 1 }), color::red);
  CHECK_EQ(h.pixel({ 4, 1 }), color::green);
  CHECK_EQ(h.pixel({ 5, 1 }), color::green);
  CHECK_EQ(h.pixel({ 6, 1 }), color::green);
  CHECK_EQ(h.pixel({ 8, 1 }), color::blue);
  CHECK_EQ(h.pixel({ 10, 1 }), color::blue);
}


TEST_CASE(shader_gradient_set)
{
  // array ctor and set()
  static const pixel_type pts[2] = { { { 0, 0 }, color::red }, { { 10, 0 }, color::red } };
  head::memory<11, 3> h;
  dc d(h);
  d.alpha_blending_enable(false);
  d.set_color(color::white);
  shader::gradient<2> gr(pts);
  d.shader_register(&gr);
  d.plot({ 5, 0 });
  CHECK_EQ(h.pixel({ 5, 0 }), color::red);
  static const pixel_type pts2[3] = { { { 0, 0 }, color::blue }, { { 10, 0 }, color::blue }, { { 5, 0 }, color::blue } };
  gr.set(pts2, 3U);    // more than Max_Pixel: only the first two are taken
  d.plot({ 5, 1 });
  CHECK_EQ(h.pixel({ 5, 1 }), color::blue);
  gr.set(pts, 1U);
  d.plot({ 5, 2 });
  CHECK_EQ(h.pixel({ 5, 2 }), color::red);
}


///////////////////////////////////////////////////////////////////////////////
// B R U S H

TEST_CASE(shader_brush_default)
{
  // a brush without an explicit shape is a 1 pixel solid pen
  head_type h;
  dc d(h);
  d.set_color(color::white);
  shader::brush br;
  d.shader_register(&br);
  d.plot({ 3, 3 });
  CHECK_EQ(vic_test::count_pixels(h, color::white), 1U);
  CHECK_EQ(h.pixel({ 3, 3 }), color::white);
}


TEST_CASE(shader_brush_stock_shapes)
{
  shader::brush br;
  shader::brush::shape_type shape;
  for (std::uint8_t w = 1U; w <= 4U; ++w) {
    CHECK(br.get_stock_shape(shape, shader::brush::style_solid, w));
    CHECK_EQ(shape.width, w);
    CHECK_EQ(shape.height, w);
    CHECK(shape.alpha != nullptr);
    CHECK_EQ(static_cast<int>(shape.style), static_cast<int>(shader::brush::style_solid));
  }
  // invalid widths are rejected
  shape.width = 99U;
  CHECK(!br.get_stock_shape(shape, shader::brush::style_solid, 0U));
  CHECK(!br.get_stock_shape(shape, shader::brush::style_solid, 5U));
  CHECK_EQ(shape.width, 99U);   // unchanged
}


TEST_CASE(shader_brush_solid_width)
{
  head_type h;
  dc d(h);
  d.set_color(color::white);
  shader::brush br;
  shader::brush::shape_type shape;
  br.get_stock_shape(shape, shader::brush::style_solid, 2U);
  br.set_shape(shape);
  d.shader_register(&br);
  d.plot({ 3, 3 });
  CHECK_RASTER(h, PAL_X,
    "............",
    "............",
    "..XX........",
    "..XX........",
    "............",
    "............",
    "............",
    "............",
    "............",
    "............");

  // width 3 has half transparent corners
  d.cls();
  br.get_stock_shape(shape, shader::brush::style_solid, 3U);
  br.set_shape(shape);
  d.alpha_blending_enable(false);
  d.plot({ 3, 3 });
  CHECK_EQ(h.pixel({ 3, 3 }), color::white);
  CHECK_EQ(h.pixel({ 2, 3 }), color::white);
  CHECK_EQ(h.pixel({ 2, 2 }), 0x80FFFFFFUL);
  CHECK_EQ(h.pixel({ 4, 4 }), 0x80FFFFFFUL);
  CHECK_EQ(vic_test::count_pixels_not(h, color::none), 9U);

  // a thick line
  d.cls();
  d.alpha_blending_enable(true);
  br.get_stock_shape(shape, shader::brush::style_solid, 2U);
  br.set_shape(shape);
  d.line_horz({ 1, 5 }, { 10, 5 });
  // the 2 pixel pen extends one pixel to the left/top of the vertex
  CHECK_RASTER(h, PAL_X,
    "............",
    "............",
    "............",
    "............",
    "XXXXXXXXXXX.",
    "XXXXXXXXXXX.",
    "............",
    "............",
    "............",
    "............");
}


TEST_CASE(shader_brush_styles)
{
  head_type h;
  dc d(h);
  d.set_color(color::white);
  shader::brush br;
  shader::brush::shape_type shape;
  d.shader_register(&br);

  // dotted: every second pixel
  br.get_stock_shape(shape, shader::brush::style_dot, 1U);
  br.set_shape(shape);
  br.reset();
  d.line_horz({ 0, 0 }, { 11, 0 });
  CHECK_RASTER_PART(h, PAL_X, (vertex_type{ 0, 0 }), "X.X.X.X.X.X.");

  // dashed: 3 on, 1 off
  br.get_stock_shape(shape, shader::brush::style_dash, 1U);
  br.set_shape(shape);
  br.reset();
  d.line_horz({ 0, 2 }, { 11, 2 });
  CHECK_RASTER_PART(h, PAL_X, (vertex_type{ 0, 2 }), "XXX.XXX.XXX.");

  // dash dot: 3 on, 1 off, 1 on, 1 off
  br.get_stock_shape(shape, shader::brush::style_dashdot, 1U);
  br.set_shape(shape);
  br.reset();
  d.line_horz({ 0, 4 }, { 11, 4 });
  CHECK_RASTER_PART(h, PAL_X, (vertex_type{ 0, 4 }), "XXX.X.XXX.X.");

  // reset restarts the pattern
  br.get_stock_shape(shape, shader::brush::style_dot, 1U);
  br.set_shape(shape);
  br.reset();
  d.line_horz({ 0, 6 }, { 2, 6 });   // X.X -> next would be off
  br.reset();
  d.line_horz({ 3, 6 }, { 5, 6 });   // restart: X.X
  CHECK_RASTER_PART(h, PAL_X, (vertex_type{ 0, 6 }), "X.XX.X");
}


///////////////////////////////////////////////////////////////////////////////
// R O T A T E

TEST_CASE(shader_rotate_right_angles)
{
  head_type h;
  dc d(h);
  d.set_color(color::white);
  shader::rotate rot({ 5, 5 }, 90);
  d.shader_register(&rot);

  // a horizontal line through the center becomes a vertical line
  d.line_horz({ 2, 5 }, { 8, 5 });
  CHECK_EQ(vic_test::count_pixels(h, color::white), 7U);
  for (std::int16_t y = 2; y <= 8; ++y) {
    CHECK_EQ(h.pixel({ 5, y }), color::white);
  }
  // pixel_get reads back through the rotation
  CHECK_EQ(d.pixel_get({ 2, 5 }), color::white);
  CHECK_EQ(d.pixel_get({ 5, 2 }), color::none);

  // 180 degree: point mirror
  d.cls();
  rot.set({ 5, 5 }, 180);
  d.plot({ 2, 3 });
  CHECK_EQ(h.pixel({ 8, 7 }), color::white);
  CHECK_EQ(vic_test::count_pixels(h, color::white), 1U);

  // 0 degree: identity
  d.cls();
  rot.set({ 5, 5 }, 0);
  d.box({ 1, 1, 4, 4 });
  CHECK_EQ(vic_test::count_pixels(h, color::white), 9U);
  CHECK_EQ(h.pixel({ 1, 1 }), color::white);
  CHECK_EQ(h.pixel({ 3, 3 }), color::white);
}


TEST_CASE(shader_rotate_45)
{
  // a rotated box must stay connected (no holes) and keep roughly its area
  head::memory<24, 24> h;
  dc d(h);
  d.set_color(color::white);
  shader::rotate rot({ 12, 12 }, 45);
  d.shader_register(&rot);
  d.box({ 8, 8, 16, 16 });
  const std::size_t cnt = vic_test::count_pixels(h, color::white);
  if (!CHECK(cnt >= 60U && cnt <= 64U + 40U)) {
    std::printf("    count: %u\n", static_cast<unsigned>(cnt));
    vic_test::dump_raster(h, PAL_X, "rotated box");
  }
  // the center stays, the far corners are empty
  CHECK_EQ(h.pixel({ 12, 12 }), color::white);
  CHECK_EQ(h.pixel({ 0, 0 }), color::none);
  // no interior holes: every pixel with white left/right and up/down neighbors is white
  std::size_t holes = 0U;
  for (std::int16_t y = 1; y < 23; ++y) {
    for (std::int16_t x = 1; x < 23; ++x) {
      if ((h.pixel({ x, y }) == color::none) &&
          (h.pixel({ static_cast<std::int16_t>(x - 1), y }) == color::white) && (h.pixel({ static_cast<std::int16_t>(x + 1), y }) == color::white) &&
          (h.pixel({ x, static_cast<std::int16_t>(y - 1) }) == color::white) && (h.pixel({ x, static_cast<std::int16_t>(y + 1) }) == color::white)) {
        ++holes;
      }
    }
  }
  CHECK_EQ(holes, 0U);
}


namespace {

// a head which can't read its pixels back
class blind_head : public head_type
{
protected:
  virtual color::value_type pixel_get(vertex_type vertex)
  {
    (void)vertex;
    return color::none;
  }
};

} // namespace


TEST_CASE(shader_output_unknown_background)
{
  blind_head h;
  dc d(h);
  d.cls(color::blue);
  // without a background color the pixel is blended against 'none' (unchanged behaviour)
  d.plot({ 1, 0 }, 0x80FF0000UL);
  CHECK_EQ(h.pixel({ 1, 0 }), color::alpha_blend(0x80FF0000UL, color::none));
  // with a background color the output shader blends against it, pixel_get() stays 'unknown'
  d.set_bg_color(color::blue);
  CHECK_EQ(d.pixel_get({ 0, 0 }), color::none);
  d.plot({ 0, 0 }, 0x80FF0000UL);
  const color::value_type b = h.pixel({ 0, 0 });
  CHECK(color::is_opaque(b));
  CHECK(color::get_red(b) >= 127U && color::get_red(b) <= 128U);
  CHECK(color::get_blue(b) >= 127U && color::get_blue(b) <= 128U);
  CHECK_EQ(color::get_green(b), 0U);
  // anti aliased edges are blended against the background color as well: same result as on a
  // readable head cleared to that color
  head_type m;
  dc dm(m);
  dm.cls(color::blue);
  d.cls(color::blue);
  d.anti_aliasing_enable(true);
  dm.anti_aliasing_enable(true);
  d.set_color(color::white);
  dm.set_color(color::white);
  // (only drawing on the uniform background is exact: the blind head blends against the background
  //  color, not against pixels drawn before, so a pixel written twice differs)
  d.disc({ 3, 6 }, 2U);
  dm.disc({ 3, 6 }, 2U);
  CHECK_SAME_BUFFER(h, m);
  // an anti aliased line on the blind head consists of opaque blends of white and blue only
  d.line({ 7, 0 }, { 11, 9 });
  for (std::int16_t y = 0; y < 10; ++y) {
    for (std::int16_t x = 7; x < 12; ++x) {
      const color::value_type c = h.pixel({ x, y });
      CHECK(color::is_opaque(c));
      CHECK_EQ(color::get_red(c), color::get_green(c));
      CHECK(color::get_blue(c) >= color::get_red(c));
    }
  }
}
