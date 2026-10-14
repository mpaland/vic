///////////////////////////////////////////////////////////////////////////////
// \brief Compile and smoke tests for all heads in src/head
// Every head is instantiated with the io stub and the basic driver functions
// are called. This mainly guarantees that all heads compile against the
// current drv API.
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "io_stub.h"
#include "helpers.h"

#include "dc.h"
#include "tc.h"
#include "head/memory.h"
#include "head/dummy.h"
#include "head/skeleton.h"
#include "head/framebuffer.h"
#include "head/seven_segment.h"
#include "head/STPxxP05.h"
#include "head/MAX7219.h"

using namespace vic;


TEST_CASE(heads_skeleton_framebuffer)
{
  head::skeleton<8, 4> sk(nullptr);
  sk.init();
  dc d(sk);
  d.set_color(color::white);
  d.line({ 0, 0 }, { 7, 3 });
  d.box({ 1, 1, 3, 3 });
  d.move({ 0, 0, 2, 2 }, { 4, 0 });
  CHECK(sk.version() != nullptr);
  CHECK(sk.is_graphic());

  // framebuffer with two planes over a memory head
  head::memory<8, 4> mem;
  head::framebuffer<8, 4, 2> fb(mem);
  fb.init();
  CHECK_EQ(fb.framebuffer_get_count(), 2U);
  dc df(fb);
  df.set_color(color::red);
  df.plot({ 1, 1 });
  CHECK_EQ(mem.pixel({ 1, 1 }), color::red);     // plane 0 is displayed
  CHECK(fb.framebuffer_set_access(1U));
  df.set_color(color::green);
  df.plot({ 2, 2 });
  CHECK_EQ(mem.pixel({ 2, 2 }), color::none);    // plane 1 is not displayed yet
  CHECK_EQ(df.pixel_get({ 2, 2 }), color::green);
  CHECK(fb.framebuffer_set_display(1U));
  CHECK_EQ(mem.pixel({ 2, 2 }), color::green);
  CHECK_EQ(mem.pixel({ 1, 1 }), color::none);    // plane 1 has no red pixel
  CHECK(!fb.framebuffer_set_display(2U));
  CHECK(!fb.framebuffer_set_access(2U));
}


TEST_CASE(heads_leds)
{
  io_stub::clear();
  head::MAX7219<8, 8> max(nullptr);
  max.init();
  CHECK(io_stub::write_calls() > 0U);
  dc d(max);
  d.set_color(color::white);
  d.plot({ 3, 2 });
  CHECK_EQ(d.pixel_get({ 3, 2 }), color::white);
  CHECK_EQ(d.pixel_get({ 4, 2 }), color::black);
  max.display_brightness(255U);

  io_stub::clear();
  head::STPxxP05<8, 1> stp(nullptr);
  dc ds(stp);
  ds.set_color(color::white);
  ds.plot({ 0, 0 });
  ds.plot({ 7, 0 });
  CHECK_EQ(stp.buffer()[0], 0x81U);
  CHECK_EQ(io_stub::written_size(), 2U);   // two presents of one byte
  CHECK_EQ(ds.pixel_get({ 7, 0 }), color::white);
  CHECK_EQ(ds.pixel_get({ 1, 0 }), color::black);

  io_stub::clear();
  head::seven_segment<4> seg(nullptr);
  tc t(seg);
  t.out({ 0, 0 }, reinterpret_cast<const std::uint8_t*>("1.2-"));
  CHECK_EQ(seg.segments(0U), 0x86U);   // '1' with decimal point
  CHECK_EQ(seg.segments(1U), 0x5BU);   // '2'
  CHECK_EQ(seg.segments(2U), 0x40U);   // '-'
  CHECK_EQ(seg.segments(3U), 0x00U);
  // the drv text_out fallback and the tc both present: two frames of 4 bytes
  CHECK_EQ(io_stub::written_size(), 8U);
  CHECK_EQ(io_stub::written()[1], 0x5BU);
  CHECK_EQ(io_stub::written()[5], 0x5BU);
}


TEST_CASE(heads_memory)
{
  head::memory<8, 4> h;
  CHECK_EQ(h.screen_width(),  8U);
  CHECK_EQ(h.screen_height(), 4U);
  CHECK_EQ(h.viewport_width(),  8U);
  CHECK_EQ(h.viewport_height(), 4U);
  CHECK(h.is_graphic());
  CHECK(h.version() != nullptr);
  CHECK_EQ(h.framebuffer_get_count(), 1U);

  h.init();
  CHECK_EQ(h.cls_count(), 1U);
  CHECK_EQ(h.cls_color(), color::none);

  dc _dc(h);
  _dc.set_color(color::red);
  _dc.plot({ 1, 1 });
  CHECK_EQ(h.pixel({ 1, 1 }), color::red);
  CHECK_EQ(h.pixel_set_count(), 1U);
  CHECK_EQ(h.present_count(), 1U);
  CHECK_EQ(h.buffer()[1U * 8U + 1U], color::red);

  // out of bounds is rejected and counted
  _dc.plot({ 8, 0 });
  _dc.plot({ 0, 4 });
  _dc.plot({ -1, 0 });
  _dc.plot({ 0, -1 });
  CHECK_EQ(h.oob_count(), 4U);
  CHECK_EQ(h.pixel_set_count(), 1U);
  CHECK_EQ(h.pixel({ 8, 0 }), color::none);
  CHECK_EQ(h.pixel({ -1, 0 }), color::none);

  // cls with color
  _dc.cls(color::blue);
  CHECK_EQ(h.cls_count(), 2U);
  CHECK_EQ(h.cls_color(), color::blue);
  CHECK_EQ(vic_test::count_pixels(h, color::blue), 32U);

  h.reset_counters();
  CHECK_EQ(h.present_count(), 0U);
  CHECK_EQ(h.oob_count(), 0U);
}


TEST_CASE(heads_memory_viewport)
{
  head::memory<16, 8, 8, 4> h(2, 1);
  CHECK_EQ(h.viewport_width(),  8U);
  CHECK_EQ(h.viewport_height(), 4U);
  CHECK((h.viewport_get() == vertex_type{ 2, 1 }));
  CHECK(h.viewport_is_inside({ 2, 1 }));
  CHECK(h.viewport_is_inside({ 9, 4 }));
  CHECK(!h.viewport_is_inside({ 10, 4 }));
  CHECK(!h.viewport_is_inside({ 1, 1 }));
  CHECK(h.screen_is_inside({ 15, 7 }));
  CHECK(!h.screen_is_inside({ 16, 7 }));

  h.viewport_set({ 4, 4 });
  CHECK((h.viewport_get() == vertex_type{ 4, 4 }));
  CHECK_EQ(h.present_count(), 1U);   // viewport_set presents
}


TEST_CASE(heads_dummy)
{
  head::dummy<10, 10> g;
  head::dummy<10, 10> t(0, 0, false);
  CHECK(g.is_graphic());
  CHECK(!t.is_graphic());
  CHECK(g.version() != nullptr);
  g.init();
  t.init();

  // drawing on a dummy is a no-op and must not crash
  dc _dc(g);
  _dc.cls();
  _dc.set_color(color::white);
  _dc.line({ 0, 0 }, { 9, 9 });
  _dc.box({ 1, 1, 5, 5 });
  _dc.circle({ 5, 5 }, 3);
  CHECK_EQ(_dc.pixel_get({ 0, 0 }), color::none);

  tc _tc(t);
  _tc.cls();
  _tc.set_pos({ 0, 0 });
  CHECK_EQ(_tc.out(reinterpret_cast<const std::uint8_t*>("Hello")), 5U);
  g.shutdown();
  t.shutdown();
}


#include "head/BA6x.h"
#include "head/Digole.h"
#include "head/ILI9325.h"
#include "head/SSD1306.h"
#include "head/SSD1331.h"
#include "head/HD44780.h"
#include "head/MIPI_DCS.h"
#include "head/PCD8544.h"
#include "head/ST7565.h"
#include "head/ST7920.h"
#include "head/SSD1680.h"
#include "head/SSD1322.h"
#include "head/SSD1351.h"


