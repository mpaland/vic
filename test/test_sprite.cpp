///////////////////////////////////////////////////////////////////////////////
// \brief Tests for sprite.h: canvas, sheet and canvas_sheet sprites
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "helpers.h"
#include "sprite.h"
#include "head/memory.h"

using namespace vic;
using vic_test::PAL_X;

typedef head::memory<16, 12> head_type;


TEST_CASE(sprite_canvas_render_restore)
{
  head_type h;
  dc d(h);
  d.cls(color::blue);
  h.reset_counters();

  sprite::canvas<64, 1> s(h, 0);
  s.set_color(color::white);
  s.box({ 0, 0, 3, 2 });                  // 3 x 2 pixel pattern
  CHECK_EQ(s.pixel_count(), 6U);
  // drawing on the canvas doesn't touch the head
  CHECK_EQ(vic_test::count_pixels(h, color::white), 0U);

  s.render({ 2, 3 });
  CHECK((s.get_position() == vertex_type{ 2, 3 }));
  CHECK_RASTER_PART(h, PAL_X, (vertex_type{ 2, 3 }),
    "XXX",
    "XXX");
  CHECK_EQ(vic_test::count_pixels(h, color::white), 6U);
  CHECK_EQ(h.present_count(), 1U);

  // move the sprite: the old position is restored with the background
  s.render({ 5, 3 });
  CHECK_EQ(vic_test::count_pixels(h, color::white), 6U);
  CHECK_EQ(vic_test::count_pixels(h, color::blue), 16U * 12U - 6U);
  CHECK_EQ(h.pixel({ 2, 3 }), color::blue);
  CHECK_EQ(h.pixel({ 5, 3 }), color::white);
  CHECK_EQ(h.pixel({ 7, 4 }), color::white);

  // overlapping move
  s.render({ 6, 4 });
  CHECK_EQ(vic_test::count_pixels(h, color::white), 6U);
  CHECK_EQ(h.pixel({ 5, 3 }), color::blue);
  CHECK_EQ(h.pixel({ 6, 4 }), color::white);
  CHECK_EQ(h.pixel({ 8, 5 }), color::white);

  // render partially outside the screen
  s.render({ -1, -1 });
  CHECK_EQ(vic_test::count_pixels(h, color::white), 2U);
  CHECK_EQ(h.pixel({ 0, 0 }), color::white);
  CHECK_EQ(h.pixel({ 1, 0 }), color::white);
  s.render({ 14, 11 });
  CHECK_EQ(vic_test::count_pixels(h, color::white), 2U);
  CHECK_EQ(vic_test::count_pixels(h, color::blue), 16U * 12U - 2U);

  // the background is restored when the canvas is cleared and rendered
  s.cls();
  CHECK_EQ(s.pixel_count(), 0U);
  s.render({ 14, 11 });
  CHECK_EQ(vic_test::count_pixels(h, color::blue), 16U * 12U);
}


TEST_CASE(sprite_canvas_pattern_edit)
{
  head_type h;
  sprite::canvas<64, 1> s(h, 0);
  s.set_color(color::white);
  s.box({ 0, 0, 4, 4 });
  CHECK_EQ(s.pixel_count(), 16U);
  // punch out a pixel with color::none
  s.pixel_set({ 1, 1 }, color::none);
  CHECK_EQ(s.pixel_count(), 15U);
  // punching out a non existing pixel does nothing
  s.pixel_set({ 10, 10 }, color::none);
  CHECK_EQ(s.pixel_count(), 15U);
  // pixel_get of the canvas
  CHECK_EQ(s.pixel_get({ 0, 0 }), color::white);
  CHECK_EQ(s.pixel_get({ 1, 1 }), color::none);
  CHECK_EQ(s.pixel_get({ 10, 10 }), color::none);
  // drawing over an existing pixel blends
  s.pixel_set({ 0, 0 }, 0x80FF0000UL);
  const color::value_type c = s.pixel_get({ 0, 0 });
  CHECK(color::is_opaque(c));
  CHECK(color::get_red(c) == 255U);
  CHECK(color::get_green(c) >= 127U && color::get_green(c) <= 128U);

  // the pattern buffer is limited
  sprite::canvas<4, 1> tiny(h, 0);
  tiny.set_color(color::white);
  tiny.box({ 0, 0, 3, 3 });
  CHECK_EQ(tiny.pixel_count(), 4U);
  tiny.render({ 0, 0 });
  CHECK_EQ(vic_test::count_pixels(h, color::white), 4U);
}


