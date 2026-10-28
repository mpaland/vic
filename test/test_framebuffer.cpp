///////////////////////////////////////////////////////////////////////////////
// \brief Tests for head/framebuffer.h: storage formats, dirty rect transfer via
// blit(), planes and the use in front of a head which can't read its pixels
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "helpers.h"
#include "dc.h"
#include "sprite.h"
#include "head/memory.h"
#include "head/framebuffer.h"

using namespace vic;

namespace {

typedef head::memory<16, 12> head_type;

// a memory head which can't read its pixels back (like a write only serial display)
class blind_head : public head_type
{
protected:
  virtual color::value_type pixel_get(vertex_type vertex)
  {
    (void)vertex;
    return color::none;
  }
};

// a memory head which records the display control calls
class control_head : public head_type
{
public:
  control_head() : enable_(false), brightness_(0U), contrast_(0U) { }
  virtual void display_enable(bool enable = true)     { enable_ = enable; }
  virtual void display_brightness(std::uint8_t level) { brightness_ = level; }
  virtual void display_contrast(std::uint8_t level)   { contrast_ = level; }
  bool         enable_;
  std::uint8_t brightness_;
  std::uint8_t contrast_;
};

} // namespace


TEST_CASE(framebuffer_formats)
{
  {
    head_type m;
    head::framebuffer<16, 12, 1, color::format_ARGB8888> fb(m);
    CHECK_EQ(fb.plane_size(), 16U * 12U * 4U);
    dc d(fb);
    d.plot({ 0, 0 }, color::red);
    d.plot({ 1, 0 }, 0xFF123456UL);
    CHECK_EQ(d.pixel_get({ 0, 0 }), color::red);
    CHECK_EQ(d.pixel_get({ 1, 0 }), 0xFF123456UL);
    CHECK_EQ(m.pixel({ 1, 0 }), 0xFF123456UL);
  }
  {
    head_type m;
    head::framebuffer<16, 12, 1, color::format_RGB565> fb(m);
    CHECK_EQ(fb.plane_size(), 16U * 12U * 2U);
    dc d(fb);
    d.plot({ 0, 0 }, color::red);
    d.plot({ 1, 0 }, color::green);
    d.plot({ 2, 0 }, color::blue);
    d.plot({ 3, 0 }, color::white);
    CHECK_EQ(d.pixel_get({ 0, 0 }), 0xFFF80000UL);
    CHECK_EQ(d.pixel_get({ 1, 0 }), 0xFF00FC00UL);
    CHECK_EQ(d.pixel_get({ 2, 0 }), 0xFF0000F8UL);
    CHECK_EQ(d.pixel_get({ 3, 0 }), 0xFFF8FCF8UL);
    CHECK_EQ(m.pixel({ 0, 0 }), 0xFFF80000UL);
    CHECK_EQ(m.pixel({ 3, 0 }), 0xFFF8FCF8UL);
  }
  {
    head_type m;
    head::framebuffer<16, 12, 1, color::format_RGB332> fb(m);
    CHECK_EQ(fb.plane_size(), 16U * 12U);
    dc d(fb);
    d.plot({ 0, 0 }, color::red);
    d.plot({ 1, 0 }, color::blue);
    CHECK_EQ(d.pixel_get({ 0, 0 }), 0xFFE00000UL);
    CHECK_EQ(d.pixel_get({ 1, 0 }), 0xFF0000C0UL);
    CHECK_EQ(m.pixel({ 1, 0 }), 0xFF0000C0UL);
  }
  {
    head_type m;
    head::framebuffer<16, 12, 1, color::format_L1> fb(m);
    CHECK_EQ(fb.plane_size(), 16U * 12U / 8U);
    dc d(fb);
    d.cls(color::black);
    d.plot({ 0, 0 }, color::white);
    d.plot({ 15, 11 }, color::white);
    d.plot({ 7, 3 }, color::white);
    d.plot({ 7, 3 }, color::black);
    CHECK_EQ(d.pixel_get({ 0, 0 }), color::white);
    CHECK_EQ(d.pixel_get({ 15, 11 }), color::white);
    CHECK_EQ(d.pixel_get({ 1, 0 }), color::black);
    CHECK_EQ(d.pixel_get({ 7, 3 }), color::black);
    CHECK_EQ(m.pixel({ 15, 11 }), color::white);
    CHECK_EQ(vic_test::count_pixels(m, color::white), 2U);
  }
}