// find a byte sequence in the recorded io stream, returns the index or -1
static long find_bytes(const std::uint8_t* data, std::size_t size, const std::uint8_t* seq, std::size_t len)
{
  for (std::size_t i = 0U; i + len <= size; ++i) {
    std::size_t j = 0U;
    while ((j < len) && (data[i + j] == seq[j])) {
      ++j;
    }
    if (j == len) {
      return static_cast<long>(i);
    }
  }
  return -1L;
}


TEST_CASE(heads_BA6x)
{
  io_stub::clear();
  head::BA6x<head::BA63> ba(nullptr);
  CHECK(!ba.is_graphic());
  CHECK_EQ(ba.screen_width(), 20U);
  CHECK_EQ(ba.screen_height(), 2U);
  ba.init();
  CHECK_EQ(io_stub::init_calls(), 1U);
  CHECK(io_stub::delay_total() >= 750U);
  // cls command: 02 00 04 1B 5B 32 4A
  REQUIRE(io_stub::written_size() >= 7U);
  CHECK_EQ(io_stub::written()[0], 0x02U);
  CHECK_EQ(io_stub::written()[2], 0x04U);
  CHECK_EQ(io_stub::written()[3], 0x1BU);
  CHECK_EQ(io_stub::written()[6], 0x4AU);

  // text output through a text context
  tc t(ba);
  io_stub::clear();
  t.set_pos({ 2, 1 });
  // cursor command: ESC [ 2 ; 3 H (1 based)
  REQUIRE(io_stub::written_size() == 3U + 6U);
  CHECK_EQ(io_stub::written()[3], 0x1BU);
  CHECK_EQ(io_stub::written()[4], 0x5BU);
  CHECK_EQ(io_stub::written()[5], '2');
  CHECK_EQ(io_stub::written()[6], ';');
  CHECK_EQ(io_stub::written()[7], '3');
  CHECK_EQ(io_stub::written()[8], 'H');
  io_stub::clear();
  CHECK_EQ(t.out(reinterpret_cast<const std::uint8_t*>("Hi")), 2U);
  REQUIRE(io_stub::written_size() == 3U + 2U);
  CHECK_EQ(io_stub::written()[2], 0x02U);
  CHECK_EQ(io_stub::written()[3], 'H');
  CHECK_EQ(io_stub::written()[4], 'i');
  // the string is clipped to the display
  io_stub::clear();
  t.set_pos({ 18, 0 });
  io_stub::clear();
  CHECK_EQ(t.out(reinterpret_cast<const std::uint8_t*>("Hello")), 2U);
  CHECK_EQ(io_stub::written_size(), 3U + 2U);
  io_stub::clear();
  t.set_pos({ -2, 0 });
  io_stub::clear();
  CHECK_EQ(t.out(reinterpret_cast<const std::uint8_t*>("Hello")), 3U);
  CHECK_EQ(io_stub::written()[3], 'l');
  // a string completely outside is not sent
  t.set_pos({ 0, 5 });
  io_stub::clear();
  CHECK_EQ(t.out(reinterpret_cast<const std::uint8_t*>("Hello")), 0U);
  CHECK_EQ(io_stub::written_size(), 0U);
  // model sizes
  head::BA6x<head::BA60> b60(nullptr);
  CHECK_EQ(b60.screen_width(), 16U);
  CHECK_EQ(b60.screen_height(), 1U);
  head::BA6x<head::BA66> b66(nullptr);
  CHECK_EQ(b66.screen_height(), 4U);
}


TEST_CASE(heads_Digole)
{
  io_stub::clear();
  head::digole<128, 64> dg(nullptr, 115200U);
  CHECK(dg.is_graphic());
  dg.init();
  CHECK_EQ(io_stub::init_calls(), 1U);
  // the init sequence contains the clear screen and the baudrate command
  const std::uint8_t* w = io_stub::written();
  const std::size_t n = io_stub::written_size();
  bool cls = false, baud = false;
  for (std::size_t i = 0U; i + 7U < n; ++i) {
    cls  |= (w[i] == 'C') && (w[i + 1] == 'L');
    baud |= (w[i] == 'S') && (w[i + 1] == 'B') && (w[i + 2] == '1') && (w[i + 3] == '1') && (w[i + 4] == '5') && (w[i + 5] == '2');
  }
  CHECK(cls);
  CHECK(baud);

  // a pixel: color setup + DP x y, the second pixel of the same color is just DP x y
  dc d(dg);
  d.set_color(color::argb(0x11U, 0x22U, 0x33U));
  io_stub::clear();
  d.plot({ 5, 7 });
  REQUIRE(io_stub::written_size() == 10U);
  CHECK_EQ(io_stub::written()[0], 'E');
  CHECK_EQ(io_stub::written()[3], 0x11U);
  CHECK_EQ(io_stub::written()[5], 0x33U);
  CHECK_EQ(io_stub::written()[6], 'D');
  CHECK_EQ(io_stub::written()[7], 'P');
  CHECK_EQ(io_stub::written()[8], 5U);
  CHECK_EQ(io_stub::written()[9], 7U);
  io_stub::clear();
  d.plot({ 6, 7 });
  CHECK_EQ(io_stub::written_size(), 4U);
  // a box is one native filled rectangle command with inclusive corners
  io_stub::clear();
  d.box({ 2, 3, 10, 8 });
  REQUIRE(io_stub::written_size() == 6U);
  CHECK_EQ(io_stub::written()[0], 'F');
  CHECK_EQ(io_stub::written()[2], 2U);
  CHECK_EQ(io_stub::written()[3], 3U);
  CHECK_EQ(io_stub::written()[4], 9U);
  CHECK_EQ(io_stub::written()[5], 7U);
  // a disc is a few line commands
  io_stub::clear();
  d.disc({ 20, 20 }, 3U);
  CHECK_EQ(io_stub::written_size(), 7U * 6U);
  // out of bounds pixels are not sent
  io_stub::clear();
  d.plot({ 200, 7 });
  CHECK_EQ(io_stub::written_size(), 0U);
  // I2C variant compiles
  head::digole<128, 64, 128, 64, drv::orientation_0, head::DIGOLE_I2C> i2c(reinterpret_cast<io::handle_type>(static_cast<std::uintptr_t>(0x3CU)));
  io_stub::clear();
  i2c.init();
  CHECK(io_stub::written_size() > 0U);
}


