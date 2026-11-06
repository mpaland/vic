///////////////////////////////////////////////////////////////////////////////
// \brief Tests for the vector (outline) fonts: exact coverage of the rasterizer
// in the text renderer (src/txr.h) with synthetic outlines, and the generated
// fonts of tools/vecfontgen.py at different sizes
///////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "vic_test.h"
#include "helpers.h"
#include "dc.h"
#include "tc.h"
#include "head/memory.h"
#include "font/Montserrat_vec.h"
#include "font/Michroma_vec.h"
#include "font/Doto_vec.h"
#include "font/Montserrat_24_aa4.h"

using namespace vic;

namespace {

typedef head::memory<80, 80> head_type;

// encoded x of an outline point, the LSB is the on-curve flag
inline std::int16_t on(int x)  { return static_cast<std::int16_t>(x * 2 + 1); }
inline std::int16_t off(int x) { return static_cast<std::int16_t>(x * 2); }


// a vector font with the single glyph 'A', the line height is 'units' font units, ascent = units,
// the advance is 'xdist' font units (default: units)
class test_font final : public font::font_type
{
public:
  test_font(const std::int16_t* data, std::uint16_t size, std::uint16_t units = 64U, std::uint16_t xdist = 0U)
  {
    glyph_.xdist      = xdist ? xdist : units;
    glyph_.size       = size;
    glyph_.data       = data;
    block_.first      = 'A';
    block_.last       = 'A';
    block_.char_info  = &glyph_;
    block_.next       = nullptr;
    vector_.units     = units;
    vector_.ascent    = static_cast<std::int16_t>(units);
    vector_.block     = &block_;
    info_.attr        = font::FORMAT_VECTOR | font::ENCODING_UNICODE | font::TYPE_PROP;
    info_.ysize       = 64U;
    info_.ydist       = 64U;
    info_.baseline    = 64U;
    info_.family.vector = &vector_;
  }
  virtual const font::info_type& info() const { return info_; }

private:
  font::vector_charinfo_type glyph_;
  font::vector_block_type    block_;
  font::vector_type          vector_;
  font::info_type            info_;
};


// render 'A' of the font at the given size at 4/4
template<typename Head>
void render(Head& h, const font::font_type& f, std::uint8_t size)
{
  dc d(h);
  tc t(d);
  t.set_font(f, size);
  t.set_color(color::white);
  t.set_pos({ 4, 4 });
  t.out(static_cast<std::uint16_t>('A'));
}

// alpha of a pixel, 0 for an unset pixel
template<typename Head>
std::uint32_t alpha(const Head& h, int x, int y)
{
  const color::value_type c = h.pixel({ static_cast<std::int16_t>(x), static_cast<std::int16_t>(y) });
  return c == color::none ? 0U : color::get_alpha(c);
}

// sum of the alpha of all pixels (coverage area * 255)
template<typename Head>
std::uint32_t alpha_sum(const Head& h)
{
  std::uint32_t sum = 0U;
  for (std::int16_t y = 0; y < static_cast<std::int16_t>(h.screen_height()); ++y) {
    for (std::int16_t x = 0; x < static_cast<std::int16_t>(h.screen_width()); ++x) {
      sum += alpha(h, x, y);
    }
  }
  return sum;
}

// coverage area matches within 0.5 % (the alpha is truncated per pixel)
bool area_near(std::uint32_t sum, double area)
{
  return std::fabs(sum / 255.0 - area) <= area * 0.005 + 1.0;
}

// pixel box of the rendered pixels
template<typename Head>
rect_type ink_box(const Head& h)
{
  rect_type r = { 32767, 32767, -1, -1 };
  for (std::int16_t y = 0; y < static_cast<std::int16_t>(h.screen_height()); ++y) {
    for (std::int16_t x = 0; x < static_cast<std::int16_t>(h.screen_width()); ++x) {
      if (h.pixel({ x, y }) != color::none) {
        r.left   = x < r.left   ? x : r.left;
        r.top    = y < r.top    ? y : r.top;
        r.right  = x > r.right  ? x : r.right;
        r.bottom = y > r.bottom ? y : r.bottom;
      }
    }
  }
  return r;
}

} // namespace


