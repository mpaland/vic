///////////////////////////////////////////////////////////////////////////////
// \brief Tests for drv.h: the driver base class fallback functions
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "helpers.h"
#include "drv.h"
#include "head/memory.h"

using namespace vic;


namespace {

// memory head with public access to the protected drv fallback functions
template<std::uint16_t X, std::uint16_t Y>
class exposed : public head::memory<X, Y>
{
public:
  using drv::line_horz;
  using drv::line_vert;
  using drv::box;
  using drv::move;
  using drv::blit;
  void blit_fallback(const rect_type& r, const color::value_type* p, std::size_t s) { drv::blit(r, p, s); }
  using drv::text_out;
  using drv::pixel_set;
  using drv::pixel_get;
  using drv::cls;
  using drv::present;
};


// alpha numeric head which records all text_out() characters
class text_spy : public drv
{
public:
  std::uint16_t chars[64];
  std::size_t   count;
  vertex_type   pos;
  bool          inverse;
  std::size_t   present_count;

  text_spy()
    : drv(20U, 4U, 20U, 4U)
    , count(0U)
    , pos({ 0, 0 })
    , inverse(false)
    , present_count(0U)
  { }

  virtual void init() { }
  virtual void shutdown() { }
  virtual const char* version() const { return "text spy"; }
  virtual bool is_graphic() const { return false; }

  // the overrides are public here so the test can call them directly
  using drv::text_out;
  virtual void present() { ++present_count; }
  virtual void text_out(std::uint16_t ch)
  {
    if (count < sizeof(chars) / sizeof(chars[0])) {
      chars[count] = ch;
    }
    ++count;
    ++pos.x;
  }
  virtual void text_set_pos(vertex_type p) { pos = p; }
  virtual void text_set_inverse(bool inv) { inverse = inv; }
};

} // namespace


TEST_CASE(drv_line_horz_fallback)
{
  exposed<8, 4> h;
  h.line_horz({ 2, 1 }, { 5, 1 }, color::white);
  CHECK_RASTER(h, vic_test::PAL_X,
    "........",
    "..XXXX..",
    "........",
    "........");

  // reversed end points give the same line, the y of v1 is ignored
  exposed<8, 4> r;
  r.line_horz({ 5, 1 }, { 2, 3 }, color::white);
  CHECK_SAME_BUFFER(h, r);

  // single pixel
  exposed<8, 4> s;
  s.line_horz({ 3, 2 }, { 3, 2 }, color::white);
  CHECK_RASTER(s, vic_test::PAL_X,
    "........",
    "........",
    "...X....",
    "........");

  // clipping, nothing outside the screen is touched
  exposed<8, 4> c;
  c.line_horz({ -3, 0 }, { 10, 0 }, color::white);
  CHECK_RASTER(c, vic_test::PAL_X,
    "XXXXXXXX",
    "........",
    "........",
    "........");
  CHECK_EQ(c.oob_count(), 6U);
}


TEST_CASE(drv_line_vert_fallback)
{
  exposed<4, 8> h;
  h.line_vert({ 1, 2 }, { 1, 5 }, color::white);
  CHECK_RASTER(h, vic_test::PAL_X,
    "....",
    "....",
    ".X..",
    ".X..",
    ".X..",
    ".X..",
    "....",
    "....");

  exposed<4, 8> r;
  r.line_vert({ 1, 5 }, { 3, 2 }, color::white);
  CHECK_SAME_BUFFER(h, r);
}


