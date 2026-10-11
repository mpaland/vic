///////////////////////////////////////////////////////////////////////////////
// \brief Tests for util.h: vertex_type, pixel_type, rect_type and util functions
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "util.h"

#include <cmath>

using namespace vic;


///////////////////////////////////////////////////////////////////////////////
// V E R T E X

TEST_CASE(util_vertex_compare)
{
  const vertex_type a = { 1, 2 }, b = { 1, 2 }, c = { 2, 1 };
  CHECK(a == b);
  CHECK(!(a != b));
  CHECK(a != c);
  CHECK(!(a == c));

  // operator< is y-major
  CHECK(c < a);
  CHECK(!(a < c));
  CHECK(!(a < b));
  const vertex_type d = { 0, 2 };
  CHECK(d < a);
}


TEST_CASE(util_vertex_arithmetic)
{
  vertex_type a = { 10, -5 };
  const vertex_type b = { 3, 7 };
  const vertex_type sum  = a + b;
  const vertex_type diff = a - b;
  CHECK_EQ(sum.x,  13);
  CHECK_EQ(sum.y,   2);
  CHECK_EQ(diff.x,  7);
  CHECK_EQ(diff.y, -12);

  a += b;
  CHECK(a == sum);
  a -= b;
  a -= b;
  CHECK(a == diff);
}


TEST_CASE(util_vertex_swap_min)
{
  vertex_type a = { 5, 1 }, b = { 2, 9 };
  a.swap(b);
  CHECK((a == vertex_type{ 2, 9 }));
  CHECK((b == vertex_type{ 5, 1 }));

  // min_x: a gets the smaller x
  a = { 5, 1 }; b = { 2, 9 };
  a.min_x(b);
  CHECK_EQ(a.x, 2);
  CHECK_EQ(b.x, 5);
  a.min_x(b);   // already ordered, nothing happens
  CHECK_EQ(a.x, 2);
  CHECK_EQ(b.x, 5);

  // min_y: a gets the smaller y
  a = { 5, 9 }; b = { 2, 1 };
  a.min_y(b);
  CHECK_EQ(a.y, 1);
  CHECK_EQ(b.y, 9);
}


///////////////////////////////////////////////////////////////////////////////
// P I X E L

TEST_CASE(util_pixel_ops)
{
  pixel_type p = { { 3, 4 }, 0xFF112233UL };
  const pixel_type q = { { 3, 4 }, 0xFF000000UL };
  // comparison only uses the vertex
  CHECK(p == q);
  CHECK(!(p != q));

  const vertex_type off = { 10, 20 };
  pixel_type r = p + off;
  CHECK_EQ(r.vertex.x, 13);
  CHECK_EQ(r.vertex.y, 24);
  CHECK_EQ(r.color, 0xFF112233UL);
  r = r - off;
  CHECK(r.vertex == p.vertex);
  p += off;
  CHECK_EQ(p.vertex.x, 13);
  CHECK_EQ(p.vertex.y, 24);
  CHECK_EQ(p.color, 0xFF112233UL);
}


///////////////////////////////////////////////////////////////////////////////
// R E C T

TEST_CASE(util_rect_basics)
{
  rect_type r = { 2, 3, 7, 9 };
  CHECK_EQ(r.width(),  5);
  CHECK_EQ(r.height(), 6);
  CHECK(!r.empty());
  CHECK((r.top_left()     == vertex_type{ 2, 3 }));
  CHECK((r.bottom_right() == vertex_type{ 7, 9 }));

  r.top_left() = { 1, 1 };
  CHECK_EQ(r.left, 1);
  CHECK_EQ(r.top,  1);
  r.bottom_right() = { 4, 2 };
  CHECK_EQ(r.right,  4);
  CHECK_EQ(r.bottom, 2);

  r.clear();
  CHECK((r == rect_type{ 0, 0, 0, 0 }));
  CHECK(r.empty());

  const rect_type a = { 0, 0, 5, 0 };
  CHECK(a.empty());
  const rect_type b = { 0, 0, 0, 5 };
  CHECK(b.empty());

  CHECK((rect_type{ 1, 2, 3, 4 } == rect_type{ 1, 2, 3, 4 }));
  CHECK((rect_type{ 1, 2, 3, 4 } != rect_type{ 1, 2, 3, 5 }));
}