TEST_CASE(sprite_canvas_frames)
{
  head_type h;
  dc d(h);
  d.cls(color::blue);

  sprite::canvas<64, 2> s(h, 0);
  s.set_color(color::red);
  s.plot({ 0, 0 });                       // frame 0: one red pixel
  s.select_frame(1);
  CHECK_EQ(s.get_frame(), 1U);
  s.set_color(color::green);
  s.box({ 0, 0, 2, 2 });                  // frame 1: 2 x 2 green
  CHECK_EQ(s.pixel_count(), 4U);
  s.select_frame(0);
  CHECK_EQ(s.pixel_count(), 1U);
  // invalid frames are ignored
  s.select_frame(2);
  CHECK_EQ(s.get_frame(), 0U);

  s.render({ 3, 3 }, 0U);
  CHECK_EQ(h.pixel({ 3, 3 }), color::red);
  CHECK_EQ(vic_test::count_pixels(h, color::red), 1U);
  s.render({ 3, 3 }, 1U);
  CHECK_EQ(vic_test::count_pixels(h, color::green), 4U);
  CHECK_EQ(vic_test::count_pixels(h, color::red), 0U);
  CHECK_EQ(h.pixel({ 3, 3 }), color::green);
  // back to frame 0
  s.render({ 3, 3 }, 0U);
  CHECK_EQ(h.pixel({ 3, 3 }), color::red);
  CHECK_EQ(h.pixel({ 4, 4 }), color::blue);
  // an invalid frame renders nothing, the background is restored
  s.render({ 3, 3 }, 5U);
  CHECK_EQ(vic_test::count_pixels(h, color::blue), 16U * 12U);
}


TEST_CASE(sprite_z_order)
{
  head_type h;
  dc d(h);
  d.cls(color::blue);

  // create the sprites in non sorted z order
  sprite::canvas<16, 1> s10(h, 10);
  sprite::canvas<16, 1> s5(h, 5);
  sprite::canvas<16, 1> s7(h, 7);
  s10.set_color(color::red);   s10.plot({ 0, 0 });
  s5.set_color(color::green);  s5.plot({ 0, 0 });
  s7.set_color(color::white);  s7.plot({ 0, 0 });

  // all at the same position: the highest z-index is visible
  s5.render({ 4, 4 });
  CHECK_EQ(h.pixel({ 4, 4 }), color::green);
  s7.render({ 4, 4 });
  CHECK_EQ(h.pixel({ 4, 4 }), color::white);
  s10.render({ 4, 4 });
  CHECK_EQ(h.pixel({ 4, 4 }), color::red);

  // re-render the lower ones: the top one stays visible
  s5.render({ 4, 4 });
  CHECK_EQ(h.pixel({ 4, 4 }), color::red);
  s7.render({ 4, 4 });
  CHECK_EQ(h.pixel({ 4, 4 }), color::red);

  // move the top one away: the middle one (z 7) appears, not z 5
  s10.render({ 8, 8 });
  CHECK_EQ(h.pixel({ 8, 8 }), color::red);
  CHECK_EQ(h.pixel({ 4, 4 }), color::white);
  // move the middle one away: z 5 appears
  s7.render({ 9, 9 });
  CHECK_EQ(h.pixel({ 4, 4 }), color::green);
  // move the last one: background
  s5.render({ 10, 10 });
  CHECK_EQ(h.pixel({ 4, 4 }), color::blue);
  CHECK_EQ(vic_test::count_pixels(h, color::blue), 16U * 12U - 3U);
}


TEST_CASE(sprite_z_order_same_index)
{
  head_type h;
  dc d(h);
  d.cls(color::blue);
  sprite::canvas<16, 1> a(h, 0);
  sprite::canvas<16, 1> b(h, 0);
  a.set_color(color::red);   a.plot({ 0, 0 });
  b.set_color(color::green); b.plot({ 0, 0 });
  a.render({ 2, 2 });
  b.render({ 2, 2 });
  // the sprite created first is in front for equal z-indices
  CHECK_EQ(h.pixel({ 2, 2 }), color::red);
  a.render({ 5, 5 });
  CHECK_EQ(h.pixel({ 2, 2 }), color::green);
  CHECK_EQ(h.pixel({ 5, 5 }), color::red);
}


TEST_CASE(sprite_alpha)
{
  head_type h;
  dc d(h);
  d.cls(color::blue);
  sprite::canvas<16, 1> s(h, 0);
  s.alpha_blending_enable(false);
  s.set_color(0x80FF0000UL);
  s.plot({ 0, 0 });
  s.render({ 1, 1 });
  // the half transparent sprite pixel is blended with the background
  const color::value_type c = h.pixel({ 1, 1 });
  CHECK(color::is_opaque(c));
  CHECK(color::get_red(c)  >= 127U && color::get_red(c)  <= 128U);
  CHECK(color::get_blue(c) >= 127U && color::get_blue(c) <= 128U);
  // and the background is restored exactly
  s.render({ 2, 2 });
  CHECK_EQ(h.pixel({ 1, 1 }), color::blue);
  // moving back and forth doesn't accumulate blending
  s.render({ 1, 1 });
  CHECK_EQ(h.pixel({ 1, 1 }), c);
  s.render({ 2, 2 });
  s.render({ 1, 1 });
  CHECK_EQ(h.pixel({ 1, 1 }), c);
}


///////////////////////////////////////////////////////////////////////////////
// S H E E T

