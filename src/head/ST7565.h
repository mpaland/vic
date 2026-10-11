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
// \brief ST7565 / ST7567 / UC1701 head, 132 x 64 monochrome COG LCD controllers (128 x 64 panels)
// The three controllers share the command set (page / column addressing, bit 0 = top
// row of a page). The head renders into a RAM framebuffer and sends it page by page
// with present(). All orientations are done in software, the SEG / COM direction
// commands stay at their defaults. See doc/ST7565.md for the programming summary.
//
// io layer convention: io::write() option 0 = command, 1 = data (the A0 line),
// SPI mode 0 / 3. The serial interfaces can't read back.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_ST7565_H_
#define _VIC_HEAD_ST7565_H_

#include "../drv.h"


// defines the head name and version
#define VIC_HEAD_ST7565_VERSION   "ST7565 head 1.00"

namespace vic {
namespace head {


// controller type
typedef enum tag_ST7565_controller_type {
  ST7565_ST7565,    // Sitronix ST7565R/P, booster ratio command F8h
  ST7565_ST7567,    // Sitronix ST7567, no booster ratio command
  ST7565_UC1701     // UltraChip UC1701, booster ratio command F8h
} ST7565_controller_type;


/**
 * ST7565 / ST7567 / UC1701 display controller
 * The screen size is the LOGICAL size and must match the orientation: 128 x 64 for orientation_0 / 180,
 * 64 x 128 for orientation_90 / 270.
 * \param Controller Controller type
 * \param Screen_Size_X Logical screen width in pixel
 * \param Screen_Size_Y Logical screen height in pixel
 * \param Orientation Orientation of the display (done in software)
 * \param Column_Offset First RAM column of the panel (the RAM has 132 columns, some 128 pixel modules start at 4)
 * \param Contrast Electronic volume 0 .. 63
 * \param Resistor_Ratio V0 regulator resistor ratio 0 .. 7
 * \param Bias_1_7 True for 1/7 bias, false for 1/9 bias
 */
template<ST7565_controller_type Controller = ST7565_ST7565,
         std::uint16_t Screen_Size_X = 128U, std::uint16_t Screen_Size_Y = 64U,  // X, Y size, must match the orientation
         drv::orientation_type Orientation = drv::orientation_0,                  // orientation of the display
         std::uint8_t Column_Offset  = 0U,                                        // first RAM column of the panel
         std::uint8_t Contrast       = 0x20U,                                     // electronic volume
         std::uint8_t Resistor_Ratio = 0x06U,                                     // V0 resistor ratio
         bool Bias_1_7 = false                                                    // 1/7 bias instead of 1/9
>
class ST7565 final : public drv
{
  // commands
  static const std::uint8_t CMD_DISPLAY_OFF         = 0xAEU;
  static const std::uint8_t CMD_DISPLAY_ON          = 0xAFU;
  static const std::uint8_t CMD_START_LINE          = 0x40U;    // | line 0 .. 63
  static const std::uint8_t CMD_PAGE                = 0xB0U;    // | page 0 .. 8
  static const std::uint8_t CMD_COLUMN_HIGH         = 0x10U;    // | upper nibble of the column
  static const std::uint8_t CMD_COLUMN_LOW          = 0x00U;    // | lower nibble of the column
  static const std::uint8_t CMD_SEG_NORMAL          = 0xA0U;    // ADC select normal
  static const std::uint8_t CMD_SEG_REVERSE         = 0xA1U;    // ADC select reverse
  static const std::uint8_t CMD_NORMAL              = 0xA6U;    // normal display
  static const std::uint8_t CMD_INVERSE             = 0xA7U;    // inverse display
  static const std::uint8_t CMD_ALL_POINTS_OFF      = 0xA4U;
  static const std::uint8_t CMD_ALL_POINTS_ON       = 0xA5U;
  static const std::uint8_t CMD_BIAS_1_9            = 0xA2U;
  static const std::uint8_t CMD_BIAS_1_7            = 0xA3U;
  static const std::uint8_t CMD_RESET               = 0xE2U;    // software reset
  static const std::uint8_t CMD_COM_NORMAL          = 0xC0U;    // COM output scan direction normal
  static const std::uint8_t CMD_COM_REVERSE         = 0xC8U;    // COM output scan direction reverse
  static const std::uint8_t CMD_POWER_CONTROL       = 0x28U;    // | booster (04h) | regulator (02h) | follower (01h)
  static const std::uint8_t CMD_RESISTOR_RATIO      = 0x20U;    // | ratio 0 .. 7
  static const std::uint8_t CMD_VOLUME_MODE         = 0x81U;    // electronic volume, followed by 0 .. 63
  static const std::uint8_t CMD_BOOSTER_RATIO       = 0xF8U;    // booster ratio, followed by 0 (2x..4x), 1 (5x), 3 (6x)
  static const std::uint8_t CMD_NOP                 = 0xE3U;

  // physical panel
  static const bool          ROTATED = (Orientation == orientation_90)  || (Orientation == orientation_270) ||
                                       (Orientation == orientation_90m) || (Orientation == orientation_270m);
  static const std::uint16_t PHYS_X  = ROTATED ? Screen_Size_Y : Screen_Size_X;   // columns
  static const std::uint16_t PHYS_Y  = ROTATED ? Screen_Size_X : Screen_Size_Y;   // rows
  static const std::size_t   PAGES   = PHYS_Y / 8U;
  static const std::size_t   FB_SIZE = PAGES * PHYS_X;