TEST_CASE(heads_ILI9325)
{
  io_stub::clear();
  head::ILI9325<240, 320> ili(nullptr, nullptr, nullptr, false);
  CHECK(ili.is_graphic());
  CHECK_EQ(ili.screen_width(), 240U);
  ili.init();
  CHECK_EQ(io_stub::init_calls(), 1U);
  CHECK(io_stub::write_calls() > 40U);
  CHECK(io_stub::delay_total() >= 300U);

  dc d(ili);
  d.set_color(color::red);
  io_stub::clear();
  d.plot({ 3, 4 });
  // index writes (2 bytes) and data writes, the pixel data is RGB565 red 0xF800 big endian
  REQUIRE(io_stub::written_size() >= 2U);
  CHECK_EQ(io_stub::written()[io_stub::written_size() - 2U], 0xF8U);
  CHECK_EQ(io_stub::written()[io_stub::written_size() - 1U], 0x00U);
  // a horizontal line is one address setup plus one data word per pixel
  io_stub::clear();
  d.line_horz({ 10, 5 }, { 19, 5 });
  CHECK_EQ(io_stub::write_calls(), 4U + 1U + 10U);   // 2 register writes (idx + data each), idx, 10 pixels
  // the next pixel right of the line needs no new address
  io_stub::clear();
  d.plot({ 20, 5 });
  CHECK_EQ(io_stub::write_calls(), 2U);
  // pixel_get without read support is unknown
  CHECK_EQ(d.pixel_get({ 3, 4 }), color::none);
  // a blit sets the GRAM window once (4 regs), the start address (2 regs), the data index and
  // streams the pixels, then restores the full window (4 regs): 10 * 2 + 1 + N write calls
  static const color::value_type block[6] = { color::red, color::green, color::blue, color::white, color::black, color::red };
  io_stub::clear();
  d.blit({ 5, 6, 8, 8 }, block, 3U);
  CHECK_EQ(io_stub::write_calls(), 21U + 6U);
  // register 0x50 (window start) with value 5, the data bytes of the first pixel are RGB565 red
  const std::uint8_t* w = io_stub::written();
  CHECK_EQ(w[1], 0x50U);
  CHECK_EQ(w[3], 0x05U);
  CHECK_EQ(w[26], 0xF8U);           // 6 register writes (idx 2 + data 2 bytes each) = 24 bytes, data idx (2 bytes), first pixel
  CHECK_EQ(w[27], 0x00U);
  // the next single pixel needs a new address
  io_stub::clear();
  d.plot({ 8, 6 });
  CHECK_EQ(io_stub::write_calls(), 4U + 2U);
  // 256k color variant with the 8 bit memory interface compiles
  std::uint8_t reg = 0U, data = 0U;
  head::ILI9325<240, 320, drv::orientation_0, 8U, true> mem(nullptr, &reg, &data, false);
  dc dm(mem);
  dm.set_color(color::blue);
  dm.plot({ 1, 1 });
  CHECK_EQ(data, 0x3FU);     // the last byte of RGB666 blue (RRRRRRGGGGGGBBBBBB)
  ili.display_enable(false);

  // landscape: the entry mode selects AM = 1, I/D = 10 and the pixel is written transposed into the 240 x 320 GRAM
  head::ILI9325<320, 240, drv::orientation_90> rot(nullptr, nullptr, nullptr, false);
  io_stub::clear();
  rot.init();
  static const std::uint8_t entry_90[4] = { 0x00U, 0x03U, 0x00U, 0x28U };
  CHECK(find_bytes(io_stub::written(), io_stub::written_size(), entry_90, 4U) >= 0L);
  dc dr(rot);
  dr.set_color(color::red);
  io_stub::clear();
  dr.plot({ 3, 4 });
  w = io_stub::written();
  CHECK_EQ(w[1], 0x20U);     // R20h horizontal GRAM address = 239 - y
  CHECK_EQ(w[3], 0xEBU);
  CHECK_EQ(w[5], 0x21U);     // R21h vertical GRAM address = x
  CHECK_EQ(w[6], 0x00U);
  CHECK_EQ(w[7], 0x03U);
  CHECK_EQ(w[10], 0xF8U);
  CHECK_EQ(w[11], 0x00U);
  // the full GRAM window is 240 x 320 regardless of the orientation
  static const std::uint8_t hea[4] = { 0x00U, 0x51U, 0x00U, 0xEFU };
  static const std::uint8_t vea[4] = { 0x00U, 0x53U, 0x01U, 0x3FU };
  io_stub::clear();
  rot.init();
  CHECK(find_bytes(io_stub::written(), io_stub::written_size(), hea, 4U) >= 0L);
  CHECK(find_bytes(io_stub::written(), io_stub::written_size(), vea, 4U) >= 0L);

  // 9 bit memory interface: registers are two bytes on DB17..DB10, pixels are two 9 bit transfers
  std::uint16_t reg16 = 0U, data16 = 0U;
  head::ILI9325<240, 320, drv::orientation_0, 9U> m9(nullptr, &reg16, &data16, false);
  m9.display_enable(true);
  CHECK_EQ(reg16, 0x000EU);   // index 07h << 1
  CHECK_EQ(data16, 0x0066U);  // low byte of 0x0133 << 1
  dc d9(m9);
  d9.set_color(color::blue);
  d9.plot({ 1, 1 });
  CHECK_EQ(reg16, 0x0044U);   // index 22h << 1
  CHECK_EQ(data16, 0x003FU);  // low 9 bits of RGB666 blue

  // 18 bit memory interface: index and register data on DB15..DB0, pixels are one 18 bit word
  std::uint32_t reg32 = 0U, data32 = 0U;
  head::ILI9325<240, 320, drv::orientation_0, 18U> m18(nullptr, &reg32, &data32, false);
  m18.display_enable(true);
  CHECK_EQ(reg32, 0x00000007UL);
  CHECK_EQ(data32, 0x00000133UL);
  dc d18(m18);
  d18.set_color(color::blue);
  d18.plot({ 1, 1 });
  CHECK_EQ(reg32, 0x00000022UL);
  CHECK_EQ(data32, 0x0000003FUL);
}


TEST_CASE(heads_SSD1306)
{
  io_stub::clear();
  head::SSD1306<128, 64> oled(nullptr);
  CHECK(oled.is_graphic());
  oled.init();
  CHECK_EQ(io_stub::init_calls(), 1U);
  // the init sequence is an I2C command stream starting with the control byte and display off
  CHECK_EQ(io_stub::written()[0], 0x00U);
  CHECK_EQ(io_stub::written()[1], 0xAEU);
  // the framebuffer is sent with present(): control byte 0x40 followed by 1024 bytes
  io_stub::clear();
  dc d(oled);
  d.set_color(color::white);
  d.plot({ 3, 5 });
  CHECK_EQ(oled.framebuffer()[3], 0x20U);        // page 0, bit 5
  CHECK_EQ(d.pixel_get({ 3, 5 }), color::white);
  CHECK_EQ(d.pixel_get({ 3, 6 }), color::black);
  const std::uint8_t* w = io_stub::written();
  const std::size_t n = io_stub::written_size();
  REQUIRE(n >= 1025U);
  CHECK_EQ(w[n - 1025U], 0x40U);
  CHECK_EQ(w[n - 1024U + 3U], 0x20U);
  // clearing
  d.cls();
  CHECK_EQ(oled.framebuffer()[3], 0x00U);
  d.plot({ 127, 63 });
  CHECK_EQ(oled.framebuffer()[7U * 128U + 127U], 0x80U);
  // a vertical line sets bits in one column
  d.line_vert({ 10, 0 }, { 10, 15 });
  CHECK_EQ(oled.framebuffer()[10], 0xFFU);
  CHECK_EQ(oled.framebuffer()[128U + 10U], 0xFFU);
  CHECK_EQ(oled.framebuffer()[256U + 10U], 0x00U);
  oled.display_contrast(100U);
  oled.display_enable(false);

  // 32 line SPI variant without framebuffer: every pixel is a read-modify-write on the display
  head::SSD1306<128, 32, drv::orientation_0, head::SSD1306_SPI4, false> spi(nullptr);
  spi.init();
  dc ds(spi);
  ds.set_color(color::white);
  io_stub::clear();
  ds.plot({ 1, 1 });
  CHECK(io_stub::read_calls() == 1U);
  CHECK(io_stub::write_calls() >= 3U);
  CHECK_EQ(io_stub::written()[io_stub::written_size() - 1U], 0x02U);   // page byte with bit 1

  // the charge pump command 8Dh 14h is sent by default and omitted for SSD1309 / external VCC
  static const std::uint8_t charge_pump[2] = { 0x8DU, 0x14U };
  io_stub::clear();
  oled.init();
  CHECK(find_bytes(io_stub::written(), 40U, charge_pump, 2U) >= 0L);
  head::SSD1306<128, 64, drv::orientation_0, head::SSD1306_I2C, true, false> nocp(nullptr);
  io_stub::clear();
  nocp.init();
  CHECK(find_bytes(io_stub::written(), 40U, charge_pump, 2U) < 0L);
}


