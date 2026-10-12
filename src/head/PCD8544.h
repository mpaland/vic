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
// \brief PCD8544 head, 84 x 48 monochrome LCD controller (Nokia 5110 / 3310 displays)
// The RAM is organized in 6 banks of 8 vertical pixels (bit 0 = top row), the
// head renders into a RAM framebuffer and sends it with present() in horizontal
// addressing mode (one 504 byte block). All orientations are done in software.
// See doc/PCD8544.md for the programming summary.
//
// io layer convention: io::write() option 0 = command, 1 = data (the D/C line),
// SPI mode 0, max. 4 MHz. The controller has no read back.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_PCD8544_H_
#define _VIC_HEAD_PCD8544_H_

#include "../drv.h"


// defines the head name and version
#define VIC_HEAD_PCD8544_VERSION   "PCD8544 head 1.00"

namespace vic {
namespace head {


/**
 * PCD8544 display controller
 * The screen size is the LOGICAL size and must match the orientation: 84 x 48 for orientation_0 / 180,
 * 48 x 84 for orientation_90 / 270.
 * \param Screen_Size_X Logical screen width in pixel
 * \param Screen_Size_Y Logical screen height in pixel
 * \param Orientation Orientation of the display (done in software)
 * \param Vop Operation voltage (contrast) 0 .. 127, VLCD = 3.06 V + Vop x 0.06 V, 0x38 is a typical start value
 *            (Adafruit_PCD8544 uses 40 .. 60, the Sparkfun examples 0x38 .. 0x3F)
 * \param Bias Bias system 0 .. 7 (mux rate 1:100 .. 1:10), 4 as in Adafruit_PCD8544 (the datasheet table says 3 = 1:48)
 */
template<std::uint16_t Screen_Size_X = 84U, std::uint16_t Screen_Size_Y = 48U,   // X, Y size, must match the orientation
         drv::orientation_type Orientation = drv::orientation_0,                  // orientation of the display
         std::uint8_t Vop  = 0x38U,                                               // operation voltage (contrast)
         std::uint8_t Bias = 0x04U                                                // bias system
>
class PCD8544 final : public drv
{
  // commands (H = 0 or 1)
  static const std::uint8_t CMD_FUNCTION_SET        = 0x20U;    // | PD (04h) | V (02h) | H (01h)
  static const std::uint8_t FUNC_POWER_DOWN         = 0x04U;
  static const std::uint8_t FUNC_VERTICAL           = 0x02U;
  static const std::uint8_t FUNC_EXTENDED           = 0x01U;
  // basic instruction set (H = 0)
  static const std::uint8_t CMD_DISPLAY_CONTROL     = 0x08U;    // | D (04h) | E (01h): 08h blank, 0Ch normal, 09h all on, 0Dh inverse
  static const std::uint8_t DISPLAY_NORMAL          = 0x04U;
  static const std::uint8_t DISPLAY_INVERSE         = 0x05U;
  static const std::uint8_t CMD_SET_Y               = 0x40U;    // | bank 0 .. 5
  static const std::uint8_t CMD_SET_X               = 0x80U;    // | column 0 .. 83
  // extended instruction set (H = 1)
  static const std::uint8_t CMD_TEMP_CONTROL        = 0x04U;    // | TC 0 .. 3
  static const std::uint8_t CMD_BIAS_SYSTEM         = 0x10U;    // | BS 0 .. 7
  static const std::uint8_t CMD_SET_VOP             = 0x80U;    // | Vop 0 .. 127

  // physical panel
  static const bool          ROTATED = (Orientation == orientation_90)  || (Orientation == orientation_270) ||
                                       (Orientation == orientation_90m) || (Orientation == orientation_270m);
  static const std::uint16_t PHYS_X  = ROTATED ? Screen_Size_Y : Screen_Size_X;   // 84 columns
  static const std::uint16_t PHYS_Y  = ROTATED ? Screen_Size_X : Screen_Size_Y;   // 48 rows
  static const std::size_t   BANKS   = PHYS_Y / 8U;
  static const std::size_t   FB_SIZE = BANKS * PHYS_X;

  static_assert(PHYS_Y % 8U == 0U, "PCD8544: the physical height must be a multiple of 8");

  // vars
  const io::handle_type device_handle_;
  std::uint8_t          framebuffer_[FB_SIZE];    // bank organized: data[bank * PHYS_X + x], bit (y & 7)

public:

  /**
   * ctor
   * \param device_handle Device handle
   */
  PCD8544(io::handle_type device_handle)
    : drv(Screen_Size_X, Screen_Size_Y,
          Screen_Size_X, Screen_Size_Y,         // no viewport support (screen = viewport)
          0, 0,                                 // no viewport support
          Orientation)
    , device_handle_(device_handle)
  {
    for (std::size_t i = 0U; i < FB_SIZE; ++i) {
      framebuffer_[i] = 0U;
    }
  }