namespace {

// 4 x 2 pixel sheet with 2 x 2 frames (2 frames side by side) in RGB565, LSB first
// frame 0: red  blue    frame 1: green white
//          bg   green            bg    bg
const std::uint8_t sheet565[] = {
  0x00, 0xF8,  0x1F, 0x00,  0xE0, 0x07,  0xFF, 0xFF,
  0x00, 0x00,  0xE0, 0x07,  0x00, 0x00,  0x00, 0x00
};
const sprite::sheet_info_type info565 = { 4U, 2U, 2U, 2U, color::black, color::format_RGB565, sheet565 };

// 2 x 1 pixel sheet in RGB888 (B G R), two frames of 1 x 1
const std::uint8_t sheet888[] = { 0x00, 0x00, 0xFF,  0xFF, 0x00, 0x00 };
const sprite::sheet_info_type info888 = { 2U, 1U, 1U, 1U, color::black, color::format_RGB888, sheet888 };

// 2 x 1 pixel sheet in ARGB8888 (B G R A): half transparent red, opaque green
const std::uint8_t sheetARGB[] = { 0x00, 0x00, 0xFF, 0x80,  0x00, 0xFF, 0x00, 0xFF };
const sprite::sheet_info_type infoARGB = { 2U, 1U, 1U, 1U, color::none, color::format_ARGB8888, sheetARGB };

// 2 x 1 pixel sheet in RGBA8888 (A B G R): opaque blue, half transparent white
const std::uint8_t sheetRGBA[] = { 0xFF, 0xFF, 0x00, 0x00,  0x80, 0xFF, 0xFF, 0xFF };
const sprite::sheet_info_type infoRGBA = { 2U, 1U, 1U, 1U, color::none, color::format_RGBA8888, sheetRGBA };

// 2 x 1 pixel sheet in RGB332
const std::uint8_t sheet332[] = { 0xE0, 0x03 };
const sprite::sheet_info_type info332 = { 2U, 1U, 1U, 1U, color::black, color::format_RGB332, sheet332 };

} // namespace


TEST_CASE(sprite_sheet_decode)
{
  CHECK_EQ(sprite::sheet_pixel_color(info565, &sheet565[0]), 0xFFF80000UL);
  CHECK_EQ(sprite::sheet_pixel_color(info565, &sheet565[2]), 0xFF0000F8UL);
  CHECK_EQ(sprite::sheet_pixel_color(info565, &sheet565[6]), 0xFFF8FCF8UL);
  CHECK_EQ(sprite::sheet_pixel_color(info888, &sheet888[0]), color::red);
  CHECK_EQ(sprite::sheet_pixel_color(info888, &sheet888[3]), color::blue);
  CHECK_EQ(sprite::sheet_pixel_color(infoARGB, &sheetARGB[0]), 0x80FF0000UL);
  CHECK_EQ(sprite::sheet_pixel_color(infoARGB, &sheetARGB[4]), color::green);
  CHECK_EQ(sprite::sheet_pixel_color(infoRGBA, &sheetRGBA[0]), color::blue);
  CHECK_EQ(sprite::sheet_pixel_color(infoRGBA, &sheetRGBA[4]), 0x80FFFFFFUL);
  CHECK_EQ(sprite::sheet_pixel_color(info332, &sheet332[0]), 0xFFE00000UL);
  CHECK_EQ(sprite::sheet_pixel_color(info332, &sheet332[1]), 0xFF0000C0UL);
}


TEST_CASE(sprite_sheet_render)
{
  head_type h;
  dc d(h);
  d.cls(color::blue);
  h.reset_counters();

  sprite::sheet<2, 2> s(h, &info565, 0);
  CHECK_EQ(s.frame_count(), 2U);

  s.render({ 3, 3 }, 0U);
  CHECK_EQ(h.pixel({ 3, 3 }), 0xFFF80000UL);   // red
  CHECK_EQ(h.pixel({ 4, 3 }), 0xFF0000F8UL);   // blue
  CHECK_EQ(h.pixel({ 3, 4 }), color::blue);    // background (transparent sheet pixel)
  CHECK_EQ(h.pixel({ 4, 4 }), 0xFF00FC00UL);   // green
  CHECK_EQ(h.pixel_set_count(), 3U);           // only the 3 non background pixels are written
  CHECK_EQ(h.present_count(), 1U);

  // frame 1 at another position, the old position is restored
  s.render({ 6, 6 }, 1U);
  CHECK_EQ(h.pixel({ 3, 3 }), color::blue);
  CHECK_EQ(h.pixel({ 4, 4 }), color::blue);
  CHECK_EQ(h.pixel({ 6, 6 }), 0xFF00FC00UL);   // green
  CHECK_EQ(h.pixel({ 7, 6 }), 0xFFF8FCF8UL);   // white
  CHECK_EQ(h.pixel({ 6, 7 }), color::blue);
  CHECK_EQ(h.pixel({ 7, 7 }), color::blue);
  CHECK_EQ(vic_test::count_pixels(h, color::blue), 16U * 12U - 2U);

  // invalid frame: nothing is rendered, the background is restored
  s.render({ 6, 6 }, 2U);
  CHECK_EQ(vic_test::count_pixels(h, color::blue), 16U * 12U);

  // a sheet whose first pixel is background (frame 1, second row) starts with the first real pixel
  s.render({ 0, 0 }, 1U);
  CHECK_EQ(h.pixel({ 0, 0 }), 0xFF00FC00UL);
  CHECK_EQ(h.pixel({ 0, 1 }), color::blue);
}


