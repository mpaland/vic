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
// \brief ST7920 head, 128 x 64 monochrome graphic LCD controller (graphic mode)
// The GDRAM is written in 16 bit words, the 64 rows of a 128 x 64 panel are two
// halves of 32 rows side by side in the RAM (words 0 .. 7 and 8 .. 15 of the same
// vertical address). The head renders into a row organized framebuffer (bit 7 =
// left pixel) and sends it with present(), 32 rows of 32 bytes. All orientations
// are done in software. See doc/ST7920.md for the programming summary.
//
// io layer convention: io::write() option 0 = instruction (RS = 0), 1 = data
// (RS = 1); the serial interface sync byte (F8h / FAh) and the nibble split are
// built by the io layer. Every byte needs 72 us (clear: 1.6 ms, waited by the
// head), the io layer has to pace the transfers or poll the busy flag.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_ST7920_H_
#define _VIC_HEAD_ST7920_H_

#include "../drv.h"


// defines the head name and version
#define VIC_HEAD_ST7920_VERSION   "ST7920 head 1.00"

namespace vic {
namespace head {


/**
 * ST7920 display controller in graphic mode
 * The screen size is the LOGICAL size and must match the orientation: 128 x 64 for orientation_0 / 180,
 * 64 x 128 for orientation_90 / 270.
 * \param Screen_Size_X Logical screen width in pixel
 * \param Screen_Size_Y Logical screen height in pixel
 * \param Orientation Orientation of the display (done in software)
 */
template<std::uint16_t Screen_Size_X = 128U, std::uint16_t Screen_Size_Y = 64U,  // X, Y size, must match the orientation
         drv::orientation_type Orientation = drv::orientation_0                   // orientation of the display
>
class ST7920 final : public drv
{
  // basic instructions (RE = 0)
  static const std::uint8_t CMD_CLEAR               = 0x01U;    // clear DDRAM, 1.6 ms
  static const std::uint8_t CMD_HOME                = 0x02U;
  static const std::uint8_t CMD_ENTRY_MODE          = 0x04U;    // | I/D (02h) | S (01h)
  static const std::uint8_t CMD_DISPLAY             = 0x08U;    // | D (04h) | C (02h) | B (01h)
  static const std::uint8_t CMD_FUNCTION_SET        = 0x20U;    // | DL (10h) | RE (04h) | G (02h, with RE)
  static const std::uint8_t FUNC_8BIT               = 0x10U;
  static const std::uint8_t FUNC_EXTENDED           = 0x04U;
  static const std::uint8_t FUNC_GRAPHIC            = 0x02U;
  // extended instructions (RE = 1)
  static const std::uint8_t CMD_GDRAM_ADDR          = 0x80U;    // | vertical address 0 .. 63, then | horizontal address 0 .. 15

  // physical panel: 32 common lines x 2 halves, 16 words per RAM row
  static const bool          ROTATED = (Orientation == orientation_90)  || (Orientation == orientation_270) ||
                                       (Orientation == orientation_90m) || (Orientation == orientation_270m);
  static const std::uint16_t PHYS_X  = ROTATED ? Screen_Size_Y : Screen_Size_X;   // 128 columns
  static const std::uint16_t PHYS_Y  = ROTATED ? Screen_Size_X : Screen_Size_Y;   // 64 rows
  static const std::size_t   BYTES_X = PHYS_X / 8U;                               // 16 bytes per row
  static const std::size_t   HALF    = PHYS_Y / 2U;                               // 32 rows per half
  static const std::size_t   FB_SIZE = BYTES_X * PHYS_Y;

  static_assert((PHYS_X % 16U == 0U) && (PHYS_Y % 2U == 0U), "ST7920: the physical width must be a multiple of 16, the height even");
  static_assert(BYTES_X * 2U <= 32U, "ST7920: the two halves of a row must fit into the 16 GDRAM words");

  // vars
  const io::handle_type device_handle_;
  std::uint8_t          framebuffer_[FB_SIZE];    // row organized: data[y * BYTES_X + x / 8], bit 7 = left pixel

public:

  /**
   * ctor
   * \param device_handle Device handle
   */
  ST7920(io::handle_type device_handle)
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


  ~ST7920()
  {
    shutdown();
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S

  virtual void init()
  {
    io::init(device_handle_);
    // datasheet init flow (also used by u8g2), the 128 x 64 modules are upright with orientation_0
    io::delay(50U);                                                   // > 40 ms after power on

    write_cmd(static_cast<std::uint8_t>(CMD_FUNCTION_SET | FUNC_8BIT));             // basic instruction set
    io::delay(1U);
    write_cmd(static_cast<std::uint8_t>(CMD_FUNCTION_SET | FUNC_8BIT));
    write_cmd(static_cast<std::uint8_t>(CMD_DISPLAY | 0x04U));                      // display on, cursor off
    write_cmd(CMD_CLEAR);                                                           // clear the text DDRAM
    io::delay(2U);
    write_cmd(static_cast<std::uint8_t>(CMD_ENTRY_MODE | 0x02U));                   // increment, no shift
    write_cmd(static_cast<std::uint8_t>(CMD_FUNCTION_SET | FUNC_8BIT | FUNC_EXTENDED));                // extended instruction set
    write_cmd(static_cast<std::uint8_t>(CMD_FUNCTION_SET | FUNC_8BIT | FUNC_EXTENDED | FUNC_GRAPHIC)); // graphic display on

    cls();
    present();
  }


  virtual void shutdown()
  {
    // graphic off, basic instruction set, display off
    write_cmd(static_cast<std::uint8_t>(CMD_FUNCTION_SET | FUNC_8BIT | FUNC_EXTENDED));
    write_cmd(static_cast<std::uint8_t>(CMD_FUNCTION_SET | FUNC_8BIT));
    write_cmd(CMD_DISPLAY);
  }


  inline virtual const char* version() const
  {
    return (const char*)VIC_HEAD_ST7920_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    return true;
  }


  /**
   * Access the framebuffer (row organized, physical orientation)
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
   * Copy the framebuffer to the GDRAM: for every vertical address the upper half row and the lower half row
   */
  virtual void present()
  {
    for (std::size_t y = 0U; y < HALF; ++y) {
      const std::uint8_t cmd[] = { static_cast<std::uint8_t>(CMD_GDRAM_ADDR | y), CMD_GDRAM_ADDR };
      write_cmd(cmd, sizeof(cmd));
      io::write(device_handle_, 1U, &framebuffer_[y * BYTES_X], BYTES_X, nullptr, 0U);
      io::write(device_handle_, 1U, &framebuffer_[(y + HALF) * BYTES_X], BYTES_X, nullptr, 0U);
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
    std::uint8_t& data = framebuffer_[static_cast<std::size_t>(p.y) * BYTES_X + static_cast<std::size_t>(p.x >> 3U)];
    const std::uint8_t mask = static_cast<std::uint8_t>(0x80U >> (static_cast<unsigned>(p.x) & 0x07U));
    data = static_cast<std::uint8_t>(color::color_to_L1(color) ? (data | mask) : (data & ~mask));
  }


  virtual color::value_type pixel_get(vertex_type vertex)
  {
    if (!screen_is_inside(vertex)) {
      return color::none;
    }
    const vertex_type p = to_phys(vertex);
    const std::uint8_t data = framebuffer_[static_cast<std::size_t>(p.y) * BYTES_X + static_cast<std::size_t>(p.x >> 3U)];
    return color::L1_to_color(static_cast<std::uint8_t>((data >> (7U - (static_cast<unsigned>(p.x) & 0x07U))) & 0x01U));
  }


  ///////////////////////////////////////////////////////////////////////////////
  // D I S P L A Y   C O N T R O L
  //

public:

  /**
   * Enable/disable the graphic display (G bit), the GDRAM is retained
   */
  virtual void display_enable(bool enable = true)
  {
    write_cmd(static_cast<std::uint8_t>(CMD_FUNCTION_SET | FUNC_8BIT | FUNC_EXTENDED | (enable ? FUNC_GRAPHIC : 0U)));
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

#endif  // _VIC_HEAD_ST7920_H_