TEST_CASE(drv_box_fallback)
{
  // right and bottom are exclusive
  exposed<8, 6> h;
  h.box({ 2, 1, 5, 4 }, color::white);
  CHECK_RASTER(h, vic_test::PAL_X,
    "........",
    "..XXX...",
    "..XXX...",
    "..XXX...",
    "........",
    "........");
  CHECK_EQ(h.pixel_set_count(), 9U);

  // empty rects draw nothing
  exposed<8, 6> e;
  e.box({ 3, 3, 3, 5 }, color::white);
  e.box({ 3, 3, 5, 3 }, color::white);
  e.box({ 5, 5, 3, 3 }, color::white);   // not normalized
  CHECK_EQ(e.pixel_set_count(), 0U);
  CHECK_EQ(e.oob_count(), 0U);

  // full screen
  exposed<8, 6> f;
  f.box({ 0, 0, 8, 6 }, color::white);
  CHECK_EQ(vic_test::count_pixels(f, color::white), 48U);
  CHECK_EQ(f.oob_count(), 0U);

  // partially outside
  exposed<8, 6> p;
  p.box({ -2, -2, 2, 2 }, color::white);
  CHECK_RASTER(p, vic_test::PAL_X,
    "XX......",
    "XX......",
    "........",
    "........",
    "........",
    "........");
  exposed<8, 6> q;
  q.box({ 6, 4, 10, 10 }, color::white);
  CHECK_RASTER(q, vic_test::PAL_X,
    "........",
    "........",
    "........",
    "........",
    "......XX",
    "......XX");
}


TEST_CASE(drv_move_fallback)
{
  // the moved area is copied to the destination, all four overlap directions
  const char* const pattern[] = {
    "ABCD....",
    "EFGH....",
    "IJKL....",
    "........",
    "........",
    "........"
  };
  const vic_test::palette pal = { { { 'A', 0xFF000001UL }, { 'B', 0xFF000002UL }, { 'C', 0xFF000003UL }, { 'D', 0xFF000004UL },
                                    { 'E', 0xFF000005UL }, { 'F', 0xFF000006UL }, { 'G', 0xFF000007UL }, { 'H', 0xFF000008UL } }, 8U, color::none };
  const vic_test::palette pal2 = { { { 'I', 0xFF000009UL }, { 'J', 0xFF00000AUL }, { 'K', 0xFF00000BUL }, { 'L', 0xFF00000CUL } }, 4U, color::none };

  struct setup {
    static void fill(exposed<8, 6>& h, const char* const* rows, const vic_test::palette& p1, const vic_test::palette& p2)
    {
      for (std::int16_t y = 0; y < 6; ++y) {
        for (std::int16_t x = 0; x < 8; ++x) {
          color::value_type c = color::none;
          if (!p1.lookup(rows[y][x], c) || (c == color::none)) {
            p2.lookup(rows[y][x], c);
          }
          if (c != color::none) {
            h.pixel_set({ x, y }, c);
          }
        }
      }
    }
  };

  // move right/down (overlapping)
  {
    exposed<8, 6> h;
    setup::fill(h, pattern, pal, pal2);
    h.move({ 0, 0, 4, 3 }, { 1, 1 });
    CHECK_EQ(h.pixel({ 1, 1 }), 0xFF000001UL);   // A
    CHECK_EQ(h.pixel({ 4, 1 }), 0xFF000004UL);   // D
    CHECK_EQ(h.pixel({ 1, 3 }), 0xFF000009UL);   // I
    CHECK_EQ(h.pixel({ 4, 3 }), 0xFF00000CUL);   // L
    CHECK_EQ(h.pixel({ 2, 2 }), 0xFF000006UL);   // F
    CHECK_EQ(h.pixel({ 0, 0 }), 0xFF000001UL);   // untouched source corner
  }
  // move left/up (overlapping)
  {
    exposed<8, 6> h;
    setup::fill(h, pattern, pal, pal2);
    h.move({ 1, 1, 4, 3 }, { 0, 0 });
    CHECK_EQ(h.pixel({ 0, 0 }), 0xFF000006UL);   // F
    CHECK_EQ(h.pixel({ 2, 0 }), 0xFF000008UL);   // H
    CHECK_EQ(h.pixel({ 0, 1 }), 0xFF00000AUL);   // J
    CHECK_EQ(h.pixel({ 2, 1 }), 0xFF00000CUL);   // L
  }
  // move right/up
  {
    exposed<8, 6> h;
    setup::fill(h, pattern, pal, pal2);
    h.move({ 0, 1, 4, 3 }, { 1, 0 });
    CHECK_EQ(h.pixel({ 1, 0 }), 0xFF000005UL);   // E
    CHECK_EQ(h.pixel({ 4, 0 }), 0xFF000008UL);   // H
    CHECK_EQ(h.pixel({ 1, 1 }), 0xFF000009UL);   // I
    CHECK_EQ(h.pixel({ 4, 1 }), 0xFF00000CUL);   // L
  }
  // move left/down
  {
    exposed<8, 6> h;
    setup::fill(h, pattern, pal, pal2);
    h.move({ 1, 0, 4, 2 }, { 0, 1 });
    CHECK_EQ(h.pixel({ 0, 1 }), 0xFF000002UL);   // B
    CHECK_EQ(h.pixel({ 2, 1 }), 0xFF000004UL);   // D
    CHECK_EQ(h.pixel({ 0, 2 }), 0xFF000006UL);   // F
    CHECK_EQ(h.pixel({ 2, 2 }), 0xFF000008UL);   // H
  }
  // non overlapping move keeps the source
  {
    exposed<8, 6> h;
    setup::fill(h, pattern, pal, pal2);
    h.move({ 0, 0, 2, 2 }, { 5, 3 });
    CHECK_EQ(h.pixel({ 5, 3 }), 0xFF000001UL);
    CHECK_EQ(h.pixel({ 6, 3 }), 0xFF000002UL);
    CHECK_EQ(h.pixel({ 5, 4 }), 0xFF000005UL);
    CHECK_EQ(h.pixel({ 6, 4 }), 0xFF000006UL);
    CHECK_EQ(h.pixel({ 0, 0 }), 0xFF000001UL);
    CHECK_EQ(h.pixel({ 7, 5 }), color::none);
  }
}