TEST_CASE(sprite_sheet_formats)
{
  head_type h;
  dc d(h);
  d.cls(color::blue);

  sprite::sheet<1, 1> s888(h, &info888, 0);
  s888.render({ 0, 0 }, 1U);
  CHECK_EQ(h.pixel({ 0, 0 }), color::blue);    // frame 1 of the RGB888 sheet is blue
  s888.render({ 0, 0 }, 0U);
  CHECK_EQ(h.pixel({ 0, 0 }), color::red);

  sprite::sheet<1, 1> sargb(h, &infoARGB, 1);
  sargb.render({ 2, 0 }, 0U);
  // half transparent red over blue
  const color::value_type c = h.pixel({ 2, 0 });
  CHECK(color::is_opaque(c));
  CHECK(color::get_red(c) >= 127U && color::get_red(c) <= 128U);
  sargb.render({ 2, 0 }, 1U);
  CHECK_EQ(h.pixel({ 2, 0 }), color::green);

  sprite::sheet<1, 1> srgba(h, &infoRGBA, 2);
  srgba.render({ 4, 0 }, 0U);
  CHECK_EQ(h.pixel({ 4, 0 }), color::blue);
  srgba.render({ 4, 0 }, 1U);
  CHECK(color::get_red(h.pixel({ 4, 0 })) >= 127U);

  sprite::sheet<1, 1> s332(h, &info332, 3);
  s332.render({ 6, 0 }, 0U);
  CHECK_EQ(h.pixel({ 6, 0 }), 0xFFE00000UL);
}


TEST_CASE(sprite_sheet_invalid_info)
{
  // a sheet with zero sized frames has no frames and renders nothing
  head_type h;
  dc d(h);
  d.cls(color::blue);
  const sprite::sheet_info_type bad = { 4U, 2U, 0U, 0U, color::black, color::format_RGB565, sheet565 };
  sprite::sheet<2, 2> s(h, &bad, 0);
  CHECK_EQ(s.frame_count(), 0U);
  s.render({ 0, 0 }, 0U);
  CHECK_EQ(vic_test::count_pixels(h, color::blue), 16U * 12U);
}


TEST_CASE(sprite_canvas_sheet)
{
  head_type h;
  dc d(h);
  d.cls(color::blue);

  sprite::canvas_sheet<16> s(h, &info565, 0);
  CHECK_EQ(s.frame_count(), 2U);
  s.render({ 3, 3 }, 0U);
  CHECK_EQ(h.pixel({ 3, 3 }), 0xFFF80000UL);
  CHECK_EQ(h.pixel({ 4, 3 }), 0xFF0000F8UL);
  CHECK_EQ(h.pixel({ 3, 4 }), color::blue);
  CHECK_EQ(h.pixel({ 4, 4 }), 0xFF00FC00UL);

  s.render({ 6, 6 }, 1U);
  CHECK_EQ(h.pixel({ 3, 3 }), color::blue);
  CHECK_EQ(h.pixel({ 6, 6 }), 0xFF00FC00UL);
  CHECK_EQ(h.pixel({ 7, 6 }), 0xFFF8FCF8UL);
  CHECK_EQ(vic_test::count_pixels(h, color::blue), 16U * 12U - 2U);

  // the canvas sheet can be modified like a canvas before rendering: render, then draw on top
  s.render({ 6, 6 }, 5U);   // invalid frame: empty
  CHECK_EQ(vic_test::count_pixels(h, color::blue), 16U * 12U);
}


TEST_CASE(sprite_chain_cleanup)
{
  // sprites unregister themselves, so rendering after other sprites are gone works
  head_type h;
  dc d(h);
  d.cls(color::blue);
  sprite::canvas<16, 1> keep(h, 0);
  keep.set_color(color::red);
  keep.plot({ 0, 0 });
  {
    sprite::canvas<16, 1> temp1(h, 3);
    sprite::canvas<16, 1> temp2(h, -3);
    temp1.set_color(color::green); temp1.plot({ 0, 0 });
    temp2.set_color(color::white); temp2.plot({ 0, 0 });
    temp1.render({ 1, 1 });
    temp2.render({ 1, 1 });
    keep.render({ 1, 1 });
    CHECK_EQ(h.pixel({ 1, 1 }), color::green);
    // the temporaries are destroyed here (without restoring the background)
  }
  keep.render({ 2, 2 });
  CHECK_EQ(h.pixel({ 2, 2 }), color::red);
  keep.render({ 3, 3 });
  CHECK_EQ(h.pixel({ 2, 2 }), color::blue);
}


///////////////////////////////////////////////////////////////////////////////
// R O T A T I O N

namespace {

// an asymmetric L shaped 4 pixel pattern around the rotation center (2, 2)
void draw_l(sprite::canvas<64, 1>& s, color::value_type c)
{
  s.set_color(c);
  s.pixel_set({ 0, 0 });
  s.pixel_set({ 0, 1 });
  s.pixel_set({ 0, 2 });
  s.pixel_set({ 1, 2 });
}

const vertex_type l_pixels[4] = { { 0, 0 }, { 0, 1 }, { 0, 2 }, { 1, 2 } };

} // namespace


