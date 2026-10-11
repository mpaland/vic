///////////////////////////////////////////////////////////////////////////////
// \brief Tests for txr.h (text renderer) and tc.h (text context)
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "helpers.h"
#include "dc.h"
#include "tc.h"
#include "head/memory.h"
#include "font/LCD_6x8.h"
#include "font/LCD_8x8.h"
#include "font/Arial_num_16x24_AA4.h"
#include "font/Arial_std_12_aa2.h"

using namespace vic;
using vic_test::PAL_X;

namespace {

inline const std::uint8_t* u8(const char* s)
{
  return reinterpret_cast<const std::uint8_t*>(s);
}

// alpha numeric head which records text output
class text_spy : public drv
{
public:
  std::uint16_t chars[64];
  std::size_t   count;
  vertex_type   pos;
  bool          inverse;
  std::size_t   present_count;
  std::size_t   cls_count;
  std::size_t   clear_eol_count, clear_sol_count, clear_line_count;

  text_spy()
    : drv(20U, 4U, 20U, 4U)
    , count(0U), pos({ 0, 0 }), inverse(false), present_count(0U), cls_count(0U)
    , clear_eol_count(0U), clear_sol_count(0U), clear_line_count(0U)
  { }

  virtual void init() { }
  virtual void shutdown() { }
  virtual const char* version() const { return "text spy"; }
  virtual bool is_graphic() const { return false; }

protected:
  virtual void cls(color::value_type) { ++cls_count; }
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
  virtual void text_clear_eol()  { ++clear_eol_count; }
  virtual void text_clear_sol()  { ++clear_sol_count; }
  virtual void text_clear_line() { ++clear_line_count; }
};

} // namespace


TEST_CASE(txr_mono_font)
{
  head::memory<16, 10> h;
  dc d(h);
  tc t(d);
  font::LCD_6x8 lcd;
  t.set_font(lcd);
  t.set_color(color::white);
  CHECK_EQ(t.char_width(), 6U);
  CHECK_EQ(t.char_height(), 8U);

  t.set_pos({ 0, 0 });
  t.out(static_cast<std::uint16_t>('A'));
  CHECK_RASTER(h, PAL_X,
    "..XXX...........",
    ".X...X..........",
    ".X...X..........",
    ".X...X..........",
    ".XXXXX..........",
    ".X...X..........",
    ".X...X..........",
    "................",
    "................",
    "................");
  // every pixel of a 1 bpp font is exactly the pen color
  CHECK_EQ(vic_test::count_pixels_not(h, color::none), vic_test::count_pixels(h, color::white));

  // the cursor advances by the char width
  t.out(static_cast<std::uint16_t>('A'));
  CHECK_EQ(h.pixel({ 8, 0 }), color::white);    // second A starts at x = 6
  CHECK_EQ(h.pixel({ 7, 1 }), color::white);

  // string extent
  rect_type r;
  CHECK_EQ(t.get_extend(r, u8("AB")), 2U);
  CHECK_EQ(r.left, 0);
  CHECK_EQ(r.top, 0);
  CHECK_EQ(r.right, 12);
  CHECK_EQ(r.bottom, 8);

  // control chars are ignored
  d.cls();
  t.set_pos({ 0, 0 });
  t.out(static_cast<std::uint16_t>(0x01U));
  CHECK_EQ(vic_test::count_pixels(h, color::white), 0U);
}


TEST_CASE(txr_prop_aa4_font)
{
  head::memory<16, 26> h;
  dc d(h);
  tc t(d);
  font::Arial_num_16x24_aa4 arial;
  t.set_font(arial);
  CHECK_EQ(t.char_height(), 24U);
  // 'A' is not part of this font, the width of the first available char is taken
  CHECK_EQ(t.char_width(), 8U);

  // '!' is 8 pixel wide, row 2 is 06 FF FF 60 (4 bit per pixel)
  t.set_color(color::white);
  t.set_pos({ 0, 0 });
  t.out(static_cast<std::uint16_t>('!'));
  CHECK_PIXELS(h,
    { { 0, 2 }, color::none },
    { { 1, 2 }, 0x6FFFFFFFUL },
    { { 2, 2 }, color::white },
    { { 3, 2 }, color::white },
    { { 4, 2 }, color::white },
    { { 5, 2 }, color::white },
    { { 6, 2 }, 0x6FFFFFFFUL },
    { { 7, 2 }, color::none },
    { { 3, 0 }, color::none },
    { { 3, 1 }, color::none });

  // the pen alpha is multiplied with the pixel intensity
  d.cls();
  t.set_color(0x80FF0000UL);
  t.set_pos({ 0, 0 });
  t.out(static_cast<std::uint16_t>('!'));
  CHECK_EQ(h.pixel({ 1, 2 }), 0x37FF0000UL);   // 128 * 111 / 255 = 55
  CHECK_EQ(h.pixel({ 2, 2 }), 0x80FF0000UL);

  // the cursor advances by xdist
  d.cls();
  t.set_color(color::white);
  t.set_pos({ 0, 0 });
  t.out(u8("!!"));
  CHECK_EQ(h.pixel({ 10, 2 }), color::white);   // second '!' at x = 8 + 2

  // extent of a string in a chained prop font
  rect_type r;
  CHECK_EQ(t.get_extend(r, u8("1:2")), 3U);
  CHECK_EQ(r.right, 16 + 8 + 16);
  CHECK_EQ(r.bottom, 24);

  // chars not in the font are skipped but counted
  d.cls();
  t.set_pos({ 0, 0 });
  CHECK_EQ(t.out(u8("A")), 1U);
  CHECK_EQ(vic_test::count_pixels_not(h, color::none), 0U);
}