TEST_CASE(fonts_vector_square_exact)
{
  // a square 16..48 in a 64 unit line: pixel aligned at 64 and 32 pixel, all pixels solid
  static const std::int16_t square[] = { 0, 0, 4, on(16), 16, on(16), 48, on(48), 48, on(48), 16 };
  test_font f(square, sizeof(square) / sizeof(square[0]));
  {
    head_type h;
    render(h, f, 64U);
    CHECK_EQ(vic_test::count_pixels_not(h, color::none), 32U * 32U);
    CHECK_EQ(vic_test::count_pixels(h, color::white), 32U * 32U);
    CHECK_EQ(alpha(h, 4 + 16, 4 + 16), 255U);
    CHECK_EQ(alpha(h, 4 + 47, 4 + 47), 255U);
    CHECK_EQ(alpha(h, 4 + 48, 4 + 47), 0U);
    CHECK_EQ(alpha(h, 4 + 15, 4 + 16), 0U);
  }
  {
    head_type h;
    render(h, f, 32U);
    CHECK_EQ(vic_test::count_pixels(h, color::white), 16U * 16U);
    CHECK_EQ(vic_test::count_pixels_not(h, color::none), 16U * 16U);
    CHECK_EQ(alpha(h, 4 + 8, 4 + 8), 255U);
    CHECK_EQ(alpha(h, 4 + 23, 4 + 23), 255U);
  }
}


TEST_CASE(fonts_vector_square_subpixel)
{
  // at 33 pixel the square edges are at 8.25 and 24.75 pixel: 3/4 covered edge pixels, 9/16 covered corners
  static const std::int16_t square[] = { 0, 0, 4, on(16), 16, on(16), 48, on(48), 48, on(48), 16 };
  test_font f(square, sizeof(square) / sizeof(square[0]));
  head_type h;
  render(h, f, 33U);
  CHECK_EQ(vic_test::count_pixels_not(h, color::none), 17U * 17U);
  CHECK_EQ(alpha(h, 4 + 8,  4 + 8),  255U * 36U / 64U);    // corner
  CHECK_EQ(alpha(h, 4 + 24, 4 + 24), 255U * 36U / 64U);    // corner
  CHECK_EQ(alpha(h, 4 + 8,  4 + 16), 255U * 48U / 64U);    // left edge
  CHECK_EQ(alpha(h, 4 + 16, 4 + 24), 255U * 48U / 64U);    // bottom edge
  CHECK_EQ(alpha(h, 4 + 16, 4 + 16), 255U);                // inside
  CHECK(area_near(alpha_sum(h), 16.5 * 16.5));
}


TEST_CASE(fonts_vector_triangle)
{
  // right triangle, the hypotenuse is the pixel diagonal: half covered diagonal pixels
  static const std::int16_t triangle[] = { 0, 0, 3, on(0), 0, on(64), 0, on(0), 64 };
  test_font f(triangle, sizeof(triangle) / sizeof(triangle[0]));
  head_type h;
  render(h, f, 64U);
  CHECK(area_near(alpha_sum(h), 64.0 * 64.0 / 2.0));
  CHECK_EQ(alpha(h, 4 + 10, 4 + 10), 255U / 2U);          // diagonal
  CHECK_EQ(alpha(h, 4 + 5,  4 + 20), 255U);                // below the diagonal (y down)
  CHECK_EQ(alpha(h, 4 + 20, 4 + 5),  0U);                  // above
}


