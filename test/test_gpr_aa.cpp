///////////////////////////////////////////////////////////////////////////////
// \brief Tests for the anti aliased rendering of the gpr primitives
// The tests disable the alpha blending of the output shader, so the heads
// store the raw coverage alpha of every edge pixel.
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "helpers.h"
#include "dc.h"
#include "sprite.h"
#include "head/memory.h"

using namespace vic;

namespace {

typedef head::memory<24, 24> head_type;

struct aa_dc
{
  head_type head;
  dc        d;
  aa_dc()
    : d(head)
  {
    d.alpha_blending_enable(false);   // store the raw coverage alpha
    d.anti_aliasing_enable(true);
    d.set_color(color::white);
  }
};

inline std::uint32_t alpha_at(const head_type& h, std::int32_t x, std::int32_t y)
{
  const color::value_type c = h.pixel({ static_cast<std::int16_t>(x), static_cast<std::int16_t>(y) });
  return c == color::none ? 0U : color::get_alpha(c);
}

// sum of the alphas of a column (or row) within a y (x) range
inline std::uint32_t column_alpha(const head_type& h, std::int32_t x, std::int32_t y0, std::int32_t y1, std::uint32_t& pixels)
{
  std::uint32_t sum = 0U;
  pixels = 0U;
  for (std::int32_t y = y0; y <= y1; ++y) {
    const std::uint32_t a = alpha_at(h, x, y);
    sum += a;
    pixels += a ? 1U : 0U;
  }
  return sum;
}

inline std::uint32_t row_alpha(const head_type& h, std::int32_t y, std::int32_t x0, std::int32_t x1, std::uint32_t& pixels)
{
  std::uint32_t sum = 0U;
  pixels = 0U;
  for (std::int32_t x = x0; x <= x1; ++x) {
    const std::uint32_t a = alpha_at(h, x, y);
    sum += a;
    pixels += a ? 1U : 0U;
  }
  return sum;
}

inline std::uint32_t total_alpha(const head_type& h)
{
  std::uint32_t sum = 0U;
  for (std::int32_t y = 0; y < 24; ++y) {
    for (std::int32_t x = 0; x < 24; ++x) {
      sum += alpha_at(h, x, y);
    }
  }
  return sum;
}

} // namespace


TEST_CASE(gpr_aa_line_wu)
{
  // shallow line: every column has one or two pixels whose coverage sums up to 255
  aa_dc a;
  a.d.line({ 2, 2 }, { 17, 8 });
  for (std::int32_t x = 3; x <= 16; ++x) {
    std::uint32_t pixels = 0U;
    const std::uint32_t sum = column_alpha(a.head, x, 0, 23, pixels);
    CHECK_EQ(sum, 255U);
    CHECK(pixels >= 1U && pixels <= 2U);
  }
  // exact end points
  CHECK_EQ(alpha_at(a.head, 2, 2), 255U);
  CHECK_EQ(alpha_at(a.head, 17, 8), 255U);
  std::uint32_t n = 0U;
  CHECK_EQ(column_alpha(a.head, 2, 0, 23, n), 255U);
  CHECK_EQ(n, 1U);
  CHECK_EQ(column_alpha(a.head, 17, 0, 23, n), 255U);
  CHECK_EQ(n, 1U);
  // nothing outside the columns of the line
  CHECK_EQ(column_alpha(a.head, 1, 0, 23, n), 0U);
  CHECK_EQ(column_alpha(a.head, 18, 0, 23, n), 0U);
  // all pixels have the pen color
  for (std::int32_t y = 0; y < 24; ++y) {
    for (std::int32_t x = 0; x < 24; ++x) {
      const color::value_type c = a.head.pixel({ static_cast<std::int16_t>(x), static_cast<std::int16_t>(y) });
      if (c != color::none) {
        CHECK_EQ(c & 0x00FFFFFFUL, 0x00FFFFFFUL);
      }
    }
  }
  CHECK_EQ(a.head.present_count(), 1U);

  // the reversed line is identical
  aa_dc b;
  b.d.line({ 17, 8 }, { 2, 2 });
  CHECK_SAME_BUFFER(a.head, b.head);

  // steep line: every row sums up to 255
  aa_dc s;
  s.d.line({ 3, 2 }, { 9, 21 });
  for (std::int32_t y = 3; y <= 20; ++y) {
    std::uint32_t pixels = 0U;
    CHECK_EQ(row_alpha(s.head, y, 0, 23, pixels), 255U);
    CHECK(pixels >= 1U && pixels <= 2U);
  }
  CHECK_EQ(alpha_at(s.head, 3, 2), 255U);
  CHECK_EQ(alpha_at(s.head, 9, 21), 255U);

  // a diagonal line is a plain line (every pixel exactly on the line)
  aa_dc dg;
  dg.d.line({ 2, 2 }, { 12, 12 });
  CHECK_EQ(vic_test::count_pixels(dg.head, color::white), 11U);
  CHECK_EQ(vic_test::count_pixels_not(dg.head, color::none), 11U);

  // axis aligned lines are not affected by anti aliasing
  aa_dc h;
  h.d.line({ 2, 5 }, { 20, 5 });
  h.d.line({ 4, 1 }, { 4, 22 });
  head_type plain;
  dc dp(plain);
  dp.set_color(color::white);
  dp.line({ 2, 5 }, { 20, 5 });
  dp.line({ 4, 1 }, { 4, 22 });
  CHECK_SAME_BUFFER(h.head, plain);
}


