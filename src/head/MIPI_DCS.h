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
// \brief Generic MIPI DCS head for the SPI / parallel TFT controllers which share
// the MIPI Display Command Set: ILI9341, ILI9488, ST7735, ST7789, ST7796 and
// GC9A01. The controllers differ only in their vendor init sequence, the panel
// size, the GRAM offset of the panel and the pixel format on SPI (the ILI9488
// takes 18 bit pixels only). The head writes directly into the GRAM through the
// column / row address window (2Ah / 2Bh / 2Ch) with native auto increment, the
// orientation is done with the MADCTL register (36h).
// The init tables follow the reference drivers (TFT_eSPI, Adafruit), see
// doc/MIPI_DCS.md.
//
// io layer convention: io::write() option 0 = command, 1 = data (the D/C line),
// io::read() option 1 reads GRAM pixels (3 bytes per pixel, the io layer sends
// the dummy clock / byte of the controller). A write only io layer returns false
// from io::read(), pixel_get() then returns color::none.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_MIPI_DCS_H_
#define _VIC_HEAD_MIPI_DCS_H_

#include "../drv.h"


// defines the head name and version
#define VIC_HEAD_MIPI_DCS_VERSION   "MIPI DCS head 1.00"

namespace vic {
namespace head {


// supported controllers
typedef enum tag_MIPI_DCS_controller_type {
  MIPI_DCS_ILI9341,     // 240 x 320
  MIPI_DCS_ILI9488,     // 320 x 480, 18 bit pixels on SPI
  MIPI_DCS_ST7735,      // 128 x 160 (128 x 128, 80 x 160 with offsets)
  MIPI_DCS_ST7789,      // 240 x 320 (240 x 240, 135 x 240 with offsets)
  MIPI_DCS_ST7796,      // 320 x 480
  MIPI_DCS_GC9A01       // 240 x 240 round
} MIPI_DCS_controller_type;


// vendor init tables: cmd, arg count (bit 7 set: a delay in ms follows the args, FFh = 500 ms), args, [delay]
// MADCTL (36h) and the pixel format (3Ah) are not part of the tables, the head sets them
namespace MIPI_DCS_init {

  static const std::uint8_t ILI9341[] = {
    0xEF, 3, 0x03, 0x80, 0x02,
    0xCF, 3, 0x00, 0xC1, 0x30,                      // power control B
    0xED, 4, 0x64, 0x03, 0x12, 0x81,                // power on sequence control
    0xE8, 3, 0x85, 0x00, 0x78,                      // driver timing control A
    0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,          // power control A
    0xF7, 1, 0x20,                                  // pump ratio control
    0xEA, 2, 0x00, 0x00,                            // driver timing control B
    0xC0, 1, 0x23,                                  // power control 1, VRH = 4.6 V
    0xC1, 1, 0x10,                                  // power control 2
    0xC5, 2, 0x3E, 0x28,                            // VCOM control 1
    0xC7, 1, 0x86,                                  // VCOM control 2
    0x37, 1, 0x00,                                  // vertical scroll start
    0xB1, 2, 0x00, 0x18,                            // frame rate control 79 Hz
    0xB6, 3, 0x08, 0x82, 0x27,                      // display function control
    0xF2, 1, 0x00,                                  // 3 gamma off
    0x26, 1, 0x01,                                  // gamma curve 1
    0xE0, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
    0xE1, 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
    0x11, 0x80, 120,                                // sleep out
    0x29, 0x80, 20                                  // display on
  };