TEST_CASE(drv_defaults)
{
  exposed<4, 4> h;
  CHECK_EQ(h.pixel_get({ 0, 0 }), color::none);
  CHECK_EQ(h.framebuffer_get_count(), 1U);
  CHECK(!h.framebuffer_set_display(0U));
  CHECK(!h.framebuffer_set_access(0U));
  // display control defaults are no-ops
  h.display_enable(true);
  h.display_enable(false);
  h.display_brightness(100U);
  h.display_contrast(100U);
  CHECK_EQ(h.pixel_set_count(), 0U);
  CHECK_EQ(h.oob_count(), 0U);
}


TEST_CASE(drv_text_out_utf8)
{
  text_spy t;

  // ASCII
  CHECK_EQ(t.text_out(reinterpret_cast<const std::uint8_t*>("Hi!")), 3U);
  REQUIRE(t.count == 3U);
  CHECK_EQ(t.chars[0], 'H');
  CHECK_EQ(t.chars[1], 'i');
  CHECK_EQ(t.chars[2], '!');
  CHECK_EQ(t.present_count, 1U);

  // 2 and 3 byte sequences: ä (U+00E4), € (U+20AC)
  t.count = 0U;
  CHECK_EQ(t.text_out(reinterpret_cast<const std::uint8_t*>("a\xC3\xA4\xE2\x82\xACz")), 4U);
  REQUIRE(t.count == 4U);
  CHECK_EQ(t.chars[0], 'a');
  CHECK_EQ(t.chars[1], 0x00E4U);
  CHECK_EQ(t.chars[2], 0x20ACU);
  CHECK_EQ(t.chars[3], 'z');

  // CR and LF are counted but not output
  t.count = 0U;
  CHECK_EQ(t.text_out(reinterpret_cast<const std::uint8_t*>("a\nb\rc")), 5U);
  REQUIRE(t.count == 3U);
  CHECK_EQ(t.chars[1], 'b');
  CHECK_EQ(t.chars[2], 'c');

  // unicode chars whose low byte is a control code must NOT be treated as control code
  // U+010A, U+010D, U+0107
  t.count = 0U;
  t.text_out(reinterpret_cast<const std::uint8_t*>("\xC4\x8A\xC4\x8D\xC4\x87"));
  REQUIRE(t.count == 3U);
  CHECK_EQ(t.chars[0], 0x010AU);
  CHECK_EQ(t.chars[1], 0x010DU);
  CHECK_EQ(t.chars[2], 0x0107U);
}