TEST_CASE(gpr_aa_line_polyline_triangle)
{
  // polyline and triangle outlines use the anti aliased line
  aa_dc p;
  static const vertex_type v[3] = { { 2, 20 }, { 10, 3 }, { 21, 18 } };
  p.d.polyline(v, 3U);
  CHECK(vic_test::count_pixels_not(p.head, color::none) > 30U);
  CHECK(vic_test::count_pixels(p.head, color::white) < vic_test::count_pixels_not(p.head, color::none));   // partial pixels exist
  CHECK_EQ(p.head.present_count(), 1U);
  aa_dc t;
  t.d.triangle(v[0], v[1], v[2]);
  CHECK(vic_test::count_pixels_not(t.head, color::none) > vic_test::count_pixels_not(p.head, color::none));
  CHECK_EQ(t.head.present_count(), 1U);
}


TEST_CASE(gpr_aa_circle)
{
  aa_dc a;
  const std::int32_t cx = 11, cy = 11, r = 7;
  a.d.circle({ static_cast<std::int16_t>(cx), static_cast<std::int16_t>(cy) }, static_cast<std::uint16_t>(r));
  CHECK_EQ(a.head.present_count(), 1U);

  // eight fold symmetry
  for (std::int32_t dy = 0; dy <= r + 1; ++dy) {
    for (std::int32_t dx = 0; dx <= r + 1; ++dx) {
      const std::uint32_t v = alpha_at(a.head, cx + dx, cy + dy);
      CHECK_EQ(alpha_at(a.head, cx - dx, cy + dy), v);
      CHECK_EQ(alpha_at(a.head, cx + dx, cy - dy), v);
      CHECK_EQ(alpha_at(a.head, cx - dx, cy - dy), v);
      CHECK_EQ(alpha_at(a.head, cx + dy, cy + dx), v);
    }
  }
  // on the axes the ring is exactly one solid pixel wide, the neighbors are partially covered
  CHECK_EQ(alpha_at(a.head, cx + r, cy), 255U);
  CHECK_EQ(alpha_at(a.head, cx, cy - r), 255U);
  CHECK(alpha_at(a.head, cx + r, cy + 1) > 0U && alpha_at(a.head, cx + r, cy + 1) < 255U);
  CHECK(alpha_at(a.head, cx + r + 1, cy) < 64U);
  CHECK(alpha_at(a.head, cx + r - 1, cy) < 64U);
  // nothing far outside or inside
  CHECK_EQ(alpha_at(a.head, cx + r + 2, cy), 0U);
  CHECK_EQ(alpha_at(a.head, cx, cy), 0U);
  CHECK_EQ(alpha_at(a.head, cx + 3, cy + 3), 0U);
  // 45 degree: the ring passes between the pixels, both are partially covered
  CHECK(alpha_at(a.head, cx + 5, cy + 5) > 0U && alpha_at(a.head, cx + 5, cy + 5) < 255U);
  // the total coverage is the area of the one pixel wide ring: 2 * pi * r
  const std::uint32_t total = total_alpha(a.head);
  const std::uint32_t expect = static_cast<std::uint32_t>(2.0 * 3.14159 * r * 255.0);
  CHECK(total > expect * 95U / 100U && total < expect * 105U / 100U);
  const std::size_t lit = vic_test::count_pixels_not(a.head, color::none);
  CHECK(lit >= 44U && lit <= 132U);

  // radius 0 and 1
  aa_dc z;
  z.d.circle({ 5, 5 }, 0U);
  CHECK_EQ(alpha_at(z.head, 5, 5), 255U);
  CHECK_EQ(vic_test::count_pixels_not(z.head, color::none), 1U);
  aa_dc one;
  one.d.circle({ 5, 5 }, 1U);
  CHECK_EQ(alpha_at(one.head, 6, 5), 255U);
  CHECK_EQ(alpha_at(one.head, 5, 4), 255U);
  CHECK(alpha_at(one.head, 5, 5) < 96U);    // the ring from 0.5 to 1.5 touches the corners of the center pixel
}


