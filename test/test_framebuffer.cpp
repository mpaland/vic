///////////////////////////////////////////////////////////////////////////////
// \brief Tests for head/framebuffer.h: storage formats, dirty rect transfer via
// blit() and blit_raw(), planes and the use in front of a head which can't read its pixels
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "helpers.h"
#include "dc.h"
#include "sprite.h"
#include "head/memory.h"
#include "head/framebuffer.h"
#include "head/MIPI_DCS.h"
#include "io_stub.h"

#include <vector>

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


///////////////////////////////////////////////////////////////////////////////
// transfer to the head behind: an RGB565 framebuffer in front of a MIPI DCS display hands its dirty rect over as
// it is stored (blit_raw): one address window and a stream of large blocks. Any other pair goes the old way, ARGB
// pieces of a line through blit(). The display is a model of the controller RAM behind the io stub: it follows
// the address window like a MIPI DCS controller and counts the io jobs. So the tests compare what both ways leave
// in the display RAM, and what they cost.

namespace {

// the RAM of a MIPI DCS controller: address window 2Ah / 2Bh, memory write 2Ch with the pointer that runs through
// the window, pixels of two bytes (high byte first)
class panel
{
public:
  static const std::size_t size = 512U;     // columns and rows of the model

  panel()
    : gram_(size * size, 0xFFFFU)
    , cmd_(0U)
    , args_(0U)
    , x0_(0U), x1_(0U), y0_(0U), y1_(0U), x_(0U), y_(0U)
    , high_(0U)
    , half_(false)
    , jobs_(0U)
    , windows_(0U)
    , outside_(0U)
  { }

  // io_stub write hook: the device handle is the panel
  static void hook(io::handle_type device_handle, std::uint32_t option, const std::uint8_t* data, std::size_t length)
  {
    const_cast<panel*>(static_cast<const panel*>(device_handle))->write(option, data, length);
  }

  void write(std::uint32_t option, const std::uint8_t* data, std::size_t length)
  {
    ++jobs_;
    if (option == 0U) {
      // a command
      cmd_  = data[0];
      args_ = 0U;
      half_ = false;
      if (cmd_ == 0x2CU) {
        x_ = x0_;
        y_ = y0_;
        ++windows_;
      }
      return;
    }
    for (std::size_t i = 0U; i < length; ++i) {
      const std::uint8_t b = data[i];
      if (cmd_ == 0x2AU) {
        arg(x0_, x1_, b);
      }
      else if (cmd_ == 0x2BU) {
        arg(y0_, y1_, b);
      }
      else if (cmd_ == 0x2CU) {
        if (!half_) {
          high_ = b;
          half_ = true;
          continue;
        }
        half_ = false;
        if ((x_ < size) && (y_ < size) && (x_ <= x1_) && (y_ <= y1_)) {
          gram_[y_ * size + x_] = static_cast<std::uint16_t>((static_cast<std::uint16_t>(high_) << 8U) | b);
        }
        else {
          ++outside_;
        }
        if (++x_ > x1_) {
          x_ = x0_;
          if (++y_ > y1_) {
            y_ = y0_;
          }
        }
      }
    }
  }

  const std::vector<std::uint16_t>& gram() const
  { return gram_; }

  // io jobs (calls of io::write) so far
  std::size_t jobs() const
  { return jobs_; }

  // memory writes (2Ch) so far: one per address window
  std::size_t windows() const
  { return windows_; }

  // pixels written outside the window or the model
  std::size_t outside() const
  { return outside_; }

  void reset_counters()
  {
    jobs_    = 0U;
    windows_ = 0U;
  }

private:
  // the four argument bytes of 2Ah / 2Bh: start and end, high byte first
  void arg(std::size_t& start, std::size_t& end, std::uint8_t b)
  {
    switch (args_++) {
      case 0U : start = static_cast<std::size_t>(b) << 8U;  break;
      case 1U : start |= b;                                 break;
      case 2U : end   = static_cast<std::size_t>(b) << 8U;  break;
      case 3U : end   |= b;                                 break;
      default :                                             break;
    }
  }