TEST_CASE(heads_SSD1331)
{
  io_stub::clear();
  head::SSD1331<96, 64> oled(nullptr);
  CHECK(oled.is_graphic());
  CHECK_EQ(oled.screen_width(), 96U);
  oled.init();
  CHECK_EQ(io_stub::init_calls(), 1U);
  // init: unlock, display off, remap = 65k format 1 + COM split (no mirror), ..., display on, then the GAC clear window
  const std::uint8_t* w = io_stub::written();
  std::size_t n = io_stub::written_size();
  CHECK_EQ(w[0], 0xFDU);
  CHECK_EQ(w[1], 0x12U);
  CHECK_EQ(w[2], 0xAEU);
  CHECK_EQ(w[3], 0xA0U);
  CHECK_EQ(w[4], 0x60U);
  CHECK_EQ(w[n - 6U], 0xAFU);
  CHECK_EQ(w[n - 5U], 0x25U);     // clear window 0,0 .. 95,63
  CHECK_EQ(w[n - 2U], 0x5FU);
  CHECK_EQ(w[n - 1U], 0x3FU);
  CHECK_EQ(io_stub::write_calls(), 2U);

  // a single pixel: column/row window from the pixel to the screen end, then RGB565 big endian
  dc d(oled);
  d.set_color(color::red);
  io_stub::clear();
  d.plot({ 3, 4 });
  w = io_stub::written();
  REQUIRE(io_stub::written_size() == 8U);
  CHECK_EQ(w[0], 0x15U); CHECK_EQ(w[1], 0x03U); CHECK_EQ(w[2], 0x5FU);
  CHECK_EQ(w[3], 0x75U); CHECK_EQ(w[4], 0x04U); CHECK_EQ(w[5], 0x3FU);
  CHECK_EQ(w[6], 0xF8U); CHECK_EQ(w[7], 0x00U);
  // the next pixel to the right needs no address
  io_stub::clear();
  d.plot({ 4, 4 });
  CHECK_EQ(io_stub::write_calls(), 1U);
  CHECK_EQ(io_stub::written_size(), 2U);
  // a horizontal line is one window and one data block
  io_stub::clear();
  d.line_horz({ 10, 5 }, { 19, 5 });
  CHECK_EQ(io_stub::write_calls(), 2U);
  CHECK_EQ(io_stub::written_size(), 6U + 20U);
  io_stub::clear();
  d.plot({ 20, 5 });
  CHECK_EQ(io_stub::write_calls(), 1U);
  // a vertical line uses a one column wide window
  io_stub::clear();
  d.line_vert({ 10, 0 }, { 10, 15 });
  w = io_stub::written();
  CHECK_EQ(io_stub::written_size(), 6U + 32U);
  CHECK_EQ(w[1], 0x0AU); CHECK_EQ(w[2], 0x0AU);
  CHECK_EQ(w[4], 0x00U); CHECK_EQ(w[5], 0x3FU);
  // a box is one GAC draw rectangle with outline and fill color: C B A = red 5 bit in bits 5:1, 0, 0
  io_stub::clear();
  d.box({ 5, 6, 8, 8 });
  w = io_stub::written();
  REQUIRE(io_stub::written_size() == 11U);
  CHECK_EQ(w[0], 0x22U);
  CHECK_EQ(w[1], 0x05U); CHECK_EQ(w[2], 0x06U); CHECK_EQ(w[3], 0x07U); CHECK_EQ(w[4], 0x07U);
  CHECK_EQ(w[5], 0x3EU); CHECK_EQ(w[6], 0x00U); CHECK_EQ(w[7], 0x00U);
  CHECK_EQ(w[8], 0x3EU); CHECK_EQ(w[9], 0x00U); CHECK_EQ(w[10], 0x00U);
  // a non black cls is a filled rectangle over the whole RAM
  io_stub::clear();
  d.cls(color::white);
  w = io_stub::written();
  REQUIRE(io_stub::written_size() == 11U);
  CHECK_EQ(w[0], 0x22U);
  CHECK_EQ(w[3], 0x5FU); CHECK_EQ(w[4], 0x3FU);
  CHECK_EQ(w[5], 0x3EU); CHECK_EQ(w[6], 0x3FU); CHECK_EQ(w[7], 0x3EU);
  // move is a GAC copy, source and destination are clipped to the screen
  io_stub::clear();
  d.move({ 0, 0, 6, 6 }, { 3, 3 });
  w = io_stub::written();
  REQUIRE(io_stub::written_size() == 7U);
  CHECK_EQ(w[0], 0x23U);
  CHECK_EQ(w[1], 0x00U); CHECK_EQ(w[2], 0x00U); CHECK_EQ(w[3], 0x05U); CHECK_EQ(w[4], 0x05U);
  CHECK_EQ(w[5], 0x03U); CHECK_EQ(w[6], 0x03U);
  io_stub::clear();
  d.move({ 90, 0, 100, 10 }, { 92, 0 });
  w = io_stub::written();
  REQUIRE(io_stub::written_size() == 7U);
  CHECK_EQ(w[1], 0x5AU); CHECK_EQ(w[3], 0x5DU); CHECK_EQ(w[4], 0x09U); CHECK_EQ(w[5], 0x5CU);
  // blit sets the window to the block and streams it
  static const color::value_type block[6] = { color::red, color::green, color::blue, color::white, color::black, color::red };
  io_stub::clear();
  d.blit({ 5, 6, 8, 8 }, block, 3U);
  w = io_stub::written();
  REQUIRE(io_stub::written_size() == 6U + 12U);
  CHECK_EQ(w[1], 0x05U); CHECK_EQ(w[2], 0x07U); CHECK_EQ(w[4], 0x06U); CHECK_EQ(w[5], 0x07U);
  CHECK_EQ(w[6], 0xF8U); CHECK_EQ(w[7], 0x00U);
  CHECK_EQ(w[8], 0x07U); CHECK_EQ(w[9], 0xE0U);
  // out of bounds pixels are not sent, SPI can't read back
  io_stub::clear();
  d.plot({ 200, 7 });
  CHECK_EQ(io_stub::written_size(), 0U);
  CHECK_EQ(d.pixel_get({ 3, 4 }), color::none);
  CHECK_EQ(io_stub::read_calls(), 0U);
  oled.display_brightness(255U);
  oled.display_contrast(128U);
  oled.display_inverse(false);
  oled.display_enable(false);

  // parallel bus: pixel read back of RGB565
  head::SSD1331<96, 64, drv::orientation_0, head::SSD1331_PAR8> par(nullptr);
  par.init();
  dc dp(par);
  static const std::uint8_t pixel_red[2] = { 0xF8U, 0x00U };
  io_stub::clear();
  io_stub::set_read_data(pixel_red, 2U);
  CHECK_EQ(dp.pixel_get({ 1, 1 }), color::RGB565_to_color(0xF800U));
  CHECK_EQ(io_stub::read_calls(), 1U);

  // rotated: 64 x 96 screen, remap has vertical increment and column remap, addresses are transposed
  head::SSD1331<64, 96, drv::orientation_90> rot(nullptr);
  io_stub::clear();
  rot.init();
  CHECK_EQ(io_stub::written()[4], 0x63U);
  dc dr(rot);
  dr.set_color(color::red);
  io_stub::clear();
  dr.plot({ 3, 4 });
  w = io_stub::written();
  REQUIRE(io_stub::written_size() == 8U);
  CHECK_EQ(w[0], 0x15U); CHECK_EQ(w[1], 0x04U); CHECK_EQ(w[2], 0x5FU);
  CHECK_EQ(w[3], 0x75U); CHECK_EQ(w[4], 0x03U); CHECK_EQ(w[5], 0x3FU);
  io_stub::clear();
  dr.box({ 1, 2, 3, 5 });
  w = io_stub::written();
  CHECK_EQ(w[1], 0x02U); CHECK_EQ(w[2], 0x01U); CHECK_EQ(w[3], 0x04U); CHECK_EQ(w[4], 0x02U);

  // without GAC everything is streamed, the GAC delay is applied after every GAC command
  head::SSD1331<96, 64, drv::orientation_0, head::SSD1331_SPI, false, false> plain(nullptr);
  io_stub::clear();
  plain.init();
  CHECK_EQ(io_stub::written_size(), 42U + 6U + 96U * 64U * 2U);   // init commands, window, pixel stream
  dc dn(plain);
  dn.set_color(color::blue);
  io_stub::clear();
  dn.box({ 0, 0, 2, 2 });
  CHECK_EQ(io_stub::written_size(), 6U + 8U);
  head::SSD1331<96, 64, drv::orientation_0, head::SSD1331_SPI, false, true, 2U> delayed(nullptr);
  io_stub::clear();
  delayed.init();
  CHECK_EQ(io_stub::delay_total(), 2U);
}


