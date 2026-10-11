///////////////////////////////////////////////////////////////////////////////
// \brief Tests for color.h
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "color.h"

using namespace vic;


TEST_CASE(color_assembly_access)
{
  const color::value_type c = color::argb(0x11U, 0x22U, 0x33U, 0x44U);
  CHECK_EQ(c, 0x44112233UL);
  CHECK_EQ(color::get_alpha(c), 0x44U);
  CHECK_EQ(color::get_red(c),   0x11U);
  CHECK_EQ(color::get_green(c), 0x22U);
  CHECK_EQ(color::get_blue(c),  0x33U);

  // default alpha is opaque
  CHECK_EQ(color::argb(1U, 2U, 3U), 0xFF010203UL);
  CHECK((color::rgb_static<0x11U, 0x22U, 0x33U>::value == 0xFF112233UL));
  CHECK((color::rgb_static<0x11U, 0x22U, 0x33U, 0x80U>::value == 0x80112233UL));

  CHECK_EQ(color::set_alpha(c, 0xAAU), 0xAA112233UL);
  CHECK_EQ(color::set_red(c,   0xAAU), 0x44AA2233UL);
  CHECK_EQ(color::set_green(c, 0xAAU), 0x4411AA33UL);
  CHECK_EQ(color::set_blue(c,  0xAAU), 0x441122AAUL);

  CHECK(color::is_opaque(color::white));
  CHECK(color::is_opaque(color::black));
  CHECK(!color::is_opaque(color::none));
  CHECK(!color::is_opaque(0xFEFFFFFFUL));
  CHECK_EQ(color::none, 0U);
  CHECK_EQ(color::transparent, color::none);
}


TEST_CASE(color_hsv)
{
  // primaries at full saturation and value
  CHECK_EQ(color::hsv(0U,   255U, 255U), color::red);
  CHECK_EQ(color::hsv(120U, 255U, 255U), color::green);
  CHECK_EQ(color::hsv(240U, 255U, 255U), color::blue);
  CHECK_EQ(color::hsv(60U,  255U, 255U), color::yellow);
  CHECK_EQ(color::hsv(180U, 255U, 255U), color::cyan);
  CHECK_EQ(color::hsv(300U, 255U, 255U), color::magenta);
  // hue wraps
  CHECK_EQ(color::hsv(360U, 255U, 255U), color::red);
  CHECK_EQ(color::hsv(480U, 255U, 255U), color::green);
  // no saturation: gray of the given value
  CHECK_EQ(color::hsv(0U,   0U, 255U), color::white);
  CHECK_EQ(color::hsv(200U, 0U, 128U), 0xFF808080UL);
  // no value: black
  CHECK_EQ(color::hsv(123U, 255U, 0U), color::black);
  // alpha is passed through
  CHECK_EQ(color::get_alpha(color::hsv(0U, 255U, 255U, 0x55U)), 0x55U);
  // intermediate hue: 30 degree is orange (255, 128, 0) within rounding
  const color::value_type o = color::hsv(30U, 255U, 255U);
  CHECK_EQ(color::get_red(o), 255U);
  CHECK(color::get_green(o) >= 126U && color::get_green(o) <= 128U);
  CHECK_EQ(color::get_blue(o), 0U);
}


TEST_CASE(color_dim)
{
  CHECK_EQ(color::dim_25(color::white), 0xFF3F3F3FUL);
  CHECK_EQ(color::dim_50(color::white), 0xFF7F7F7FUL);
  CHECK_EQ(color::dim_75(color::white), 0xFFBEBEBEUL);
  CHECK_EQ(color::dim_25(color::black), color::black);
  // alpha is unaffected
  CHECK_EQ(color::get_alpha(color::dim_25(0x80FFFFFFUL)), 0x80U);
  CHECK_EQ(color::get_alpha(color::dim_50(0x80FFFFFFUL)), 0x80U);
  CHECK_EQ(color::get_alpha(color::dim_75(0x80FFFFFFUL)), 0x80U);
  CHECK_EQ(color::dim_75(0x80FFFFFFUL), 0x80BEBEBEUL);

  // dim(): 255 keeps the color, 0 is (nearly) black, alpha unaffected
  CHECK_EQ(color::dim(0x80FFFFFFUL, 255U), 0x80FFFFFFUL);
  CHECK_EQ(color::dim(0x80FFFFFFUL, 0U),   0x80000000UL);
  CHECK_EQ(color::dim(color::white, 127U), 0xFF7F7F7FUL);
  CHECK_EQ(color::dim(0xFF102030UL, 255U), 0xFF102030UL);
  CHECK_EQ(color::dim(0xFF102030UL, 127U), 0xFF081018UL);
  // channels never bleed into each other
  for (unsigned lum = 0U; lum < 256U; lum += 5U) {
    const color::value_type d = color::dim(0xFFFF00FFUL, static_cast<std::uint8_t>(lum));
    CHECK_EQ(color::get_green(d), 0U);
    CHECK_EQ(color::get_alpha(d), 0xFFU);
  }
}