TEST_CASE(util_rect_normalize)
{
  rect_type r;
  // both vertices are inside the rect, so right/bottom are one past the second vertex
  r.normalize({ 2, 3 }, { 7, 9 });
  CHECK((r == rect_type{ 2, 3, 8, 10 }));
  CHECK(r.contain({ 2, 3 }));
  CHECK(r.contain({ 7, 9 }));

  // swapped order gives the same rect
  r.normalize({ 7, 9 }, { 2, 3 });
  CHECK((r == rect_type{ 2, 3, 8, 10 }));

  // mixed order
  r.normalize({ 7, 3 }, { 2, 9 });
  CHECK((r == rect_type{ 2, 3, 8, 10 }));
  r.normalize({ 2, 9 }, { 7, 3 });
  CHECK((r == rect_type{ 2, 3, 8, 10 }));

  // single pixel
  r.normalize({ 5, 5 }, { 5, 5 });
  CHECK((r == rect_type{ 5, 5, 6, 6 }));
  CHECK_EQ(r.width(),  1);
  CHECK_EQ(r.height(), 1);

  // negative coords
  r.normalize({ -3, -4 }, { 1, 2 });
  CHECK((r == rect_type{ -3, -4, 2, 3 }));

  // extreme coordinate must not wrap
  r.normalize({ 0, 0 }, { 32767, 32767 });
  CHECK(r.right  > 0);
  CHECK(r.bottom > 0);
  CHECK(r.contain({ 32766, 32766 }));
}


TEST_CASE(util_rect_contain)
{
  const rect_type r = { 2, 3, 7, 9 };
  // left/top inside
  CHECK(r.contain({ 2, 3 }));
  // right/bottom outside
  CHECK(!r.contain({ 7, 3 }));
  CHECK(!r.contain({ 2, 9 }));
  CHECK(!r.contain({ 7, 9 }));
  // last inside pixel
  CHECK(r.contain({ 6, 8 }));
  // outside
  CHECK(!r.contain({ 1, 5 }));
  CHECK(!r.contain({ 5, 2 }));
  CHECK(!r.contain({ -100, -100 }));

  // empty rect contains nothing
  const rect_type e = { 5, 5, 5, 5 };
  CHECK(!e.contain({ 5, 5 }));
}


TEST_CASE(util_rect_inflate)
{
  rect_type r = { 2, 3, 7, 9 };
  // inside: nothing changes
  r.inflate({ 4, 4 });
  CHECK((r == rect_type{ 2, 3, 7, 9 }));

  // right/bottom border is outside, so inflate must grow
  r.inflate({ 7, 9 });
  CHECK((r == rect_type{ 2, 3, 8, 10 }));
  CHECK(r.contain({ 7, 9 }));

  // left/top
  r.inflate({ 0, 1 });
  CHECK((r == rect_type{ 0, 1, 8, 10 }));
  CHECK(r.contain({ 0, 1 }));

  // far away
  r.inflate({ 20, -5 });
  CHECK((r == rect_type{ 0, -5, 21, 10 }));
  CHECK(r.contain({ 20, -5 }));
}


///////////////////////////////////////////////////////////////////////////////
// U T I L   F U N C T I O N S

TEST_CASE(util_min_max)
{
  CHECK_EQ(util::min2(3, 5), 3);
  CHECK_EQ(util::min2(5, 3), 3);
  int a = 7, b = 2;
  CHECK_EQ(util::max2(a, b), 7);
  CHECK_EQ(util::max2(b, a), 7);
  CHECK_EQ(util::min3(3, 1, 2), 1);
  CHECK_EQ(util::min3(1, 2, 3), 1);
  CHECK_EQ(util::min3(2, 3, 1), 1);
  CHECK_EQ(util::max3(3, 1, 2), 3);
  CHECK_EQ(util::max3(1, 2, 3), 3);
  CHECK_EQ(util::max3(2, 3, 1), 3);

  std::int16_t mn = 9, mx = 4;
  util::min_max_swap(mn, mx);
  CHECK_EQ(mn, 4);
  CHECK_EQ(mx, 9);
  util::min_max_swap(mn, mx);
  CHECK_EQ(mn, 4);
  CHECK_EQ(mx, 9);
}