TEST_CASE(sprite_rotation_right_angles)
{
  // for multiples of 90 degree the rotation is exact: the rendered pixels are the forward
  // rotated pattern pixels
  static const std::int16_t angles[] = { 90, 180, 270, -90, 450 };
  for (std::size_t a = 0U; a < sizeof(angles) / sizeof(angles[0]); ++a) {
    head_type h;
    dc d(h);
    d.cls(color::blue);
    sprite::canvas<64, 1> s(h, 0);
    draw_l(s, color::white);
    CHECK_EQ(s.pixel_count(), 4U);
    s.set_rotation(angles[a], { 2, 2 });
    CHECK_EQ(s.get_rotation(), static_cast<std::int16_t>(angles[a] % 360));
    CHECK_EQ(s.pixel_count(), 4U);             // the pattern is untouched
    const vertex_type pos = { 6, 5 };
    s.render(pos);
    CHECK_EQ(vic_test::count_pixels(h, color::white), 4U);
    for (std::size_t i = 0U; i < 4U; ++i) {
      const vertex_type r = util::vertex_rotate(l_pixels[i], { 2, 2 }, angles[a]) + pos;
      CHECK_EQ(h.pixel(r), color::white);
    }
    // move: the background is restored everywhere
    s.render({ 1, 1 });
    CHECK_EQ(vic_test::count_pixels(h, color::white), 4U);
    CHECK_EQ(vic_test::count_pixels(h, color::blue), 16U * 12U - 4U);
  }
}


TEST_CASE(sprite_rotation_zero)
{
  // angle 0 and 360 render exactly like the unrotated sprite
  head_type a, b, c;
  dc da(a), db(b), dcc(c);
  da.cls(color::blue); db.cls(color::blue); dcc.cls(color::blue);
  sprite::canvas<64, 1> sa(a, 0), sb(b, 0), sc(c, 0);
  draw_l(sa, color::white);
  draw_l(sb, color::white);
  draw_l(sc, color::white);
  sb.set_rotation(0, { 2, 2 });
  sc.set_rotation(360, { 2, 2 });
  CHECK_EQ(sb.get_rotation(), 0);
  CHECK_EQ(sc.get_rotation(), 0);
  sa.render({ 5, 5 });
  sb.render({ 5, 5 });
  sc.render({ 5, 5 });
  CHECK_SAME_BUFFER(a, b);
  CHECK_SAME_BUFFER(a, c);
  // switching the rotation off again
  sb.set_rotation(90, { 2, 2 });
  sb.render({ 5, 5 });
  sb.set_rotation(0, { 2, 2 });
  sb.render({ 5, 5 });
  CHECK_SAME_BUFFER(a, b);
}


TEST_CASE(sprite_rotation_45)
{
  // a 7 x 7 box rotated by 45 degree: a diamond without holes, about the same pixel count
  head::memory<24, 24> h;
  dc d(h);
  d.cls(color::blue);
  sprite::canvas<128, 1> s(h, 0);
  s.set_color(color::white);
  s.box({ 0, 0, 7, 7 });
  s.set_rotation(45, { 3, 3 });
  const vertex_type pos = { 8, 8 };
  s.render(pos);

  const std::size_t cnt = vic_test::count_pixels(h, color::white);
  CHECK(cnt >= 39U && cnt <= 64U);
  CHECK_EQ(h.pixel({ 11, 11 }), color::white);    // center
  CHECK_EQ(h.pixel({ 11, 7 }), color::white);     // top corner of the diamond (distance 4)
  CHECK_EQ(h.pixel({ 7, 11 }), color::white);
  CHECK_EQ(h.pixel({ 8, 8 }), color::blue);       // the old box corners are empty now
  CHECK_EQ(h.pixel({ 14, 14 }), color::blue);
  // no interior holes
  std::size_t holes = 0U;
  for (std::int16_t y = 1; y < 23; ++y) {
    for (std::int16_t x = 1; x < 23; ++x) {
      if ((h.pixel({ x, y }) == color::blue) &&
          (h.pixel({ static_cast<std::int16_t>(x - 1), y }) == color::white) && (h.pixel({ static_cast<std::int16_t>(x + 1), y }) == color::white) &&
          (h.pixel({ x, static_cast<std::int16_t>(y - 1) }) == color::white) && (h.pixel({ x, static_cast<std::int16_t>(y + 1) }) == color::white)) {
        ++holes;
      }
    }
  }
  CHECK_EQ(holes, 0U);
  // everything inside the rotated bounding box (diagonal 4.2 -> max distance 5 from the center)
  for (std::int16_t y = 0; y < 24; ++y) {
    for (std::int16_t x = 0; x < 24; ++x) {
      if (h.pixel({ x, y }) == color::white) {
        CHECK(util::abs<std::int16_t>(static_cast<std::int16_t>(x - 11)) + util::abs<std::int16_t>(static_cast<std::int16_t>(y - 11)) <= 5);
      }
    }
  }
  // move it (fully visible): all restored, same pixel count
  s.render({ 12, 12 });
  CHECK_EQ(vic_test::count_pixels(h, color::white), cnt);
  CHECK_EQ(h.pixel({ 15, 15 }), color::white);
  s.set_rotation(0);
  s.render({ 100, 100 });
  CHECK_EQ(vic_test::count_pixels(h, color::blue), 24U * 24U);
}


