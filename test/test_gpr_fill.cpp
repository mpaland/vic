///////////////////////////////////////////////////////////////////////////////
// \brief Tests for gpr.h: flood fill
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "helpers.h"
#include "dc.h"
#include "head/memory.h"

using namespace vic;
using vic_test::PAL_X;


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