TEST_CASE(fonts_vector_circle)
{
  // circle of 8 quadratic arcs, radius 24 pixel at 64 pixel: the implied on-curve points
  // between consecutive off-curve points are used (a TrueType 'O' is made like this)
  static std::int16_t circle[3 + 8 * 2];
  const double r = 384.0 / std::cos(3.14159265358979 / 8.0);    // control points of a 384 unit circle
  circle[0] = 0;
  circle[1] = 0;
  circle[2] = 8;
  for (int k = 0; k < 8; ++k) {
    const double a = (k + 0.5) * 3.14159265358979 / 4.0;
    circle[3 + 2 * k] = off(static_cast<int>(std::lround(512.0 + r * std::cos(a))));
    circle[4 + 2 * k] = static_cast<std::int16_t>(std::lround(512.0 + r * std::sin(a)));
  }
  test_font f(circle, sizeof(circle) / sizeof(circle[0]), 1024U);
  head_type h;
  render(h, f, 64U);
  CHECK(std::fabs(alpha_sum(h) / 255.0 - 3.14159265358979 * 24.0 * 24.0) < 3.14159265358979 * 24.0 * 24.0 * 0.01);
  CHECK_EQ(alpha(h, 4 + 32, 4 + 32), 255U);                // center
  CHECK_EQ(alpha(h, 4 + 32, 4 + 9),  255U);                // inside, top
  CHECK_EQ(alpha(h, 4 + 32, 4 + 7),  0U);                  // outside, top
  CHECK_EQ(alpha(h, 4 + 6,  4 + 6),  0U);                  // outside, corner of the box
  // the circle is symmetric (within 1/64 coverage, the fixed point rounding)
  for (std::int16_t k = 0; k < 32; ++k) {
    CHECK(std::abs(static_cast<int>(alpha(h, 4 + k, 4 + 20)) - static_cast<int>(alpha(h, 4 + 63 - k, 4 + 43))) <= 4);
    CHECK(std::abs(static_cast<int>(alpha(h, 4 + 20, 4 + k)) - static_cast<int>(alpha(h, 4 + 43, 4 + 63 - k))) <= 4);
  }
}


TEST_CASE(fonts_vector_steep_edge)
{
  // 1 font unit = 1/64 pixel: the left edge leans 2/64 pixel over 64 rows, so the edge pieces of the rows
  // are 0 or 1/64 pixel wide, every row must have the same coverage (53/64) in column 10
  static const std::int16_t stem[] = { 0, 0, 4, on(650), 4096, on(652), 0, on(2560), 0, on(2560), 4096 };
  test_font f(stem, sizeof(stem) / sizeof(stem[0]), 4096U);
  head_type h;
  render(h, f, 64U);
  bool same = true;
  for (std::int16_t y = 0; y < 64; ++y) {
    const std::uint32_t a = alpha(h, 4 + 10, 4 + y);
    same = same && (a >= 255U * 52U / 64U) && (a <= 255U * 54U / 64U);
    same = same && (alpha(h, 4 + 11, 4 + y) == 255U) && (alpha(h, 4 + 9, 4 + y) == 0U);
    same = same && (alpha(h, 4 + 39, 4 + y) == 255U) && (alpha(h, 4 + 40, 4 + y) == 0U);
  }
  CHECK(same);
  CHECK(area_near(alpha_sum(h), (40.0 - 651.0 / 64.0) * 64.0));
}


