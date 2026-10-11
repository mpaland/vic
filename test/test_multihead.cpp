///////////////////////////////////////////////////////////////////////////////
// \brief Tests for head/multihead.h
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "helpers.h"
#include "dc.h"
#include "tc.h"
#include "head/memory.h"
#include "head/multihead.h"

using namespace vic;
using vic_test::PAL_X;

namespace {

// alpha numeric head which records text output
class text_spy : public drv
{
public:
  std::uint16_t chars[64];
  std::size_t   count;
  vertex_type   pos;
  bool          inverse;
  std::size_t   init_count;
  std::size_t   shutdown_count;
  std::size_t   cls_count;
  std::size_t   present_count;
  std::uint8_t  brightness;

  text_spy()
    : drv(20U, 2U, 20U, 2U)
    , count(0U), pos({ 0, 0 }), inverse(false), init_count(0U), shutdown_count(0U), cls_count(0U), present_count(0U), brightness(0U)
  { }

  virtual void init() { ++init_count; }
  virtual void shutdown() { ++shutdown_count; }
  virtual const char* version() const { return "text spy"; }
  virtual bool is_graphic() const { return false; }
  virtual void display_brightness(std::uint8_t level) { brightness = level; }

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
};

} // namespace


TEST_CASE(multihead_graphic)
{
  head::memory<4, 4> h0, h1;
  head::multihead<8, 4, 2> mh = { { h0, { 0, 0 } }, { h1, { 4, 0 } } };
  CHECK_EQ(mh.head_count(), 2U);
  CHECK(mh.is_graphic());
  CHECK(mh.version() != nullptr);
  CHECK_EQ(mh.screen_width(), 8U);
  CHECK_EQ(mh.screen_height(), 4U);

  mh.init();
  CHECK_EQ(h0.cls_count(), 1U);     // memory::init() clears
  CHECK_EQ(h1.cls_count(), 1U);

  dc d(mh);
  d.set_color(color::white);
  d.plot({ 5, 1 });
  CHECK_EQ(h1.pixel({ 1, 1 }), color::white);
  CHECK_EQ(vic_test::count_pixels(h0, color::white), 0U);
  CHECK_EQ(vic_test::count_pixels(h1, color::white), 1U);
  CHECK_EQ(h0.present_count(), 1U);
  CHECK_EQ(h1.present_count(), 1U);
  // the out of range pixel is rejected by h0 only
  CHECK_EQ(h0.oob_count(), 1U);
  CHECK_EQ(h1.oob_count(), 0U);

  // pixel_get routes to the right head
  CHECK_EQ(d.pixel_get({ 5, 1 }), color::white);
  CHECK_EQ(d.pixel_get({ 1, 1 }), color::none);
  CHECK_EQ(d.pixel_get({ 9, 1 }), color::none);
  CHECK_EQ(d.pixel_get({ -1, 1 }), color::none);

  // a line across the seam
  d.line_horz({ 0, 2 }, { 7, 2 });
  CHECK_RASTER(h0, PAL_X,
    "....",
    "....",
    "XXXX",
    "....");
  CHECK_RASTER(h1, PAL_X,
    "....",
    ".X..",
    "XXXX",
    "....");

  // cls goes to all heads
  d.cls(color::blue);
  CHECK_EQ(h0.cls_color(), color::blue);
  CHECK_EQ(h1.cls_color(), color::blue);
  CHECK_EQ(vic_test::count_pixels(h1, color::blue), 16U);

  // a box over both heads with the head fast path (opaque, no shader)
  d.set_color(color::red);
  d.box({ 2, 1, 6, 3 });
  CHECK_EQ(vic_test::count_pixels(h0, color::red), 4U);
  CHECK_EQ(vic_test::count_pixels(h1, color::red), 4U);
  CHECK_EQ(h0.pixel({ 2, 1 }), color::red);
  CHECK_EQ(h1.pixel({ 1, 2 }), color::red);
  CHECK_EQ(h1.pixel({ 2, 2 }), color::blue);

  mh.display_enable(true);
  mh.display_brightness(100U);
  mh.display_contrast(100U);
  mh.shutdown();
}


TEST_CASE(multihead_vertical_offset)
{
  head::memory<4, 2> top, bottom;
  head::multihead<4, 4, 2> mh = { { top, { 0, 0 } }, { bottom, { 0, 2 } } };
  dc d(mh);
  d.set_color(color::white);
  d.line_vert({ 1, 0 }, { 1, 3 });
  CHECK_EQ(vic_test::count_pixels(top, color::white), 2U);
  CHECK_EQ(vic_test::count_pixels(bottom, color::white), 2U);
  CHECK_EQ(bottom.pixel({ 1, 0 }), color::white);
  CHECK_EQ(bottom.pixel({ 1, 1 }), color::white);
  CHECK_EQ(d.pixel_get({ 1, 3 }), color::white);
  CHECK_EQ(d.pixel_get({ 2, 3 }), color::none);
}


TEST_CASE(multihead_text)
{
  text_spy s0, s1;
  head::multihead<40, 2, 2> mh = { { s0, { 0, 0 } }, { s1, { 20, 0 } } };
  CHECK(!mh.is_graphic());
  mh.init();
  CHECK_EQ(s0.init_count, 1U);
  CHECK_EQ(s1.init_count, 1U);

  tc t(mh);
  CHECK_EQ(t.screen_width(), 40U);
  t.cls();
  CHECK_EQ(s0.cls_count, 1U);
  CHECK_EQ(s1.cls_count, 1U);

  // the position is translated for every head
  t.set_pos({ 25, 1 });
  CHECK((s0.pos == vertex_type{ 25, 1 }));
  CHECK((s1.pos == vertex_type{ 5, 1 }));

  // the string goes to all heads, they clip themselves
  CHECK_EQ(t.out(reinterpret_cast<const std::uint8_t*>("ab")), 2U);
  CHECK_EQ(s0.count, 2U);
  CHECK_EQ(s1.count, 2U);
  CHECK_EQ(s1.chars[0], 'a');
  CHECK(s0.present_count >= 1U);
  CHECK(s1.present_count >= 1U);

  t.set_inverse(true);
  CHECK(s0.inverse);
  CHECK(s1.inverse);

  mh.display_brightness(77U);
  CHECK_EQ(s0.brightness, 77U);
  CHECK_EQ(s1.brightness, 77U);

  mh.shutdown();
  CHECK_EQ(s0.shutdown_count, 1U);
}


TEST_CASE(multihead_fewer_heads)
{
  // fewer heads than Head_Count must not crash
  head::memory<4, 4> h0;
  head::multihead<8, 4, 3> mh = { { h0, { 0, 0 } } };
  CHECK_EQ(mh.head_count(), 1U);
  mh.init();
  dc d(mh);
  d.set_color(color::white);
  d.line_horz({ 0, 0 }, { 7, 0 });
  CHECK_EQ(vic_test::count_pixels(h0, color::white), 4U);
  CHECK_EQ(d.pixel_get({ 6, 0 }), color::none);
  // more heads than Head_Count: the extra one is ignored
  head::memory<4, 4> h1, h2;
  head::multihead<8, 4, 1> one = { { h1, { 0, 0 } }, { h2, { 4, 0 } } };
  CHECK_EQ(one.head_count(), 1U);
  dc d1(one);
  d1.set_color(color::white);
  d1.line_horz({ 0, 0 }, { 7, 0 });
  CHECK_EQ(vic_test::count_pixels(h1, color::white), 4U);
  CHECK_EQ(vic_test::count_pixels(h2, color::white), 0U);
}