TEST_CASE(gpr_aa_circle_arcs_tile)
{
  // the four quadrant arcs sum up to the full circle (within the rounding of the 16 samples per tile)
  aa_dc full, q0, q1, q2, q3;
  const vertex_type c = { 11, 11 };
  full.d.circle(c, 8U);
  q0.d.circle(c, 8U, 0U, 90U);
  q1.d.circle(c, 8U, 90U, 180U);
  q2.d.circle(c, 8U, 180U, 270U);
  q3.d.circle(c, 8U, 270U, 360U);
  for (std::int32_t y = 0; y < 24; ++y) {
    for (std::int32_t x = 0; x < 24; ++x) {
      const std::int32_t sum = static_cast<std::int32_t>(alpha_at(q0.head, x, y) + alpha_at(q1.head, x, y) + alpha_at(q2.head, x, y) + alpha_at(q3.head, x, y));
      CHECK(util::abs<std::int32_t>(sum - static_cast<std::int32_t>(alpha_at(full.head, x, y))) <= 4);
    }
  }
  // the quadrant arcs only touch their own quadrant (plus the shared axes)
  CHECK_EQ(alpha_at(q0.head, 11 - 6, 11 - 5), 0U);
  CHECK(alpha_at(q0.head, 11 + 6, 11 - 5) > 0U);
  CHECK(vic_test::count_pixels_not(q0.head, color::none) > 8U);
  // the arc of a rounded rectangle
  aa_dc rr;
  rr.d.rectangle(rect_type{ 2, 2, 20, 16 }, 4U);
  CHECK_EQ(alpha_at(rr.head, 10, 2), 255U);     // straight part
  CHECK_EQ(alpha_at(rr.head, 2, 9), 255U);
  CHECK_EQ(alpha_at(rr.head, 2, 2), 0U);        // corner is cut
  CHECK(alpha_at(rr.head, 3, 3) > 0U);          // and anti aliased
  CHECK_EQ(alpha_at(rr.head, 10, 9), 0U);       // inside empty
}