TEST_CASE(color_mix)
{
  // mix of two opaque colors stays opaque
  CHECK_EQ(color::mix_50(color::white, color::black), 0xFF7F7F7FUL);
  CHECK_EQ(color::mix_25(color::white, color::black), 0xFF3F3F3FUL);
  CHECK_EQ(color::mix_75(color::white, color::black), 0xFFBEBEBEUL);
  CHECK(color::is_opaque(color::mix_50(color::red, color::blue)));
  CHECK(color::is_opaque(color::mix_25(color::red, color::blue)));
  CHECK(color::is_opaque(color::mix_75(color::red, color::blue)));
  CHECK_EQ(color::mix_50(color::red, color::blue), 0xFF7F007FUL);

  // mix(): lum 255 is foreground, 0 is background, alpha is taken from foreground
  CHECK_EQ(color::mix(color::red, color::blue, 255U), color::red);
  CHECK_EQ(color::mix(color::red, color::blue, 0U),   color::blue);
  CHECK_EQ(color::mix(color::white, color::black, 127U), 0xFF7F7F7FUL);
  CHECK_EQ(color::get_alpha(color::mix(0x80FF0000UL, color::blue, 100U)), 0x80U);
  CHECK(color::is_opaque(color::mix(color::white, color::white, 200U)));
  CHECK_EQ(color::mix(color::white, color::white, 200U), color::white);
}


TEST_CASE(color_alpha_blend)
{
  // opaque front wins
  CHECK_EQ(color::alpha_blend(color::red, color::blue), color::red);
  // transparent back: front color
  CHECK_EQ(color::alpha_blend(0x80FF0000UL, color::none), 0x80FF0000UL);
  // transparent front: back color
  CHECK_EQ(color::alpha_blend(0x00FF0000UL, color::blue), color::blue);
  // 50% red over opaque blue
  const color::value_type b = color::alpha_blend(0x80FF0000UL, color::blue);
  CHECK(color::is_opaque(b));
  CHECK(color::get_red(b)  >= 127U && color::get_red(b)  <= 128U);
  CHECK(color::get_blue(b) >= 127U && color::get_blue(b) <= 128U);
  CHECK_EQ(color::get_green(b), 0U);
  // 50% over 50%: resulting alpha is 75%
  const color::value_type h = color::alpha_blend(0x80FFFFFFUL, 0x80000000UL);
  CHECK(color::get_alpha(h) >= 190U && color::get_alpha(h) <= 192U);
  // white over white stays white
  CHECK_EQ(color::alpha_blend(0x40FFFFFFUL, color::white), color::white);
  // never produces an alpha above 255 or garbage in channels
  for (unsigned a = 0U; a < 256U; a += 15U) {
    const color::value_type r = color::alpha_blend(color::argb(255U, 255U, 255U, static_cast<std::uint8_t>(a)), color::white);
    CHECK_EQ(r & 0x00FFFFFFUL, 0x00FFFFFFUL);
  }
}