  static const std::uint8_t ILI9488[] = {
    0xE0, 15, 0x00, 0x03, 0x09, 0x08, 0x16, 0x0A, 0x3F, 0x78, 0x4C, 0x09, 0x0A, 0x08, 0x16, 0x1A, 0x0F,
    0xE1, 15, 0x00, 0x16, 0x19, 0x03, 0x0F, 0x05, 0x32, 0x45, 0x46, 0x04, 0x0E, 0x0D, 0x35, 0x37, 0x0F,
    0xC0, 2, 0x17, 0x15,                            // power control 1
    0xC1, 1, 0x41,                                  // power control 2
    0xC5, 3, 0x00, 0x12, 0x80,                      // VCOM control
    0xB0, 1, 0x00,                                  // interface mode control
    0xB1, 1, 0xA0,                                  // frame rate 60 Hz
    0xB4, 1, 0x02,                                  // display inversion control, 2 dot
    0xB6, 2, 0x02, 0x02,                            // display function control
    0xE9, 1, 0x00,                                  // set image function, 24 bit data bus off
    0xF7, 4, 0xA9, 0x51, 0x2C, 0x82,                // adjust control 3
    0x11, 0x80, 120,                                // sleep out
    0x29, 0x80, 25                                  // display on
  };

  static const std::uint8_t ST7735[] = {
    0x01, 0x80, 150,                                // software reset
    0x11, 0x80, 0xFF,                               // sleep out, 500 ms
    0xB1, 3, 0x01, 0x2C, 0x2D,                      // frame rate normal mode
    0xB2, 3, 0x01, 0x2C, 0x2D,                      // frame rate idle mode
    0xB3, 6, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D,    // frame rate partial mode
    0xB4, 1, 0x07,                                  // display inversion control, no inversion
    0xC0, 3, 0xA2, 0x02, 0x84,                      // power control 1
    0xC1, 1, 0xC5,                                  // power control 2
    0xC2, 2, 0x0A, 0x00,                            // power control 3
    0xC3, 2, 0x8A, 0x2A,                            // power control 4
    0xC4, 2, 0x8A, 0xEE,                            // power control 5
    0xC5, 1, 0x0E,                                  // VCOM control 1
    0xE0, 16, 0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10,
    0xE1, 16, 0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10,
    0x13, 0x80, 10,                                 // normal display mode on
    0x29, 0x80, 100                                 // display on
  };

  static const std::uint8_t ST7789[] = {
    0x01, 0x80, 150,                                // software reset
    0x11, 0x80, 10,                                 // sleep out
    0x13, 0x80, 10,                                 // normal display mode on
    0x29, 0x80, 10                                  // display on
  };

  static const std::uint8_t ST7796[] = {
    0x01, 0x80, 120,                                // software reset
    0x11, 0x80, 120,                                // sleep out
    0xF0, 1, 0xC3,                                  // command set control, enable part 1
    0xF0, 1, 0x96,                                  // enable part 2
    0xB4, 1, 0x01,                                  // display inversion control, 1 dot
    0xB6, 3, 0x80, 0x02, 0x3B,                      // display function control
    0xE8, 8, 0x40, 0x8A, 0x00, 0x00, 0x29, 0x19, 0xA5, 0x33,   // display output ctrl adjust
    0xC1, 1, 0x06,                                  // power control 2
    0xC2, 1, 0xA7,                                  // power control 3
    0xC5, 0x81, 0x18, 120,                          // VCOM control
    0xE0, 14, 0xF0, 0x09, 0x0B, 0x06, 0x04, 0x15, 0x2F, 0x54, 0x42, 0x3C, 0x17, 0x14, 0x18, 0x1B,
    0xE1, 0x8E, 0xE0, 0x09, 0x0B, 0x06, 0x04, 0x03, 0x2B, 0x43, 0x42, 0x3B, 0x16, 0x14, 0x17, 0x1B, 120,
    0xF0, 1, 0x3C,                                  // disable part 1
    0xF0, 0x81, 0x69, 120,                          // disable part 2
    0x29, 0x80, 20                                  // display on
  };