TEST_CASE(sprite_rotation_alpha_and_zorder)
{
  head_type h;
  dc d(h);
  d.cls(color::blue);

  // translucent rotated pixels keep their alpha when blended with the background (half white over blue)
  sprite::canvas<64, 1> t(h, 20);
  t.alpha_blending_enable(false);
  draw_l(t, 0x80FFFFFFUL);
  t.set_rotation(90, { 2, 2 });
  t.render({ 0, 0 });                                     // rotated pixels: (4,0) (3,0) (2,0) (2,1)
  const vertex_type r0 = util::vertex_rotate(l_pixels[0], { 2, 2 }, 90) + vertex_type{ 0, 0 };
  CHECK((r0 == vertex_type{ 4, 0 }));
  const color::value_type c = h.pixel(r0);
  CHECK(color::is_opaque(c));
  CHECK(color::get_red(c)   >= 127U && color::get_red(c)   <= 128U);
  CHECK(color::get_green(c) >= 127U && color::get_green(c) <= 128U);
  CHECK_EQ(color::get_blue(c), 255U);
  CHECK_EQ(vic_test::count_pixels(h, c), 4U);
  CHECK_EQ(h.pixel({ 2, 1 }), c);

  // a rotated sprite in front of another one: overlap tests go through the rotation
  sprite::canvas<64, 1> back(h, 0);
  back.set_color(color::red);
  back.box({ 0, 0, 6, 6 });
  back.render({ 6, 4 });                                  // covers 6..11 x 4..9
  CHECK_EQ(h.pixel(r0), c);                               // untouched
  sprite::canvas<64, 1> front(h, 10);
  draw_l(front, color::green);
  front.set_rotation(180, { 2, 2 });
  front.render({ 7, 5 });                                 // rotated pixels: (11,9) (11,8) (11,7) (10,7), all inside the box
  for (std::size_t i = 0U; i < 4U; ++i) {
    const vertex_type r = util::vertex_rotate(l_pixels[i], { 2, 2 }, 180) + vertex_type{ 7, 5 };
    CHECK_EQ(h.pixel(r), color::green);
  }
  CHECK_EQ(vic_test::count_pixels(h, color::red), 36U - 4U);
  // re-render the back sprite: the rotated front sprite still wins
  back.render({ 6, 4 });
  const vertex_type g = util::vertex_rotate(l_pixels[3], { 2, 2 }, 180) + vertex_type{ 7, 5 };
  CHECK_EQ(h.pixel(g), color::green);
  // move the front sprite away: the back sprite appears
  front.render({ 0, 0 });
  CHECK_EQ(h.pixel(g), color::red);
}


TEST_CASE(sprite_rotation_canvas_sheet)
{
  head_type h;
  dc d(h);
  d.cls(color::blue);
  sprite::canvas_sheet<16> s(h, &info565, 0);
  // frame 0 (red, blue / bg, green) rotated by 90 degree around the frame center (1, 1):
  // (0,0) -> (2,0), (1,0) -> (2,1), (1,1) -> (1,1)
  s.render({ 4, 4 }, 0U, 90);
  CHECK_EQ(s.get_rotation(), 90);
  CHECK((s.get_rotation_center() == vertex_type{ 1, 1 }));
  CHECK_EQ(h.pixel({ 6, 4 }), 0xFFF80000UL);
  CHECK_EQ(h.pixel({ 6, 5 }), 0xFF0000F8UL);
  CHECK_EQ(h.pixel({ 5, 5 }), 0xFF00FC00UL);
  CHECK_EQ(vic_test::count_pixels(h, color::blue), 16U * 12U - 3U);
  // the rotation stays for the next render
  s.render({ 4, 4 }, 0U);
  CHECK_EQ(h.pixel({ 6, 4 }), 0xFFF80000UL);
  // invalid frame still renders nothing
  s.render({ 4, 4 }, 7U, 45);
  CHECK_EQ(vic_test::count_pixels(h, color::blue), 16U * 12U);
  // back to unrotated
  s.render({ 4, 4 }, 0U, 0);
  CHECK_EQ(h.pixel({ 4, 4 }), 0xFFF80000UL);
  CHECK_EQ(h.pixel({ 5, 4 }), 0xFF0000F8UL);
  CHECK_EQ(h.pixel({ 5, 5 }), 0xFF00FC00UL);
}


///////////////////////////////////////////////////////////////////////////////
// C O L L I S I O N   D E T E C T I O N

namespace {

// collision recorder, used as handler argument
struct collision_log {
  std::size_t   count;
  sprite::base* self;
  sprite::base* other;
  sprite::base* others[4];
  collision_log() : count(0U), self(nullptr), other(nullptr) { others[0] = others[1] = others[2] = others[3] = nullptr; }
  void reset() { count = 0U; self = nullptr; other = nullptr; }
};

void on_collision(sprite::base& self, sprite::base& other, void* arg)
{
  collision_log& log = *static_cast<collision_log*>(arg);
  if (log.count < 4U) {
    log.others[log.count] = &other;
  }
  log.count++;
  log.self  = &self;
  log.other = &other;
}

// draw a solid w x h box into a canvas sprite
void draw_box(sprite::canvas<64, 1>& s, std::int16_t w, std::int16_t h, color::value_type c)
{
  s.set_color(c);
  s.box({ 0, 0, w, h });
}

} // namespace