  ~PCD8544()
  {
    shutdown();
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S

  virtual void init()
  {
    io::init(device_handle_);

    // init sequence of the Adafruit_PCD8544 driver, the Nokia 5110 modules are upright with orientation_0
    const std::uint8_t cmd[] = {
      static_cast<std::uint8_t>(CMD_FUNCTION_SET | FUNC_EXTENDED),      // extended instruction set, chip active, horizontal addressing
      static_cast<std::uint8_t>(CMD_SET_VOP | (Vop & 0x7FU)),           // contrast
      static_cast<std::uint8_t>(CMD_TEMP_CONTROL | 0x00U),              // temperature coefficient 0
      static_cast<std::uint8_t>(CMD_BIAS_SYSTEM | (Bias & 0x07U)),      // bias
      CMD_FUNCTION_SET,                                                 // basic instruction set
      static_cast<std::uint8_t>(CMD_DISPLAY_CONTROL | DISPLAY_NORMAL)   // normal display
    };
    write_cmd(cmd, sizeof(cmd));

    cls();
    present();
  }


  virtual void shutdown()
  {
    // blank display and power down
    const std::uint8_t cmd[] = { CMD_DISPLAY_CONTROL, static_cast<std::uint8_t>(CMD_FUNCTION_SET | FUNC_POWER_DOWN) };
    write_cmd(cmd, sizeof(cmd));
  }


  inline virtual const char* version() const
  {
    return (const char*)VIC_HEAD_PCD8544_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    return true;
  }


  /**
   * Access the framebuffer (bank organized, physical orientation)
   */
  inline const std::uint8_t* framebuffer() const
  {
    return framebuffer_;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

protected:

  virtual void cls(color::value_type bg_color = color::none)
  {
    const std::uint8_t fill = color::color_to_L1(bg_color) ? 0xFFU : 0x00U;
    for (std::size_t i = 0U; i < FB_SIZE; ++i) {
      framebuffer_[i] = fill;
    }
  }


  /**
   * Copy the framebuffer to the display: address (0, 0), then all banks in one block
   */
  virtual void present()
  {
    const std::uint8_t cmd[] = { CMD_SET_Y, CMD_SET_X };
    write_cmd(cmd, sizeof(cmd));
    io::write(device_handle_, 1U, framebuffer_, FB_SIZE, nullptr, 0U);
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
    std::uint8_t& data = framebuffer_[static_cast<std::size_t>(p.y >> 3U) * PHYS_X + static_cast<std::size_t>(p.x)];
    const std::uint8_t mask = static_cast<std::uint8_t>(0x01U << (static_cast<unsigned>(p.y) & 0x07U));
    data = static_cast<std::uint8_t>(color::color_to_L1(color) ? (data | mask) : (data & ~mask));
  }


  virtual color::value_type pixel_get(vertex_type vertex)
  {
    if (!screen_is_inside(vertex)) {
      return color::none;
    }
    const vertex_type p = to_phys(vertex);
    const std::uint8_t data = framebuffer_[static_cast<std::size_t>(p.y >> 3U) * PHYS_X + static_cast<std::size_t>(p.x)];
    return color::L1_to_color(static_cast<std::uint8_t>((data >> (static_cast<unsigned>(p.y) & 0x07U)) & 0x01U));
  }


  ///////////////////////////////////////////////////////////////////////////////
  // D I S P L A Y   C O N T R O L
  //

public:

  /**
   * Enable/disable the display (display control normal / blank), the RAM is retained
   */
  virtual void display_enable(bool enable = true)
  {
    const std::uint8_t cmd = static_cast<std::uint8_t>(CMD_DISPLAY_CONTROL | (enable ? DISPLAY_NORMAL : 0U));
    write_cmd(&cmd, 1U);
  }


  /**
   * Set the contrast (operation voltage Vop)
   * \param level 0: minimum; 255: maximum (Vop = level / 2)
   */
  virtual void display_contrast(std::uint8_t level)
  {
    const std::uint8_t cmd[] = { static_cast<std::uint8_t>(CMD_FUNCTION_SET | FUNC_EXTENDED),
                                 static_cast<std::uint8_t>(CMD_SET_VOP | (level >> 1U)),
                                 CMD_FUNCTION_SET };
    write_cmd(cmd, sizeof(cmd));
  }


  /**
   * Inverse video
   */
  void display_inverse(bool inverse)
  {
    const std::uint8_t cmd = static_cast<std::uint8_t>(CMD_DISPLAY_CONTROL | (inverse ? DISPLAY_INVERSE : DISPLAY_NORMAL));
    write_cmd(&cmd, 1U);
  }


private:

  /**
   * Map a logical screen vertex to the physical panel, the 'm' orientations mirror the panel top/bottom after the rotation
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


  inline void write_cmd(const std::uint8_t* cmd, std::size_t length) const
  {
    io::write(device_handle_, 0U, cmd, length, nullptr, 0U);
  }

};

} // namespace head
} // namespace vic

#endif  // _VIC_HEAD_PCD8544_H_