TEST_CASE(drv_text_out_utf8_truncated)
{
  // the bytes after the terminating NUL look like valid text, a decoder which
  // reads past the NUL would output them
  static const std::uint8_t trunc2[] = { 'A', 0xC3U, 0x00U, 'B', 'C', 0x00U };
  static const std::uint8_t trunc3a[] = { 'A', 0xE2U, 0x00U, 'B', 'C', 0x00U };
  static const std::uint8_t trunc3b[] = { 'A', 0xE2U, 0x82U, 0x00U, 'B', 'C', 0x00U };

  text_spy t;
  t.text_out(trunc2);
  CHECK_EQ(t.count, 1U);
  CHECK_EQ(t.chars[0], 'A');

  t.count = 0U;
  t.text_out(trunc3a);
  CHECK_EQ(t.count, 1U);

  t.count = 0U;
  t.text_out(trunc3b);
  CHECK_EQ(t.count, 1U);

  // invalid lead bytes (stray continuation, 4 byte sequence) are skipped
  t.count = 0U;
  t.text_out(reinterpret_cast<const std::uint8_t*>("\x80\xBF" "A" "\xF0\x9F\x98\x80" "B"));
  REQUIRE(t.count == 2U);
  CHECK_EQ(t.chars[0], 'A');
  CHECK_EQ(t.chars[1], 'B');

  // empty string
  t.count = 0U;
  CHECK_EQ(t.text_out(reinterpret_cast<const std::uint8_t*>("")), 0U);
  CHECK_EQ(t.count, 0U);
}


TEST_CASE(drv_text_pos_inverse)
{
  text_spy t;
  t.text_set_pos({ 3, 1 });
  CHECK((t.pos == vertex_type{ 3, 1 }));
  t.text_out(static_cast<std::uint16_t>('x'));
  CHECK((t.pos == vertex_type{ 4, 1 }));
  t.text_set_inverse(true);
  CHECK(t.inverse);
}


TEST_CASE(drv_blit_fallback)
{
  exposed<8, 6> h;
  h.cls(color::black);
  static const color::value_type block[8] = { color::red,  color::green, color::blue,  color::white,
                                              color::white, color::red,  color::green, color::blue };
  // fully inside, stride 4 (the memory head overrides blit, so call the drv fallback explicitly)
  h.blit_fallback({ 1, 1, 5, 3 }, block, 4U);
  CHECK_EQ(h.pixel({ 1, 1 }), color::red);
  CHECK_EQ(h.pixel({ 4, 1 }), color::white);
  CHECK_EQ(h.pixel({ 1, 2 }), color::white);
  CHECK_EQ(h.pixel({ 4, 2 }), color::blue);
  CHECK_EQ(h.pixel_set_count(), 8U);
  CHECK_EQ(vic_test::count_pixels_not(h, color::black), 8U);

  // clipped at the top/left: the skipped pixels of the block are not written
  h.cls(color::black);
  h.reset_counters();
  h.blit_fallback({ -2, -1, 2, 1 }, block, 4U);
  CHECK_EQ(h.pixel({ 0, 0 }), color::green);       // block (2,1)
  CHECK_EQ(h.pixel({ 1, 0 }), color::blue);
  CHECK_EQ(h.pixel_set_count(), 2U);
  CHECK_EQ(h.oob_count(), 0U);

  // clipped at the bottom/right, stride larger than the block width
  h.cls(color::black);
  h.reset_counters();
  h.blit_fallback({ 6, 5, 10, 7 }, block, 4U);
  CHECK_EQ(h.pixel({ 6, 5 }), color::red);
  CHECK_EQ(h.pixel({ 7, 5 }), color::green);
  CHECK_EQ(h.pixel_set_count(), 2U);
  CHECK_EQ(h.oob_count(), 0U);

  // completely outside or empty: nothing happens
  h.blit_fallback({ 20, 20, 24, 22 }, block, 4U);
  h.blit_fallback({ 3, 3, 3, 5 }, block, 4U);
  CHECK_EQ(h.pixel_set_count(), 2U);
}