TEST_CASE(sprite_collision_basic)
{
  head_type h;
  dc d(h);
  d.cls(color::blue);
  collision_log log;

  sprite::canvas<64, 1> a(h, 0);
  sprite::canvas<64, 1> b(h, 1);
  draw_box(a, 4, 4, color::white);
  draw_box(b, 4, 4, color::red);
  b.set_collision_handler(on_collision, &log);

  a.render({ 2, 2 });
  CHECK_EQ(log.count, 0U);                       // a has no handler

  // b overlaps a in the 2 x 2 area (4..5, 4..5): exactly one report with the right partner
  b.render({ 4, 4 });
  CHECK_EQ(log.count, 1U);
  CHECK_EQ(static_cast<const void*>(log.self),  static_cast<const void*>(&b));
  CHECK_EQ(static_cast<const void*>(log.other), static_cast<const void*>(&a));
  CHECK_EQ(static_cast<const void*>(b.get_collision()), static_cast<const void*>(&a));
  CHECK_EQ(static_cast<const void*>(a.get_collision()), static_cast<const void*>(nullptr));
  CHECK_EQ(h.pixel({ 4, 4 }), color::red);     // rendering is unchanged
  CHECK_EQ(h.pixel({ 2, 2 }), color::white);

  // the partner is not notified when it is rendered itself (it has no handler)
  a.render({ 2, 2 });
  CHECK_EQ(log.count, 1U);

  // move apart: no collision, the restore phase (which sees a) doesn't report
  log.reset();
  b.render({ 8, 8 });
  CHECK_EQ(log.count, 0U);
  CHECK_EQ(static_cast<const void*>(b.get_collision()), static_cast<const void*>(nullptr));
  CHECK_EQ(vic_test::count_pixels(h, color::blue), 16U * 12U - 32U);

  // touching edges are no collision (exclusive rects)
  b.render({ 6, 2 });
  CHECK_EQ(log.count, 0U);

  // handler removed: detection off
  b.set_collision_handler(nullptr);
  b.render({ 4, 4 });
  CHECK_EQ(log.count, 0U);
  CHECK_EQ(static_cast<const void*>(b.get_collision()), static_cast<const void*>(nullptr));
}


TEST_CASE(sprite_collision_pixel_exact)
{
  head_type h;
  dc d(h);
  d.cls(color::blue);
  collision_log log;

  // a is an L, b is the complementary corner: the bounding boxes overlap, the pixels don't
  sprite::canvas<64, 1> a(h, 0);
  sprite::canvas<64, 1> b(h, 1);
  draw_l(a, color::white);                       // (0,0) (0,1) (0,2) (1,2)
  b.set_color(color::red);
  b.plot({ 1, 0 }); b.plot({ 2, 0 }); b.plot({ 2, 1 });
  b.set_collision_handler(on_collision, &log);

  a.render({ 5, 5 });
  b.render({ 5, 5 });
  CHECK_EQ(log.count, 0U);
  CHECK_EQ(static_cast<const void*>(b.get_collision()), static_cast<const void*>(nullptr));

  // shift b one pixel left: b's (1,0) lands on a's (0,0)
  b.render({ 4, 5 });
  CHECK_EQ(log.count, 1U);
  CHECK_EQ(static_cast<const void*>(log.other), static_cast<const void*>(&a));
}


TEST_CASE(sprite_collision_hidden_behind_opaque)
{
  head_type h;
  dc d(h);
  d.cls(color::blue);
  collision_log log;

  // a small sprite which is completely hidden behind an opaque top sprite
  sprite::canvas<64, 1> small(h, 0);
  sprite::canvas<64, 1> top(h, 10);
  draw_box(small, 1, 1, color::green);
  draw_box(top, 4, 4, color::red);               // opaque
  top.set_collision_handler(on_collision, &log);

  small.render({ 5, 5 });
  top.render({ 4, 4 });
  CHECK_EQ(log.count, 1U);
  CHECK_EQ(static_cast<const void*>(log.other), static_cast<const void*>(&small));
  // render again at the same place: now the restore buffer knows the background, so the blend
  // loop would stop early, the hidden sprite must still be reported
  log.reset();
  top.render({ 4, 4 });
  CHECK_EQ(log.count, 1U);
  CHECK_EQ(static_cast<const void*>(top.get_collision()), static_cast<const void*>(&small));
  CHECK_EQ(h.pixel({ 5, 5 }), color::red);
}


TEST_CASE(sprite_collision_multiple_partners)
{
  head_type h;
  dc d(h);
  d.cls(color::blue);
  collision_log log;

  sprite::canvas<64, 1> a(h, 1);
  sprite::canvas<64, 1> b(h, 2);
  sprite::canvas<64, 1> c(h, 3);
  sprite::canvas<64, 1> far(h, 4);
  draw_box(a, 2, 2, color::white);
  draw_box(b, 6, 2, color::red);
  draw_box(c, 2, 2, color::green);
  draw_box(far, 2, 2, color::yellow);
  b.set_collision_handler(on_collision, &log);

  a.render({ 2, 4 });
  c.render({ 6, 4 });
  far.render({ 12, 10 });
  b.render({ 2, 4 });                            // covers a (2..3) and c (6..7)
  CHECK_EQ(log.count, 2U);                       // one call per partner, not per pixel
  CHECK_EQ(static_cast<const void*>(log.others[0]), static_cast<const void*>(&c));   // highest z-index first
  CHECK_EQ(static_cast<const void*>(log.others[1]), static_cast<const void*>(&a));
  CHECK_EQ(static_cast<const void*>(b.get_collision()), static_cast<const void*>(&c));
  // c is above b, so c is visible, a is below b
  CHECK_EQ(h.pixel({ 6, 4 }), color::green);
  CHECK_EQ(h.pixel({ 2, 4 }), color::red);
}