TEST_CASE(gpr_aa_disc)
{
  aa_dc a;
  const std::int32_t cx = 11, cy = 11, r = 6;
  a.d.disc({ static_cast<std::int16_t>(cx), static_cast<std::int16_t>(cy) }, static_cast<std::uint16_t>(r));
  CHECK_EQ(a.head.present_count(), 1U);

  CHECK_EQ(alpha_at(a.head, cx, cy), 255U);
  CHECK_EQ(alpha_at(a.head, cx + r, cy), 255U);
  CHECK_EQ(alpha_at(a.head, cx - r, cy), 255U);
  CHECK_EQ(alpha_at(a.head, cx, cy + r), 255U);
  CHECK_EQ(alpha_at(a.head, cx + r + 1, cy), 0U);
  // the pixels next to the axis points are partially covered
  CHECK(alpha_at(a.head, cx + r, cy + 1) > 0U && alpha_at(a.head, cx + r, cy + 1) < 255U);
  CHECK_EQ(alpha_at(a.head, cx + 4, cy + 4), 255U);                                            // well inside (d = 5.66 < 6.5)
  CHECK(alpha_at(a.head, cx + 5, cy + 4) > 0U && alpha_at(a.head, cx + 5, cy + 4) < 255U);   // on the edge (d = 6.4)
  // symmetry
  for (std::int32_t dy = 0; dy <= r + 1; ++dy) {
    for (std::int32_t dx = 0; dx <= r + 1; ++dx) {
      const std::uint32_t v = alpha_at(a.head, cx + dx, cy + dy);
      CHECK_EQ(alpha_at(a.head, cx - dx, cy + dy), v);
      CHECK_EQ(alpha_at(a.head, cx + dx, cy - dy), v);
      CHECK_EQ(alpha_at(a.head, cx + dy, cy + dx), v);
    }
  }
  // the coverage never increases from the center outwards along a row
  for (std::int32_t dy = 0; dy <= r; ++dy) {
    for (std::int32_t dx = 0; dx <= r; ++dx) {
      CHECK(alpha_at(a.head, cx + dx, cy + dy) >= alpha_at(a.head, cx + dx + 1, cy + dy));
    }
  }
  // the total coverage is the area of a circle with radius r + 0.5
  const std::uint32_t total = total_alpha(a.head);
  const std::uint32_t expect = static_cast<std::uint32_t>(3.14159 * 6.5 * 6.5 * 255.0);
  CHECK(total > expect * 97U / 100U && total < expect * 103U / 100U);

  // the fast path (head line runs) and the pipe path render identical pixels
  head_type p;
  dc dp(p);
  vic_test::passthrough_shader pass;
  dp.shader_register(&pass);
  dp.alpha_blending_enable(false);
  dp.anti_aliasing_enable(true);
  dp.set_color(color::white);
  dp.disc({ static_cast<std::int16_t>(cx), static_cast<std::int16_t>(cy) }, static_cast<std::uint16_t>(r));
  CHECK_SAME_BUFFER(a.head, p);

  // small radii
  aa_dc z;
  z.d.disc({ 5, 5 }, 0U);
  CHECK_EQ(alpha_at(z.head, 5, 5), 255U);
  CHECK(alpha_at(z.head, 6, 5) < 64U);
  aa_dc one;
  one.d.disc({ 5, 5 }, 1U);
  CHECK_EQ(alpha_at(one.head, 5, 5), 255U);
  CHECK_EQ(alpha_at(one.head, 6, 5), 255U);
  CHECK(alpha_at(one.head, 6, 6) > 0U && alpha_at(one.head, 6, 6) < 255U);
}


TEST_CASE(gpr_aa_disc_sectors)
{
  // the four quadrants cover the disc, quadrants share only the center row/column
  aa_dc full, q0, q1, q2, q3;
  const vertex_type c = { 11, 11 };
  full.d.disc(c, 6U);
  q0.d.disc_sector(c, 6U, 0U);
  q1.d.disc_sector(c, 6U, 1U);
  q2.d.disc_sector(c, 6U, 2U);
  q3.d.disc_sector(c, 6U, 3U);
  for (std::int32_t y = 0; y < 24; ++y) {
    for (std::int32_t x = 0; x < 24; ++x) {
      const std::uint32_t a[4] = { alpha_at(q0.head, x, y), alpha_at(q1.head, x, y), alpha_at(q2.head, x, y), alpha_at(q3.head, x, y) };
      std::uint32_t mx = 0U, cnt = 0U;
      for (int i = 0; i < 4; ++i) {
        mx = a[i] > mx ? a[i] : mx;
        cnt += a[i] ? 1U : 0U;
      }
      CHECK_EQ(mx, alpha_at(full.head, x, y));
      if ((x != c.x) && (y != c.y)) {
        CHECK(cnt <= 1U);
      }
    }
  }
  // quadrant 0 is top/right
  CHECK_EQ(alpha_at(q0.head, 14, 8), 255U);
  CHECK_EQ(alpha_at(q0.head, 8, 8), 0U);
  CHECK_EQ(alpha_at(q0.head, 14, 14), 0U);

  // the rounded box corners are anti aliased, the rest is solid
  aa_dc rb;
  rb.d.box(rect_type{ 2, 2, 20, 16 }, 4U);
  CHECK_EQ(alpha_at(rb.head, 10, 8), 255U);
  CHECK_EQ(alpha_at(rb.head, 10, 2), 255U);
  CHECK_EQ(alpha_at(rb.head, 2, 9), 255U);
  CHECK_EQ(alpha_at(rb.head, 2, 2), 0U);
  CHECK(alpha_at(rb.head, 3, 3) > 0U && alpha_at(rb.head, 3, 3) < 255U);
  CHECK_EQ(alpha_at(rb.head, 1, 8), 0U);
  CHECK_EQ(alpha_at(rb.head, 20, 8), 0U);
  CHECK_EQ(alpha_at(rb.head, 10, 16), 0U);
}


