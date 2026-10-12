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
// \brief SSD1351 head, 128 x 128 RGB OLED controller with 65k colors (RGB565)
// The head writes directly into the GDDRAM of the controller (no framebuffer),
// pixels are streamed through the column/row address window with native auto
// increment, the orientation is done with the remap bits. The controller has no
// graphic accelerator. The GDDRAM can only be read on the parallel bus, on SPI
// pixel_get() returns color::none. See doc/SSD1351.md for the programming summary.
//
// io layer convention: io::write() option 0 = command, 1 = data (the D/C# line),
// io::read() option 1 reads two pixel bytes after the read RAM command (5Dh).
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_SSD1351_H_
#define _VIC_HEAD_SSD1351_H_

#include "../drv.h"


// defines the head name and version
#define VIC_HEAD_SSD1351_VERSION   "SSD1351 head 1.00"

namespace vic {
namespace head {


/**
 * SSD1351 display controller
 * The screen size is the LOGICAL size and must match the orientation (128 x 128, or 128 x 96 / 96 x 128 for
 * the 1.27" panels). orientation_0 is the datasheet default (no remap), the common 1.5" modules are upright
 * with orientation_0m.
 * \param Screen_Size_X Logical screen width in pixel
 * \param Screen_Size_Y Logical screen height in pixel
 * \param Orientation Hardware orientation of the display
 * \param RGB_to_BGR Swap the color order (remap bit A[2]), the common modules need it with RGB565 data
 */
template<std::uint16_t Screen_Size_X = 128U, std::uint16_t Screen_Size_Y = 128U, // X, Y size, must match the orientation
         drv::orientation_type Orientation = drv::orientation_0,                  // hardware orientation of the display
         bool RGB_to_BGR = true                                                   // color order swap
>
class SSD1351 final : public drv
{
  // commands
  static const std::uint8_t CMD_COLUMN_ADDR         = 0x15U;
  static const std::uint8_t CMD_ROW_ADDR            = 0x75U;
  static const std::uint8_t CMD_WRITE_RAM           = 0x5CU;
  static const std::uint8_t CMD_READ_RAM            = 0x5DU;
  static const std::uint8_t CMD_REMAP               = 0xA0U;
  static const std::uint8_t CMD_START_LINE          = 0xA1U;
  static const std::uint8_t CMD_DISPLAY_OFFSET      = 0xA2U;
  static const std::uint8_t CMD_DISPLAY_ALL_OFF     = 0xA4U;
  static const std::uint8_t CMD_DISPLAY_ALL_ON      = 0xA5U;
  static const std::uint8_t CMD_DISPLAY_NORMAL      = 0xA6U;
  static const std::uint8_t CMD_DISPLAY_INVERSE     = 0xA7U;
  static const std::uint8_t CMD_FUNCTION_SELECT     = 0xABU;
  static const std::uint8_t CMD_DISPLAY_OFF         = 0xAEU;
  static const std::uint8_t CMD_DISPLAY_ON          = 0xAFU;
  static const std::uint8_t CMD_PHASE_PERIOD        = 0xB1U;
  static const std::uint8_t CMD_ENHANCEMENT         = 0xB2U;
  static const std::uint8_t CMD_CLOCK_DIV           = 0xB3U;
  static const std::uint8_t CMD_VSL                 = 0xB4U;
  static const std::uint8_t CMD_GPIO                = 0xB5U;
  static const std::uint8_t CMD_PRECHARGE2          = 0xB6U;
  static const std::uint8_t CMD_GRAY_TABLE          = 0xB8U;
  static const std::uint8_t CMD_GRAY_LINEAR         = 0xB9U;
  static const std::uint8_t CMD_PRECHARGE_VOLTAGE   = 0xBBU;
  static const std::uint8_t CMD_VCOMH               = 0xBEU;
  static const std::uint8_t CMD_CONTRAST_ABC        = 0xC1U;
  static const std::uint8_t CMD_MASTER_CONTRAST     = 0xC7U;
  static const std::uint8_t CMD_MUX_RATIO           = 0xCAU;
  static const std::uint8_t CMD_COMMAND_LOCK        = 0xFDU;    // 12h unlock, 16h lock, B0h / B1h lock / unlock the extra commands

