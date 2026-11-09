///////////////////////////////////////////////////////////////////////////////
// \brief Tests for gpr.h: flood fill
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "helpers.h"
#include "dc.h"
#include "head/memory.h"
#include "head/framebuffer.h"
#include "shader/gradient.h"

using namespace vic;
using vic_test::PAL_X;


namespace {

// a memory head which counts the native line_horz() calls
class line_head : public head::memory<10, 8>
{
public:
  line_head() : lines_(0U) { }
  virtual void line_horz(vertex_type v0, vertex_type v1, color::value_type color)
  {
    ++lines_;
    drv::line_horz(v0, v1, color);
  }
  std::uint32_t lines_;
};

} // namespace


TEST_CASE(gpr_fill_bounded)
{
  // fill the inside of a rectangle, the frame color is the bounding color
  head::memory<10, 8> h;
  dc d(h);
  d.set_color(color::red);
  d.rectangle({ 1, 1, 9, 7 });
  d.set_color(color::white);
  d.fill({ 4, 4 }, color::red);
  CHECK_RASTER(h, vic_test::PAL_RG,
    "..........",
    ".RRRRRRRR.",
    ".RWWWWWWR.",
    ".RWWWWWWR.",
    ".RWWWWWWR.",
    ".RWWWWWWR.",
    ".RRRRRRRR.",
    "..........");
  CHECK_EQ(h.present_count(), 2U);
}


TEST_CASE(gpr_fill_background)
{
  // fill all connected pixels of the background color
  head::memory<10, 8> h;
  dc d(h);
  d.cls(color::blue);
  d.set_color(color::red);
  d.line_vert({ 5, 0 }, { 5, 7 });
  d.set_color(color::white);
  d.fill({ 2, 2 }, color::blue, false);
  CHECK_RASTER(h, vic_test::PAL_RG,
    "WWWWWRBBBB",
    "WWWWWRBBBB",
    "WWWWWRBBBB",
    "WWWWWRBBBB",
    "WWWWWRBBBB",
    "WWWWWRBBBB",
    "WWWWWRBBBB",
    "WWWWWRBBBB");
}


TEST_CASE(gpr_fill_whole_screen)
{
  head::memory<12, 10> h;
  dc d(h);
  d.set_color(color::white);
  d.fill({ 0, 0 }, color::none, false);
  CHECK_EQ(vic_test::count_pixels(h, color::white), 120U);
  CHECK_EQ(h.oob_count(), 0U);
}


TEST_CASE(gpr_fill_concave)
{
  // a U shaped region
  head::memory<9, 7> h;
  dc d(h);
  d.set_color(color::red);
  d.rectangle({ 0, 0, 9, 7 });
  d.box({ 3, 0, 6, 4 });
  d.set_color(color::white);
  d.fill({ 1, 1 }, color::red);
  CHECK_RASTER(h, vic_test::PAL_RG,
    "RRRRRRRRR",
    "RWWRRRWWR",
    "RWWRRRWWR",
    "RWWRRRWWR",
    "RWWWWWWWR",
    "RWWWWWWWR",
    "RRRRRRRRR");
}


TEST_CASE(gpr_fill_circle)
{
  head::memory<11, 11> h;
  dc d(h);
  d.set_color(color::red);
  d.circle({ 5, 5 }, 4U);
  d.set_color(color::white);
  d.fill({ 5, 5 }, color::red);
  // inside is white, the circle is red, outside untouched
  CHECK_EQ(h.pixel({ 5, 5 }), color::white);
  CHECK_EQ(h.pixel({ 2, 5 }), color::white);
  CHECK_EQ(h.pixel({ 8, 5 }), color::white);
  CHECK_EQ(h.pixel({ 5, 2 }), color::white);
  CHECK_EQ(h.pixel({ 1, 5 }), color::red);
  CHECK_EQ(h.pixel({ 0, 0 }), color::none);
  CHECK_EQ(h.pixel({ 10, 10 }), color::none);
  CHECK_EQ(h.pixel({ 0, 5 }), color::none);
  CHECK_EQ(vic_test::count_pixels(h, color::none), 121U - 57U);   // the disc of radius 4 has 57 pixels (red or white)
}


TEST_CASE(gpr_fill_start_on_bound)
{
  // starting on the bounding color fills nothing but the start pixel
  head::memory<6, 6> h;
  dc d(h);
  d.set_color(color::red);
  d.box({ 0, 0, 6, 6 });
  d.set_color(color::white);
  d.fill({ 2, 2 }, color::red);
  CHECK_EQ(vic_test::count_pixels(h, color::white), 1U);
}