TEST_CASE(fonts_vector_hint_stem)
{
  // a horizontal bar from 310 to 365 units (1/32 pixel per unit at 32 pixel): 9.69 .. 11.41 pixel above the baseline
  static const std::int16_t bar[] = { 0, 0, 4, on(0), 310, on(0), 365, on(512), 365, on(512), 310 };
  static const std::int16_t bar_hinted[] = { 4, (310 * 2) | 1, -32768, 365, -32768, 0, 4, on(0), 310, on(0), 365, on(512), 365, on(512), 310 };
  {
    // unhinted: the bar is spread over 3 rows with partial coverage
    test_font f(bar, sizeof(bar) / sizeof(bar[0]), 1024U);
    head_type h;
    render(h, f, 32U);
    CHECK_EQ(vic_test::count_pixels_not(h, color::none), 16U * 3U);
    CHECK_EQ(vic_test::count_pixels(h, color::white), 16U);
  }
  {
    // hinted: 1.72 pixel thick, centered: 2 solid rows (10, 11 above the baseline at 32 = rows 20, 21)
    test_font f(bar_hinted, sizeof(bar_hinted) / sizeof(bar_hinted[0]), 1024U);
    head_type h;
    render(h, f, 32U);
    CHECK_EQ(vic_test::count_pixels_not(h, color::none), 16U * 2U);
    CHECK_EQ(vic_test::count_pixels(h, color::white), 16U * 2U);
    CHECK_EQ(alpha(h, 4, 4 + 20), 255U);
    CHECK_EQ(alpha(h, 4 + 15, 4 + 21), 255U);
    // the thickness is at least one pixel: 0.43 pixel at 8 pixel is one solid row
    head_type h8;
    render(h8, f, 8U);
    CHECK_EQ(vic_test::count_pixels_not(h8, color::none), 4U);
    CHECK_EQ(vic_test::count_pixels(h8, color::white), 4U);
  }
}


TEST_CASE(fonts_vector_hint_xstem)
{
  // a vertical bar from 310 to 365 units (1/32 pixel per unit at 32 pixel): 9.69 .. 11.41 pixel right of the origin
  static const std::int16_t bar[] = { 0, 0, 4, on(310), 0, on(310), 512, on(365), 512, on(365), 0 };
  static const std::int16_t bar_hinted[] = { 0, 2, (310 * 2) | 1, 365, 4, on(310), 0, on(310), 512, on(365), 512, on(365), 0 };
  {
    // unhinted: the bar is spread over 3 columns with partial coverage
    test_font f(bar, sizeof(bar) / sizeof(bar[0]), 1024U);
    head_type h;
    render(h, f, 32U);
    CHECK_EQ(vic_test::count_pixels_not(h, color::none), 16U * 3U);
    CHECK_EQ(vic_test::count_pixels(h, color::white), 16U);
  }
  {
    // hinted: 1.72 pixel thick, centered: 2 solid columns (10, 11)
    test_font f(bar_hinted, sizeof(bar_hinted) / sizeof(bar_hinted[0]), 1024U);
    head_type h;
    render(h, f, 32U);
    CHECK_EQ(vic_test::count_pixels_not(h, color::none), 16U * 2U);
    CHECK_EQ(vic_test::count_pixels(h, color::white), 16U * 2U);
    CHECK_EQ(alpha(h, 4 + 10, 4 + 16), 255U);
    CHECK_EQ(alpha(h, 4 + 11, 4 + 31), 255U);
    // the thickness is at least one pixel: 0.43 pixel at 8 pixel is one solid column
    head_type h8;
    render(h8, f, 8U);
    CHECK_EQ(vic_test::count_pixels_not(h8, color::none), 4U);
    CHECK_EQ(vic_test::count_pixels(h8, color::white), 4U);
  }
}


TEST_CASE(fonts_vector_subpixel_cursor)
{
  // a 2 pixel stem with an advance of 10.25 pixel at 32 pixel: the cursor keeps the sub-pixels, every stem is
  // snapped to the columns nearest to its exact position 0, 10.25, 20.5, ... 71.75 (whole pixel advances: 70)
  static const std::int16_t stem[] = { 0, 2, (0 * 2) | 1, 64, 4, on(0), 0, on(0), 512, on(64), 512, on(64), 0 };
  test_font f(stem, sizeof(stem) / sizeof(stem[0]), 1024U, 328U);
  head_type h;
  dc d(h);
  tc t(d);
  t.set_font(f, 32U);
  t.set_color(color::white);
  t.set_pos({ 4, 4 });
  t.out(reinterpret_cast<const std::uint8_t*>("AAAAAAAA"));
  const int column[8] = { 0, 10, 21, 31, 41, 51, 62, 72 };
  for (int k = 0; k < 8; ++k) {
    CHECK_EQ(alpha(h, 4 + column[k] - 1, 4 + 16), 0U);      // the stem: rows 16 .. 31
    CHECK_EQ(alpha(h, 4 + column[k],     4 + 16), 255U);
    CHECK_EQ(alpha(h, 4 + column[k] + 1, 4 + 31), 255U);
    CHECK_EQ(alpha(h, 4 + column[k] + 2, 4 + 31), 0U);
  }
  CHECK_EQ(vic_test::count_pixels_not(h, color::none), 8U * 2U * 16U);
  CHECK_EQ(vic_test::count_pixels(h, color::white), 8U * 2U * 16U);
  // the extent is the exact advance, rounded
  rect_type r;
  t.get_extend(r, reinterpret_cast<const std::uint8_t*>("AAAAAAAA"));
  CHECK_EQ(r.right, 82);
}