TEST_CASE(util_abs)
{
  CHECK_EQ(util::abs<std::int16_t>(0), 0);
  CHECK_EQ(util::abs<std::int16_t>(5), 5);
  CHECK_EQ(util::abs<std::int16_t>(-5), 5);
  CHECK_EQ(util::abs<std::int16_t>(-32767), 32767);
  CHECK_EQ(util::abs<std::int32_t>(-100000), 100000);
  CHECK_EQ(util::abs<std::int32_t>(100000), 100000);
  CHECK_EQ(util::abs<std::int8_t>(-100), 100);
}


TEST_CASE(util_byte_reverse)
{
  CHECK_EQ(util::byte_reverse(0x00U), 0x00U);
  CHECK_EQ(util::byte_reverse(0xFFU), 0xFFU);
  CHECK_EQ(util::byte_reverse(0x80U), 0x01U);
  CHECK_EQ(util::byte_reverse(0x01U), 0x80U);
  CHECK_EQ(util::byte_reverse(0x40U), 0x02U);
  CHECK_EQ(util::byte_reverse(0xA5U), 0xA5U);
  CHECK_EQ(util::byte_reverse(0x12U), 0x48U);
  // reverse twice is identity for all values
  for (unsigned i = 0U; i < 256U; ++i) {
    CHECK_EQ(util::byte_reverse(util::byte_reverse(static_cast<std::uint8_t>(i))), i);
  }
}


TEST_CASE(util_div255)
{
  // valid for the whole product range of two bytes
  for (unsigned i = 0U; i <= 255U * 255U; i += 7U) {
    CHECK_EQ(util::div255(static_cast<std::uint16_t>(i)), i / 255U);
  }
  CHECK_EQ(util::div255(0U), 0U);
  CHECK_EQ(util::div255(255U), 1U);
  CHECK_EQ(util::div255(254U), 0U);
  CHECK_EQ(util::div255(65025U), 255U);
}


TEST_CASE(util_isqrt)
{
  CHECK_EQ(util::isqrt(0U), 0U);
  CHECK_EQ(util::isqrt(1U), 1U);
  CHECK_EQ(util::isqrt(3U), 1U);
  CHECK_EQ(util::isqrt(4U), 2U);
  CHECK_EQ(util::isqrt(99U), 9U);
  CHECK_EQ(util::isqrt(100U), 10U);
  CHECK_EQ(util::isqrt(65535U), 255U);
  CHECK_EQ(util::isqrt(65536U), 256U);
  CHECK_EQ(util::isqrt(0xFFFFFFFFUL), 65535U);
  for (std::uint32_t v = 0U; v < 70000U; v += 7U) {
    const std::uint32_t s = util::isqrt(v);
    CHECK(s * s <= v);
    CHECK((s + 1U) * (s + 1U) > v);
  }
}


TEST_CASE(util_div_round_closest)
{
  CHECK_EQ(util::div_round_closest(8, 3), 3);
  CHECK_EQ(util::div_round_closest(7, 3), 2);
  CHECK_EQ(util::div_round_closest(9, 3), 3);
  CHECK_EQ(util::div_round_closest(10, 4), 3);    // 2.5 rounds away from zero
  CHECK_EQ(util::div_round_closest(-8, 3), -3);
  CHECK_EQ(util::div_round_closest(-7, 3), -2);
  CHECK_EQ(util::div_round_closest(8, -3), -3);
  CHECK_EQ(util::div_round_closest(-8, -3), 3);
  CHECK_EQ(util::div_round_closest(0, 5), 0);
  CHECK_EQ(util::div_round_closest(100000, 7), 14286);
}