TEST_CASE(heads_HD44780)
{
  io_stub::clear();
  head::HD44780<16, 2> lcd(nullptr);
  CHECK(!lcd.is_graphic());
  CHECK_EQ(lcd.screen_width(), 16U);
  CHECK_EQ(lcd.screen_height(), 2U);
  lcd.init();
  CHECK_EQ(io_stub::init_calls(), 1U);
  CHECK(io_stub::delay_total() >= 57U);
  // init by instruction: 3 x function set 8 bit, function set 8 bit / 2 lines / 5x8, display off, clear, entry mode, display on
  static const std::uint8_t init8[8] = { 0x30U, 0x30U, 0x30U, 0x38U, 0x08U, 0x01U, 0x06U, 0x0CU };
  REQUIRE(io_stub::written_size() == 8U);
  CHECK_EQ(find_bytes(io_stub::written(), io_stub::written_size(), init8, 8U), 0L);
  CHECK_EQ(io_stub::write_calls(), 8U);

  // text output through a text context: set DDRAM address of line 2 column 2, then the characters
  tc t(lcd);
  io_stub::clear();
  t.set_pos({ 2, 1 });
  REQUIRE(io_stub::written_size() == 1U);
  CHECK_EQ(io_stub::written()[0], 0xC2U);
  io_stub::clear();
  CHECK_EQ(t.out(reinterpret_cast<const std::uint8_t*>("Hi")), 2U);
  REQUIRE(io_stub::written_size() == 2U);
  CHECK_EQ(io_stub::written()[0], 'H');
  CHECK_EQ(io_stub::written()[1], 'i');
  // the next character continues without a new address
  io_stub::clear();
  t.out(reinterpret_cast<const std::uint8_t*>("!"));
  CHECK_EQ(io_stub::written_size(), 1U);
  // a string is clipped at the end of the line
  t.set_pos({ 14, 0 });
  io_stub::clear();
  CHECK_EQ(t.out(reinterpret_cast<const std::uint8_t*>("Hello")), 5U);
  REQUIRE(io_stub::written_size() == 2U);
  CHECK_EQ(io_stub::written()[0], 'H');
  CHECK_EQ(io_stub::written()[1], 'e');
  // a string starting left of the screen: the first visible character sets the address
  t.set_pos({ -2, 0 });
  io_stub::clear();
  CHECK_EQ(t.out(reinterpret_cast<const std::uint8_t*>("Hello")), 5U);
  REQUIRE(io_stub::written_size() == 4U);
  CHECK_EQ(io_stub::written()[0], 0x80U);
  CHECK_EQ(io_stub::written()[1], 'l');
  CHECK_EQ(io_stub::written()[3], 'o');
  // a position outside the screen is not sent, the string neither
  io_stub::clear();
  t.set_pos({ 0, 5 });
  CHECK_EQ(t.out(reinterpret_cast<const std::uint8_t*>("Hello")), 5U);
  CHECK_EQ(io_stub::written_size(), 0U);
  // clear to the end of the line: spaces and the cursor address again
  t.set_pos({ 14, 0 });
  io_stub::clear();
  t.clear_eol();
  REQUIRE(io_stub::written_size() == 3U);
  CHECK_EQ(io_stub::written()[0], 0x20U);
  CHECK_EQ(io_stub::written()[1], 0x20U);
  CHECK_EQ(io_stub::written()[2], 0x8EU);
  io_stub::clear();
  t.clear_sol();
  CHECK_EQ(io_stub::written_size(), 1U + 15U + 1U);   // address 80h, 15 spaces, address 8Eh
  CHECK_EQ(io_stub::written()[0], 0x80U);
  io_stub::clear();
  t.clear_line();
  CHECK_EQ(io_stub::written_size(), 1U + 16U + 1U);
  // cls clears and homes
  io_stub::clear();
  t.cls();
  CHECK_EQ(io_stub::written_size(), 1U);
  CHECK_EQ(io_stub::written()[0], 0x01U);
  // user character: CGRAM address, 8 rows, DDRAM address of the cursor
  static const std::uint8_t heart[8] = { 0x00U, 0x0AU, 0x1FU, 0x1FU, 0x0EU, 0x04U, 0x00U, 0x00U };
  io_stub::clear();
  lcd.char_define(1U, heart);
  REQUIRE(io_stub::written_size() == 10U);
  CHECK_EQ(io_stub::written()[0], 0x48U);
  CHECK_EQ(io_stub::written()[3], 0x1FU);
  CHECK_EQ(io_stub::written()[9], 0x80U);
  io_stub::clear();
  t.out(1U);                                           // the CGRAM character is sent as its code
  CHECK_EQ(io_stub::written_size(), 1U);
  CHECK_EQ(io_stub::written()[0], 0x01U);
  // display control
  io_stub::clear();
  lcd.cursor_set(true, true);
  CHECK_EQ(io_stub::written()[0], 0x0FU);
  lcd.display_enable(false);
  CHECK_EQ(io_stub::written()[1], 0x0BU);
  lcd.display_shift(true);
  CHECK_EQ(io_stub::written()[2], 0x1CU);
  lcd.display_home();
  CHECK_EQ(io_stub::written()[3], 0x02U);
  // ROM A00 character mapping: a umlaut (UTF-8 C3 A4) is E1h, unknown characters are '?'
  t.set_pos({ 0, 0 });
  io_stub::clear();
  CHECK_EQ(t.out(reinterpret_cast<const std::uint8_t*>("\xC3\xA4\xE2\x82\xAC")), 2U);
  REQUIRE(io_stub::written_size() == 2U);
  CHECK_EQ(io_stub::written()[0], 0xE1U);
  CHECK_EQ(io_stub::written()[1], '?');

  // 20 x 4: lines 3 and 4 continue lines 1 and 2 in the DDRAM
  head::HD44780<20, 4> big(nullptr);
  big.init();
  tc tb(big);
  io_stub::clear();
  tb.set_pos({ 0, 2 });
  CHECK_EQ(io_stub::written()[0], 0x94U);
  tb.set_pos({ 3, 3 });
  CHECK_EQ(io_stub::written()[1], 0xD7U);

  // 4 bit bus: the reset function sets are single nibble transfers, then the 4 bit function set
  head::HD44780<16, 2, head::HD44780_BUS4, head::HD44780_WAIT_IO, head::HD44780_ROM_A02> lcd4(nullptr);
  io_stub::clear();
  lcd4.init();
  static const std::uint8_t init4[9] = { 0x30U, 0x30U, 0x30U, 0x20U, 0x28U, 0x08U, 0x01U, 0x06U, 0x0CU };
  REQUIRE(io_stub::written_size() == 9U);
  CHECK_EQ(find_bytes(io_stub::written(), io_stub::written_size(), init4, 9U), 0L);
  CHECK_EQ(io_stub::delay_total(), 50U + 5U + 1U + 1U + 1U + 2U);   // reset waits and the clear, the io layer waits otherwise
  // ROM A02: ISO 8859-1 characters pass through
  tc t4(lcd4);
  io_stub::clear();
  t4.out(reinterpret_cast<const std::uint8_t*>("\xC3\xA4"));
  CHECK_EQ(io_stub::written()[0], 0xE4U);

  // busy flag polling: one io::read() per byte when the controller reports not busy
  head::HD44780<16, 2, head::HD44780_BUS8, head::HD44780_WAIT_BUSY> lcdb(nullptr);
  lcdb.init();
  tc tbf(lcdb);
  tbf.set_pos({ 0, 0 });
  static const std::uint8_t not_busy[1] = { 0x00U };
  io_stub::clear();
  io_stub::set_read_data(not_busy, 1U);
  tbf.out(reinterpret_cast<const std::uint8_t*>("H"));
  CHECK_EQ(io_stub::write_calls(), 1U);
  CHECK_EQ(io_stub::read_calls(), 1U);
  CHECK_EQ(io_stub::delay_total(), 0U);
  // without read support the busy mode falls back to the delay
  io_stub::clear();
  tbf.out(reinterpret_cast<const std::uint8_t*>("i"));
  CHECK_EQ(io_stub::read_calls(), 1U);
  CHECK_EQ(io_stub::delay_total(), 1U);
}