TEST_CASE(color_argb_to_rgb)
{
  CHECK_EQ(color::argb_to_rgb(color::red, color::black), color::red);
  CHECK_EQ(color::argb_to_rgb(0x00FF0000UL, color::blue), color::blue);
  CHECK_EQ(color::argb_to_rgb(0x00FF0000UL, 0x12345678UL), 0xFF345678UL);   // alpha of background is ignored
  const color::value_type h = color::argb_to_rgb(0x80FFFFFFUL, color::black);
  CHECK(color::is_opaque(h));
  CHECK(color::get_red(h) >= 127U && color::get_red(h) <= 128U);
  CHECK_EQ(color::argb_to_rgb(0xFF000000UL, color::white), color::black);
}


///////////////////////////////////////////////////////////////////////////////
// F O R M A T   C O N V E R S I O N

namespace {
  const color::value_type test_colors[] = {
    color::black, color::white, color::red, color::green, color::blue,
    color::cyan, color::magenta, color::yellow, color::gray,
    0xFF102030UL, 0xFFF8FCF8UL, 0xFF07030FUL, 0xFF80FF40UL, 0xFFFF8000UL
  };
  const std::size_t test_color_count = sizeof(test_colors) / sizeof(test_colors[0]);

  // compare two channels with a maximum quantization error
  bool near(std::uint8_t a, std::uint8_t b, unsigned tol)
  {
    return static_cast<unsigned>(a > b ? a - b : b - a) <= tol;
  }
}


TEST_CASE(color_L1)
{
  CHECK_EQ(color::color_to_L1(color::black), 0U);
  CHECK_EQ(color::color_to_L1(color::white), 1U);
  CHECK_EQ(color::color_to_L1(0xFF000001UL), 1U);
  CHECK_EQ(color::color_to_L1(0x00000000UL), 0U);
  CHECK_EQ(color::L1_to_color(0U), color::black);
  CHECK_EQ(color::L1_to_color(1U), color::white);
  CHECK_EQ(color::L1_to_color(0x80U), color::white);
}


TEST_CASE(color_L2_L4_L8)
{
  // encoders
  CHECK_EQ(color::color_to_L8(color::white), 255U);
  CHECK_EQ(color::color_to_L8(color::black), 0U);
  CHECK_EQ(color::color_to_L8(color::gray),  128U);
  CHECK_EQ(color::color_to_L8(color::red),   85U);
  CHECK_EQ(color::color_to_L4(color::white), 15U);
  CHECK_EQ(color::color_to_L4(color::black), 0U);
  CHECK_EQ(color::color_to_L4(color::gray),  8U);
  CHECK_EQ(color::color_to_L2(color::white), 3U);
  CHECK_EQ(color::color_to_L2(color::black), 0U);
  CHECK_EQ(color::color_to_L2(color::gray),  2U);

  // decoders: 0 is black (must not divide by zero), max is white, linear in between
  CHECK_EQ(color::L2_to_color(0U), color::black);
  CHECK_EQ(color::L2_to_color(1U), 0xFF555555UL);
  CHECK_EQ(color::L2_to_color(2U), 0xFFAAAAAAUL);
  CHECK_EQ(color::L2_to_color(3U), color::white);
  CHECK_EQ(color::L2_to_color(0xFFU), color::white);   // only the low bits count

  CHECK_EQ(color::L4_to_color(0U),  color::black);
  CHECK_EQ(color::L4_to_color(1U),  0xFF111111UL);
  CHECK_EQ(color::L4_to_color(8U),  0xFF888888UL);
  CHECK_EQ(color::L4_to_color(15U), color::white);
  CHECK_EQ(color::L4_to_color(0xFFU), color::white);

  CHECK_EQ(color::L8_to_color(0U),   color::black);
  CHECK_EQ(color::L8_to_color(255U), color::white);
  CHECK_EQ(color::L8_to_color(128U), 0xFF808080UL);

  // round trips
  for (unsigned i = 0U; i < 4U; ++i) {
    CHECK_EQ(color::color_to_L2(color::L2_to_color(static_cast<std::uint8_t>(i))), i);
  }
  for (unsigned i = 0U; i < 16U; ++i) {
    CHECK_EQ(color::color_to_L4(color::L4_to_color(static_cast<std::uint8_t>(i))), i);
  }
  for (unsigned i = 0U; i < 256U; ++i) {
    CHECK_EQ(color::color_to_L8(color::L8_to_color(static_cast<std::uint8_t>(i))), i);
  }
}