TEST_CASE(fonts_vector_hint_rows)
{
  // the zones align the glyphs: 'x' / 'o' and 'H' / 'T' / 'E' share their top and bottom rows at every size,
  // the horizontal bars are solid rows, 'H' / 'I' / 'l' (straight stems and bars only) have no partial pixels,
  // the stem of an 'i' is solid (not its round dot)
  font::Montserrat_vec montserrat;
  bool aligned = true, crisp = true, solid = true, stems = true;
  for (std::uint8_t size = 8U; size <= 48U; ++size) {
    rect_type box[8];
    const char chars[] = "xoHTEIli";
    for (std::size_t c = 0U; c < 8U; ++c) {
      static head::memory<80, 60> h;
      dc d(h);
      d.cls();
      tc t(d);
      t.set_font(montserrat, size);
      t.set_color(color::white);
      t.set_pos({ 4, 4 });
      t.out(static_cast<std::uint16_t>(chars[c]));
      box[c] = ink_box(h);
      if ((chars[c] == 'T') || (chars[c] == 'E')) {
        // the top row of the bar is solid, partial pixels only at its ends
        std::size_t partial = 0U;
        for (std::int16_t x = box[c].left; x <= box[c].right; ++x) {
          const std::uint32_t a = alpha(h, x, box[c].top);
          partial += ((a > 0U) && (a < 255U)) ? 1U : 0U;
        }
        crisp = crisp && (partial <= 2U);
      }
      if ((chars[c] == 'H') || (chars[c] == 'I') || (chars[c] == 'l')) {
        solid = solid && (vic_test::count_pixels_not(h, color::none) == vic_test::count_pixels(h, color::white));
      }
      if (chars[c] == 'i') {
        for (int x = box[c].left; x <= box[c].right; ++x) {
          const std::uint32_t a = alpha(h, x, box[c].bottom - 1);
          solid = solid && ((a == 0U) || (a == 255U));
        }
      }
      if (chars[c] == 'H') {
        // both stems have the same width (the hidden ends of the overlapping crossbar are no stem edges)
        const int y = box[c].top + 1;
        std::size_t runs = 0U, width[2] = { 0U, 0U };
        for (int x = box[c].left; x <= box[c].right; ++x) {
          const bool ink = alpha(h, x, y) > 0U;
          runs += (ink && ((x == box[c].left) || (alpha(h, x - 1, y) == 0U))) ? 1U : 0U;
          width[runs == 2U ? 1U : 0U] += ink ? 1U : 0U;
        }
        stems = stems && (runs == 2U) && (width[0] == width[1]);
      }
    }
    aligned = aligned && (box[0].top == box[1].top) && (box[0].bottom == box[1].bottom);
    aligned = aligned && (box[2].top == box[3].top) && (box[2].top == box[4].top);
    aligned = aligned && (box[2].bottom == box[3].bottom) && (box[2].bottom == box[4].bottom);
  }
  CHECK(aligned);
  CHECK(crisp);
  CHECK(solid);
  CHECK(stems);
}