  // remap bits (A0h)
  static const std::uint8_t REMAP_VERTICAL_INC      = 0x01U;
  static const std::uint8_t REMAP_COLUMN            = 0x02U;    // column 127 -> SEG0 (X mirror)
  static const std::uint8_t REMAP_BGR               = 0x04U;    // color sequence C, B, A
  static const std::uint8_t REMAP_COM_SCAN          = 0x10U;    // scan COM[N-1] -> COM0 (Y mirror)
  static const std::uint8_t REMAP_COM_SPLIT         = 0x20U;
  static const std::uint8_t REMAP_COLOR_65K         = 0x40U;    // A[7:6] = 01

  // default contrast of the three colors
  static const std::uint8_t CONTRAST_A              = 0xC8U;
  static const std::uint8_t CONTRAST_B              = 0x80U;
  static const std::uint8_t CONTRAST_C              = 0xC8U;

  // orientation
  static const bool ROTATED  = (Orientation == orientation_90)  || (Orientation == orientation_270) ||
                               (Orientation == orientation_90m) || (Orientation == orientation_270m);
  static const bool MIRROR_X = (Orientation == orientation_180) || (Orientation == orientation_180m) ||
                               (Orientation == orientation_90)  || (Orientation == orientation_90m);
  static const bool MIRROR_Y = (Orientation == orientation_0m)  || (Orientation == orientation_180) ||
                               (Orientation == orientation_270) || (Orientation == orientation_90m);
  static const std::uint16_t RAM_SIZE_X = ROTATED ? Screen_Size_Y : Screen_Size_X;   // RAM columns
  static const std::uint16_t RAM_SIZE_Y = ROTATED ? Screen_Size_X : Screen_Size_Y;   // RAM rows
  static const std::uint8_t  REMAP      = REMAP_COLOR_65K | REMAP_COM_SPLIT |
                                          (ROTATED    ? REMAP_VERTICAL_INC : 0U) |
                                          (MIRROR_X   ? REMAP_COLUMN       : 0U) |
                                          (MIRROR_Y   ? REMAP_COM_SCAN     : 0U) |
                                          (RGB_to_BGR ? REMAP_BGR          : 0U);

  static_assert((RAM_SIZE_X <= 128U) && (RAM_SIZE_Y <= 128U), "SSD1351: the panel exceeds the 128 x 128 RAM");

  // pixels per io::write() in a data stream
  static const std::size_t STREAM_PIXELS = 32U;

  // vars
  const io::handle_type device_handle_;
  std::uint8_t          stream_[STREAM_PIXELS * 2U];   // data stream buffer
  std::size_t           stream_idx_;
  struct tag_window_type {
    std::int16_t x0, x1, y0, y1;                      // logical address window, inclusive
  } win_;
  vertex_type           pos_;                         // logical position the controller writes next, x < 0 if unknown

public:

  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S

  /**
   * ctor
   * \param device_handle Device handle
   */
  SSD1351(io::handle_type device_handle)
    : drv(Screen_Size_X, Screen_Size_Y,
          Screen_Size_X, Screen_Size_Y,         // no viewport support (screen = viewport)
          0, 0,                                 // no viewport support
          Orientation)
    , device_handle_(device_handle)
    , stream_idx_(0U)
    , win_({ 0, 0, 0, 0 })
    , pos_({ -1, -1 })
  { }


  ~SSD1351()
  {
    shutdown();
  }