TEST_CASE(framebuffer_dirty_present)
{
  head_type m;
  head::framebuffer<16, 12> fb(m);
  dc d(fb);
  d.cls(color::blue);
  CHECK_EQ(m.cls_count(), 1U);
  CHECK(fb.dirty().empty());
  m.reset_counters();

  // a box is one native box on the buffer, present() transfers only the box rows
  d.present_lock();
  d.set_color(color::white);
  d.box({ 2, 3, 12, 7 });
  CHECK(!fb.dirty().empty());
  CHECK_EQ(fb.dirty().left, 2);
  CHECK_EQ(fb.dirty().right, 12);
  CHECK_EQ(m.blit_count(), 0U);                    // nothing transferred yet
  d.present_lock(false);
  CHECK(fb.dirty().empty());
  CHECK_EQ(m.blit_count(), 4U);                    // one row per blit
  CHECK_EQ(m.blit_pixels(), 40U);
  CHECK_EQ(m.pixel_set_count(), 0U);               // no single pixel writes
  CHECK_EQ(m.present_count(), 1U);
  CHECK_EQ(vic_test::count_pixels(m, color::white), 40U);
  CHECK_EQ(vic_test::count_pixels(m, color::blue), 16U * 12U - 40U);

  // a present without changes transfers nothing
  d.present();
  CHECK_EQ(m.blit_count(), 4U);
  CHECK_EQ(m.present_count(), 2U);

  // two distant pixels: the dirty rect spans both, so the transfer covers the bounding rows
  m.reset_counters();
  d.present_lock();
  d.plot({ 0, 0 });
  d.plot({ 15, 1 });
  d.present_lock(false);
  CHECK_EQ(m.blit_count(), 2U);
  CHECK_EQ(m.blit_pixels(), 32U);
  CHECK_EQ(m.pixel({ 0, 0 }), color::white);
  CHECK_EQ(m.pixel({ 15, 1 }), color::white);
  CHECK_EQ(m.pixel({ 8, 0 }), color::blue);        // untouched pixels keep their content
}


TEST_CASE(framebuffer_planes)
{
  head_type m;
  head::framebuffer<16, 12, 2> fb(m);
  fb.init();
  CHECK_EQ(fb.framebuffer_get_count(), 2U);
  dc d(fb);
  d.cls(color::blue);
  d.plot({ 1, 1 }, color::red);
  CHECK_EQ(m.pixel({ 1, 1 }), color::red);       // plane 0 is displayed

  // draw on plane 1: nothing reaches the head
  CHECK(fb.framebuffer_set_access(1U));
  CHECK_EQ(fb.framebuffer_get_access(), 1U);
  m.reset_counters();
  d.cls(color::black);
  d.plot({ 2, 2 }, color::green);
  CHECK_EQ(m.blit_count(), 0U);
  CHECK_EQ(m.cls_count(), 0U);
  CHECK_EQ(m.pixel({ 2, 2 }), color::blue);
  CHECK_EQ(d.pixel_get({ 2, 2 }), color::green);

  // switch the display: the whole plane 1 is transferred once
  CHECK(fb.framebuffer_set_display(1U));
  CHECK_EQ(fb.framebuffer_get_display(), 1U);
  CHECK_EQ(m.blit_pixels(), 16U * 12U);
  CHECK_EQ(m.pixel({ 2, 2 }), color::green);
  CHECK_EQ(m.pixel({ 1, 1 }), color::black);     // plane 1 has no red pixel
  CHECK_EQ(vic_test::count_pixels(m, color::black), 16U * 12U - 1U);
  // further drawing on plane 1 is transferred
  m.reset_counters();
  d.plot({ 3, 3 }, color::white);
  CHECK_EQ(m.pixel({ 3, 3 }), color::white);
  CHECK_EQ(m.blit_count(), 1U);
  // invalid planes
  CHECK(!fb.framebuffer_set_display(2U));
  CHECK(!fb.framebuffer_set_access(2U));
}