TEST_CASE(util_distance_orient)
{
  CHECK_EQ(util::distance_squared({ 0, 0 }, { 3, 4 }), 25U);
  CHECK_EQ(util::distance_squared({ 3, 4 }, { 0, 0 }), 25U);
  CHECK_EQ(util::distance_squared({ -1, -1 }, { 2, 3 }), 25U);
  CHECK_EQ(util::distance_squared({ 5, 5 }, { 5, 5 }), 0U);

  // orient_2d: > 0 right side, 0 on line, < 0 left side (screen coords)
  const vertex_type v0 = { 0, 0 }, v1 = { 10, 0 };
  CHECK(util::orient_2d({ 5, 0 }, v0, v1) == 0);
  CHECK(util::orient_2d({ 20, 0 }, v0, v1) == 0);
  const std::int32_t below = util::orient_2d({ 5, 5 }, v0, v1);
  const std::int32_t above = util::orient_2d({ 5, -5 }, v0, v1);
  CHECK(below != 0);
  CHECK(above != 0);
  CHECK((below > 0) != (above > 0));
  // swapping the line direction flips the sign
  CHECK_EQ(util::orient_2d({ 5, 5 }, v1, v0), -below);
}


TEST_CASE(util_sin_cos)
{
  // exact values
  CHECK_EQ(util::sin(0),    0);
  CHECK_EQ(util::sin(90),   16384);
  CHECK_EQ(util::sin(180),  0);
  CHECK_EQ(util::sin(270), -16384);
  CHECK_EQ(util::sin(360),  0);
  CHECK_EQ(util::sin(450),  16384);
  CHECK_EQ(util::sin(-90), -16384);
  CHECK_EQ(util::sin(-270), 16384);
  CHECK_EQ(util::cos(0),    16384);
  CHECK_EQ(util::cos(90),   0);
  CHECK_EQ(util::cos(180), -16384);
  CHECK_EQ(util::cos(270),  0);
  CHECK_EQ(util::cos(-90),  0);

  // compare against the real sine for every degree in a wide range, tolerance 2 LSB
  for (int a = -720; a <= 720; ++a) {
    const double s = std::sin(a * 3.14159265358979323846 / 180.0) * 16384.0;
    const double c = std::cos(a * 3.14159265358979323846 / 180.0) * 16384.0;
    const int si = util::sin(static_cast<std::int16_t>(a));
    const int ci = util::cos(static_cast<std::int16_t>(a));
    if (!CHECK_MSG(std::fabs(si - s) <= 2.0, "sin deviation")) {
      std::printf("    angle %d: sin %d, expected %.1f\n", a, si, s);
    }
    if (!CHECK_MSG(std::fabs(ci - c) <= 2.0, "cos deviation")) {
      std::printf("    angle %d: cos %d, expected %.1f\n", a, ci, c);
    }
  }

  // extreme input must not index out of bounds and must stay in range
  const int smin = util::sin(-32768);
  CHECK(smin >= -16384 && smin <= 16384);
  const int smax = util::sin(32767);
  CHECK(smax >= -16384 && smax <= 16384);
  const int cmin = util::cos(-32768);
  CHECK(cmin >= -16384 && cmin <= 16384);
  const int cmax = util::cos(32767);
  CHECK(cmax >= -16384 && cmax <= 16384);
}


TEST_CASE(util_vertex_rotate)
{
  const vertex_type c = { 10, 10 };
  // 0 degree: identity
  CHECK((util::vertex_rotate({ 15, 10 }, c, 0) == vertex_type{ 15, 10 }));
  // 90 degree clockwise in screen coords: (15,10) -> (10,15)
  CHECK((util::vertex_rotate({ 15, 10 }, c, 90) == vertex_type{ 10, 15 }));
  CHECK((util::vertex_rotate({ 15, 10 }, c, 180) == vertex_type{ 5, 10 }));
  CHECK((util::vertex_rotate({ 15, 10 }, c, 270) == vertex_type{ 10, 5 }));
  CHECK((util::vertex_rotate({ 15, 10 }, c, -90) == vertex_type{ 10, 5 }));
  CHECK((util::vertex_rotate({ 15, 10 }, c, 360) == vertex_type{ 15, 10 }));
  // center is invariant
  CHECK((util::vertex_rotate(c, c, 123) == c));
  // 45 degree: (10+10, 10) -> (10+7, 10+7)
  CHECK((util::vertex_rotate({ 20, 10 }, c, 45) == vertex_type{ 17, 17 }));
  // rotation keeps the distance (within rounding)
  for (int a = 0; a < 360; a += 15) {
    const vertex_type p = util::vertex_rotate({ 110, 10 }, c, static_cast<std::int16_t>(a));
    const std::uint32_t d = util::distance_squared(p, c);
    CHECK(d >= 99U * 99U && d <= 101U * 101U);
  }
}