  static const std::uint8_t GC9A01[] = {
    0xEF, 0,
    0xEB, 1, 0x14,
    0xFE, 0,                                        // inter register enable 1
    0xEF, 0,                                        // inter register enable 2
    0xEB, 1, 0x14,
    0x84, 1, 0x40,
    0x85, 1, 0xFF,
    0x86, 1, 0xFF,
    0x87, 1, 0xFF,
    0x88, 1, 0x0A,
    0x89, 1, 0x21,
    0x8A, 1, 0x00,
    0x8B, 1, 0x80,
    0x8C, 1, 0x01,
    0x8D, 1, 0x01,
    0x8E, 1, 0xFF,
    0x8F, 1, 0xFF,
    0xB6, 2, 0x00, 0x00,                            // display function control
    0x90, 4, 0x08, 0x08, 0x08, 0x08,
    0xBD, 1, 0x06,
    0xBC, 1, 0x00,
    0xFF, 3, 0x60, 0x01, 0x04,
    0xC3, 1, 0x13,                                  // power control 2
    0xC4, 1, 0x13,                                  // power control 3
    0xC9, 1, 0x22,                                  // power control 4
    0xBE, 1, 0x11,
    0xE1, 2, 0x10, 0x0E,
    0xDF, 3, 0x21, 0x0C, 0x02,
    0xF0, 6, 0x45, 0x09, 0x08, 0x08, 0x26, 0x2A,    // gamma 1
    0xF1, 6, 0x43, 0x70, 0x72, 0x36, 0x37, 0x6F,    // gamma 2
    0xF2, 6, 0x45, 0x09, 0x08, 0x08, 0x26, 0x2A,    // gamma 3
    0xF3, 6, 0x43, 0x70, 0x72, 0x36, 0x37, 0x6F,    // gamma 4
    0xED, 2, 0x1B, 0x0B,
    0xAE, 1, 0x77,
    0xCD, 1, 0x63,
    0x70, 9, 0x07, 0x07, 0x04, 0x0E, 0x0F, 0x09, 0x07, 0x08, 0x03,
    0xE8, 1, 0x34,                                  // frame rate
    0x62, 12, 0x18, 0x0D, 0x71, 0xED, 0x70, 0x70, 0x18, 0x0F, 0x71, 0xEF, 0x70, 0x70,
    0x63, 12, 0x18, 0x11, 0x71, 0xF1, 0x70, 0x70, 0x18, 0x13, 0x71, 0xF3, 0x70, 0x70,
    0x64, 7, 0x28, 0x29, 0xF1, 0x01, 0xF1, 0x00, 0x07,
    0x66, 10, 0x3C, 0x00, 0xCD, 0x67, 0x45, 0x45, 0x10, 0x00, 0x00, 0x00,
    0x67, 10, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x01, 0x54, 0x10, 0x32, 0x98,
    0x74, 7, 0x10, 0x85, 0x80, 0x00, 0x00, 0x4E, 0x00,
    0x98, 2, 0x3E, 0x07,
    0x35, 0,                                        // tearing effect line on
    0x11, 0x80, 120,                                // sleep out
    0x29, 0x80, 20                                  // display on
  };

} // namespace MIPI_DCS_init


/**
 * Generic MIPI DCS display controller
 * The screen size is the LOGICAL size and must match the orientation (e.g. ILI9341: 240 x 320 for
 * orientation_0 / 180, 320 x 240 for orientation_90 / 270). orientation_0 is the datasheet default of
 * MADCTL (no MX / MY / MV), the reference drivers use MX as their rotation 0 on most modules, which is
 * orientation_180m here.
 * \param Controller Controller type, selects the init table and the defaults
 * \param Screen_Size_X Logical screen width in pixel
 * \param Screen_Size_Y Logical screen height in pixel
 * \param Orientation Hardware orientation of the display
 * \param RGB_to_BGR MADCTL BGR bit, most modules need it with RGB565 data (default: all but ST7789)
 * \param Invert Display inversion (21h), the ST7789 and GC9A01 modules need it
 * \param Color_18bit 18 bit pixels (3 bytes, 3Ah = 66h) instead of RGB565 (3Ah = 55h), mandatory for the ILI9488 on SPI
 * \param Offset_X Column offset of the panel in the GRAM, in the orientation the window commands see (after MADCTL),
 *                 e.g. ST7789 240 x 240: 0 / 0 for orientation_0, 0 / 80 for orientation_0m, 80 / 0 for orientation_90;
 *                 ST7735 128 x 160 "green tab": 2 / 1, 128 x 128: 2 / 3 (TFT_eSPI colstart / rowstart)
 * \param Offset_Y Row offset of the panel in the GRAM
 */
template<MIPI_DCS_controller_type Controller,
         std::uint16_t Screen_Size_X, std::uint16_t Screen_Size_Y,                 // X, Y size, must match the orientation
         drv::orientation_type Orientation = drv::orientation_0,                   // hardware orientation of the display
         bool RGB_to_BGR  = (Controller != MIPI_DCS_ST7789),                       // MADCTL BGR bit
         bool Invert      = (Controller == MIPI_DCS_ST7789) || (Controller == MIPI_DCS_GC9A01),   // display inversion on
         bool Color_18bit = (Controller == MIPI_DCS_ILI9488),                      // 18 bit pixels
         std::uint16_t Offset_X = 0U, std::uint16_t Offset_Y = 0U                  // panel offset in the GRAM
>
class MIPI_DCS final : public drv
{
  // MIPI DCS commands
  static const std::uint8_t CMD_NOP                 = 0x00U;
  static const std::uint8_t CMD_SWRESET             = 0x01U;    // software reset
  static const std::uint8_t CMD_SLPIN               = 0x10U;    // sleep in
  static const std::uint8_t CMD_SLPOUT              = 0x11U;    // sleep out
  static const std::uint8_t CMD_NORON               = 0x13U;    // normal display mode
  static const std::uint8_t CMD_INVOFF              = 0x20U;    // display inversion off
  static const std::uint8_t CMD_INVON               = 0x21U;    // display inversion on
  static const std::uint8_t CMD_DISPOFF             = 0x28U;    // display off
  static const std::uint8_t CMD_DISPON              = 0x29U;    // display on
  static const std::uint8_t CMD_CASET               = 0x2AU;    // column address set
  static const std::uint8_t CMD_RASET               = 0x2BU;    // row address set
  static const std::uint8_t CMD_RAMWR               = 0x2CU;    // memory write
  static const std::uint8_t CMD_RAMRD               = 0x2EU;    // memory read
  static const std::uint8_t CMD_MADCTL              = 0x36U;    // memory access control
  static const std::uint8_t CMD_COLMOD              = 0x3AU;    // pixel format
  static const std::uint8_t CMD_WRDISBV             = 0x51U;    // write display brightness
  static const std::uint8_t CMD_WRCTRLD             = 0x53U;    // write control display (brightness control on)