TEST_CASE(framebuffer_native_ops_equal_memory)
{
  // everything drawn through the framebuffer must end up on the head exactly like direct drawing
  head_type a, b;
  head::framebuffer<16, 12> fb(a);
  dc da(fb), db(b);
  for (int i = 0; i < 2; ++i) {
    dc& d = i == 0 ? da : db;
    d.cls(color::blue);
    d.set_color(color::white);
    d.line_horz({ -3, 1 }, { 20, 1 });
    d.line_vert({ 2, -3 }, { 2, 20 });
    d.box({ 4, 3, 9, 6 });
    d.box({ 12, 8, 30, 30 });
    d.set_color(color::red);
    d.line({ 0, 11 }, { 15, 0 });
    d.disc({ 10, 5 }, 3U);
    d.set_color(0x80FF0000UL);                     // translucent: blended with the buffer content
    d.box({ 0, 0, 8, 4 });
    d.move({ 4, 3, 9, 6 }, { 6, 7 });
    d.move({ 6, 7, 11, 10 }, { 5, 6 });            // overlapping move back
  }
  CHECK_SAME_BUFFER(a, b);
  CHECK_EQ(a.cls_color(), color::blue);
}


TEST_CASE(framebuffer_blind_head)
{
  // a head which can't read its pixels: anti aliasing and sprites need the framebuffer in front
  blind_head bh;
  head_type m;
  head::framebuffer<16, 12> fb(bh);
  dc da(fb), db(m);
  for (int i = 0; i < 2; ++i) {
    dc& d = i == 0 ? da : db;
    d.cls(color::blue);
    d.anti_aliasing_enable(true);
    d.set_color(color::white);
    d.disc({ 7, 6 }, 4U);
    d.line({ 0, 0 }, { 15, 5 });
    d.set_color(0x80FF0000UL);
    d.box({ 2, 2, 6, 9 });
  }
  CHECK_SAME_BUFFER(bh, m);

  // a sprite on the blind head restores the background through the framebuffer
  dc dd(fb);
  dd.cls(color::blue);
  sprite::canvas<16, 1> s(fb, 0);
  s.set_color(color::green);
  s.box({ 0, 0, 2, 2 });
  s.render({ 3, 3 });
  CHECK_EQ(bh.pixel({ 3, 3 }), color::green);
  s.render({ 8, 8 });
  CHECK_EQ(bh.pixel({ 3, 3 }), color::blue);
  CHECK_EQ(bh.pixel({ 8, 8 }), color::green);
  CHECK_EQ(vic_test::count_pixels(bh, color::green), 4U);
}


TEST_CASE(framebuffer_blit_clip)
{
  head_type m;
  head::framebuffer<16, 12> fb(m);
  dc d(fb);
  d.cls(color::black);
  // a 4 x 3 block with stride 5, partly outside at the top/left
  static const color::value_type block[15] = {
    color::red,   color::green, color::blue,  color::white, color::none,
    color::white, color::red,   color::green, color::blue,  color::none,
    color::blue,  color::white, color::red,   color::green, color::none };
  d.blit({ -1, -1, 3, 2 }, block, 5U);
  CHECK_EQ(m.pixel({ 0, 0 }), color::red);         // block (1,1)
  CHECK_EQ(m.pixel({ 1, 0 }), color::green);
  CHECK_EQ(m.pixel({ 2, 0 }), color::blue);
  CHECK_EQ(m.pixel({ 0, 1 }), color::white);       // block (1,2)
  CHECK_EQ(m.pixel({ 2, 1 }), color::green);
  CHECK_EQ(vic_test::count_pixels_not(m, color::black), 6U);
  // at the bottom/right border
  d.blit({ 14, 11, 18, 14 }, block, 5U);
  CHECK_EQ(m.pixel({ 14, 11 }), color::red);
  CHECK_EQ(m.pixel({ 15, 11 }), color::green);
  CHECK_EQ(vic_test::count_pixels_not(m, color::black), 8U);
  CHECK_EQ(m.oob_count(), 0U);
}


TEST_CASE(framebuffer_display_control)
{
  // the framebuffer has no display of its own: enable, brightness and contrast reach the bound head
  control_head h;
  head::framebuffer<16, 12> fb(h);
  drv& d = fb;
  d.display_enable(true);
  d.display_brightness(127U);
  d.display_contrast(200U);
  CHECK(h.enable_);
  CHECK_EQ(h.brightness_, 127U);
  CHECK_EQ(h.contrast_, 200U);
  d.display_enable(false);
  d.display_brightness(255U);
  CHECK(!h.enable_);
  CHECK_EQ(h.brightness_, 255U);
}
