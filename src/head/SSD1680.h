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
// \brief SSD1680 / SSD1681 head, black/white (/red) e-paper display controllers
// (2.13" 122 x 250, 2.9" 128 x 296, 1.54" 200 x 200 panels). The head renders into
// a RAM framebuffer (1 bit per pixel, 1 = white, bit 7 = left pixel) and present()
// uploads it and starts a full refresh with the waveform LUT from the OTP, which
// takes 1 .. 4 s. present() is skipped when nothing changed since the last
// refresh; use dc::present_lock() to batch primitives into one refresh.
// All orientations are done in software. See doc/SSD1680.md for the programming
// summary.
//
// io layer convention: io::write() option 0 = command, 1 = data (the D/C line);
// io::read() option 2 returns one byte with bit 0 = level of the BUSY pin, the
// head polls it after reset and refresh. The hardware reset (RES# low >= 10 us)
// is done by io::init(). An io layer without BUSY read returns false, the head
// then waits Refresh_Delay ms instead.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_SSD1680_H_
#define _VIC_HEAD_SSD1680_H_

#include "../drv.h"


// defines the head name and version
#define VIC_HEAD_SSD1680_VERSION   "SSD1680 head 1.00"

namespace vic {
namespace head {


/**
 * SSD1680 / SSD1681 e-paper display controller
 * The screen size is the LOGICAL size and must match the orientation. The physical panel is
 * sources (X, 122 for 2.13", 128 for 2.9", 200 for 1.54") x gates (Y, 250 / 296 / 200), the usual
 * landscape use of the 2.13" panel is 250 x 122 with orientation_90 or 270.
 * \param Screen_Size_X Logical screen width in pixel
 * \param Screen_Size_Y Logical screen height in pixel
 * \param Orientation Orientation of the display (done in software)
 * \param Tri_Color True for black/white/red panels (second RAM plane)
 * \param Source_S8 Display update control 1: use sources S8 .. S167 (true for the 122 / 128 wide panels, false for 176 / 200 wide);
 *                  the reference drivers (Adafruit_EPD) always send 80h, toggle this if the image is shifted by 8 pixels
 * \param Refresh_Delay Delay in ms used instead of the BUSY poll when the io layer can't read the BUSY pin
 */
template<std::uint16_t Screen_Size_X = 122U, std::uint16_t Screen_Size_Y = 250U,   // X, Y size, must match the orientation
         drv::orientation_type Orientation = drv::orientation_0,                    // orientation of the display
         bool Tri_Color = false,                                                    // black/white/red panel
         bool Source_S8 = ((((Orientation == drv::orientation_90)  || (Orientation == drv::orientation_270) ||
                             (Orientation == drv::orientation_90m) || (Orientation == drv::orientation_270m)) ? Screen_Size_Y : Screen_Size_X) <= 160U),
         std::uint16_t Refresh_Delay = 3000U                                       // ms, fallback for the BUSY poll
>
class SSD1680 final : public drv
{
  // commands
  static const std::uint8_t CMD_DRIVER_OUTPUT       = 0x01U;    // MUX gate lines, scan order
  static const std::uint8_t CMD_SOFT_START          = 0x0CU;    // booster soft start
  static const std::uint8_t CMD_DEEP_SLEEP          = 0x10U;    // 01h / 03h deep sleep, hardware reset to leave
  static const std::uint8_t CMD_DATA_ENTRY          = 0x11U;    // address increment direction
  static const std::uint8_t CMD_SW_RESET            = 0x12U;
  static const std::uint8_t CMD_TEMP_SENSOR         = 0x18U;    // 80h internal sensor
  static const std::uint8_t CMD_MASTER_ACTIVATION   = 0x20U;    // run the update sequence
  static const std::uint8_t CMD_UPDATE_CONTROL1     = 0x21U;    // RAM options, source mode
  static const std::uint8_t CMD_UPDATE_CONTROL2     = 0x22U;    // update sequence, F7h = load temperature, LUT and display
  static const std::uint8_t CMD_WRITE_RAM_BW        = 0x24U;    // black / white RAM, 1 = white
  static const std::uint8_t CMD_WRITE_RAM_RED       = 0x26U;    // red RAM, 1 = red
  static const std::uint8_t CMD_BORDER_WAVEFORM     = 0x3CU;
  static const std::uint8_t CMD_RAM_X_RANGE         = 0x44U;    // start / end in bytes
  static const std::uint8_t CMD_RAM_Y_RANGE         = 0x45U;    // start / end in rows, 16 bit
  static const std::uint8_t CMD_RAM_X_COUNTER       = 0x4EU;
  static const std::uint8_t CMD_RAM_Y_COUNTER       = 0x4FU;
  static const std::uint8_t CMD_NOP                 = 0x7FU;