  // MADCTL bits
  static const std::uint8_t MADCTL_MY               = 0x80U;    // row address order
  static const std::uint8_t MADCTL_MX               = 0x40U;    // column address order
  static const std::uint8_t MADCTL_MV               = 0x20U;    // row / column exchange
  static const std::uint8_t MADCTL_ML               = 0x10U;    // vertical refresh order
  static const std::uint8_t MADCTL_BGR              = 0x08U;    // BGR color order
  static const std::uint8_t MADCTL_MH               = 0x04U;    // horizontal refresh order

  // orientation: MV exchanges the axes, MX / MY mirror the panel. orientation_0 = datasheet default, 90 = MX | MV,
  // 180 = MX | MY, 270 = MY | MV (the rotation sequence of the reference drivers), the 'm' orientations toggle MY
  static const std::uint8_t MADCTL = (RGB_to_BGR ? MADCTL_BGR : 0U) |
                                     ((Orientation == orientation_0)    ? 0U :
                                      (Orientation == orientation_0m)   ? MADCTL_MY :
                                      (Orientation == orientation_180)  ? (MADCTL_MX | MADCTL_MY) :
                                      (Orientation == orientation_180m) ? MADCTL_MX :
                                      (Orientation == orientation_90)   ? (MADCTL_MX | MADCTL_MV) :
                                      (Orientation == orientation_90m)  ? (MADCTL_MX | MADCTL_MY | MADCTL_MV) :
                                      (Orientation == orientation_270)  ? (MADCTL_MY | MADCTL_MV) :
                                                                          MADCTL_MV);

  // pixels per io::write() in a data stream
  static const std::size_t STREAM_PIXELS = 32U;
  static const std::size_t PIXEL_BYTES   = Color_18bit ? 3U : 2U;

