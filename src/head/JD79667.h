///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// This file is part of the vic library.
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// \brief JD79667 head, black/white/yellow/red e-paper display controller
// (up to 200 x 384, e.g. 2.66" 184 x 360, 2.9" 168 x 384, 1.54" 200 x 200 panels).
// The head renders into a RAM framebuffer (2 bit per pixel: 00 black, 01 white,
// 10 yellow, 11 red, bits 7:6 = left pixel) and present() uploads it and starts
// a full refresh with the waveform LUT from the MTP, which takes 12 .. 25 s.
// present() is skipped when nothing changed since the last refresh; use
// dc::present_lock() to batch primitives into one refresh.
// Horizontal and vertical lines (and with them boxes and filled primitives) are
// written directly into the framebuffer.
// All orientations are done in software. See doc/JD79667.md for the programming
// summary.
//
// io layer convention: io::write() option 0 = command, 1 = data (the D/C line);
// io::read() option 2 returns one byte with bit 0 = 1 while the controller is
// busy (BUSY_N pin low, so the inverted pin level), the head polls it after reset,
// power on / off and refresh. The hardware reset (RST_N low) is done by io::init().
// An io layer without BUSY read returns false, the head then waits a fixed time
// (Refresh_Delay ms after a refresh).
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_JD79667_H_
#define _VIC_HEAD_JD79667_H_

#include "../drv.h"


// defines the head name and version
#define VIC_HEAD_JD79667_VERSION   "JD79667 head 1.00"

namespace vic {
namespace head {


/**
 * JD79667 four color e-paper display controller
 * The screen size is the LOGICAL size and must match the orientation. The physical panel is
 * sources (X, max. 200, multiple of 4) x gates (Y, max. 384), the landscape use of a 200 x 384 panel
 * is 384 x 200 with orientation_90 or 270.
 * \param Screen_Size_X Logical screen width in pixel
 * \param Screen_Size_Y Logical screen height in pixel
 * \param Orientation Orientation of the display (done in software)
 * \param Refresh_Delay Delay in ms used instead of the BUSY poll after a refresh when the io layer can't read the BUSY_N pin
 */
template<std::uint16_t Screen_Size_X = 200U, std::uint16_t Screen_Size_Y = 384U,   // X, Y size, must match the orientation
         drv::orientation_type Orientation = drv::orientation_0,                    // orientation of the display
         std::uint16_t Refresh_Delay = 25000U                                      // ms, fallback for the BUSY poll
>
class JD79667 final : public drv
{
  // commands
  static const std::uint8_t CMD_PANEL_SETTING       = 0x00U;    // PSR: resolution, scan directions, LUT source, VCOM / end options
  static const std::uint8_t CMD_POWER_SETTING       = 0x01U;    // PWR: internal / external supplies, gate voltage
  static const std::uint8_t CMD_POWER_OFF           = 0x02U;    // POF
  static const std::uint8_t CMD_POWER_OFF_SEQUENCE  = 0x03U;    // POFS (not in the datasheet)
  static const std::uint8_t CMD_POWER_ON            = 0x04U;    // PON
  static const std::uint8_t CMD_BOOSTER_SOFT_START  = 0x06U;    // BTST
  static const std::uint8_t CMD_DEEP_SLEEP          = 0x07U;    // DSLP, check code A5h, hardware reset to leave
  static const std::uint8_t CMD_DATA_START          = 0x10U;    // DTM: frame data, 2 bit per pixel
  static const std::uint8_t CMD_DISPLAY_REFRESH     = 0x12U;    // DRF
  static const std::uint8_t CMD_PLL_CONTROL         = 0x30U;    // PLL: frame rate
  static const std::uint8_t CMD_VCOM_DATA_INTERVAL  = 0x50U;    // CDI: border color, data polarity, VCOM to data interval
  static const std::uint8_t CMD_TCON                = 0x60U;    // TCON (not in the datasheet)
  static const std::uint8_t CMD_RESOLUTION          = 0x61U;    // TRES: sources, gates
  static const std::uint8_t CMD_POWER_SAVING        = 0xE3U;    // PWS