  // io options
  static const std::uint32_t OPT_COMMAND            = 0U;
  static const std::uint32_t OPT_DATA               = 1U;
  static const std::uint32_t OPT_BUSY               = 2U;

  // physical panel
  static const bool          ROTATED = (Orientation == orientation_90)  || (Orientation == orientation_270) ||
                                       (Orientation == orientation_90m) || (Orientation == orientation_270m);
  static const std::uint16_t PHYS_X  = ROTATED ? Screen_Size_Y : Screen_Size_X;   // sources
  static const std::uint16_t PHYS_Y  = ROTATED ? Screen_Size_X : Screen_Size_Y;   // gates
  static const std::size_t   BYTES_X = (PHYS_X + 7U) / 8U;
  static const std::size_t   FB_SIZE = BYTES_X * PHYS_Y;

  static_assert((PHYS_X <= 200U) && (PHYS_Y <= 296U), "SSD1680: the panel exceeds the RAM (176 x 296 SSD1680, 200 x 200 SSD1681)");

  // vars
  const io::handle_type device_handle_;
  std::uint8_t          bw_[FB_SIZE];                       // 1 = white, bit 7 = left pixel
  std::uint8_t          red_[Tri_Color ? FB_SIZE : 1U];     // 1 = red
  bool                  dirty_;                             // framebuffer changed since the last refresh

public:

  /**
   * ctor
   * \param device_handle Device handle
   */
  SSD1680(io::handle_type device_handle)
    : drv(Screen_Size_X, Screen_Size_Y,
          Screen_Size_X, Screen_Size_Y,         // no viewport support (screen = viewport)
          0, 0,                                 // no viewport support
          Orientation)
    , device_handle_(device_handle)
    , dirty_(true)
  {
    for (std::size_t i = 0U; i < FB_SIZE; ++i) {
      bw_[i] = 0xFFU;
    }
    for (std::size_t i = 0U; i < (Tri_Color ? FB_SIZE : 1U); ++i) {
      red_[i] = 0x00U;
    }
  }


