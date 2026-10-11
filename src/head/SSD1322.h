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
// \brief SSD1322 head, 480 x 128 OLED controller with 16 gray levels (256 x 64 panels)
// The RAM holds 4 bits per pixel, two pixels per byte. The head renders into a RAM
// framebuffer (8 KB for 256 x 64) and sends it with present(). All orientations are
// done in software, the remap register keeps the nibble remap (high nibble = left
// pixel). See doc/SSD1322.md for the programming summary.
//
// io layer convention: io::write() option 0 = command, 1 = data (the D/C# line).
// The serial interfaces can't read back.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_SSD1322_H_
#define _VIC_HEAD_SSD1322_H_

#include "../drv.h"


// defines the head name and version
#define VIC_HEAD_SSD1322_VERSION   "SSD1322 head 1.00"

namespace vic {
namespace head {


/**
 * SSD1322 display controller
 * The screen size is the LOGICAL size and must match the orientation: 256 x 64 for orientation_0 / 180,
 * 64 x 256 for orientation_90 / 270. The common 256 x 64 modules have the panel in the middle of the 480
 * column RAM (column offset 28 in units of 4 pixels) and are upright with orientation_0m.
 * \param Screen_Size_X Logical screen width in pixel
 * \param Screen_Size_Y Logical screen height in pixel
 * \param Orientation Orientation of the display (done in software)
 * \param Column_Offset First RAM column of the panel in units of 4 pixels (1Ch for the 256 pixel modules)
 * \param Contrast Contrast current 0 .. 255
 */
template<std::uint16_t Screen_Size_X = 256U, std::uint16_t Screen_Size_Y = 64U,  // X, Y size, must match the orientation
         drv::orientation_type Orientation = drv::orientation_0,                  // orientation of the display
         std::uint8_t Column_Offset = 0x1CU,                                      // first RAM column in 4 pixel units
         std::uint8_t Contrast      = 0x9FU                                       // contrast current
>
class SSD1322 final : public drv
{
  // commands
  static const std::uint8_t CMD_ENABLE_GRAY_TABLE   = 0x00U;
  static const std::uint8_t CMD_COLUMN_ADDR         = 0x15U;    // start, end in 4 pixel units 0 .. 119
  static const std::uint8_t CMD_WRITE_RAM           = 0x5CU;
  static const std::uint8_t CMD_READ_RAM            = 0x5DU;
  static const std::uint8_t CMD_ROW_ADDR            = 0x75U;    // start, end 0 .. 127
  static const std::uint8_t CMD_REMAP               = 0xA0U;    // A: increment, column remap, nibble remap, COM scan, COM split; B: dual COM
  static const std::uint8_t CMD_START_LINE          = 0xA1U;
  static const std::uint8_t CMD_DISPLAY_OFFSET      = 0xA2U;
  static const std::uint8_t CMD_DISPLAY_ALL_OFF     = 0xA4U;
  static const std::uint8_t CMD_DISPLAY_ALL_ON      = 0xA5U;
  static const std::uint8_t CMD_DISPLAY_NORMAL      = 0xA6U;
  static const std::uint8_t CMD_DISPLAY_INVERSE     = 0xA7U;
  static const std::uint8_t CMD_PARTIAL_ON          = 0xA8U;
  static const std::uint8_t CMD_PARTIAL_OFF         = 0xA9U;
  static const std::uint8_t CMD_FUNCTION_SELECT     = 0xABU;    // 01h internal VDD regulator
  static const std::uint8_t CMD_DISPLAY_OFF         = 0xAEU;
  static const std::uint8_t CMD_DISPLAY_ON          = 0xAFU;
  static const std::uint8_t CMD_PHASE_LENGTH        = 0xB1U;
  static const std::uint8_t CMD_CLOCK_DIV           = 0xB3U;
  static const std::uint8_t CMD_ENHANCEMENT_A       = 0xB4U;
  static const std::uint8_t CMD_GPIO                = 0xB5U;
  static const std::uint8_t CMD_PRECHARGE2          = 0xB6U;
  static const std::uint8_t CMD_GRAY_TABLE          = 0xB8U;
  static const std::uint8_t CMD_GRAY_LINEAR         = 0xB9U;
  static const std::uint8_t CMD_PRECHARGE_VOLTAGE   = 0xBBU;
  static const std::uint8_t CMD_VCOMH               = 0xBEU;
  static const std::uint8_t CMD_CONTRAST            = 0xC1U;
  static const std::uint8_t CMD_MASTER_CONTRAST     = 0xC7U;
  static const std::uint8_t CMD_MUX_RATIO           = 0xCAU;
  static const std::uint8_t CMD_ENHANCEMENT_B       = 0xD1U;
  static const std::uint8_t CMD_COMMAND_LOCK        = 0xFDU;    // 12h unlock, 16h lock