  // pixel codes, defined by the waveform LUT of the panel
  static const std::uint8_t PIXEL_BLACK             = 0x00U;
  static const std::uint8_t PIXEL_WHITE             = 0x01U;
  static const std::uint8_t PIXEL_YELLOW            = 0x02U;
  static const std::uint8_t PIXEL_RED               = 0x03U;

  // io options
  static const std::uint32_t OPT_COMMAND            = 0U;
  static const std::uint32_t OPT_DATA               = 1U;
  static const std::uint32_t OPT_BUSY               = 2U;

  static const std::uint32_t POWER_DELAY            = 200U;     // ms, fallback for the BUSY poll after reset, power on and power off

  // physical panel
  static const bool          ROTATED = (Orientation == orientation_90)  || (Orientation == orientation_270) ||
                                       (Orientation == orientation_90m) || (Orientation == orientation_270m);
  static const std::uint16_t PHYS_X  = ROTATED ? Screen_Size_Y : Screen_Size_X;   // sources
  static const std::uint16_t PHYS_Y  = ROTATED ? Screen_Size_X : Screen_Size_Y;   // gates
  static const std::size_t   BYTES_X = PHYS_X / 4U;
  static const std::size_t   FB_SIZE = BYTES_X * PHYS_Y;

  static_assert((PHYS_X <= 200U) && (PHYS_Y <= 384U), "JD79667: the panel exceeds the frame memory (200 sources x 384 gates)");
  static_assert((PHYS_X % 4U) == 0U, "JD79667: the horizontal resolution (sources) must be a multiple of 4");

  // vars
  const io::handle_type device_handle_;
  std::uint8_t          fb_[FB_SIZE];                       // 2 bit per pixel, bits 7:6 = left pixel
  bool                  dirty_;                             // framebuffer changed since the last refresh

public:

  /**
   * ctor
   * \param device_handle Device handle
   */
  JD79667(io::handle_type device_handle)
    : drv(Screen_Size_X, Screen_Size_Y,
          Screen_Size_X, Screen_Size_Y,         // no viewport support (screen = viewport)
          0, 0,                                 // no viewport support
          Orientation)
    , device_handle_(device_handle)
    , dirty_(true)
  {
    for (std::size_t i = 0U; i < FB_SIZE; ++i) {
      fb_[i] = 0x55U;
    }
  }