TEST_CASE(sprite_collision_sheet_background)
{
  head_type h;
  dc d(h);
  d.cls(color::blue);
  collision_log log;

  // sheet frame 0 is: red blue / background green
  sprite::sheet<2, 2> s(h, &info565, 0);
  sprite::canvas<64, 1> dot(h, 1);
  draw_box(dot, 1, 1, color::white);
  dot.set_collision_handler(on_collision, &log);

  s.render({ 3, 3 }, 0U);
  dot.render({ 3, 4 });                          // the background pixel of the sheet: no collision
  CHECK_EQ(log.count, 0U);
  dot.render({ 4, 4 });                          // the green pixel
  CHECK_EQ(log.count, 1U);
  CHECK_EQ(static_cast<const void*>(log.other), static_cast<const void*>(&s));

  // the sheet reports too when it gets a handler and is rendered over the dot
  log.reset();
  s.set_collision_handler(on_collision, &log);
  s.render({ 3, 3 }, 0U);
  CHECK_EQ(log.count, 1U);
  CHECK_EQ(static_cast<const void*>(log.self),  static_cast<const void*>(&s));
  CHECK_EQ(static_cast<const void*>(log.other), static_cast<const void*>(&dot));
  // an invalid (empty) frame has no pixels and no collisions
  log.reset();
  s.render({ 3, 3 }, 5U);
  CHECK_EQ(log.count, 0U);
}


TEST_CASE(sprite_collision_rotation)
{
  head_type h;
  dc d(h);
  d.cls(color::blue);
  collision_log log;

  // a horizontal bar (0..4, 2) and a dot where the vertical (rotated) bar would be
  sprite::canvas<64, 1> bar(h, 1);
  sprite::canvas<64, 1> dot(h, 0);
  bar.set_color(color::white);
  bar.line_horz({ 0, 2 }, { 4, 2 });
  draw_box(dot, 1, 1, color::red);
  bar.set_collision_handler(on_collision, &log);

  dot.render({ 6, 4 });                          // relative to the bar origin {4,4}: (2,0)
  bar.render({ 4, 4 });
  CHECK_EQ(log.count, 0U);
  bar.set_rotation(90, { 2, 2 });                // the bar is vertical now: (2, 0..4)
  bar.render({ 4, 4 });
  CHECK_EQ(log.count, 1U);
  CHECK_EQ(static_cast<const void*>(log.other), static_cast<const void*>(&dot));
  // the dot reports the rotated bar as well
  log.reset();
  dot.set_collision_handler(on_collision, &log);
  dot.render({ 6, 4 });
  CHECK_EQ(log.count, 1U);
  CHECK_EQ(static_cast<const void*>(log.other), static_cast<const void*>(&bar));
  dot.render({ 8, 4 });                          // beside the vertical bar
  CHECK_EQ(log.count, 1U);
}


TEST_CASE(sprite_blind_head_background)
{
  // a head which can't read its pixels back: the sprite takes its background color for the restore
  class blind_head : public head_type
  {
  protected:
    virtual color::value_type pixel_get(vertex_type vertex)
    {
      (void)vertex;
      return color::none;
    }
  };
  blind_head h;
  dc d(h);
  d.cls(color::blue);
  sprite::canvas<16, 1> s(h, 0);
  s.set_bg_color(color::blue);
  s.set_color(color::green);
  s.box({ 0, 0, 2, 2 });
  s.render({ 3, 3 });
  CHECK_EQ(h.pixel({ 3, 3 }), color::green);
  s.render({ 8, 8 });
  CHECK_EQ(h.pixel({ 3, 3 }), color::blue);        // restored with the background color, not with none
  CHECK_EQ(h.pixel({ 4, 4 }), color::blue);
  CHECK_EQ(vic_test::count_pixels(h, color::green), 4U);
  CHECK_EQ(vic_test::count_pixels(h, color::blue), 16U * 12U - 4U);
}


TEST_CASE(sprite_canvas_restore_transparent_background)
{
  // an untouched head is transparent (color::none), the sprite must keep that as the background of
  // its kept pixels and must not read its own pixels back when it moves to an overlapping position
  head::memory<16, 12> h;
  sprite::canvas<16, 1> s(h, 0);
  s.set_color(color::white);
  s.box({ 0, 0, 3, 3 });
  s.render({ 2, 2 });
  s.render({ 3, 3 });   // overlaps the old position
  s.render({ 4, 4 });
  CHECK_EQ(vic_test::count_pixels(h, color::white), 9U);
  CHECK_EQ(h.pixel({ 2, 2 }), color::none);
  CHECK_EQ(h.pixel({ 3, 3 }), color::none);
  CHECK_EQ(h.pixel({ 4, 3 }), color::none);
  CHECK_EQ(h.pixel({ 4, 4 }), color::white);
  CHECK_EQ(h.pixel({ 6, 6 }), color::white);
}