TEST_CASE(heads_MIPI_DCS)
{
  io_stub::clear();
  head::MIPI_DCS<head::MIPI_DCS_ILI9341, 240, 320> tft(nullptr);
  CHECK(tft.is_graphic());
  tft.init();
  CHECK_EQ(io_stub::init_calls(), 1U);
  CHECK(io_stub::delay_total() >= 140U);
  // vendor table first, then MADCTL (BGR, no mirror), pixel format RGB565 and inversion off
  const std::uint8_t* w = io_stub::written();
  CHECK_EQ(w[0], 0xEFU);
  CHECK_EQ(w[1], 0x03U);
  static const std::uint8_t madctl_0[5] = { 0x36U, 0x08U, 0x3AU, 0x55U, 0x20U };
  CHECK(find_bytes(w, 300U, madctl_0, 5U) >= 0L);

  // a pixel: column / row window from the pixel to the screen end, memory write, RGB565 big endian
  dc d(tft);
  d.set_color(color::red);
  io_stub::clear();
  d.plot({ 3, 4 });
  w = io_stub::written();
  REQUIRE(io_stub::written_size() == 13U);
  CHECK_EQ(w[0], 0x2AU); CHECK_EQ(w[1], 0x00U); CHECK_EQ(w[2], 0x03U); CHECK_EQ(w[3], 0x00U); CHECK_EQ(w[4], 0xEFU);
  CHECK_EQ(w[5], 0x2BU); CHECK_EQ(w[6], 0x00U); CHECK_EQ(w[7], 0x04U); CHECK_EQ(w[8], 0x01U); CHECK_EQ(w[9], 0x3FU);
  CHECK_EQ(w[10], 0x2CU);
  CHECK_EQ(w[11], 0xF8U); CHECK_EQ(w[12], 0x00U);
  CHECK_EQ(io_stub::write_calls(), 6U);
  io_stub::clear();
  d.plot({ 4, 4 });
  CHECK_EQ(io_stub::written_size(), 2U);
  // a box is one window and a stream
  io_stub::clear();
  d.box({ 5, 6, 8, 8 });
  w = io_stub::written();
  REQUIRE(io_stub::written_size() == 11U + 12U);
  CHECK_EQ(w[2], 0x05U); CHECK_EQ(w[4], 0x07U); CHECK_EQ(w[7], 0x06U); CHECK_EQ(w[9], 0x07U);
  // read back: unknown without read support, 18 bit pixels otherwise
  CHECK_EQ(d.pixel_get({ 3, 4 }), color::none);
  static const std::uint8_t pixel_read[3] = { 0xF8U, 0x00U, 0x00U };
  io_stub::set_read_data(pixel_read, 3U);
  CHECK_EQ(d.pixel_get({ 3, 4 }), color::argb(0xF8U, 0x00U, 0x00U));
  tft.display_brightness(128U);
  tft.display_inverse(true);
  tft.display_enable(false);

  // landscape: MADCTL MX | MV | BGR, the window is given in logical coordinates
  head::MIPI_DCS<head::MIPI_DCS_ILI9341, 320, 240, drv::orientation_90> land(nullptr);
  io_stub::clear();
  land.init();
  static const std::uint8_t madctl_90[2] = { 0x36U, 0x68U };
  CHECK(find_bytes(io_stub::written(), 300U, madctl_90, 2U) >= 0L);
  dc dl(land);
  dl.set_color(color::red);
  io_stub::clear();
  dl.plot({ 3, 4 });
  w = io_stub::written();
  CHECK_EQ(w[2], 0x03U); CHECK_EQ(w[3], 0x01U); CHECK_EQ(w[4], 0x3FU);
  CHECK_EQ(w[7], 0x04U); CHECK_EQ(w[8], 0x00U); CHECK_EQ(w[9], 0xEFU);

  // ILI9488: 18 bit pixels (3Ah = 66h, three bytes per pixel)
  head::MIPI_DCS<head::MIPI_DCS_ILI9488, 320, 480> big(nullptr);
  io_stub::clear();
  big.init();
  static const std::uint8_t colmod_18[2] = { 0x3AU, 0x66U };
  CHECK(find_bytes(io_stub::written(), 300U, colmod_18, 2U) >= 0L);
  dc db(big);
  db.set_color(color::red);
  io_stub::clear();
  db.plot({ 1, 1 });
  w = io_stub::written();
  REQUIRE(io_stub::written_size() == 14U);
  CHECK_EQ(w[11], 0xFCU); CHECK_EQ(w[12], 0x00U); CHECK_EQ(w[13], 0x00U);   // 6 bit components left aligned

  // ST7789 240 x 240 panel rotated by 180 degree: GRAM row offset 80, MADCTL MX | MY without BGR, inversion on
  head::MIPI_DCS<head::MIPI_DCS_ST7789, 240, 240, drv::orientation_180, false, true, false, 0U, 80U> square(nullptr);
  io_stub::clear();
  square.init();
  static const std::uint8_t madctl_180[5] = { 0x36U, 0xC0U, 0x3AU, 0x55U, 0x21U };
  CHECK(find_bytes(io_stub::written(), 100U, madctl_180, 5U) >= 0L);
  dc ds(square);
  ds.set_color(color::red);
  io_stub::clear();
  ds.plot({ 3, 4 });
  w = io_stub::written();
  CHECK_EQ(w[5], 0x2BU); CHECK_EQ(w[6], 0x00U); CHECK_EQ(w[7], 0x54U); CHECK_EQ(w[8], 0x01U); CHECK_EQ(w[9], 0x3FU);

  // the other controllers compile and init
  head::MIPI_DCS<head::MIPI_DCS_ST7735, 128, 160> small(nullptr);
  small.init();
  head::MIPI_DCS<head::MIPI_DCS_ST7796, 320, 480> st96(nullptr);
  st96.init();
  head::MIPI_DCS<head::MIPI_DCS_GC9A01, 240, 240> round(nullptr);
  io_stub::clear();
  round.init();
  CHECK_EQ(io_stub::written()[0], 0xEFU);

  // JD9853 1.47" 172 x 320 panel: OTP init (reset, sleep out, normal mode, display on), RGB order, no inversion,
  // the panel is centred on the 240 GRAM columns (offset 34)
  head::MIPI_DCS<head::MIPI_DCS_JD9853, 172, 320, drv::orientation_0, false, false, false, 34U, 0U> jd(nullptr);
  io_stub::clear();
  jd.init();
  w = io_stub::written();
  CHECK_EQ(w[0], 0x01U); CHECK_EQ(w[1], 0x11U); CHECK_EQ(w[2], 0x13U); CHECK_EQ(w[3], 0x29U);
  static const std::uint8_t madctl_jd[5] = { 0x36U, 0x00U, 0x3AU, 0x55U, 0x20U };
  CHECK(find_bytes(w, 20U, madctl_jd, 5U) >= 0L);
  CHECK(io_stub::delay_total() >= 145U);
  dc dj(jd);
  dj.set_color(color::red);
  io_stub::clear();
  dj.plot({ 3, 4 });
  w = io_stub::written();
  REQUIRE(io_stub::written_size() == 13U);
  CHECK_EQ(w[0], 0x2AU); CHECK_EQ(w[1], 0x00U); CHECK_EQ(w[2], 0x25U); CHECK_EQ(w[3], 0x00U); CHECK_EQ(w[4], 0xCDU);
  CHECK_EQ(w[5], 0x2BU); CHECK_EQ(w[6], 0x00U); CHECK_EQ(w[7], 0x04U); CHECK_EQ(w[8], 0x01U); CHECK_EQ(w[9], 0x3FU);
}