  ~JD79667()
  {
    shutdown();
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S

  /**
   * Init: the datasheet has no init sequence, this is the common sequence of the GxEPD2, Waveshare and Adafruit
   * drivers (crosscheck against the datasheet in doc/JD79667.md). The power stays on until shutdown().
   * The screen is not refreshed, call present() after drawing.
   */
  virtual void init()
  {
    io::init(device_handle_);           // includes the hardware reset
    io::delay(10U);
    wait_busy(POWER_DELAY);

    const std::uint8_t r4d[1]  = { 0x78U };
    const std::uint8_t psr[2]  = { 0x0FU, 0x29U };              // scan up, shift right, booster on; LUT from MTP, no scan after the waveform
    const std::uint8_t pwr[2]  = { 0x07U, 0x00U };              // internal gate / source supplies, VGP / VGN +-20 V
    const std::uint8_t pofs[3] = { 0x10U, 0x54U, 0x44U };
    const std::uint8_t btst[7] = { 0x05U, 0x00U, 0x3FU, 0x0AU, 0x25U, 0x12U, 0x1AU };
    const std::uint8_t cdi[1]  = { 0x37U };                     // white border, 10 hsync
    const std::uint8_t tcon[2] = { 0x02U, 0x02U };
    const std::uint8_t tres[4] = { static_cast<std::uint8_t>(PHYS_X >> 8U), static_cast<std::uint8_t>(PHYS_X),
                                   static_cast<std::uint8_t>(PHYS_Y >> 8U), static_cast<std::uint8_t>(PHYS_Y) };
    const std::uint8_t re7[1]  = { 0x1CU };
    const std::uint8_t pws[1]  = { 0x22U };
    const std::uint8_t rb4[1]  = { 0xD0U };
    const std::uint8_t rb5[1]  = { 0x03U };
    const std::uint8_t re9[1]  = { 0x01U };
    const std::uint8_t pll[1]  = { 0x08U };                     // dynamic frame rate
    write_cmd(0x4DU,                  r4d,  1U);
    write_cmd(CMD_PANEL_SETTING,      psr,  2U);
    write_cmd(CMD_POWER_SETTING,      pwr,  2U);
    write_cmd(CMD_POWER_OFF_SEQUENCE, pofs, 3U);
    write_cmd(CMD_BOOSTER_SOFT_START, btst, 7U);
    write_cmd(CMD_VCOM_DATA_INTERVAL, cdi,  1U);
    write_cmd(CMD_TCON,               tcon, 2U);
    write_cmd(CMD_RESOLUTION,         tres, 4U);
    write_cmd(0xE7U,                  re7,  1U);
    write_cmd(CMD_POWER_SAVING,       pws,  1U);
    write_cmd(0xB4U,                  rb4,  1U);
    write_cmd(0xB5U,                  rb5,  1U);
    write_cmd(0xE9U,                  re9,  1U);
    write_cmd(CMD_PLL_CONTROL,        pll,  1U);
    write_cmd(CMD_POWER_ON,           nullptr, 0U);
    wait_busy(POWER_DELAY);

    cls();
    dirty_ = true;
  }


  /**
   * Shutdown: power off and deep sleep, a hardware reset (io::init) wakes the controller up
   */
  virtual void shutdown()
  {
    const std::uint8_t off[1]   = { 0x00U };
    const std::uint8_t sleep[1] = { 0xA5U };
    write_cmd(CMD_POWER_OFF, off, 1U);
    wait_busy(POWER_DELAY);
    write_cmd(CMD_DEEP_SLEEP, sleep, 1U);
  }


  inline virtual const char* version() const
  {
    return (const char*)VIC_HEAD_JD79667_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    return true;
  }


  /**
   * Access the framebuffer (row organized, physical orientation, 2 bit per pixel, bits 7:6 = left pixel)
   */
  inline const std::uint8_t* framebuffer() const
  {
    return fb_;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

protected:

  /**
   * Clear the framebuffer, color::none (the default) is white paper
   */
  virtual void cls(color::value_type bg_color = color::none)
  {
    const std::uint8_t fill = static_cast<std::uint8_t>(to_pixel(bg_color) * 0x55U);
    for (std::size_t i = 0U; i < FB_SIZE; ++i) {
      fb_[i] = fill;
    }
    dirty_ = true;
  }


  /**
   * Upload the framebuffer and refresh the panel (full update, 12 .. 25 s), only if something changed
   */
  virtual void present()
  {
    if (!dirty_) {
      return;
    }
    dirty_ = false;

    const std::uint8_t refresh[1] = { 0x00U };
    write_cmd(CMD_DATA_START, fb_, FB_SIZE);
    write_cmd(CMD_DISPLAY_REFRESH, refresh, 1U);
    wait_busy(Refresh_Delay);
  }


  ///////////////////////////////////////////////////////////////////////////////
  // G R A P H I C   F U N C T I O N S
  //

  virtual void pixel_set(vertex_type vertex, color::value_type color)
  {
    if (!screen_is_inside(vertex)) {
      return;
    }
    const vertex_type p = to_phys(vertex);
    const std::size_t idx   = static_cast<std::size_t>(p.y) * BYTES_X + static_cast<std::size_t>(p.x >> 2U);
    const unsigned    shift = 6U - ((static_cast<unsigned>(p.x) & 0x03U) << 1U);
    fb_[idx] = static_cast<std::uint8_t>((fb_[idx] & ~(0x03U << shift)) | (static_cast<unsigned>(to_pixel(color)) << shift));
    dirty_ = true;
  }


  virtual color::value_type pixel_get(vertex_type vertex)
  {
    if (!screen_is_inside(vertex)) {
      return color::none;
    }
    const vertex_type p = to_phys(vertex);
    const std::size_t idx   = static_cast<std::size_t>(p.y) * BYTES_X + static_cast<std::size_t>(p.x >> 2U);
    const unsigned    shift = 6U - ((static_cast<unsigned>(p.x) & 0x03U) << 1U);
    switch ((fb_[idx] >> shift) & 0x03U) {
      case PIXEL_BLACK  : return color::black;
      case PIXEL_YELLOW : return color::yellow;
      case PIXEL_RED    : return color::red;
      default           : return color::white;
    }
  }


  /**
   * Horizontal line, written directly into the framebuffer (box() and the filled primitives use it)
   */
  virtual void line_horz(vertex_type v0, vertex_type v1, color::value_type color)
  {
    util::min_max_swap(v0.x, v1.x);
    if ((v0.y < 0) || (v0.y >= static_cast<std::int16_t>(Screen_Size_Y)) || (v1.x < 0) || (v0.x >= static_cast<std::int16_t>(Screen_Size_X))) {
      return;
    }
    if (v0.x < 0) { v0.x = 0; }
    if (v1.x >= static_cast<std::int16_t>(Screen_Size_X)) { v1.x = static_cast<std::int16_t>(Screen_Size_X - 1U); }
    v1.y = v0.y;
    fill(to_phys(v0), to_phys(v1), to_pixel(color));
  }


  /**
   * Vertical line, written directly into the framebuffer
   */
  virtual void line_vert(vertex_type v0, vertex_type v1, color::value_type color)
  {
    util::min_max_swap(v0.y, v1.y);
    if ((v0.x < 0) || (v0.x >= static_cast<std::int16_t>(Screen_Size_X)) || (v1.y < 0) || (v0.y >= static_cast<std::int16_t>(Screen_Size_Y))) {
      return;
    }
    if (v0.y < 0) { v0.y = 0; }
    if (v1.y >= static_cast<std::int16_t>(Screen_Size_Y)) { v1.y = static_cast<std::int16_t>(Screen_Size_Y - 1U); }
    v1.x = v0.x;
    fill(to_phys(v0), to_phys(v1), to_pixel(color));
  }


private:

  /**
   * Fill an axis aligned run of pixels between two physical vertices (both included, inside the panel)
   * A run along the sources is written as whole bytes, a run along the gates is one pixel per row
   * \param a First physical vertex
   * \param b Second physical vertex, same x or same y as a
   * \param code Pixel code
   */
  void fill(vertex_type a, vertex_type b, std::uint8_t code)
  {
    dirty_ = true;
    if (a.y != b.y) {
      // along the gates
      util::min_max_swap(a.y, b.y);
      const unsigned     shift = 6U - ((static_cast<unsigned>(a.x) & 0x03U) << 1U);
      const std::uint8_t mask  = static_cast<std::uint8_t>(0x03U << shift);
      const std::uint8_t value = static_cast<std::uint8_t>(static_cast<unsigned>(code) << shift);
      std::uint8_t* p = &fb_[static_cast<std::size_t>(a.y) * BYTES_X + static_cast<std::size_t>(a.x >> 2U)];
      for (std::int_fast16_t y = a.y; y <= b.y; ++y, p += BYTES_X) {
        *p = static_cast<std::uint8_t>((*p & ~mask) | value);
      }
      return;
    }
    // along the sources: partial first and last byte, whole bytes in between
    util::min_max_swap(a.x, b.x);
    std::uint8_t* const row    = &fb_[static_cast<std::size_t>(a.y) * BYTES_X];
    const std::uint8_t  value  = static_cast<std::uint8_t>(code * 0x55U);
    const std::size_t   first  = static_cast<std::size_t>(a.x >> 2U);
    const std::size_t   last   = static_cast<std::size_t>(b.x >> 2U);
    const std::uint8_t  mask_f = static_cast<std::uint8_t>(0xFFU >> ((static_cast<unsigned>(a.x) & 0x03U) << 1U));          // a.x to the end of its byte
    const std::uint8_t  mask_l = static_cast<std::uint8_t>(0xFFU << (6U - ((static_cast<unsigned>(b.x) & 0x03U) << 1U)));   // start of its byte to b.x
    if (first == last) {
      const std::uint8_t mask = static_cast<std::uint8_t>(mask_f & mask_l);
      row[first] = static_cast<std::uint8_t>((row[first] & ~mask) | (value & mask));
      return;
    }
    row[first] = static_cast<std::uint8_t>((row[first] & ~mask_f) | (value & mask_f));
    for (std::size_t i = first + 1U; i < last; ++i) {
      row[i] = value;
    }
    row[last] = static_cast<std::uint8_t>((row[last] & ~mask_l) | (value & mask_l));
  }


  /**
   * Map a color to the pixel code: clearly red and yellow colors to their pigment, other dark colors are
   * black, light colors and color::none are white paper
   */
  static inline std::uint8_t to_pixel(color::value_type c)
  {
    if (c == color::none) {
      return PIXEL_WHITE;
    }
    if ((color::get_red(c) >= 128U) && (color::get_blue(c) < 96U)) {
      if (color::get_green(c) < 96U) {
        return PIXEL_RED;
      }
      if (color::get_green(c) >= 128U) {
        return PIXEL_YELLOW;
      }
    }
    return (color::color_to_L8(c) < 128U) ? PIXEL_BLACK : PIXEL_WHITE;
  }


  /**
   * Map a logical screen vertex to the physical panel (sources x gates), the 'm' orientations mirror the panel
   * top/bottom after the rotation
   */
  static inline vertex_type to_phys(vertex_type v)
  {
    const std::int16_t mx = static_cast<std::int16_t>(PHYS_X - 1U);
    const std::int16_t my = static_cast<std::int16_t>(PHYS_Y - 1U);
    switch (Orientation) {
      case orientation_0    : return { v.x, v.y };
      case orientation_0m   : return { v.x, static_cast<std::int16_t>(my - v.y) };
      case orientation_180  : return { static_cast<std::int16_t>(mx - v.x), static_cast<std::int16_t>(my - v.y) };
      case orientation_180m : return { static_cast<std::int16_t>(mx - v.x), v.y };
      case orientation_90   : return { static_cast<std::int16_t>(mx - v.y), v.x };
      case orientation_90m  : return { static_cast<std::int16_t>(mx - v.y), static_cast<std::int16_t>(my - v.x) };
      case orientation_270  : return { v.y, static_cast<std::int16_t>(my - v.x) };
      case orientation_270m :
      default               : return { v.y, v.x };
    }
  }


  /**
   * Wait until the controller is not busy any more (max. 30 s), with a fixed delay as fallback
   * \param fallback Delay in ms if the io layer can't read the BUSY_N pin
   */
  void wait_busy(std::uint32_t fallback) const
  {
    for (std::uint_fast16_t n = 0U; n < 3000U; ++n) {
      std::uint8_t busy = 0U;
      std::size_t  len  = 1U;
      if (!io::read(device_handle_, OPT_BUSY, &busy, len, 0U) || (len != 1U)) {
        io::delay(fallback);
        return;
      }
      if ((busy & 0x01U) == 0U) {
        return;
      }
      io::delay(10U);
    }
  }


  /**
   * Send a command with its parameters / data: command byte with io option 0, the rest with option 1
   */
  inline void write_cmd(std::uint8_t cmd, const std::uint8_t* data, std::size_t length) const
  {
    io::write(device_handle_, OPT_COMMAND, &cmd, 1U, nullptr, 0U);
    if (length != 0U) {
      io::write(device_handle_, OPT_DATA, data, length, nullptr, 0U);
    }
  }

};

} // namespace head
} // namespace vic

#endif  // _VIC_HEAD_JD79667_H_