TEST_CASE(txr_unicode_aa2_font)
{
  head::memory<16, 14> h;
  dc d(h);
  tc t(d);
  font::Arial_std_12_aa2 arial;
  t.set_font(arial);
  CHECK_EQ(t.char_height(), 12U);
  CHECK(t.char_width() > 0U && t.char_width() <= 12U);

  // '"' is 3 x 2 pixel at offset (0, 2), rows 0xCC and 0xC8 (2 bit per pixel)
  t.set_color(color::white);
  t.set_pos({ 0, 0 });
  t.out(static_cast<std::uint16_t>('"'));
  CHECK_PIXELS(h,
    { { 0, 2 }, color::white },
    { { 1, 2 }, color::none },
    { { 2, 2 }, color::white },
    { { 0, 3 }, color::white },
    { { 1, 3 }, color::none },
    { { 2, 3 }, 0xBFFFFFFFUL },
    { { 0, 1 }, color::none },
    { { 0, 4 }, color::none });
  CHECK_EQ(vic_test::count_pixels_not(h, color::none), 4U);

  // the full intensity pixels of an AA font are opaque (not inverted)
  d.cls();
  t.set_pos({ 0, 0 });
  t.out(static_cast<std::uint16_t>('!'));
  CHECK(vic_test::count_pixels(h, color::white) >= 5U);

  // a char from the second unicode block renders something
  d.cls();
  t.set_pos({ 0, 0 });
  CHECK_EQ(t.out(u8("\xC3\xA4")), 1U);    // ä
  CHECK(vic_test::count_pixels_not(h, color::none) > 0U);

  // extent
  rect_type r;
  CHECK_EQ(t.get_extend(r, u8("\"\"")), 2U);
  CHECK_EQ(r.right, 6);
  CHECK_EQ(r.bottom, 12);
}


TEST_CASE(txr_string_control)
{
  head::memory<40, 20> h;
  dc d(h);
  tc t(d);
  font::LCD_6x8 lcd;
  t.set_font(lcd);
  t.set_color(color::white);

  // line feed goes to the next line at the set x position, carriage return to the set x
  t.set_pos({ 6, 0 });
  CHECK_EQ(t.out(u8("A\nA\rA")), 5U);
  CHECK_EQ(h.pixel({ 8, 0 }), color::white);    // first A at (6, 0)
  CHECK_EQ(h.pixel({ 8, 8 }), color::white);    // second A at (6, 8)
  CHECK_EQ(h.pixel({ 14, 8 }), color::none);    // the third A overwrites the second one, nothing at x = 12
  CHECK_EQ(h.pixel({ 2, 8 }), color::none);

  // color escape \aRRGGBB
  d.cls();
  t.set_pos({ 0, 0 });
  CHECK_EQ(t.out(u8("\aFF8000A")), 2U);
  CHECK_EQ(h.pixel({ 2, 0 }), 0xFFFF8000UL);
  CHECK_EQ(vic_test::count_pixels(h, 0xFFFF8000UL), 18U);   // 'A' has 18 pixels

  // lower case hex
  d.cls();
  t.set_pos({ 0, 0 });
  t.out(u8("\a00ff00A"));
  CHECK_EQ(h.pixel({ 2, 0 }), color::green);

  // incomplete escape at the end of the string: color unchanged, no crash
  d.cls();
  t.set_color(color::white);
  t.set_pos({ 0, 0 });
  t.out(u8("\aFF"));
  t.out(u8("\aFF80"));
  t.out(u8("\a"));
  t.out(u8("A"));
  CHECK_EQ(h.pixel({ 2, 0 }), color::white);
  // invalid hex digits are not consumed
  d.cls();
  t.set_pos({ 0, 0 });
  CHECK_EQ(t.out(u8("\aZZZZZZA")), 8U);
  CHECK_EQ(h.pixel({ 2, 0 }), color::white);
}