  virtual void init()
  {
    io::init(device_handle_);

    // values of the Adafruit_SSD1351 sequence; Adafruit sends remap 74h (COM scan remap + color swap), which is
    // orientation_0m with RGB_to_BGR = true here
    const std::uint8_t unlock[1]   = { 0x12U };
    const std::uint8_t unlock_b[1] = { 0xB1U };                                 // unlock A2h, B1h, B3h, BBh, BEh
    const std::uint8_t clock[1]    = { 0xF1U };                                 // divide by 2, oscillator 15
    const std::uint8_t mux[1]      = { static_cast<std::uint8_t>(RAM_SIZE_Y - 1U) };
    const std::uint8_t remap[1]    = { REMAP };
    const std::uint8_t zero[1]     = { 0x00U };
    const std::uint8_t vdd[1]      = { 0x01U };                                 // internal VDD regulator
    const std::uint8_t phase[1]    = { 0x32U };                                 // phase 1 = 5, phase 2 = 3 DCLK
    const std::uint8_t vcomh[1]    = { 0x05U };                                 // 0.82 x VCC
    const std::uint8_t contrast[3] = { CONTRAST_A, CONTRAST_B, CONTRAST_C };
    const std::uint8_t master[1]   = { 0x0FU };
    const std::uint8_t vsl[3]      = { 0xA0U, 0xB5U, 0x55U };                   // external VSL
    const std::uint8_t precharge2[1]={ 0x01U };

    write_cmd(CMD_COMMAND_LOCK,    unlock, 1U);
    write_cmd(CMD_COMMAND_LOCK,    unlock_b, 1U);
    write_cmd(CMD_DISPLAY_OFF,     nullptr, 0U);
    write_cmd(CMD_CLOCK_DIV,       clock, 1U);
    write_cmd(CMD_MUX_RATIO,       mux, 1U);
    write_cmd(CMD_REMAP,           remap, 1U);
    write_cmd(CMD_START_LINE,      zero, 1U);
    write_cmd(CMD_DISPLAY_OFFSET,  zero, 1U);
    write_cmd(CMD_GPIO,            zero, 1U);
    write_cmd(CMD_FUNCTION_SELECT, vdd, 1U);
    write_cmd(CMD_PHASE_PERIOD,    phase, 1U);
    write_cmd(CMD_VCOMH,           vcomh, 1U);
    write_cmd(CMD_DISPLAY_NORMAL,  nullptr, 0U);
    write_cmd(CMD_CONTRAST_ABC,    contrast, 3U);
    write_cmd(CMD_MASTER_CONTRAST, master, 1U);
    write_cmd(CMD_VSL,             vsl, 3U);
    write_cmd(CMD_PRECHARGE2,      precharge2, 1U);
    write_cmd(CMD_DISPLAY_ON,      nullptr, 0U);
    pos_ = { -1, -1 };

    cls();
  }


  virtual void shutdown()
  {
    write_cmd(CMD_DISPLAY_OFF, nullptr, 0U);
  }