TEST_CASE(gpr_aa_sector)
{
  const vertex_type c = { 11, 11 };
  // four quadrant sectors sum up to the full ring (within the rounding of 16 samples per tile)
  aa_dc full, q0, q1, q2, q3;
  full.d.sector(c, 2U, 7U, 0U, 360U);
  q0.d.sector(c, 2U, 7U, 0U, 90U);
  q1.d.sector(c, 2U, 7U, 90U, 180U);
  q2.d.sector(c, 2U, 7U, 180U, 270U);
  q3.d.sector(c, 2U, 7U, 270U, 360U);
  for (std::int32_t y = 0; y < 24; ++y) {
    for (std::int32_t x = 0; x < 24; ++x) {
      const std::int32_t sum = static_cast<std::int32_t>(alpha_at(q0.head, x, y) + alpha_at(q1.head, x, y) + alpha_at(q2.head, x, y) + alpha_at(q3.head, x, y));
      const std::int32_t ref = static_cast<std::int32_t>(alpha_at(full.head, x, y));
      CHECK(util::abs<std::int32_t>(sum - ref) <= 4);
    }
  }
  CHECK_EQ(full.head.present_count(), 1U);
  // ring: inside and outside empty, the ring itself solid
  CHECK_EQ(alpha_at(full.head, 11, 11), 0U);
  CHECK_EQ(alpha_at(full.head, 11, 19), 0U);
  CHECK_EQ(alpha_at(full.head, 15, 11), 255U);
  CHECK_EQ(alpha_at(full.head, 17, 11), 255U);
  CHECK_EQ(alpha_at(full.head, 12, 11), 0U);                                       // inner edge is exactly on the pixel border
  CHECK(alpha_at(full.head, 12, 12) > 0U && alpha_at(full.head, 12, 12) < 255U);   // inner edge, 45 degree
  // an angular edge is anti aliased: the 30 degree ray crosses pixels partially
  aa_dc w;
  w.d.sector(c, 0U, 9U, 30U, 150U);
  CHECK_EQ(alpha_at(w.head, 11, 6), 255U);        // 90 degree
  CHECK_EQ(alpha_at(w.head, 11, 16), 0U);         // 270 degree
  CHECK_EQ(alpha_at(w.head, 19, 11), 0U);         // 0 degree
  bool partial = false;
  for (std::int32_t x = 12; x < 20; ++x) {
    for (std::int32_t y = 5; y < 11; ++y) {
      const std::uint32_t a = alpha_at(w.head, x, y);
      partial |= (a > 0U) && (a < 255U) && ((x - 11) * (x - 11) + (y - 11) * (y - 11) < 36);   // partial pixel well inside the radius
    }
  }
  CHECK(partial);
  // the center pixel is split between the tiles of a pie
  aa_dc p0, p1, p2, p3;
  p0.d.sector(c, 0U, 5U, 0U, 90U);
  p1.d.sector(c, 0U, 5U, 90U, 180U);
  p2.d.sector(c, 0U, 5U, 180U, 270U);
  p3.d.sector(c, 0U, 5U, 270U, 360U);
  const std::uint32_t center = alpha_at(p0.head, 11, 11) + alpha_at(p1.head, 11, 11) + alpha_at(p2.head, 11, 11) + alpha_at(p3.head, 11, 11);
  CHECK(center >= 251U && center <= 255U);
  // wrap around
  aa_dc wr;
  wr.d.sector(c, 0U, 6U, 315U, 45U);
  CHECK_EQ(alpha_at(wr.head, 15, 11), 255U);
  CHECK_EQ(alpha_at(wr.head, 7, 11), 0U);
}