TEST_CASE(gpr_fill_native_line_horz)
{
  // without shaders the spans are drawn with the native line_horz() of the head
  line_head h;
  dc d(h);
  d.set_color(color::red);
  d.rectangle({ 1, 1, 9, 7 });
  const std::uint32_t lines = h.lines_;
  d.set_color(color::white);
  d.fill({ 4, 4 }, color::red);
  CHECK_RASTER(h, vic_test::PAL_RG,
    "..........",
    ".RRRRRRRR.",
    ".RWWWWWWR.",
    ".RWWWWWWR.",
    ".RWWWWWWR.",
    ".RWWWWWWR.",
    ".RRRRRRRR.",
    "..........");
  // the start row is extended to the left and right, each of the other 3 rows is one span
  CHECK_EQ(h.lines_ - lines, 5U);
}


TEST_CASE(gpr_fill_shader_offset)
{
  // a shader which only moves the pixels: the spans are drawn through the shader pipe
  head::memory<10, 8> h;
  dc d(h);
  shader::offset off({ 1, 1 });
  d.shader_register(&off);
  d.set_color(color::red);
  d.rectangle({ 0, 0, 8, 6 });
  d.set_color(color::white);
  d.fill({ 3, 3 }, color::red);
  CHECK_RASTER(h, vic_test::PAL_RG,
    "..........",
    ".RRRRRRRR.",
    ".RWWWWWWR.",
    ".RWWWWWWR.",
    ".RWWWWWWR.",
    ".RWWWWWWR.",
    ".RRRRRRRR.",
    "..........");
  CHECK_EQ(h.present_count(), 2U);
}


TEST_CASE(gpr_fill_quantized_colors)
{
  // a RGB565 framebuffer stores red as 0xFFF80000 and white as 0xFFF8FCF8,
  // the bound and the filled pixels are only found if the colors are compared as stored
  head::memory<10, 8> m;
  head::framebuffer<10, 8, 1, color::format_RGB565> fb(m);
  dc d(fb);
  d.set_color(color::red);
  d.rectangle({ 1, 1, 9, 7 });
  d.set_color(color::white);
  d.fill({ 4, 4 }, color::red);
  CHECK_EQ(vic_test::count_pixels(m, 0xFFF8FCF8UL), 24U);   // the 6 x 4 inside
  CHECK_EQ(vic_test::count_pixels(m, 0xFFF80000UL), 24U);   // the frame is untouched
  CHECK_EQ(d.pixel_get({ 0, 0 }), 0xFF000000UL);            // outside is untouched (black framebuffer)

  // background mode with a color which RGB565 can't store exactly
  d.cls(0xFF123456UL);
  const color::value_type bg = d.pixel_get({ 5, 5 });
  CHECK(bg != 0xFF123456UL);
  d.set_color(0xFFABCDEFUL);
  d.fill({ 0, 0 }, 0xFF123456UL, false);
  const color::value_type filled = d.pixel_get({ 5, 5 });
  CHECK(filled != bg);
  CHECK_EQ(vic_test::count_pixels_not(m, filled), 0U);       // the whole screen is filled
}


TEST_CASE(gpr_fill_gradient_shader)
{
  // like demo_clock: a circle filled through a gradient shader, the pen is the bounding color
  head::memory<21, 21> h;
  dc d(h);
  d.set_color(color::gray);
  d.circle({ 10, 10 }, 8U);
  shader::gradient<2> gr = { { { 2, 10 }, color::darkred }, { { 18, 10 }, color::darkblue } };
  d.shader_register(&gr);
  d.fill({ 10, 10 }, color::gray);
  d.shader_remove(&gr);

  // the fill stays inside the circle: same pixel count as the disc
  head::memory<21, 21> ref;
  dc r(ref);
  r.disc({ 10, 10 }, 8U);
  CHECK_EQ(vic_test::count_pixels(h, color::none), vic_test::count_pixels(ref, color::none));
  CHECK_EQ(h.pixel({ 2, 10 }), color::gray);      // the circle is untouched
  CHECK(h.pixel({ 10, 10 }) != color::gray);      // the inside has the gradient colors
  CHECK(h.pixel({ 10, 10 }) != color::none);
}