  std::vector<std::uint16_t>  gram_;
  std::uint8_t                cmd_;
  std::size_t                 args_;
  std::size_t                 x0_, x1_, y0_, y1_, x_, y_;
  std::uint8_t                high_;
  bool                        half_;
  std::size_t                 jobs_;
  std::size_t                 windows_;
  std::size_t                 outside_;
};


// a picture with many colors, edges that are no multiple of the transfer pieces and single pixels
void picture(dc& d, std::int16_t w, std::int16_t h)
{
  d.present_lock();
  d.cls(color::argb(0x10U, 0x20U, 0x40U));
  for (std::int16_t i = 0; i < 12; ++i) {
    d.set_color(color::argb(static_cast<std::uint8_t>(i * 21), static_cast<std::uint8_t>(255 - i * 17), static_cast<std::uint8_t>(i * 9 + 40)));
    d.box({ static_cast<std::int16_t>(3 + i * 23), static_cast<std::int16_t>(5 + i * 7),
            static_cast<std::int16_t>(3 + i * 23 + 37), static_cast<std::int16_t>(5 + i * 7 + 61) });
  }
  d.set_color(color::argb(0xFFU, 0xFFU, 0x00U));
  d.line({ 0, 0 }, { static_cast<std::int16_t>(w - 1), static_cast<std::int16_t>(h - 1) });
  d.line({ static_cast<std::int16_t>(w - 1), 0 }, { 0, static_cast<std::int16_t>(h - 1) });
  d.set_color(color::argb(0xFFU, 0xFFU, 0xFFU));
  d.pixel_set({ 0, 0 });
  d.pixel_set({ static_cast<std::int16_t>(w - 1), static_cast<std::int16_t>(h - 1) });
  d.present_lock(false);
}


// one screen in both ways: the RGB565 framebuffer (raw transfer) and the ARGB8888 framebuffer (pieces through blit)
template<typename LCD, std::uint16_t W, std::uint16_t H>
void compare_ways()
{
  io_stub::clear();
  io_stub::set_write_hook(&panel::hook);
  static panel raw_panel, old_panel;
  static LCD raw_lcd(&raw_panel), old_lcd(&old_panel);
  static head::framebuffer<W, H, 1U, color::format_RGB565>   raw_fb(raw_lcd);
  static head::framebuffer<W, H, 1U, color::format_ARGB8888> old_fb(old_lcd);
  raw_fb.init();
  old_fb.init();
  dc raw_dc(raw_fb), old_dc(old_fb);
  const std::int16_t w = static_cast<std::int16_t>(W), h = static_cast<std::int16_t>(H);

  // a whole picture
  picture(raw_dc, w, h);
  picture(old_dc, w, h);
  CHECK(raw_panel.gram() == old_panel.gram());

  // a small change: only its rect goes to the display
  raw_panel.reset_counters();
  old_panel.reset_counters();
  raw_dc.set_color(color::argb(0x00U, 0xC0U, 0x30U));
  old_dc.set_color(color::argb(0x00U, 0xC0U, 0x30U));
  raw_dc.box({ 70, 33, 141, 52 });      // 71 x 19 pixels: more than one piece of 64 pixels per line
  old_dc.box({ 70, 33, 141, 52 });
  CHECK(raw_panel.gram() == old_panel.gram());
  CHECK_EQ(raw_panel.windows(), 1U);
  CHECK_EQ(raw_panel.jobs(), 5U + ((71U * 19U * 2U) + 511U) / 512U);
  CHECK_EQ(old_panel.windows(), 19U * 2U);

  // the whole screen: one window and blocks of 512 bytes against a window per piece of a line
  raw_panel.reset_counters();
  old_panel.reset_counters();
  for (std::size_t n = 0U; n < 2U; ++n) {
    dc& d = (n == 0U) ? raw_dc : old_dc;
    d.present_lock();
    d.set_color(color::argb(0x80U, 0x00U, 0x60U));
    d.box({ 0, 0, w, h });
    d.set_color(color::argb(0x00U, 0xFFU, 0xFFU));
    d.line({ 0, static_cast<std::int16_t>(h - 1) }, { static_cast<std::int16_t>(w - 1), 0 });
    d.present_lock(false);
  }
  CHECK(raw_panel.gram() == old_panel.gram());
  const std::size_t pieces = (static_cast<std::size_t>(W) + 63U) / 64U;
  CHECK_EQ(raw_panel.windows(), 1U);
  CHECK_EQ(raw_panel.jobs(), 5U + ((static_cast<std::size_t>(W) * H * 2U) + 511U) / 512U);
  CHECK_EQ(old_panel.windows(), static_cast<std::size_t>(H) * pieces);
  CHECK_EQ(old_panel.jobs(), static_cast<std::size_t>(H) * pieces * 6U);

  CHECK_EQ(raw_panel.outside(), 0U);
  CHECK_EQ(old_panel.outside(), 0U);
  io_stub::clear();
}

} // namespace