  // vars
  const io::handle_type device_handle_;
  std::uint8_t          stream_[STREAM_PIXELS * 3U];  // data stream buffer
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
  MIPI_DCS(io::handle_type device_handle)
    : drv(Screen_Size_X, Screen_Size_Y,
          Screen_Size_X, Screen_Size_Y,         // no viewport support (screen = viewport)
          0, 0,                                 // no viewport support
          Orientation)
    , device_handle_(device_handle)
    , stream_idx_(0U)
    , win_({ 0, 0, 0, 0 })
    , pos_({ -1, -1 })
  { }


  /**
   * dtor
   * Shutdown the driver
   */
  ~MIPI_DCS()
  {
    shutdown();
  }


  virtual void init()
  {
    io::init(device_handle_);

    // vendor init table
    std::size_t length = 0U;
    const std::uint8_t* table = init_table(length);
    std::size_t i = 0U;
    while (i < length) {
      const std::uint8_t cmd   = table[i++];
      const std::uint8_t count = table[i++];
      const std::uint8_t args  = static_cast<std::uint8_t>(count & 0x7FU);
      write_cmd(cmd, &table[i], args);
      i += args;
      if (count & 0x80U) {
        const std::uint8_t ms = table[i++];
        io::delay(ms == 0xFFU ? 500U : ms);
      }
    }

    // orientation, color order and pixel format
    const std::uint8_t madctl = MADCTL;
    write_cmd(CMD_MADCTL, &madctl, 1U);
    const std::uint8_t colmod = Color_18bit ? 0x66U : 0x55U;
    write_cmd(CMD_COLMOD, &colmod, 1U);
    write_cmd(Invert ? CMD_INVON : CMD_INVOFF, nullptr, 0U);
    pos_ = { -1, -1 };

    cls();
  }


  virtual void shutdown()
  {
    // display off and sleep in
    write_cmd(CMD_DISPOFF, nullptr, 0U);
    write_cmd(CMD_SLPIN, nullptr, 0U);
    io::delay(5U);
  }