  inline virtual const char* version() const
  {
    return (const char*)VIC_HEAD_SSD1351_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    return true;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

protected:

  virtual void cls(color::value_type bg_color = color::none)
  {
    set_window(0, static_cast<std::int16_t>(Screen_Size_X - 1U), 0, static_cast<std::int16_t>(Screen_Size_Y - 1U));
    const std::uint16_t pixel = color::color_to_RGB565(bg_color);
    stream_begin();
    for (std::uint32_t i = 0U; i < static_cast<std::uint32_t>(Screen_Size_X) * Screen_Size_Y; ++i) {
      stream_pixel(pixel);
    }
    stream_end();
    pos_ = { -1, -1 };
  }


  // unused - data is written directly to the display
  virtual void present()
  { }


  ///////////////////////////////////////////////////////////////////////////////
  // G R A P H I C   F U N C T I O N S
  //

  virtual void pixel_set(vertex_type vertex, color::value_type color)
  {
    if (!screen_is_inside(vertex)) {
      return;
    }
    if (pos_ != vertex) {
      set_window(vertex.x, static_cast<std::int16_t>(Screen_Size_X - 1U), vertex.y, static_cast<std::int16_t>(Screen_Size_Y - 1U));
    }
    stream_begin();
    stream_pixel(color::color_to_RGB565(color));
    stream_end();
    advance(1U);
  }


  virtual color::value_type pixel_get(vertex_type vertex)
  {
    if (!screen_is_inside(vertex)) {
      return color::none;
    }
    set_window(vertex.x, vertex.x, vertex.y, vertex.y, false);
    write_cmd(CMD_READ_RAM, nullptr, 0U);
    pos_ = { -1, -1 };
    std::uint8_t data[2] = { 0U, 0U };
    std::size_t  length  = 2U;
    if (!io::read(device_handle_, 1U, data, length, 0U) || (length != 2U)) {
      return color::none;
    }
    return color::RGB565_to_color(static_cast<std::uint16_t>((static_cast<std::uint16_t>(data[0]) << 8U) | data[1]));
  }


  virtual void line_horz(vertex_type v0, vertex_type v1, color::value_type color)
  {
    util::min_max_swap(v0.x, v1.x);
    if ((v0.y < 0) || (v0.y >= static_cast<std::int16_t>(Screen_Size_Y)) || (v1.x < 0) || (v0.x >= static_cast<std::int16_t>(Screen_Size_X))) {
      return;
    }
    if (v0.x < 0) {
      v0.x = 0;
    }
    if (v1.x >= static_cast<std::int16_t>(Screen_Size_X)) {
      v1.x = static_cast<std::int16_t>(Screen_Size_X - 1U);
    }
    if ((pos_ != v0) || (win_.x1 < v1.x)) {
      set_window(v0.x, static_cast<std::int16_t>(Screen_Size_X - 1U), v0.y, static_cast<std::int16_t>(Screen_Size_Y - 1U));
    }
    const std::uint16_t pixel = color::color_to_RGB565(color);
    stream_begin();
    for (std::int_fast16_t x = v0.x; x <= v1.x; ++x) {
      stream_pixel(pixel);
    }
    stream_end();
    advance(static_cast<std::size_t>(v1.x - v0.x + 1));
  }


  virtual void line_vert(vertex_type v0, vertex_type v1, color::value_type color)
  {
    util::min_max_swap(v0.y, v1.y);
    if ((v0.x < 0) || (v0.x >= static_cast<std::int16_t>(Screen_Size_X)) || (v1.y < 0) || (v0.y >= static_cast<std::int16_t>(Screen_Size_Y))) {
      return;
    }
    if (v0.y < 0) {
      v0.y = 0;
    }
    if (v1.y >= static_cast<std::int16_t>(Screen_Size_Y)) {
      v1.y = static_cast<std::int16_t>(Screen_Size_Y - 1U);
    }
    set_window(v0.x, v0.x, v0.y, static_cast<std::int16_t>(Screen_Size_Y - 1U));
    const std::uint16_t pixel = color::color_to_RGB565(color);
    stream_begin();
    for (std::int_fast16_t y = v0.y; y <= v1.y; ++y) {
      stream_pixel(pixel);
    }
    stream_end();
    advance(static_cast<std::size_t>(v1.y - v0.y + 1));
  }


  virtual void box(const rect_type& rect, color::value_type color)
  {
    rect_type r = rect;
    if (!clip(r)) {
      return;
    }
    set_window(r.left, static_cast<std::int16_t>(r.right - 1), r.top, static_cast<std::int16_t>(r.bottom - 1));
    const std::uint16_t pixel = color::color_to_RGB565(color);
    stream_begin();
    for (std::uint32_t i = 0U; i < static_cast<std::uint32_t>(r.width()) * static_cast<std::uint32_t>(r.height()); ++i) {
      stream_pixel(pixel);
    }
    stream_end();
    advance(static_cast<std::size_t>(r.width()) * static_cast<std::size_t>(r.height()));
  }


  virtual void blit(const rect_type& rect, const color::value_type* pixels, std::size_t stride)
  {
    rect_type r = rect;
    std::size_t skip_x = 0U, skip_y = 0U;
    if (r.left < 0) { skip_x = static_cast<std::size_t>(-r.left); r.left = 0; }
    if (r.top  < 0) { skip_y = static_cast<std::size_t>(-r.top);  r.top  = 0; }
    if (!clip(r)) {
      return;
    }
    set_window(r.left, static_cast<std::int16_t>(r.right - 1), r.top, static_cast<std::int16_t>(r.bottom - 1));
    const color::value_type* row = pixels + skip_y * stride + skip_x;
    stream_begin();
    for (std::int16_t y = r.top; y < r.bottom; ++y, row += stride) {
      const color::value_type* p = row;
      for (std::int16_t x = r.left; x < r.right; ++x, ++p) {
        stream_pixel(color::color_to_RGB565(*p));
      }
    }
    stream_end();
    advance(static_cast<std::size_t>(r.width()) * static_cast<std::size_t>(r.height()));
  }


  ///////////////////////////////////////////////////////////////////////////////
  // D I S P L A Y   C O N T R O L
  //

public:

  virtual void display_enable(bool enable = true)
  {
    write_cmd(enable ? CMD_DISPLAY_ON : CMD_DISPLAY_OFF, nullptr, 0U);
  }


  /**
   * Set the brightness (master contrast, 16 steps)
   */
  virtual void display_brightness(std::uint8_t level)
  {
    const std::uint8_t master = static_cast<std::uint8_t>(level >> 4U);
    write_cmd(CMD_MASTER_CONTRAST, &master, 1U);
  }


  /**
   * Set the contrast, the color balance of the three channels is kept
   */
  virtual void display_contrast(std::uint8_t level)
  {
    const std::uint8_t contrast[3] = { static_cast<std::uint8_t>((static_cast<std::uint16_t>(CONTRAST_A) * level) / 255U),
                                       static_cast<std::uint8_t>((static_cast<std::uint16_t>(CONTRAST_B) * level) / 255U),
                                       static_cast<std::uint8_t>((static_cast<std::uint16_t>(CONTRAST_C) * level) / 255U) };
    write_cmd(CMD_CONTRAST_ABC, contrast, 3U);
  }


  void display_inverse(bool inverse)
  {
    write_cmd(inverse ? CMD_DISPLAY_INVERSE : CMD_DISPLAY_NORMAL, nullptr, 0U);
  }


private:

  static inline vertex_type to_ram(vertex_type v)
  {
    if (ROTATED) {
      const vertex_type r = { v.y, v.x };
      return r;
    }
    return v;
  }


  static inline bool clip(rect_type& r)
  {
    if (r.left < 0) { r.left = 0; }
    if (r.top  < 0) { r.top  = 0; }
    if (r.right  > static_cast<std::int16_t>(Screen_Size_X)) { r.right  = static_cast<std::int16_t>(Screen_Size_X); }
    if (r.bottom > static_cast<std::int16_t>(Screen_Size_Y)) { r.bottom = static_cast<std::int16_t>(Screen_Size_Y); }
    return (r.left < r.right) && (r.top < r.bottom);
  }


  /**
   * Set the address window (logical coordinates, inclusive), optionally followed by the write RAM command
   */
  void set_window(std::int16_t x0, std::int16_t x1, std::int16_t y0, std::int16_t y1, bool write = true)
  {
    const vertex_type r0 = to_ram({ x0, y0 });
    const vertex_type r1 = to_ram({ x1, y1 });
    const std::uint8_t cols[2] = { static_cast<std::uint8_t>(r0.x), static_cast<std::uint8_t>(r1.x) };
    const std::uint8_t rows[2] = { static_cast<std::uint8_t>(r0.y), static_cast<std::uint8_t>(r1.y) };
    write_cmd(CMD_COLUMN_ADDR, cols, 2U);
    write_cmd(CMD_ROW_ADDR,    rows, 2U);
    if (write) {
      write_cmd(CMD_WRITE_RAM, nullptr, 0U);
    }
    win_ = { x0, x1, y0, y1 };
    pos_ = { x0, y0 };
  }


  inline void advance(std::size_t n)
  {
    if (pos_.x < 0) {
      return;
    }
    const std::int32_t w = static_cast<std::int32_t>(win_.x1 - win_.x0 + 1);
    const std::int32_t h = static_cast<std::int32_t>(win_.y1 - win_.y0 + 1);
    const std::int32_t x = static_cast<std::int32_t>(pos_.x - win_.x0) + static_cast<std::int32_t>(n);
    pos_.x = static_cast<std::int16_t>(win_.x0 + (x % w));
    pos_.y = static_cast<std::int16_t>(win_.y0 + ((static_cast<std::int32_t>(pos_.y - win_.y0) + (x / w)) % h));
  }


  inline void write_cmd(std::uint8_t cmd, const std::uint8_t* data, std::size_t length) const
  {
    io::write(device_handle_, 0U, &cmd, 1U, nullptr, 0U);
    if (length != 0U) {
      io::write(device_handle_, 1U, data, length, nullptr, 0U);
    }
  }


  inline void stream_begin()
  {
    stream_idx_ = 0U;
  }

  inline void stream_pixel(std::uint16_t pixel)
  {
    stream_[stream_idx_++] = static_cast<std::uint8_t>(pixel >> 8U);
    stream_[stream_idx_++] = static_cast<std::uint8_t>(pixel);
    if (stream_idx_ >= sizeof(stream_)) {
      stream_end();
    }
  }

  inline void stream_end()
  {
    if (stream_idx_ != 0U) {
      io::write(device_handle_, 1U, stream_, stream_idx_, nullptr, 0U);
      stream_idx_ = 0U;
    }
  }

};

} // namespace head
} // namespace vic

#endif  // _VIC_HEAD_SSD1351_H_