  ~SSD1680()
  {
    shutdown();
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S

  /**
   * Init: software reset, gate driver, RAM window and data entry mode, border, temperature sensor (datasheet
   * section 9, operation flow, parameter values as in Adafruit_EPD). The screen is not refreshed, call present()
   * after drawing.
   */
  virtual void init()
  {
    io::init(device_handle_);           // includes the hardware reset
    io::delay(10U);
    write_cmd(CMD_SW_RESET, nullptr, 0U);
    wait_busy();

    const std::uint8_t gates[3]  = { static_cast<std::uint8_t>(PHYS_Y - 1U), static_cast<std::uint8_t>((PHYS_Y - 1U) >> 8U), 0x00U };
    const std::uint8_t entry[1]  = { 0x03U };                   // X and Y increment, counter along X
    const std::uint8_t xrange[2] = { 0x00U, static_cast<std::uint8_t>(BYTES_X - 1U) };
    const std::uint8_t yrange[4] = { 0x00U, 0x00U, static_cast<std::uint8_t>(PHYS_Y - 1U), static_cast<std::uint8_t>((PHYS_Y - 1U) >> 8U) };
    const std::uint8_t border[1] = { 0x05U };                   // border follows the LUT transition
    const std::uint8_t ctrl1[2]  = { 0x00U, static_cast<std::uint8_t>(Source_S8 ? 0x80U : 0x00U) };
    const std::uint8_t temp[1]   = { 0x80U };                   // internal temperature sensor
    write_cmd(CMD_DRIVER_OUTPUT,   gates,  3U);
    write_cmd(CMD_DATA_ENTRY,      entry,  1U);
    write_cmd(CMD_RAM_X_RANGE,     xrange, 2U);
    write_cmd(CMD_RAM_Y_RANGE,     yrange, 4U);
    write_cmd(CMD_BORDER_WAVEFORM, border, 1U);
    write_cmd(CMD_UPDATE_CONTROL1, ctrl1,  2U);
    write_cmd(CMD_TEMP_SENSOR,     temp,   1U);

    cls();
    dirty_ = true;
  }


  /**
   * Shutdown: deep sleep mode 1 (RAM retained), a hardware reset (io::init) wakes the controller up
   */
  virtual void shutdown()
  {
    const std::uint8_t sleep[1] = { 0x01U };
    write_cmd(CMD_DEEP_SLEEP, sleep, 1U);
  }


  inline virtual const char* version() const
  {
    return (const char*)VIC_HEAD_SSD1680_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    return true;
  }


  /**
   * Access the black/white framebuffer (row organized, physical orientation, 1 = white)
   */
  inline const std::uint8_t* framebuffer() const
  {
    return bw_;
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
    const bool ink = is_ink(bg_color);
    const bool red = Tri_Color && is_red(bg_color);
    for (std::size_t i = 0U; i < FB_SIZE; ++i) {
      bw_[i] = (ink && !red) ? 0x00U : 0xFFU;
    }
    if (Tri_Color) {
      for (std::size_t i = 0U; i < FB_SIZE; ++i) {
        red_[i] = red ? 0xFFU : 0x00U;
      }
    }
    dirty_ = true;
  }


  /**
   * Upload the framebuffer and refresh the panel (full update, 1 .. 4 s), only if something changed
   */
  virtual void present()
  {
    if (!dirty_) {
      return;
    }
    dirty_ = false;

    set_counter();
    write_cmd(CMD_WRITE_RAM_BW, bw_, FB_SIZE);
    if (Tri_Color) {
      set_counter();
      write_cmd(CMD_WRITE_RAM_RED, red_, FB_SIZE);
    }
    const std::uint8_t sequence[1] = { 0xF7U };    // load temperature, load LUT, display mode 1
    write_cmd(CMD_UPDATE_CONTROL2, sequence, 1U);
    write_cmd(CMD_MASTER_ACTIVATION, nullptr, 0U);
    wait_busy();
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
    const std::size_t  idx  = static_cast<std::size_t>(p.y) * BYTES_X + static_cast<std::size_t>(p.x >> 3U);
    const std::uint8_t mask = static_cast<std::uint8_t>(0x80U >> (p.x & 0x07U));
    const bool red = Tri_Color && is_red(color);
    const bool ink = is_ink(color) && !red;
    bw_[idx] = static_cast<std::uint8_t>(ink ? (bw_[idx] & ~mask) : (bw_[idx] | mask));
    if (Tri_Color) {
      red_[idx] = static_cast<std::uint8_t>(red ? (red_[idx] | mask) : (red_[idx] & ~mask));
    }
    dirty_ = true;
  }


  virtual color::value_type pixel_get(vertex_type vertex)
  {
    if (!screen_is_inside(vertex)) {
      return color::none;
    }
    const vertex_type p = to_phys(vertex);
    const std::size_t  idx  = static_cast<std::size_t>(p.y) * BYTES_X + static_cast<std::size_t>(p.x >> 3U);
    const std::uint8_t mask = static_cast<std::uint8_t>(0x80U >> (p.x & 0x07U));
    if (Tri_Color && (red_[idx] & mask)) {
      return color::red;
    }
    return (bw_[idx] & mask) ? color::white : color::black;
  }


private:

  /**
   * Ink (black) if the color is dark, color::none is white paper
   */
  static inline bool is_ink(color::value_type c)
  {
    return (c != color::none) && (color::color_to_L8(c) < 128U);
  }

  /**
   * Red if the color is clearly red
   */
  static inline bool is_red(color::value_type c)
  {
    return (color::get_red(c) >= 128U) && (color::get_green(c) < 96U) && (color::get_blue(c) < 96U);
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
   * Set the RAM address counters to (0, 0)
   */
  inline void set_counter() const
  {
    const std::uint8_t x[1] = { 0x00U };
    const std::uint8_t y[2] = { 0x00U, 0x00U };
    write_cmd(CMD_RAM_X_COUNTER, x, 1U);
    write_cmd(CMD_RAM_Y_COUNTER, y, 2U);
  }


  /**
   * Wait until the BUSY pin is low, with the fixed delay as fallback
   */
  void wait_busy() const
  {
    for (std::uint_fast16_t n = 0U; n < 1000U; ++n) {
      std::uint8_t busy = 0U;
      std::size_t  len  = 1U;
      if (!io::read(device_handle_, OPT_BUSY, &busy, len, 0U) || (len != 1U)) {
        io::delay(Refresh_Delay);
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

#endif  // _VIC_HEAD_SSD1680_H_