  inline virtual const char* version() const
  {
    return (const char*)VIC_HEAD_MIPI_DCS_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    return true;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

protected:

  /**
   * Clear screen: one stream over the whole screen
   * \param bg_color Background color
   */
  virtual void cls(color::value_type bg_color = color::none)
  {
    set_window(0, static_cast<std::int16_t>(Screen_Size_X - 1U), 0, static_cast<std::int16_t>(Screen_Size_Y - 1U));
    stream_begin();
    for (std::uint32_t i = 0U; i < static_cast<std::uint32_t>(Screen_Size_X) * Screen_Size_Y; ++i) {
      stream_pixel(bg_color);
    }
    stream_end();
    pos_ = { -1, -1 };    // the pointer wrapped to the window start, don't rely on it
  }


  // unused - data is written directly to the display
  virtual void present()
  { }


  ///////////////////////////////////////////////////////////////////////////////
  // G R A P H I C   F U N C T I O N S
  //

  virtual void pixel_set(vertex_type vertex, color::value_type color)
  {
    // check limits
    if (!screen_is_inside(vertex)) {
      return;
    }

    // set the address pointer if the controller is not already there
    if (pos_ != vertex) {
      set_window(vertex.x, static_cast<std::int16_t>(Screen_Size_X - 1U), vertex.y, static_cast<std::int16_t>(Screen_Size_Y - 1U));
    }
    stream_begin();
    stream_pixel(color);
    stream_end();
    advance(1U);
  }


  virtual color::value_type pixel_get(vertex_type vertex)
  {
    // check limits
    if (!screen_is_inside(vertex)) {
      return color::none;
    }

    // set the window to the pixel and read it (the io layer handles the dummy byte of the controller)
    set_window(vertex.x, vertex.x, vertex.y, vertex.y);
    write_cmd(CMD_RAMRD, nullptr, 0U);
    pos_ = { -1, -1 };
    std::uint8_t data[3] = { 0U, 0U, 0U };
    std::size_t  length  = 3U;
    if (!io::read(device_handle_, 1U, data, length, 0U) || (length != 3U)) {
      // reading is not supported by the interface, the pixel is unknown
      return color::none;
    }
    // the GRAM read returns 18 bit pixels, 6 bits per byte left aligned
    return color::argb(static_cast<std::uint8_t>(data[0] & 0xFCU), static_cast<std::uint8_t>(data[1] & 0xFCU), static_cast<std::uint8_t>(data[2] & 0xFCU));
  }


  ///////////////////////////////////////////////////////////////////////////////
  // overwritten drv functions for faster rendering
  //

  /**
   * Draw a horizontal line: one address setup and a pixel stream
   */
  virtual void line_horz(vertex_type v0, vertex_type v1, color::value_type color)
  {
    // clip to the screen
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
    stream_begin();
    for (std::int_fast16_t x = v0.x; x <= v1.x; ++x) {
      stream_pixel(color);
    }
    stream_end();
    advance(static_cast<std::size_t>(v1.x - v0.x + 1));
  }


  /**
   * Draw a vertical line: a one column wide window wraps along the line
   */
  virtual void line_vert(vertex_type v0, vertex_type v1, color::value_type color)
  {
    // clip to the screen
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
    stream_begin();
    for (std::int_fast16_t y = v0.y; y <= v1.y; ++y) {
      stream_pixel(color);
    }
    stream_end();
    advance(static_cast<std::size_t>(v1.y - v0.y + 1));
  }


  /**
   * Draw a box (filled rectangle): window and pixel stream
   * \param rect Box bounding, left/top is inside, right/bottom is outside the box
   */
  virtual void box(const rect_type& rect, color::value_type color)
  {
    rect_type r = rect;
    if (!clip(r)) {
      return;
    }
    set_window(r.left, static_cast<std::int16_t>(r.right - 1), r.top, static_cast<std::int16_t>(r.bottom - 1));
    stream_begin();
    for (std::uint32_t i = 0U; i < static_cast<std::uint32_t>(r.width()) * static_cast<std::uint32_t>(r.height()); ++i) {
      stream_pixel(color);
    }
    stream_end();
    advance(static_cast<std::size_t>(r.width()) * static_cast<std::size_t>(r.height()));
  }


  /**
   * Write a block of pixels: the address window is set to the (clipped) block and the pixels are streamed
   */
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
        stream_pixel(*p);
      }
    }
    stream_end();
    advance(static_cast<std::size_t>(r.width()) * static_cast<std::size_t>(r.height()));
  }


  ///////////////////////////////////////////////////////////////////////////////
  // D I S P L A Y   C O N T R O L
  //

public:

  /**
   * Enable/disable the display (28h / 29h), the GRAM is retained
   */
  virtual void display_enable(bool enable = true)
  {
    write_cmd(enable ? CMD_DISPON : CMD_DISPOFF, nullptr, 0U);
  }


  /**
   * Set the display brightness (51h / 53h), only effective on modules with the backlight connected to the controller
   * \param level 0: dark; 255: maximum
   */
  virtual void display_brightness(std::uint8_t level)
  {
    const std::uint8_t ctrl = 0x24U;    // brightness control block on, display dimming off
    write_cmd(CMD_WRCTRLD, &ctrl, 1U);
    write_cmd(CMD_WRDISBV, &level, 1U);
  }


  /**
   * Display inversion (20h / 21h) relative to the Invert default of the panel
   */
  virtual void display_inverse(bool inverse)
  {
    write_cmd((inverse != Invert) ? CMD_INVON : CMD_INVOFF, nullptr, 0U);
  }


  ///////////////////////////////////////////////////////////////////////////////

private:

  /**
   * Select the vendor init table of the controller
   */
  static const std::uint8_t* init_table(std::size_t& length)
  {
    switch (Controller) {
      case MIPI_DCS_ILI9341 : length = sizeof(MIPI_DCS_init::ILI9341); return MIPI_DCS_init::ILI9341;
      case MIPI_DCS_ILI9488 : length = sizeof(MIPI_DCS_init::ILI9488); return MIPI_DCS_init::ILI9488;
      case MIPI_DCS_ST7735  : length = sizeof(MIPI_DCS_init::ST7735);  return MIPI_DCS_init::ST7735;
      case MIPI_DCS_ST7789  : length = sizeof(MIPI_DCS_init::ST7789);  return MIPI_DCS_init::ST7789;
      case MIPI_DCS_ST7796  : length = sizeof(MIPI_DCS_init::ST7796);  return MIPI_DCS_init::ST7796;
      case MIPI_DCS_GC9A01  :
      default               : length = sizeof(MIPI_DCS_init::GC9A01);  return MIPI_DCS_init::GC9A01;
    }
  }