TEST_CASE(fonts_vector_contour_copies)
{
  // a negative count repeats the last full contour moved: 3 squares as one square and 2 copies, rendered like
  // the 3 explicit squares
  static const std::int16_t copies[] = { 0, 0, 4, on(4), 4, on(4), 16, on(16), 16, on(16), 4, -2, 20, 0, 40, 24 };
  static const std::int16_t squares[] = { 0, 0, 4, on(4), 4, on(4), 16, on(16), 16, on(16), 4,
                                                4, on(24), 4, on(24), 16, on(36), 16, on(36), 4,
                                                4, on(44), 28, on(44), 40, on(56), 40, on(56), 28 };
  test_font f(copies, sizeof(copies) / sizeof(copies[0]));
  test_font g(squares, sizeof(squares) / sizeof(squares[0]));
  head_type a, b;
  render(a, f, 64U);
  render(b, g, 64U);
  bool same = true;
  for (int y = 0; y < 80; ++y) {
    for (int x = 0; x < 80; ++x) {
      same = same && (alpha(a, x, y) == alpha(b, x, y));
    }
  }
  CHECK(same);
  CHECK_EQ(vic_test::count_pixels(a, color::white), 3U * 12U * 12U);
}


TEST_CASE(fonts_vector_dot_rows)
{
  // Doto (dot matrix): dots of 75 units, 100 units apart, line height 975 units. At 39 and 78 pixel the dot pitch is
  // 4 and 8 pixel: the 7 hinted dot rows of an 'I' are a regular grid of 3 (6) ink rows and 1 (2) empty rows
  font::Doto_vec doto;
  const struct { std::uint8_t size; const char* rows; } grid[] = {
    { 39U, ".###.###.###.###.###.###.###." },
    { 78U, "..######..######..######..######..######..######..######.." }
  };
  for (std::size_t k = 0U; k < 2U; ++k) {
    static head::memory<80, 120> h;
    dc d(h);
    d.cls();
    tc t(d);
    t.set_font(doto, grid[k].size);
    t.set_color(color::white);
    t.set_pos({ 4, 4 });
    t.out(static_cast<std::uint16_t>('I'));
    // the rows of the glyph: '#' ink, '.' empty
    char rows[121] = { 0 };
    std::size_t ink = 0U;
    for (int y = 0; y < 120; ++y) {
      rows[y] = '.';
      for (int x = 0; x < 80; ++x) {
        rows[y] = (alpha(h, x, y) > 0U) ? '#' : rows[y];
      }
      ink += (rows[y] == '#') ? 1U : 0U;
    }
    CHECK(std::strstr(rows, grid[k].rows) != nullptr);
    CHECK_EQ(ink, 7U * (grid[k].size / 13U));
  }
}


TEST_CASE(fonts_vector_hole_and_overlap)
{
  // counter clockwise inner contour cuts a hole (like the counter of an 'O')
  static const std::int16_t ring[] = { 0, 0, 4, on(8), 8, on(8), 56, on(56), 56, on(56), 8,
                                             4, on(24), 24, on(40), 24, on(40), 40, on(24), 40 };
  test_font f(ring, sizeof(ring) / sizeof(ring[0]));
  {
    head_type h;
    render(h, f, 64U);
    CHECK_EQ(vic_test::count_pixels(h, color::white), 48U * 48U - 16U * 16U);
    CHECK_EQ(alpha(h, 4 + 32, 4 + 32), 0U);
    CHECK_EQ(alpha(h, 4 + 10, 4 + 32), 255U);
  }
  // overlapping contours with the same direction are clamped to full coverage
  static const std::int16_t overlap[] = { 0, 0, 4, on(8), 8, on(8), 40, on(40), 40, on(40), 8,
                                             4, on(24), 24, on(24), 56, on(56), 56, on(56), 24 };
  test_font g(overlap, sizeof(overlap) / sizeof(overlap[0]));
  {
    head_type h;
    render(h, g, 64U);
    CHECK_EQ(vic_test::count_pixels(h, color::white), 2U * 32U * 32U - 16U * 16U);
    CHECK_EQ(vic_test::count_pixels_not(h, color::none), 2U * 32U * 32U - 16U * 16U);
  }
}