  // remap bits
  static const std::uint8_t REMAP_VERTICAL_INC      = 0x01U;
  static const std::uint8_t REMAP_COLUMN            = 0x02U;
  static const std::uint8_t REMAP_NIBBLE            = 0x04U;    // high nibble = left pixel
  static const std::uint8_t REMAP_COM_SCAN          = 0x10U;
  static const std::uint8_t REMAP_COM_SPLIT         = 0x20U;
  static const std::uint8_t REMAP_DUAL_COM          = 0x11U;    // second byte: B[4] dual COM line mode, B[0] is fixed 1

  // physical panel
  static const bool          ROTATED = (Orientation == orientation_90)  || (Orientation == orientation_270) ||
                                       (Orientation == orientation_90m) || (Orientation == orientation_270m);
  static const std::uint16_t PHYS_X  = ROTATED ? Screen_Size_Y : Screen_Size_X;   // columns, 256
  static const std::uint16_t PHYS_Y  = ROTATED ? Screen_Size_X : Screen_Size_Y;   // rows, 64
  static const std::size_t   BYTES_X = PHYS_X / 2U;                               // two pixels per byte
  static const std::size_t   FB_SIZE = BYTES_X * PHYS_Y;

  static_assert(PHYS_X % 4U == 0U, "SSD1322: the physical width must be a multiple of 4");
  static_assert((PHYS_X / 4U + Column_Offset <= 120U) && (PHYS_Y <= 128U), "SSD1322: the panel exceeds the 480 x 128 RAM");

  // vars
  const io::handle_type device_handle_;
  std::uint8_t          framebuffer_[FB_SIZE];    // row organized, high nibble = left pixel

public:

  /**
   * ctor
   * \param device_handle Device handle
   */
  SSD1322(io::handle_type device_handle)
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