  static_assert(PHYS_Y % 8U == 0U, "ST7565: the physical height must be a multiple of 8");
  static_assert(PHYS_X + Column_Offset <= 132U, "ST7565: the panel exceeds the 132 RAM columns");

  // vars
  const io::handle_type device_handle_;
  std::uint8_t          framebuffer_[FB_SIZE];    // page organized: data[page * PHYS_X + x], bit (y & 7)

public:

  /**
   * ctor
   * \param device_handle Device handle
   */
  ST7565(io::handle_type device_handle)
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


  ~ST7565()
  {
    shutdown();
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S

  virtual void init()
  {
    io::init(device_handle_);

    // power up sequence of the Adafruit ST7565 / u8g2 drivers, SEG and COM direction stay at the datasheet default
    // (orientation is done in software), the DOGM128 / ERC12864 modules are upright with orientation_180 or 0m
    write_cmd(CMD_RESET);
    io::delay(5U);
    write_cmd(Bias_1_7 ? CMD_BIAS_1_7 : CMD_BIAS_1_9);
    write_cmd(CMD_SEG_NORMAL);
    write_cmd(CMD_COM_NORMAL);
    write_cmd(CMD_START_LINE);
    if (Controller != ST7565_ST7567) {
      // booster ratio 4x
      const std::uint8_t cmd[] = { CMD_BOOSTER_RATIO, 0x00U };
      write_cmd(cmd, sizeof(cmd));
    }
    // power supply: booster, then regulator, then follower, with settling time
    write_cmd(static_cast<std::uint8_t>(CMD_POWER_CONTROL | 0x04U));
    io::delay(50U);
    write_cmd(static_cast<std::uint8_t>(CMD_POWER_CONTROL | 0x06U));
    io::delay(50U);
    write_cmd(static_cast<std::uint8_t>(CMD_POWER_CONTROL | 0x07U));
    io::delay(10U);
    write_cmd(static_cast<std::uint8_t>(CMD_RESISTOR_RATIO | (Resistor_Ratio & 0x07U)));
    const std::uint8_t volume[] = { CMD_VOLUME_MODE, static_cast<std::uint8_t>(Contrast & 0x3FU) };
    write_cmd(volume, sizeof(volume));
    write_cmd(CMD_NORMAL);
    write_cmd(CMD_ALL_POINTS_OFF);
    write_cmd(CMD_DISPLAY_ON);

    cls();
    present();
  }


  virtual void shutdown()
  {
    // display off
    write_cmd(CMD_DISPLAY_OFF);
  }


  inline virtual const char* version() const
  {
    return (const char*)VIC_HEAD_ST7565_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    return true;
  }


  /**
   * Access the framebuffer (page organized, physical orientation)
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
   * Copy the framebuffer to the display page by page
   */
  virtual void present()
  {
    for (std::size_t page = 0U; page < PAGES; ++page) {
      const std::uint8_t cmd[] = { static_cast<std::uint8_t>(CMD_PAGE | page),
                                   static_cast<std::uint8_t>(CMD_COLUMN_HIGH | (Column_Offset >> 4U)),
                                   static_cast<std::uint8_t>(CMD_COLUMN_LOW  | (Column_Offset & 0x0FU)) };
      write_cmd(cmd, sizeof(cmd));
      io::write(device_handle_, 1U, &framebuffer_[page * PHYS_X], PHYS_X, nullptr, 0U);
    }
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
    const std::uint8_t mask = static_cast<std::uint8_t>(0x01U << (p.y & 0x07U));
    data = static_cast<std::uint8_t>(color::color_to_L1(color) ? (data | mask) : (data & ~mask));
  }


  virtual color::value_type pixel_get(vertex_type vertex)
  {
    if (!screen_is_inside(vertex)) {
      return color::none;
    }
    const vertex_type p = to_phys(vertex);
    const std::uint8_t data = framebuffer_[static_cast<std::size_t>(p.y >> 3U) * PHYS_X + static_cast<std::size_t>(p.x)];
    return color::L1_to_color(static_cast<std::uint8_t>((data >> (p.y & 0x07U)) & 0x01U));
  }


  ///////////////////////////////////////////////////////////////////////////////
  // D I S P L A Y   C O N T R O L
  //

public:

  /**
   * Enable/disable the display, the RAM is retained
   */
  virtual void display_enable(bool enable = true)
  {
    write_cmd(enable ? CMD_DISPLAY_ON : CMD_DISPLAY_OFF);
  }


  /**
   * Set the contrast (electronic volume)
   * \param level 0: minimum; 255: maximum (volume = level / 4)
   */
  virtual void display_contrast(std::uint8_t level)
  {
    const std::uint8_t cmd[] = { CMD_VOLUME_MODE, static_cast<std::uint8_t>(level >> 2U) };
    write_cmd(cmd, sizeof(cmd));
  }


  /**
   * Inverse video
   */
  virtual void display_inverse(bool inverse)
  {
    write_cmd(inverse ? CMD_INVERSE : CMD_NORMAL);
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


  inline void write_cmd(std::uint8_t cmd) const
  {
    io::write(device_handle_, 0U, &cmd, 1U, nullptr, 0U);
  }

  inline void write_cmd(const std::uint8_t* cmd, std::size_t length) const
  {
    io::write(device_handle_, 0U, cmd, length, nullptr, 0U);
  }

};

} // namespace head
} // namespace vic

#endif  // _VIC_HEAD_ST7565_H_