TEST_CASE(fonts_vector_consistency)
{
  font::Montserrat_vec montserrat;
  font::Michroma_vec   michroma;
  font::Doto_vec       doto;
  const font::font_type* fonts[] = { &montserrat, &michroma, &doto };
  for (std::size_t f = 0U; f < 3U; ++f) {
    const font::info_type& info = fonts[f]->info();
    CHECK(info.attr == (font::FORMAT_VECTOR | font::ENCODING_UNICODE | font::TYPE_PROP));
    CHECK(info.family.vector->units > 0U);
    CHECK(info.family.vector->ascent > 0);
    CHECK(info.family.vector->ascent <= static_cast<std::int16_t>(info.family.vector->units));
    std::size_t glyphs = 0U;
    std::int32_t last_code = -1;
    for (const font::vector_block_type* b = info.family.vector->block; !!b; b = b->next) {
      CHECK(b->first <= b->last);
      CHECK(static_cast<std::int32_t>(b->first) > last_code);
      last_code = b->last;
      for (std::uint32_t c = b->first; c <= b->last; ++c) {
        const font::vector_charinfo_type& g = b->char_info[c - b->first];
        CHECK(g.data != nullptr);
        CHECK(g.xdist > 0U);
        // the hints: y edges and stems (2 words per edge: position, zone), x edges and stems (1 word),
        // each with increasing position, at most 24 knots (VEC_KNOTS of txr.h) per axis
        std::uint32_t k = 0U;
        for (std::uint32_t w = 2U; (w > 0U) && (g.size > 0U); --w) {
          const std::uint32_t end = k + 1U + static_cast<std::uint32_t>(g.data[k]);
          std::int32_t last = -32768;
          std::uint32_t knots = 0U;
          bool ordered = true;
          for (++k; k < end; ) {
            const std::int32_t u = (g.data[k] - (g.data[k] & 1)) / 2;
            const bool stem = (g.data[k] & 1) != 0;
            const std::int32_t e = stem ? g.data[k + w] : u;
            ordered = ordered && (u > last) && (e >= u);
            last = e;
            knots += stem ? 2U : 1U;
            k += stem ? 2U * w : w;
          }
          CHECK(ordered);
          CHECK(knots <= 24U);
          CHECK_EQ(k, end);
        }
        // the contours fill the data exactly (a negative count -n: n moved copies of the last full contour),
        // every point is inside the line box
        bool inside = true;
        std::uint32_t full = 0U, points = 0U;   // the points of the last full contour
        while (k < g.size) {
          const std::int32_t n = g.data[k++];
          CHECK((n > 0) || (points > 0U));
          if (n > 0) {
            full   = k;
            points = static_cast<std::uint32_t>(n);
          }
          for (std::int32_t copy = (n > 0 ? -1 : n); copy < 0; ++copy) {
            const std::int32_t dy = n > 0 ? 0 : g.data[k + 1U];
            for (std::uint32_t p = 0U; p < points; ++p) {
              const std::int32_t y = g.data[full + 2U * p + 1U] + dy;
              inside = inside && (y <= info.family.vector->ascent) &&
                       (y >= info.family.vector->ascent - static_cast<std::int32_t>(info.family.vector->units));
            }
            k += n > 0 ? 2U * points : 2U;
          }
        }
        CHECK_EQ(k, static_cast<std::uint32_t>(g.size));
        CHECK(inside);
        CHECK((g.size == 0U) == ((c == 0x20U) || (c == 0xA0U)));
        ++glyphs;
      }
    }
    CHECK(glyphs >= 175U);
  }
}