TEST_CASE(gpr_aa_triangle_solid)
{
  aa_dc a;
  a.d.triangle_solid({ 2, 2 }, { 12, 2 }, { 2, 12 });
  CHECK_EQ(a.head.present_count(), 1U);
  // interior
  CHECK_EQ(alpha_at(a.head, 4, 4), 255U);
  CHECK_EQ(alpha_at(a.head, 3, 3), 255U);
  CHECK_EQ(alpha_at(a.head, 6, 5), 255U);
  // the hypotenuse runs through the pixel centers: about half coverage (samples on the edge count as inside)
  CHECK(alpha_at(a.head, 7, 7) >= 96U && alpha_at(a.head, 7, 7) <= 176U);
  CHECK(alpha_at(a.head, 5, 9) >= 96U && alpha_at(a.head, 5, 9) <= 176U);
  // outside
  CHECK_EQ(alpha_at(a.head, 9, 9), 0U);
  CHECK_EQ(alpha_at(a.head, 13, 2), 0U);
  CHECK_EQ(alpha_at(a.head, 1, 5), 0U);
  // the right angle vertex pixel is a quarter covered, the acute vertices less
  CHECK(alpha_at(a.head, 2, 2) >= 48U && alpha_at(a.head, 2, 2) <= 80U);
  CHECK(alpha_at(a.head, 12, 2) <= 48U);
  // the straight edges run through the pixel centers as well: half coverage
  CHECK(alpha_at(a.head, 6, 2) >= 112U && alpha_at(a.head, 6, 2) <= 144U);
  CHECK(alpha_at(a.head, 2, 6) >= 112U && alpha_at(a.head, 2, 6) <= 144U);

  // vertex order doesn't matter
  aa_dc b, c;
  b.d.triangle_solid({ 12, 2 }, { 2, 12 }, { 2, 2 });
  c.d.triangle_solid({ 2, 12 }, { 2, 2 }, { 12, 2 });
  CHECK_SAME_BUFFER(a.head, b.head);
  CHECK_SAME_BUFFER(a.head, c.head);

  // pipe path and fast path are identical
  head_type p;
  dc dp(p);
  vic_test::passthrough_shader pass;
  dp.shader_register(&pass);
  dp.alpha_blending_enable(false);
  dp.anti_aliasing_enable(true);
  dp.set_color(color::white);
  dp.triangle_solid({ 2, 2 }, { 12, 2 }, { 2, 12 });
  CHECK_SAME_BUFFER(a.head, p);

  // degenerated triangles still work
  aa_dc dg;
  dg.d.triangle_solid({ 2, 2 }, { 6, 6 }, { 10, 10 });
  CHECK_EQ(vic_test::count_pixels(dg.head, color::white), 9U);
  dg.d.triangle_solid({ 15, 15 }, { 15, 15 }, { 15, 15 });
  CHECK_EQ(alpha_at(dg.head, 15, 15), 255U);
}


TEST_CASE(gpr_aa_alpha_pen_and_blending)
{
  // the pen alpha is multiplied with the coverage
  aa_dc a;
  a.d.set_color(0x80FFFFFFUL);
  a.d.line({ 2, 2 }, { 17, 8 });
  for (std::int32_t x = 3; x <= 16; ++x) {
    std::uint32_t pixels = 0U;
    const std::uint32_t sum = column_alpha(a.head, x, 0, 23, pixels);
    CHECK(sum >= 126U && sum <= 128U);
  }

  // with blending enabled the edge pixels are mixed with the background
  head_type h;
  dc d(h);
  d.cls(color::blue);
  d.anti_aliasing_enable(true);
  d.set_color(color::white);
  d.disc({ 11, 11 }, 6U);
  CHECK_EQ(h.pixel({ 11, 11 }), color::white);
  const color::value_type edge = h.pixel({ 16, 15 });   // d = 6.4, the disc edge is at 6.5
  CHECK(color::is_opaque(edge));
  CHECK(color::get_red(edge) > 0U && color::get_red(edge) < 255U);
  CHECK_EQ(color::get_blue(edge), 255U);
  CHECK_EQ(h.pixel({ 15, 15 }), color::white);
  CHECK_EQ(h.pixel({ 18, 18 }), color::blue);
}


TEST_CASE(gpr_aa_sprite)
{
  // anti aliased drawing on a canvas sprite, rendered on a background
  head_type h;
  dc d(h);
  d.cls(color::blue);
  sprite::canvas<256, 1> s(h, 0);
  s.anti_aliasing_enable(true);
  s.set_color(color::white);
  s.disc({ 0, 0 }, 4U);
  CHECK(s.pixel_count() > 45U);                  // 7 x 7 core plus edge pixels
  s.render({ 11, 11 });
  CHECK_EQ(h.pixel({ 11, 11 }), color::white);
  CHECK_EQ(h.pixel({ 15, 11 }), color::white);
  const color::value_type edge = h.pixel({ 14, 14 });
  CHECK(color::is_opaque(edge));
  CHECK(color::get_red(edge) > 0U && color::get_red(edge) < 255U);
  CHECK_EQ(h.pixel({ 17, 17 }), color::blue);
  // moving the sprite restores the background exactly
  s.render({ 2, 2 });
  CHECK_EQ(h.pixel({ 14, 14 }), color::blue);
  CHECK_EQ(h.pixel({ 11, 11 }), color::blue);
}