TEST_CASE(txr_no_font)
{
  // without a font nothing is rendered and nothing crashes
  head::memory<16, 10> h;
  dc d(h);
  tc t(d);
  t.set_color(color::white);
  t.set_pos({ 0, 0 });
  t.out(static_cast<std::uint16_t>('A'));
  CHECK_EQ(t.out(u8("AB\n")), 3U);
  CHECK_EQ(vic_test::count_pixels_not(h, color::none), 0U);
  CHECK_EQ(t.char_width(), 0U);
  CHECK_EQ(t.char_height(), 0U);
  rect_type r = { 1, 2, 3, 4 };
  CHECK_EQ(t.get_extend(r, u8("AB")), 0U);
  CHECK(r.empty());
}


TEST_CASE(tc_graphic_context)
{
  head::memory<32, 16> h;
  dc d(h);
  tc t(d);
  font::LCD_8x8 lcd;
  t.set_font(lcd);
  t.set_color(color::white);

  CHECK_EQ(t.screen_width(), 32U);
  CHECK_EQ(t.screen_height(), 16U);

  t.cls(color::blue);
  CHECK_EQ(h.cls_count(), 1U);
  CHECK_EQ(h.cls_color(), color::blue);

  // out(string) presents once
  h.reset_counters();
  CHECK_EQ(t.out({ 0, 0 }, u8("Hi")), 2U);
  CHECK_EQ(h.present_count(), 1U);
  CHECK(vic_test::count_pixels(h, color::white) > 0U);

  // present lock
  h.reset_counters();
  t.present_lock();
  t.out(u8("A"));
  t.out(u8("B"));
  CHECK_EQ(h.present_count(), 0U);
  t.present_lock(false);
  CHECK_EQ(h.present_count(), 1U);
  t.present();
  CHECK_EQ(h.present_count(), 2U);

  // alpha head only functions are no-ops on a graphic context
  t.set_inverse(true);
  t.clear_eol();
  t.clear_sol();
  t.clear_line();

  // extent via the text renderer
  rect_type r;
  CHECK_EQ(t.get_extend(r, u8("ABC")), 3U);
  CHECK_EQ(r.right, 24);
  CHECK_EQ(r.bottom, 8);
}


TEST_CASE(tc_alpha_context)
{
  text_spy spy;
  tc t(spy);

  CHECK_EQ(t.screen_width(), 20U);
  CHECK_EQ(t.screen_height(), 4U);

  t.cls();
  CHECK_EQ(spy.cls_count, 1U);

  t.set_pos({ 3, 1 });
  CHECK((spy.pos == vertex_type{ 3, 1 }));
  t.out(static_cast<std::uint16_t>('x'));
  CHECK_EQ(spy.count, 1U);
  CHECK_EQ(spy.chars[0], 'x');

  // string output goes to the head as UTF-8 decoded chars, one present
  spy.count = 0U;
  spy.present_count = 0U;
  CHECK_EQ(t.out(u8("a\xC3\xA4")), 2U);
  CHECK_EQ(spy.count, 2U);
  CHECK_EQ(spy.chars[1], 0x00E4U);
  CHECK(spy.present_count >= 1U);

  // positioned output
  spy.count = 0U;
  CHECK_EQ(t.out({ 0, 2 }, u8("ab")), 2U);
  CHECK_EQ(spy.count, 2U);

  // alpha head functions
  t.set_inverse(true);
  CHECK(spy.inverse);
  t.clear_eol();
  t.clear_sol();
  t.clear_line();
  CHECK_EQ(spy.clear_eol_count, 1U);
  CHECK_EQ(spy.clear_sol_count, 1U);
  CHECK_EQ(spy.clear_line_count, 1U);

  // extent on an alpha head is the char count
  rect_type r;
  CHECK_EQ(t.get_extend(r, u8("abc\xC3\xA4")), 4U);
  CHECK_EQ(r.right, 4);
  CHECK_EQ(r.bottom, 1);
  CHECK_EQ(r.left, 0);
}


TEST_CASE(txr_extend_ignores_color_escape)
{
  // the inline color escape \aRRGGBB and line control chars have no extent, so a colored
  // string measures exactly like the plain one (word wrapping relies on it)
  head::memory<64, 16> h;
  dc d(h);
  tc t(d);
  font::LCD_6x8 lcd;
  t.set_font(lcd);
  rect_type plain, colored;
  CHECK_EQ(t.get_extend(plain, u8("red green")), 9U);
  CHECK_EQ(t.get_extend(colored, u8("\aFF0000red \a00FF00green\r\n")), 9U);
  CHECK_EQ(colored.right, plain.right);
  CHECK_EQ(colored.bottom, plain.bottom);
  CHECK_EQ(plain.right, 9 * 6);
  // a truncated escape at the end of the string is harmless
  rect_type r;
  CHECK_EQ(t.get_extend(r, u8("ab\aFF")), 2U);
  CHECK_EQ(r.right, 2 * 6);
}