TEST_CASE(color_RGB332)
{
  CHECK_EQ(color::color_to_RGB332(color::black), 0x00U);
  CHECK_EQ(color::color_to_RGB332(color::white), 0xFFU);
  CHECK_EQ(color::color_to_RGB332(color::red),   0xE0U);
  CHECK_EQ(color::color_to_RGB332(color::green), 0x1CU);
  CHECK_EQ(color::color_to_RGB332(color::blue),  0x03U);
  CHECK_EQ(color::RGB332_to_color(0xE0U), 0xFFE00000UL);
  CHECK_EQ(color::RGB332_to_color(0x1CU), 0xFF00E000UL);
  CHECK_EQ(color::RGB332_to_color(0x03U), 0xFF0000C0UL);
  for (unsigned i = 0U; i < 256U; ++i) {
    CHECK_EQ(color::color_to_RGB332(color::RGB332_to_color(static_cast<std::uint8_t>(i))), i);
  }
}


TEST_CASE(color_RGB444)
{
  CHECK_EQ(color::color_to_RGB444(color::black), 0x0000U);
  CHECK_EQ(color::color_to_RGB444(color::white), 0x0FFFU);
  CHECK_EQ(color::color_to_RGB444(color::red),   0x0F00U);
  CHECK_EQ(color::color_to_RGB444(color::green), 0x00F0U);
  CHECK_EQ(color::color_to_RGB444(color::blue),  0x000FU);
  CHECK_EQ(color::RGB444_to_color(0x0F00U), 0xFFF00000UL);
  CHECK_EQ(color::RGB444_to_color(0x00F0U), 0xFF00F000UL);
  CHECK_EQ(color::RGB444_to_color(0x000FU), 0xFF0000F0UL);
  for (unsigned i = 0U; i < 0x1000U; i += 3U) {
    CHECK_EQ(color::color_to_RGB444(color::RGB444_to_color(static_cast<std::uint16_t>(i))), i);
  }
  for (std::size_t i = 0U; i < test_color_count; ++i) {
    const color::value_type c = color::RGB444_to_color(color::color_to_RGB444(test_colors[i]));
    CHECK(near(color::get_red(c),   color::get_red(test_colors[i]),   15U));
    CHECK(near(color::get_green(c), color::get_green(test_colors[i]), 15U));
    CHECK(near(color::get_blue(c),  color::get_blue(test_colors[i]),  15U));
  }
}


TEST_CASE(color_RGB555)
{
  CHECK_EQ(color::color_to_RGB555(color::black), 0x0000U);
  CHECK_EQ(color::color_to_RGB555(color::white), 0x7FFFU);
  CHECK_EQ(color::color_to_RGB555(color::red),   0x7C00U);
  CHECK_EQ(color::color_to_RGB555(color::green), 0x03E0U);
  CHECK_EQ(color::color_to_RGB555(color::blue),  0x001FU);
  CHECK_EQ(color::RGB555_to_color(0x7C00U), 0xFFF80000UL);
  CHECK_EQ(color::RGB555_to_color(0x03E0U), 0xFF00F800UL);
  CHECK_EQ(color::RGB555_to_color(0x001FU), 0xFF0000F8UL);
  for (unsigned i = 0U; i < 0x8000U; i += 7U) {
    CHECK_EQ(color::color_to_RGB555(color::RGB555_to_color(static_cast<std::uint16_t>(i))), i);
  }
  for (std::size_t i = 0U; i < test_color_count; ++i) {
    const color::value_type c = color::RGB555_to_color(color::color_to_RGB555(test_colors[i]));
    CHECK(near(color::get_red(c),   color::get_red(test_colors[i]),   7U));
    CHECK(near(color::get_green(c), color::get_green(test_colors[i]), 7U));
    CHECK(near(color::get_blue(c),  color::get_blue(test_colors[i]),  7U));
  }
}