TEST_CASE(fonts_vector_render_sizes)
{
  font::Montserrat_vec montserrat;
  std::int16_t a_height[65] = { 0 };
  const std::uint8_t sizes[] = { 8U, 10U, 12U, 16U, 24U, 32U, 48U, 64U };
  for (std::size_t s = 0U; s < sizeof(sizes); ++s) {
    const std::uint8_t size = sizes[s];
    static head::memory<300, 80> h;
    dc d(h);
    d.cls();
    tc t(d);
    t.set_font(montserrat, size);
    t.set_color(color::white);
    CHECK_EQ(t.char_height(), size);
    CHECK(t.char_width() > size / 3U);

    // "Agj" renders inside its extent and the line box
    rect_type r;
    CHECK_EQ(t.get_extend(r, (const std::uint8_t*)"Agj"), 3U);
    CHECK_EQ(r.bottom, static_cast<std::int16_t>(size));
    CHECK(r.right > size && r.right < 3 * size);
    t.set_pos({ 4, 4 });
    t.out((const std::uint8_t*)"Agj");
    const rect_type ink = ink_box(h);
    CHECK(ink.top >= 4 && ink.bottom < 4 + size);
    CHECK(ink.left >= 2 && ink.right < 4 + r.right + 2);
    bool partial = false, full = false;
    for (std::int16_t y = 0; y < 80; ++y) {
      for (std::int16_t x = 0; x < 300; ++x) {
        const std::uint32_t a = alpha(h, x, y);
        partial = partial || ((a > 0U) && (a < 255U));
        full    = full || (a == 255U);
      }
    }
    CHECK(partial);
    CHECK(full || (size < 16U));

    // the cap height of 'A' scales with the size
    d.cls();
    t.set_pos({ 4, 4 });
    t.out(static_cast<std::uint16_t>('A'));
    const rect_type a = ink_box(h);
    a_height[size] = static_cast<std::int16_t>(a.bottom - a.top + 1);
  }
  CHECK(a_height[8] >= 4 && a_height[8] <= 8);
  CHECK(std::abs(a_height[48] - 2 * a_height[24]) <= 2);
  CHECK(std::abs(a_height[64] - 2 * a_height[32]) <= 2);

  // about the glyph size of the 24 pixel bitmap font (the line box is the ink box of the charset in both)
  font::Montserrat_24_aa4 bitmap;
  head::memory<80, 40> h;
  dc d(h);
  tc t(d);
  t.set_font(bitmap);
  t.set_color(color::white);
  t.set_pos({ 4, 4 });
  t.out(static_cast<std::uint16_t>('A'));
  const rect_type a = ink_box(h);
  // (the hinted bitmap loses up to 2 pixel to the pixel rounding of the ink box: 'A' is 13 vs. 15 rows)
  CHECK(std::abs(a.bottom - a.top + 1 - a_height[24]) <= 2);
}


TEST_CASE(fonts_vector_default_size)
{
  font::Montserrat_vec    vec;
  font::Montserrat_24_aa4 bitmap;
  head_type h;
  dc d(h);
  tc t(d);
  // a vector font without size uses its default size, a bitmap font ignores the size
  t.set_font(vec);
  CHECK_EQ(t.char_height(), static_cast<const font::font_type&>(vec).info().ysize);
  t.set_font(vec, 40U);
  CHECK_EQ(t.char_height(), 40U);
  t.set_font(bitmap, 40U);
  CHECK_EQ(t.char_height(), 24U);
  // chars outside of the font are skipped
  t.set_font(vec, 20U);
  t.set_pos({ 4, 4 });
  t.out(static_cast<std::uint16_t>(0x4E00U));
  CHECK_EQ(vic_test::count_pixels_not(h, color::none), 0U);
}


TEST_CASE(fonts_vector_clip_width)
{
  // a glyph wider than VIC_TXR_VECTOR_MAX_WIDTH is clipped, not overflowing the row buffer
  font::Michroma_vec michroma;
  static head::memory<320, 260> h;
  dc d(h);
  tc t(d);
  t.set_font(michroma, 250U);
  t.set_color(color::white);
  t.set_pos({ 0, 0 });
  t.out(static_cast<std::uint16_t>('W'));
  const rect_type ink = ink_box(h);
  CHECK(ink.right >= 0);
  CHECK(ink.right - ink.left < VIC_TXR_VECTOR_MAX_WIDTH);
}