// ST7789, 320 x 240 landscape
TEST_CASE(framebuffer_raw_transfer_320x240)
{
  compare_ways<head::MIPI_DCS<head::MIPI_DCS_ST7789, 320U, 240U, drv::orientation_90, false, true>, 320U, 240U>();
}


// ST7789, 320 x 172 landscape, the panel 34 rows into the controller RAM
TEST_CASE(framebuffer_raw_transfer_320x172_offset)
{
  compare_ways<head::MIPI_DCS<head::MIPI_DCS_ST7789, 320U, 172U, drv::orientation_270, false, true, false, 0U, 34U>, 320U, 172U>();
}


// a display with 18 bit pixels does not take RGB565 blocks: the old way, the same picture
TEST_CASE(framebuffer_raw_transfer_18bit_head)
{
  io_stub::clear();
  io_stub::set_write_hook(&panel::hook);
  static panel p;
  static head::MIPI_DCS<head::MIPI_DCS_ILI9488, 64U, 48U> lcd(&p);
  static head::framebuffer<64U, 48U, 1U, color::format_RGB565> fb(lcd);
  fb.init();
  dc d(fb);
  p.reset_counters();
  d.set_color(color::argb(0xFFU, 0x80U, 0x00U));
  d.box({ 0, 0, 64, 48 });
  CHECK_EQ(p.windows(), 48U);     // a window per line: 64 pixels are one piece
  io_stub::clear();
}


// a head without a raw transfer (the memory head): pieces of a line through blit(), as before
TEST_CASE(framebuffer_raw_transfer_memory_head)
{
  static head::memory<100U, 40U> mem;
  static head::framebuffer<100U, 40U, 1U, color::format_RGB565> fb(mem);
  fb.init();
  dc d(fb);
  mem.reset_counters();
  const color::value_type orange = color::argb(0xFFU, 0x80U, 0x10U);
  d.set_color(orange);
  d.box({ 10, 5, 90, 25 });       // 80 x 20 pixels: two pieces per line
  CHECK_EQ(mem.blit_count(), 20U * 2U);
  CHECK_EQ(mem.blit_pixels(), 80U * 20U);
  // the pixels come back as RGB565 shows them
  const color::value_type shown = color::RGB565_to_color(color::color_to_RGB565(orange));
  CHECK_EQ(mem.pixel({ 10, 5 }), shown);
  CHECK_EQ(mem.pixel({ 89, 24 }), shown);
  CHECK(mem.pixel({ 9, 5 }) != shown);
  CHECK(mem.pixel({ 90, 24 }) != shown);
}


// the 1 bit framebuffer has no bytes per pixel: always the old way
TEST_CASE(framebuffer_raw_transfer_l1)
{
  static head::memory<12U, 8U> mem;
  static head::framebuffer<12U, 8U, 1U, color::format_L1> fb(mem);
  fb.init();
  dc d(fb);
  mem.reset_counters();
  d.set_color(color::argb(0xFFU, 0xFFU, 0xFFU));
  d.box({ 2, 1, 9, 6 });          // 7 x 5 pixels
  CHECK_EQ(mem.blit_count(), 5U);
  CHECK_EQ(mem.blit_pixels(), 7U * 5U);
  CHECK_EQ(mem.pixel({ 2, 1 }), color::L1_to_color(1U));
  CHECK_EQ(mem.pixel({ 8, 5 }), color::L1_to_color(1U));
  // only the changed rect went to the head
  CHECK(mem.pixel({ 1, 1 }) != color::L1_to_color(1U));
  CHECK(mem.pixel({ 9, 5 }) != color::L1_to_color(1U));
}