TEST_CASE(heads_SH1106)
{
  // SH1106 variant of the SSD1306 head: DC-DC command ADh 8Bh, no addressing mode, page addressing with column offset 2
  head::SSD1306<128, 64, drv::orientation_0, head::SSD1306_I2C, true, true, head::SSD1306_CTRL_SH1106> sh(nullptr);
  io_stub::clear();
  sh.init();
  const std::uint8_t* w = io_stub::written();
  static const std::uint8_t dcdc[2] = { 0xADU, 0x8BU };
  static const std::uint8_t pump[2] = { 0x8DU, 0x14U };
  static const std::uint8_t mode[2] = { 0x20U, 0x00U };
  CHECK(find_bytes(w, 40U, dcdc, 2U) >= 0L);
  CHECK(find_bytes(w, 40U, pump, 2U) < 0L);
  CHECK(find_bytes(w, 40U, mode, 2U) < 0L);
  // present: every page is addressed (control byte, page, column low 2, column high 0) and sent in 33 byte I2C blocks
  dc d(sh);
  d.set_color(color::white);
  io_stub::clear();
  d.plot({ 3, 5 });
  w = io_stub::written();
  REQUIRE(io_stub::written_size() == 8U * (4U + 4U * 33U));
  CHECK_EQ(w[0], 0x00U); CHECK_EQ(w[1], 0xB0U); CHECK_EQ(w[2], 0x02U); CHECK_EQ(w[3], 0x10U);
  CHECK_EQ(w[4], 0x40U);
  CHECK_EQ(w[4U + 1U + 3U], 0x20U);
  CHECK_EQ(w[136U + 1U], 0xB1U);
  // SSD1309: no charge pump command at all
  head::SSD1306<128, 64, drv::orientation_0, head::SSD1306_SPI4, true, true, head::SSD1306_CTRL_SSD1309> s09(nullptr);
  io_stub::clear();
  s09.init();
  CHECK(find_bytes(io_stub::written(), 40U, pump, 2U) < 0L);
  // SSD1306 with a 64 x 48 panel at column 32
  head::SSD1306<64, 48, drv::orientation_0, head::SSD1306_SPI4, true, true, head::SSD1306_CTRL_SSD1306, 32U> narrow(nullptr);
  io_stub::clear();
  narrow.init();
  static const std::uint8_t cols[3] = { 0x21U, 0x20U, 0x5FU };
  CHECK(find_bytes(io_stub::written(), 60U, cols, 3U) >= 0L);
}


TEST_CASE(heads_PCD8544)
{
  io_stub::clear();
  head::PCD8544<84, 48> lcd(nullptr);
  lcd.init();
  // extended set, Vop, temperature, bias, basic set, display normal, then address (0, 0) and the 504 byte frame
  const std::uint8_t* w = io_stub::written();
  REQUIRE(io_stub::written_size() == 6U + 2U + 504U);
  CHECK_EQ(w[0], 0x21U); CHECK_EQ(w[1], 0xB8U); CHECK_EQ(w[2], 0x04U); CHECK_EQ(w[3], 0x14U); CHECK_EQ(w[4], 0x20U); CHECK_EQ(w[5], 0x0CU);
  CHECK_EQ(w[6], 0x40U); CHECK_EQ(w[7], 0x80U);
  dc d(lcd);
  d.set_color(color::white);
  d.plot({ 3, 5 });
  CHECK_EQ(lcd.framebuffer()[3], 0x20U);
  CHECK_EQ(d.pixel_get({ 3, 5 }), color::white);
  CHECK_EQ(d.pixel_get({ 4, 5 }), color::black);
  io_stub::clear();
  lcd.display_contrast(0x80U);
  w = io_stub::written();
  CHECK_EQ(w[0], 0x21U); CHECK_EQ(w[1], 0xC0U); CHECK_EQ(w[2], 0x20U);
  // rotated: logical (3, 4) is physical column 79, row 3
  head::PCD8544<48, 84, drv::orientation_90> rot(nullptr);
  rot.init();
  dc dr(rot);
  dr.set_color(color::white);
  dr.plot({ 3, 4 });
  CHECK_EQ(rot.framebuffer()[79], 0x08U);
  CHECK_EQ(dr.pixel_get({ 3, 4 }), color::white);
}


TEST_CASE(heads_ST7565)
{
  io_stub::clear();
  head::ST7565<head::ST7565_ST7565, 128, 64> lcd(nullptr);
  lcd.init();
  const std::uint8_t* w = io_stub::written();
  REQUIRE(io_stub::written_size() == 16U + 8U * (3U + 128U));
  CHECK(io_stub::delay_total() >= 115U);
  CHECK_EQ(w[0], 0xE2U); CHECK_EQ(w[1], 0xA2U); CHECK_EQ(w[2], 0xA0U); CHECK_EQ(w[3], 0xC0U); CHECK_EQ(w[4], 0x40U);
  CHECK_EQ(w[5], 0xF8U); CHECK_EQ(w[6], 0x00U);
  CHECK_EQ(w[7], 0x2CU); CHECK_EQ(w[8], 0x2EU); CHECK_EQ(w[9], 0x2FU); CHECK_EQ(w[10], 0x26U);
  CHECK_EQ(w[11], 0x81U); CHECK_EQ(w[12], 0x20U); CHECK_EQ(w[15], 0xAFU);
  CHECK_EQ(w[16], 0xB0U); CHECK_EQ(w[17], 0x10U); CHECK_EQ(w[18], 0x00U);
  CHECK_EQ(w[16U + 131U], 0xB1U);
  dc d(lcd);
  d.set_color(color::white);
  d.plot({ 3, 5 });
  CHECK_EQ(lcd.framebuffer()[3], 0x20U);
  io_stub::clear();
  lcd.display_contrast(0x80U);
  CHECK_EQ(io_stub::written()[1], 0x20U);
  // ST7567 has no booster command, UC1701 with a column offset of 4
  head::ST7565<head::ST7565_ST7567, 128, 64> st67(nullptr);
  io_stub::clear();
  st67.init();
  CHECK_EQ(io_stub::written()[5], 0x2CU);
  head::ST7565<head::ST7565_UC1701, 128, 64, drv::orientation_0, 4U> uc(nullptr);
  io_stub::clear();
  uc.init();
  CHECK_EQ(io_stub::written()[18], 0x04U);
}


TEST_CASE(heads_ST7920)
{
  io_stub::clear();
  head::ST7920<128, 64> lcd(nullptr);
  lcd.init();
  const std::uint8_t* w = io_stub::written();
  REQUIRE(io_stub::written_size() == 7U + 32U * 34U);
  CHECK(io_stub::delay_total() >= 53U);
  CHECK_EQ(w[0], 0x30U); CHECK_EQ(w[2], 0x0CU); CHECK_EQ(w[3], 0x01U); CHECK_EQ(w[4], 0x06U); CHECK_EQ(w[5], 0x34U); CHECK_EQ(w[6], 0x36U);
  CHECK_EQ(w[7], 0x80U); CHECK_EQ(w[8], 0x80U);
  CHECK_EQ(w[7U + 34U], 0x81U);
  dc d(lcd);
  d.set_color(color::white);
  io_stub::clear();
  d.plot({ 0, 0 });
  d.plot({ 127, 63 });
  CHECK_EQ(lcd.framebuffer()[0], 0x80U);
  CHECK_EQ(lcd.framebuffer()[63U * 16U + 15U], 0x01U);
  CHECK_EQ(d.pixel_get({ 127, 63 }), color::white);
  // the last present carries row 0 in the first upper half block and row 63 as the last byte of the last lower half block
  w = io_stub::written();
  const std::size_t n = io_stub::written_size();
  CHECK_EQ(w[n - 1088U + 2U], 0x80U);
  CHECK_EQ(w[n - 1U], 0x01U);
}