TEST_CASE(color_RGB565)
{
  CHECK_EQ(color::color_to_RGB565(color::black), 0x0000U);
  CHECK_EQ(color::color_to_RGB565(color::white), 0xFFFFU);
  CHECK_EQ(color::color_to_RGB565(color::red),   0xF800U);
  CHECK_EQ(color::color_to_RGB565(color::green), 0x07E0U);
  CHECK_EQ(color::color_to_RGB565(color::blue),  0x001FU);
  CHECK_EQ(color::color_to_RGB565(0xFF102030UL), 0x1106U);
  CHECK_EQ(color::RGB565_to_color(0xF800U), 0xFFF80000UL);
  CHECK_EQ(color::RGB565_to_color(0x07E0U), 0xFF00FC00UL);
  CHECK_EQ(color::RGB565_to_color(0x001FU), 0xFF0000F8UL);
  CHECK_EQ(color::RGB565_to_color(0xFFFFU), 0xFFF8FCF8UL);
  for (unsigned i = 0U; i < 0x10000U; i += 7U) {
    CHECK_EQ(color::color_to_RGB565(color::RGB565_to_color(static_cast<std::uint16_t>(i))), i);
  }
  for (std::size_t i = 0U; i < test_color_count; ++i) {
    const color::value_type c = color::RGB565_to_color(color::color_to_RGB565(test_colors[i]));
    CHECK(near(color::get_red(c),   color::get_red(test_colors[i]),   7U));
    CHECK(near(color::get_green(c), color::get_green(test_colors[i]), 3U));
    CHECK(near(color::get_blue(c),  color::get_blue(test_colors[i]),  7U));
  }
}


TEST_CASE(color_RGB666)
{
  CHECK_EQ(color::color_to_RGB666(color::black), 0x00000UL);
  CHECK_EQ(color::color_to_RGB666(color::white), 0x3FFFFUL);
  CHECK_EQ(color::color_to_RGB666(color::red),   0x3F000UL);
  CHECK_EQ(color::color_to_RGB666(color::green), 0x00FC0UL);
  CHECK_EQ(color::color_to_RGB666(color::blue),  0x0003FUL);
  CHECK_EQ(color::RGB666_to_color(0x3F000UL), 0xFFFC0000UL);
  CHECK_EQ(color::RGB666_to_color(0x00FC0UL), 0xFF00FC00UL);
  CHECK_EQ(color::RGB666_to_color(0x0003FUL), 0xFF0000FCUL);
  for (unsigned i = 0U; i < 0x40000U; i += 37U) {
    CHECK_EQ(color::color_to_RGB666(color::RGB666_to_color(i)), i);
  }
  for (std::size_t i = 0U; i < test_color_count; ++i) {
    const color::value_type c = color::RGB666_to_color(color::color_to_RGB666(test_colors[i]));
    CHECK(near(color::get_red(c),   color::get_red(test_colors[i]),   3U));
    CHECK(near(color::get_green(c), color::get_green(test_colors[i]), 3U));
    CHECK(near(color::get_blue(c),  color::get_blue(test_colors[i]),  3U));
  }
}


TEST_CASE(color_RGB888)
{
  CHECK_EQ(color::color_to_RGB888(0x80123456UL), 0x00123456UL);
  CHECK_EQ(color::RGB888_to_color(0x00123456UL), 0xFF123456UL);
  CHECK_EQ(color::RGB888_to_color(0x12345678UL), 0xFF345678UL);
  for (std::size_t i = 0U; i < test_color_count; ++i) {
    CHECK_EQ(color::RGB888_to_color(color::color_to_RGB888(test_colors[i])), test_colors[i]);
  }
}


TEST_CASE(color_format_bytes_per_pixel)
{
  // upper nibble of the format is bytes per pixel
  CHECK_EQ(static_cast<unsigned>(color::format_L1) >> 4U,       1U);
  CHECK_EQ(static_cast<unsigned>(color::format_RGB332) >> 4U,   1U);
  CHECK_EQ(static_cast<unsigned>(color::format_RGB565) >> 4U,   2U);
  CHECK_EQ(static_cast<unsigned>(color::format_RGB888) >> 4U,   3U);
  CHECK_EQ(static_cast<unsigned>(color::format_ARGB8888) >> 4U, 4U);
  CHECK_EQ(static_cast<unsigned>(color::format_RGBA8888) >> 4U, 4U);
}