  /**
   * Clip a rect to the screen
   */
  static inline bool clip(rect_type& r)
  {
    if (r.left < 0) { r.left = 0; }
    if (r.top  < 0) { r.top  = 0; }
    if (r.right  > static_cast<std::int16_t>(Screen_Size_X)) { r.right  = static_cast<std::int16_t>(Screen_Size_X); }
    if (r.bottom > static_cast<std::int16_t>(Screen_Size_Y)) { r.bottom = static_cast<std::int16_t>(Screen_Size_Y); }
    return (r.left < r.right) && (r.top < r.bottom);
  }


  /**
   * Set the address window (logical coordinates, inclusive) and start the memory write
   * With MV set the controller exchanges the axes itself, so the window is always given in logical coordinates
   */
  void set_window(std::int16_t x0, std::int16_t x1, std::int16_t y0, std::int16_t y1)
  {
    const std::uint16_t c0 = static_cast<std::uint16_t>(x0 + Offset_X);
    const std::uint16_t c1 = static_cast<std::uint16_t>(x1 + Offset_X);
    const std::uint16_t r0 = static_cast<std::uint16_t>(y0 + Offset_Y);
    const std::uint16_t r1 = static_cast<std::uint16_t>(y1 + Offset_Y);
    const std::uint8_t caset[4] = { static_cast<std::uint8_t>(c0 >> 8U), static_cast<std::uint8_t>(c0), static_cast<std::uint8_t>(c1 >> 8U), static_cast<std::uint8_t>(c1) };
    const std::uint8_t raset[4] = { static_cast<std::uint8_t>(r0 >> 8U), static_cast<std::uint8_t>(r0), static_cast<std::uint8_t>(r1 >> 8U), static_cast<std::uint8_t>(r1) };
    write_cmd(CMD_CASET, caset, 4U);
    write_cmd(CMD_RASET, raset, 4U);
    write_cmd(CMD_RAMWR, nullptr, 0U);
    win_ = { x0, x1, y0, y1 };
    pos_ = { x0, y0 };
  }


  /**
   * Advance the tracked address pointer like the controller does after n written pixels
   */
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


  /**
   * Send a command with its parameters: command byte with io option 0, parameters with option 1
   */
  inline void write_cmd(std::uint8_t cmd, const std::uint8_t* args, std::size_t length) const
  {
    io::write(device_handle_, 0U, &cmd, 1U, nullptr, 0U);
    if (length != 0U) {
      io::write(device_handle_, 1U, args, length, nullptr, 0U);
    }
  }


  /**
   * Pixel data stream, io option 1, sent in blocks of STREAM_PIXELS
   */
  inline void stream_begin()
  {
    stream_idx_ = 0U;
  }

  inline void stream_pixel(color::value_type color)
  {
    if (Color_18bit) {
      // 18 bit: three bytes, 6 bits left aligned
      stream_[stream_idx_++] = static_cast<std::uint8_t>(color::get_red(color)   & 0xFCU);
      stream_[stream_idx_++] = static_cast<std::uint8_t>(color::get_green(color) & 0xFCU);
      stream_[stream_idx_++] = static_cast<std::uint8_t>(color::get_blue(color)  & 0xFCU);
    }
    else {
      const std::uint16_t pixel = color::color_to_RGB565(color);
      stream_[stream_idx_++] = static_cast<std::uint8_t>(pixel >> 8U);
      stream_[stream_idx_++] = static_cast<std::uint8_t>(pixel);
    }
    if (stream_idx_ >= STREAM_PIXELS * PIXEL_BYTES) {
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

#endif  // _VIC_HEAD_MIPI_DCS_H_