TEST_CASE(heads_SSD1680)
{
  io_stub::clear();
  head::SSD1680<122, 250> epd(nullptr);
  epd.init();
  // software reset (BUSY not readable: fallback delay), gate driver 250 lines, data entry, RAM window, border, update control, sensor
  const std::uint8_t* w = io_stub::written();
  REQUIRE(io_stub::written_size() == 22U);
  CHECK(io_stub::delay_total() >= 3010U);
  CHECK_EQ(w[0], 0x12U);
  CHECK_EQ(w[1], 0x01U); CHECK_EQ(w[2], 0xF9U); CHECK_EQ(w[3], 0x00U); CHECK_EQ(w[4], 0x00U);
  CHECK_EQ(w[5], 0x11U); CHECK_EQ(w[6], 0x03U);
  CHECK_EQ(w[7], 0x44U); CHECK_EQ(w[8], 0x00U); CHECK_EQ(w[9], 0x0FU);
  CHECK_EQ(w[10], 0x45U); CHECK_EQ(w[13], 0xF9U); CHECK_EQ(w[14], 0x00U);
  CHECK_EQ(w[15], 0x3CU); CHECK_EQ(w[16], 0x05U);
  CHECK_EQ(w[17], 0x21U); CHECK_EQ(w[18], 0x00U); CHECK_EQ(w[19], 0x80U);
  CHECK_EQ(w[20], 0x18U); CHECK_EQ(w[21], 0x80U);
  // present: counters, 4000 byte white frame, update sequence F7h, master activation, busy wait
  dc d(epd);
  d.present_lock();
  io_stub::clear();
  d.present_lock(false);
  w = io_stub::written();
  REQUIRE(io_stub::written_size() == 6U + 4000U + 3U);
  CHECK_EQ(w[0], 0x4EU); CHECK_EQ(w[2], 0x4FU); CHECK_EQ(w[5], 0x24U); CHECK_EQ(w[6], 0xFFU);
  CHECK_EQ(w[4006], 0x22U); CHECK_EQ(w[4007], 0xF7U); CHECK_EQ(w[4008], 0x20U);
  CHECK_EQ(io_stub::delay_total(), 3000U);
  // nothing changed: no refresh
  io_stub::clear();
  d.present_lock();
  d.present_lock(false);
  CHECK_EQ(io_stub::written_size(), 0U);
  // black ink clears the bit, white paper sets it, color::none is paper
  d.set_color(color::black);
  io_stub::clear();
  d.plot({ 0, 0 });
  CHECK_EQ(epd.framebuffer()[0], 0x7FU);
  CHECK_EQ(d.pixel_get({ 0, 0 }), color::black);
  CHECK_EQ(d.pixel_get({ 1, 0 }), color::white);
  CHECK_EQ(io_stub::written()[6], 0x7FU);
  // the BUSY pin can be polled: one read when it reports idle
  static const std::uint8_t idle[1] = { 0x00U };
  io_stub::clear();
  io_stub::set_read_data(idle, 1U);
  d.plot({ 1, 1 });
  CHECK_EQ(io_stub::read_calls(), 1U);
  CHECK_EQ(io_stub::delay_total(), 0U);

  // tri color: red plane, red pixels are white in the black/white plane
  head::SSD1680<122, 250, drv::orientation_0, true> tri(nullptr);
  tri.init();
  dc dt(tri);
  dt.set_color(color::red);
  io_stub::clear();
  dt.plot({ 0, 0 });
  CHECK_EQ(dt.pixel_get({ 0, 0 }), color::red);
  CHECK_EQ(tri.framebuffer()[0], 0xFFU);
  REQUIRE(io_stub::written_size() == 6U + 4000U + 6U + 4000U + 3U);
  CHECK_EQ(io_stub::written()[4006U + 5U], 0x26U);
  CHECK_EQ(io_stub::written()[4006U + 6U], 0x80U);

  // 1.54 inch 200 x 200 panel: all 200 sources, X range 25 bytes
  head::SSD1680<200, 200> square(nullptr);
  io_stub::clear();
  square.init();
  CHECK_EQ(io_stub::written()[9], 0x18U);
  CHECK_EQ(io_stub::written()[19], 0x00U);

  // landscape 2.13 inch: logical (0, 0) is source 121, gate 0
  head::SSD1680<250, 122, drv::orientation_90> land(nullptr);
  land.init();
  dc dn(land);
  dn.set_color(color::black);
  dn.plot({ 0, 0 });
  CHECK_EQ(land.framebuffer()[15], 0xBFU);
}


TEST_CASE(heads_SSD1322)
{
  io_stub::clear();
  head::SSD1322<256, 64> oled(nullptr);
  oled.init();
  const std::uint8_t* w = io_stub::written();
  REQUIRE(io_stub::written_size() == 40U + 7U + 8192U);
  CHECK_EQ(w[0], 0xFDU); CHECK_EQ(w[1], 0x12U); CHECK_EQ(w[2], 0xAEU); CHECK_EQ(w[3], 0xB3U);
  CHECK_EQ(w[11], 0xA0U); CHECK_EQ(w[12], 0x04U); CHECK_EQ(w[13], 0x11U);
  CHECK_EQ(w[40], 0x15U); CHECK_EQ(w[41], 0x1CU); CHECK_EQ(w[42], 0x5BU);
  CHECK_EQ(w[43], 0x75U); CHECK_EQ(w[45], 0x3FU); CHECK_EQ(w[46], 0x5CU);
  dc d(oled);
  d.set_color(color::white);
  d.plot({ 0, 0 });
  d.set_color(color::argb(0x80U, 0x80U, 0x80U));
  d.plot({ 1, 0 });
  CHECK_EQ(oled.framebuffer()[0], 0xF8U);
  CHECK_EQ(d.pixel_get({ 0, 0 }), color::white);
  CHECK_EQ(d.pixel_get({ 1, 0 }), color::L4_to_color(8U));
  io_stub::clear();
  oled.display_contrast(0x80U);
  oled.display_brightness(0xFFU);
  CHECK_EQ(io_stub::written()[0], 0xC1U);
  CHECK_EQ(io_stub::written()[2], 0xC7U);
  CHECK_EQ(io_stub::written()[3], 0x0FU);
}


TEST_CASE(heads_SSD1351)
{
  io_stub::clear();
  head::SSD1351<128, 128> oled(nullptr);
  oled.init();
  const std::uint8_t* w = io_stub::written();
  CHECK_EQ(w[0], 0xFDU); CHECK_EQ(w[1], 0x12U); CHECK_EQ(w[2], 0xFDU); CHECK_EQ(w[3], 0xB1U); CHECK_EQ(w[4], 0xAEU);
  CHECK_EQ(w[9], 0xA0U); CHECK_EQ(w[10], 0x64U);     // 65k, COM split, color swap
  dc d(oled);
  d.set_color(color::red);
  io_stub::clear();
  d.plot({ 3, 4 });
  w = io_stub::written();
  REQUIRE(io_stub::written_size() == 9U);
  CHECK_EQ(w[0], 0x15U); CHECK_EQ(w[1], 0x03U); CHECK_EQ(w[2], 0x7FU);
  CHECK_EQ(w[3], 0x75U); CHECK_EQ(w[4], 0x04U); CHECK_EQ(w[5], 0x7FU);
  CHECK_EQ(w[6], 0x5CU); CHECK_EQ(w[7], 0xF8U); CHECK_EQ(w[8], 0x00U);
  io_stub::clear();
  d.plot({ 4, 4 });
  CHECK_EQ(io_stub::written_size(), 2U);
  CHECK_EQ(d.pixel_get({ 3, 4 }), color::none);
  // the common modules: orientation_0m gives the reference remap value 74h
  head::SSD1351<128, 128, drv::orientation_0m> mod(nullptr);
  io_stub::clear();
  mod.init();
  CHECK_EQ(io_stub::written()[10], 0x74U);
}