  ~SSD1322()
  {
    shutdown();
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S

  virtual void init()
  {
    io::init(device_handle_);

    const std::uint8_t unlock[1]  = { 0x12U };
    const std::uint8_t clock[1]   = { 0x91U };                                            // divide by 2, oscillator 9
    const std::uint8_t mux[1]     = { static_cast<std::uint8_t>(PHYS_Y - 1U) };
    const std::uint8_t zero[1]    = { 0x00U };
    // values of the u8g2 / NHD-3.12 reference sequence; u8g2 sends A0h 14h (COM scan remap for the module orientation),
    // here the mirrors are done in software so the remap is 04h and the module is upright with orientation_0m
    const std::uint8_t remap[2]   = { static_cast<std::uint8_t>(REMAP_NIBBLE), REMAP_DUAL_COM };   // horizontal increment, nibble remap, dual COM
    const std::uint8_t vdd[1]     = { 0x01U };                                            // internal VDD regulator
    const std::uint8_t enh_a[2]   = { 0xA0U, 0xFDU };                                     // internal VSL, enhanced low gray scale
    const std::uint8_t contrast[1]= { Contrast };
    const std::uint8_t master[1]  = { 0x0FU };
    const std::uint8_t phase[1]   = { 0xE2U };                                            // phase 1 = 5, phase 2 = 14 DCLK
    const std::uint8_t enh_b[2]   = { 0xA2U, 0x20U };                                     // normal
    const std::uint8_t precharge[1]={ 0x1FU };                                            // 0.60 x VCC
    const std::uint8_t precharge2[1]={ 0x08U };
    const std::uint8_t vcomh[1]   = { 0x07U };                                            // 0.86 x VCC

    write_cmd(CMD_COMMAND_LOCK,      unlock, 1U);
    write_cmd(CMD_DISPLAY_OFF,       nullptr, 0U);
    write_cmd(CMD_CLOCK_DIV,         clock, 1U);
    write_cmd(CMD_MUX_RATIO,         mux, 1U);
    write_cmd(CMD_DISPLAY_OFFSET,    zero, 1U);
    write_cmd(CMD_START_LINE,        zero, 1U);
    write_cmd(CMD_REMAP,             remap, 2U);
    write_cmd(CMD_GPIO,              zero, 1U);
    write_cmd(CMD_FUNCTION_SELECT,   vdd, 1U);
    write_cmd(CMD_ENHANCEMENT_A,     enh_a, 2U);
    write_cmd(CMD_CONTRAST,          contrast, 1U);
    write_cmd(CMD_MASTER_CONTRAST,   master, 1U);
    write_cmd(CMD_GRAY_LINEAR,       nullptr, 0U);
    write_cmd(CMD_PHASE_LENGTH,      phase, 1U);
    write_cmd(CMD_ENHANCEMENT_B,     enh_b, 2U);
    write_cmd(CMD_PRECHARGE_VOLTAGE, precharge, 1U);
    write_cmd(CMD_PRECHARGE2,        precharge2, 1U);
    write_cmd(CMD_VCOMH,             vcomh, 1U);
    write_cmd(CMD_DISPLAY_NORMAL,    nullptr, 0U);
    write_cmd(CMD_PARTIAL_OFF,       nullptr, 0U);
    write_cmd(CMD_DISPLAY_ON,        nullptr, 0U);

    cls();
    present();
  }


  virtual void shutdown()
  {
    write_cmd(CMD_DISPLAY_OFF, nullptr, 0U);
  }


  inline virtual const char* version() const
  {
    return (const char*)VIC_HEAD_SSD1322_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    return true;
  }


  /**
   * Access the framebuffer (row organized nibbles, physical orientation)
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
    const std::uint8_t gs   = color::color_to_L4(bg_color);
    const std::uint8_t fill = static_cast<std::uint8_t>((gs << 4U) | gs);
    for (std::size_t i = 0U; i < FB_SIZE; ++i) {
      framebuffer_[i] = fill;
    }
  }


  /**
   * Copy the framebuffer to the display: window over the panel, then all rows in one block
   */
  virtual void present()
  {
    const std::uint8_t cols[2] = { Column_Offset, static_cast<std::uint8_t>(Column_Offset + PHYS_X / 4U - 1U) };
    const std::uint8_t rows[2] = { 0x00U, static_cast<std::uint8_t>(PHYS_Y - 1U) };
    write_cmd(CMD_COLUMN_ADDR, cols, 2U);
    write_cmd(CMD_ROW_ADDR,    rows, 2U);
    write_cmd(CMD_WRITE_RAM,   framebuffer_, FB_SIZE);
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
    std::uint8_t& data = framebuffer_[static_cast<std::size_t>(p.y) * BYTES_X + static_cast<std::size_t>(p.x >> 1U)];
    const std::uint8_t gs = color::color_to_L4(color);
    data = static_cast<std::uint8_t>((p.x & 1) ? ((data & 0xF0U) | gs) : ((data & 0x0FU) | (gs << 4U)));
  }


  virtual color::value_type pixel_get(vertex_type vertex)
  {
    if (!screen_is_inside(vertex)) {
      return color::none;
    }
    const vertex_type p = to_phys(vertex);
    const std::uint8_t data = framebuffer_[static_cast<std::size_t>(p.y) * BYTES_X + static_cast<std::size_t>(p.x >> 1U)];
    return color::L4_to_color(static_cast<std::uint8_t>((p.x & 1) ? (data & 0x0FU) : (data >> 4U)));
  }


  ///////////////////////////////////////////////////////////////////////////////
  // D I S P L A Y   C O N T R O L
  //

public:

  /**
   * Enable/disable the display (sleep mode), the RAM is retained
   */
  virtual void display_enable(bool enable = true)
  {
    write_cmd(enable ? CMD_DISPLAY_ON : CMD_DISPLAY_OFF, nullptr, 0U);
  }


  /**
   * Set the contrast current
   * \param level 0: minimum; 255: maximum
   */
  virtual void display_contrast(std::uint8_t level)
  {
    write_cmd(CMD_CONTRAST, &level, 1U);
  }


  /**
   * Set the brightness (master contrast, 16 steps)
   * \param level 0: 1/16; 255: 16/16
   */
  virtual void display_brightness(std::uint8_t level)
  {
    const std::uint8_t master = static_cast<std::uint8_t>(level >> 4U);
    write_cmd(CMD_MASTER_CONTRAST, &master, 1U);
  }


  /**
   * Inverse display (GS0 <-> GS15)
   */
  virtual void display_inverse(bool inverse)
  {
    write_cmd(inverse ? CMD_DISPLAY_INVERSE : CMD_DISPLAY_NORMAL, nullptr, 0U);
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


  /**
   * Send a command with its parameters / data: command byte with io option 0, the rest with option 1
   */
  inline void write_cmd(std::uint8_t cmd, const std::uint8_t* data, std::size_t length) const
  {
    io::write(device_handle_, 0U, &cmd, 1U, nullptr, 0U);
    if (length != 0U) {
      io::write(device_handle_, 1U, data, length, nullptr, 0U);
    }
  }

};

} // namespace head
} // namespace vic

#endif  // _VIC_HEAD_SSD1322_H_
