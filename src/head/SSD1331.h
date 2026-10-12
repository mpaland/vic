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
// \brief SSD1331 head, 96 x 64 RGB OLED controller with 65k colors (RGB565)
// The head writes directly into the GDDRAM of the controller (no framebuffer).
// Pixels are streamed through the column/row address window with native auto
// increment, the orientation is done with the remap bits of the controller.
// The graphic acceleration commands (GAC) are used for clear, filled boxes and
// move (copy) when Use_GAC is set. The controller executes a GAC command
// asynchronously and has no busy flag, the datasheet gives no execution time.
// If artifacts appear on fast buses set GAC_Delay (ms) or disable the GAC.
// The GDDRAM can only be read on the parallel buses, on SPI pixel_get() returns
// color::none; put the framebuffer head in front if the pixels must be read.
// See doc/SSD1331.md for the programming summary of the controller.
//
// Interface option values of io::write(): 0 = command, 1 = data (D/C# line).
// The head emits 8 bit bytes for all interfaces, a 16 bit bus io layer packs two
// data bytes into one word (D15..D8 = first byte).
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_SSD1331_H_
#define _VIC_HEAD_SSD1331_H_

#include "../drv.h"


// defines the head name and version
#define VIC_HEAD_SSD1331_VERSION   "SSD1331 head 1.00"

namespace vic {
namespace head {


// interface type
typedef enum tag_SSD1331_interface_type {
  SSD1331_SPI,    // 4-wire SPI (D/C# line is the io option), write only
  SSD1331_PAR8,   // 8 bit 6800/8080 parallel bus, pixel read back possible
  SSD1331_PAR16   // 16 bit parallel bus, one pixel per word, pixel read back possible
} SSD1331_interface_type;


/**
 * SSD1331 display controller
 * The GDDRAM is always 96 columns x 64 rows. The screen size is the LOGICAL size and must match the orientation:
 * 96 x 64 for orientation_0 / 180 (and the mirrored ones), 64 x 96 for orientation_90 / 270.
 * orientation_0 is the datasheet default (no remap), the common 0.96" modules are upright with orientation_180.
 * \param Screen_Size_X Logical screen width in pixel
 * \param Screen_Size_Y Logical screen height in pixel
 * \param Orientation Hardware orientation of the display
 * \param Interface_Mode Interface mode
 * \param RGB_to_BGR Swap the R and B segments (remap bit A[2]), panel dependent
 * \param Use_GAC Use the graphic acceleration commands for cls(), box() and move()
 * \param GAC_Delay Delay in [ms] after every GAC command, 0 = none
 */
template<std::uint16_t Screen_Size_X = 96U, std::uint16_t Screen_Size_Y = 64U,   // X, Y size, must match the orientation
         drv::orientation_type Orientation = drv::orientation_0,                  // hardware orientation of the display
         SSD1331_interface_type Interface_Mode = SSD1331_SPI,                     // interface mode
         bool RGB_to_BGR = false,                                                 // swap R and B color components
         bool Use_GAC = true,                                                     // use the graphic acceleration commands
         std::uint8_t GAC_Delay = 0U                                              // delay in ms after a GAC command
>
class SSD1331 final : public drv
{
  // fundamental commands
  static const std::uint8_t CMD_COLUMN_ADDR         = 0x15U;    // column start and end address
  static const std::uint8_t CMD_ROW_ADDR            = 0x75U;    // row start and end address
  static const std::uint8_t CMD_CONTRAST_A          = 0x81U;    // contrast color A (SA segments)
  static const std::uint8_t CMD_CONTRAST_B          = 0x82U;    // contrast color B (SB segments)
  static const std::uint8_t CMD_CONTRAST_C          = 0x83U;    // contrast color C (SC segments)
  static const std::uint8_t CMD_MASTER_CURRENT      = 0x87U;    // master current control, 1/16 .. 16/16
  static const std::uint8_t CMD_PRECHARGE_SPEED_A   = 0x8AU;    // second precharge speed color A
  static const std::uint8_t CMD_PRECHARGE_SPEED_B   = 0x8BU;    // second precharge speed color B
  static const std::uint8_t CMD_PRECHARGE_SPEED_C   = 0x8CU;    // second precharge speed color C
  static const std::uint8_t CMD_REMAP               = 0xA0U;    // remap and data format
  static const std::uint8_t CMD_START_LINE          = 0xA1U;    // display start line
  static const std::uint8_t CMD_DISPLAY_OFFSET      = 0xA2U;    // display offset (COM shift)
  static const std::uint8_t CMD_DISPLAY_NORMAL      = 0xA4U;    // normal display (RAM content)
  static const std::uint8_t CMD_DISPLAY_ALL_ON      = 0xA5U;    // all pixels GS63
  static const std::uint8_t CMD_DISPLAY_ALL_OFF     = 0xA6U;    // all pixels GS0
  static const std::uint8_t CMD_DISPLAY_INVERSE     = 0xA7U;    // inverse display
  static const std::uint8_t CMD_MUX_RATIO           = 0xA8U;    // multiplex ratio N+1, 16..64
  static const std::uint8_t CMD_DIM_MODE            = 0xABU;    // dim mode setting
  static const std::uint8_t CMD_MASTER_CONFIG       = 0xADU;    // master configuration (external VCC)
  static const std::uint8_t CMD_DISPLAY_ON_DIM      = 0xACU;    // display on in dim mode
  static const std::uint8_t CMD_DISPLAY_OFF         = 0xAEU;    // display off (sleep)
  static const std::uint8_t CMD_DISPLAY_ON          = 0xAFU;    // display on
  static const std::uint8_t CMD_POWER_SAVE          = 0xB0U;    // power save mode, 0x1A enable, 0x0B disable
  static const std::uint8_t CMD_PHASE_PERIOD        = 0xB1U;    // phase 1 and 2 period
  static const std::uint8_t CMD_CLOCK_DIV           = 0xB3U;    // display clock divider / oscillator frequency
  static const std::uint8_t CMD_GRAY_SCALE_TABLE    = 0xB8U;    // gray scale table, 32 entries
  static const std::uint8_t CMD_GRAY_SCALE_LINEAR   = 0xB9U;    // linear gray scale table
  static const std::uint8_t CMD_PRECHARGE_LEVEL     = 0xBBU;    // precharge voltage level
  static const std::uint8_t CMD_VCOMH               = 0xBEU;    // VCOMH level
  static const std::uint8_t CMD_NOP                 = 0xE3U;    // no operation
  static const std::uint8_t CMD_COMMAND_LOCK        = 0xFDU;    // command lock, 0x12 unlock, 0x16 lock

  // graphic acceleration commands
  static const std::uint8_t CMD_DRAW_LINE           = 0x21U;    // draw line
  static const std::uint8_t CMD_DRAW_RECT           = 0x22U;    // draw rectangle
  static const std::uint8_t CMD_COPY                = 0x23U;    // copy area
  static const std::uint8_t CMD_DIM_WINDOW          = 0x24U;    // dim window
  static const std::uint8_t CMD_CLEAR_WINDOW        = 0x25U;    // clear window
  static const std::uint8_t CMD_FILL_MODE           = 0x26U;    // fill enable (A0) / reverse copy (A4)
  static const std::uint8_t CMD_SCROLL_SETUP        = 0x27U;    // continuous scrolling setup
  static const std::uint8_t CMD_SCROLL_OFF          = 0x2EU;    // deactivate scrolling
  static const std::uint8_t CMD_SCROLL_ON           = 0x2FU;    // activate scrolling

  // remap bits (A0h)
  static const std::uint8_t REMAP_VERTICAL_INC      = 0x01U;    // A[0] vertical address increment
  static const std::uint8_t REMAP_COLUMN            = 0x02U;    // A[1] column 95 is mapped to SEG0 (X mirror)
  static const std::uint8_t REMAP_BGR               = 0x04U;    // A[2] reverse segment order SC, SB, SA
  static const std::uint8_t REMAP_COM_LR            = 0x08U;    // A[3] COM left/right swap
  static const std::uint8_t REMAP_COM_SCAN          = 0x10U;    // A[4] scan from COM[N-1] to COM0 (Y mirror)
  static const std::uint8_t REMAP_COM_SPLIT         = 0x20U;    // A[5] COM split odd/even
  static const std::uint8_t REMAP_COLOR_65K         = 0x40U;    // A[7:6] = 01: 65k color format 1

  // default contrast of the three colors (typical 96 x 64 module values)
  static const std::uint8_t CONTRAST_A              = 0x91U;
  static const std::uint8_t CONTRAST_B              = 0x50U;
  static const std::uint8_t CONTRAST_C              = 0x7DU;

  // orientation: the GDDRAM is 96 columns x 64 rows, the screen size is the logical size
  static const bool ROTATED  = (Orientation == orientation_90)  || (Orientation == orientation_270) ||
                               (Orientation == orientation_90m) || (Orientation == orientation_270m);
  static const bool MIRROR_X = (Orientation == orientation_180) || (Orientation == orientation_180m) ||
                               (Orientation == orientation_90)  || (Orientation == orientation_90m);
  static const bool MIRROR_Y = (Orientation == orientation_0m)  || (Orientation == orientation_180) ||
                               (Orientation == orientation_270) || (Orientation == orientation_90m);
  static const std::uint16_t RAM_SIZE_X = ROTATED ? Screen_Size_Y : Screen_Size_X;   // RAM columns, 96
  static const std::uint16_t RAM_SIZE_Y = ROTATED ? Screen_Size_X : Screen_Size_Y;   // RAM rows, 64
  static const std::uint8_t  REMAP      = REMAP_COLOR_65K | REMAP_COM_SPLIT |
                                          (ROTATED    ? REMAP_VERTICAL_INC : 0U) |
                                          (MIRROR_X   ? REMAP_COLUMN       : 0U) |
                                          (MIRROR_Y   ? REMAP_COM_SCAN     : 0U) |
                                          (RGB_to_BGR ? REMAP_BGR          : 0U);

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
  SSD1331(io::handle_type device_handle)
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
  ~SSD1331()
  {
    shutdown();
  }


  virtual void init()
  {
    io::init(device_handle_);

    // values of the Adafruit_SSD1331 sequence; Adafruit sends remap 72h (COM scan remap + column remap), which is
    // orientation_180 here, 76h (with color swap) is orientation_180 with RGB_to_BGR = true
    const std::uint8_t cmd[] = {
      CMD_COMMAND_LOCK,       0x12U,                                    // unlock the command interface
      CMD_DISPLAY_OFF,                                                  // sleep while configuring
      CMD_REMAP,              REMAP,                                    // orientation, color order, 65k format 1
      CMD_START_LINE,         0x00U,                                    // display start line 0
      CMD_DISPLAY_OFFSET,     0x00U,                                    // no COM offset
      CMD_DISPLAY_NORMAL,                                               // display the RAM content
      CMD_MUX_RATIO,          static_cast<std::uint8_t>(RAM_SIZE_Y - 1U),   // all rows
      CMD_MASTER_CONFIG,      0x8EU,                                    // external VCC supply (A[0] must be 0)
      CMD_POWER_SAVE,         0x0BU,                                    // power save mode off
      CMD_PHASE_PERIOD,       0x31U,                                    // phase 1 = 1 DCLK, phase 2 = 3 DCLK
      CMD_CLOCK_DIV,          0xF0U,                                    // divide ratio 1, max oscillator frequency
      CMD_PRECHARGE_SPEED_A,  0x64U,                                    // second precharge speed, all three must be sent together
      CMD_PRECHARGE_SPEED_B,  0x78U,
      CMD_PRECHARGE_SPEED_C,  0x64U,
      CMD_PRECHARGE_LEVEL,    0x3AU,                                    // precharge voltage ~0.47 x VCC
      CMD_VCOMH,              0x3EU,                                    // VCOMH = 0.83 x VCC
      CMD_MASTER_CURRENT,     0x06U,                                    // master current 7/16
      CMD_CONTRAST_A,         CONTRAST_A,                               // color balance
      CMD_CONTRAST_B,         CONTRAST_B,
      CMD_CONTRAST_C,         CONTRAST_C,
      CMD_FILL_MODE,          static_cast<std::uint8_t>(Use_GAC ? 0x01U : 0x00U),   // filled rectangles, no reverse copy
      CMD_SCROLL_OFF,                                                   // no scrolling
      CMD_DISPLAY_ON                                                    // display on, SEG/COM are driven after 100 ms
    };
    write_cmd(cmd, sizeof(cmd));
    pos_ = { -1, -1 };

    cls();
  }


  virtual void shutdown()
  {
    // switch off the display (sleep mode), VCC may be removed afterwards
    display_enable(false);
  }


  inline virtual const char* version() const
  {
    return (const char*)VIC_HEAD_SSD1331_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    // SSD1331 is a true graphic display
    return true;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

protected:

  /**
   * Clear screen
   * \param bg_color Background color
   */
  virtual void cls(color::value_type bg_color = color::none)
  {
    const std::uint16_t pixel = color::color_to_RGB565(bg_color);
    if (Use_GAC) {
      if (pixel == 0U) {
        // clear window: the RAM of the window is set to zero
        const std::uint8_t cmd[] = { CMD_CLEAR_WINDOW, 0x00U, 0x00U, static_cast<std::uint8_t>(RAM_SIZE_X - 1U), static_cast<std::uint8_t>(RAM_SIZE_Y - 1U) };
        write_cmd(cmd, sizeof(cmd));
        gac_done();
      }
      else {
        fill_rect(0U, 0U, static_cast<std::uint8_t>(RAM_SIZE_X - 1U), static_cast<std::uint8_t>(RAM_SIZE_Y - 1U), pixel);
      }
    }
    else {
      set_window(0, static_cast<std::int16_t>(Screen_Size_X - 1U), 0, static_cast<std::int16_t>(Screen_Size_Y - 1U));
      stream_begin();
      for (std::uint32_t i = 0U; i < static_cast<std::uint32_t>(Screen_Size_X) * Screen_Size_Y; ++i) {
        stream_pixel(pixel);
      }
      stream_end();
      pos_ = { -1, -1 };    // the pointer wrapped to the window start, don't rely on it
    }
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
      // out of bounds
      return;
    }

    // set the address pointer if the controller is not already there
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
    // check limits
    if (!screen_is_inside(vertex)) {
      // out of bounds
      return color::none;
    }

    if (Interface_Mode == SSD1331_SPI) {
      // no read back on the serial interface, the pixel is unknown
      return color::none;
    }

    // set the address pointer and read one pixel (the io layer handles the dummy read of the controller)
    set_window(vertex.x, static_cast<std::int16_t>(Screen_Size_X - 1U), vertex.y, static_cast<std::int16_t>(Screen_Size_Y - 1U));
    pos_ = { -1, -1 };    // the pointer position after a read is not tracked
    std::uint8_t data[2] = { 0U, 0U };
    std::size_t  length  = 2U;
    if (!io::read(device_handle_, 1U, data, length, 0U) || (length != 2U)) {
      // reading is not supported by the interface, the pixel is unknown
      return color::none;
    }
    return color::RGB565_to_color(static_cast<std::uint16_t>((static_cast<std::uint16_t>(data[0]) << 8U) | data[1]));
  }


  ///////////////////////////////////////////////////////////////////////////////
  // overwritten drv functions for faster rendering
  //

  /**
   * Draw a horizontal line: one address setup and a pixel stream
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, y component is ignored
   * \param color Line color
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

    // the pointer must be at v0 and the window must cover the line
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


  /**
   * Draw a vertical line: a one column wide window wraps along the line
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, x component is ignored
   * \param color Line color
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
    const std::uint16_t pixel = color::color_to_RGB565(color);
    stream_begin();
    for (std::int_fast16_t y = v0.y; y <= v1.y; ++y) {
      stream_pixel(pixel);
    }
    stream_end();
    advance(static_cast<std::size_t>(v1.y - v0.y + 1));
  }


  /**
   * Draw a box (filled rectangle), with the GAC it is one draw rectangle command
   * \param rect Box bounding, left/top is inside, right/bottom is outside the box
   * \param color Box color
   */
  virtual void box(const rect_type& rect, color::value_type color)
  {
    // clip to the screen
    rect_type r = rect;
    if (!clip(r)) {
      return;
    }

    const std::uint16_t pixel = color::color_to_RGB565(color);
    if (Use_GAC) {
      const vertex_type r0 = to_ram({ r.left, r.top });
      const vertex_type r1 = to_ram({ static_cast<std::int16_t>(r.right - 1), static_cast<std::int16_t>(r.bottom - 1) });
      fill_rect(static_cast<std::uint8_t>(r0.x), static_cast<std::uint8_t>(r0.y), static_cast<std::uint8_t>(r1.x), static_cast<std::uint8_t>(r1.y), pixel);
    }
    else {
      set_window(r.left, static_cast<std::int16_t>(r.right - 1), r.top, static_cast<std::int16_t>(r.bottom - 1));
      stream_begin();
      for (std::uint32_t i = 0U; i < static_cast<std::uint32_t>(r.width()) * static_cast<std::uint32_t>(r.height()); ++i) {
        stream_pixel(pixel);
      }
      stream_end();
      advance(static_cast<std::size_t>(r.width()) * static_cast<std::size_t>(r.height()));
    }
  }


  /**
   * Move display area, with the GAC it is one copy command (the only way on the write only SPI bus)
   * \param source Source rect, right and bottom are outside the rect
   * \param destination Destination top/left vertex
   */
  virtual void move(const rect_type& source, vertex_type destination)
  {
    if (!Use_GAC) {
      drv::move(source, destination);
      return;
    }

    // clip the source and the destination to the screen
    rect_type s = source;
    const std::int16_t dx = static_cast<std::int16_t>(destination.x - s.left);
    const std::int16_t dy = static_cast<std::int16_t>(destination.y - s.top);
    if (!clip(s)) {
      return;
    }
    if (s.left   + dx < 0)                                          { s.left   = static_cast<std::int16_t>(-dx); }
    if (s.top    + dy < 0)                                          { s.top    = static_cast<std::int16_t>(-dy); }
    if (s.right  + dx > static_cast<std::int16_t>(Screen_Size_X))   { s.right  = static_cast<std::int16_t>(Screen_Size_X - dx); }
    if (s.bottom + dy > static_cast<std::int16_t>(Screen_Size_Y))   { s.bottom = static_cast<std::int16_t>(Screen_Size_Y - dy); }
    if ((s.left >= s.right) || (s.top >= s.bottom)) {
      return;
    }

    const vertex_type r0 = to_ram({ s.left, s.top });
    const vertex_type r1 = to_ram({ static_cast<std::int16_t>(s.right - 1), static_cast<std::int16_t>(s.bottom - 1) });
    const vertex_type d  = to_ram({ static_cast<std::int16_t>(s.left + dx), static_cast<std::int16_t>(s.top + dy) });
    const std::uint8_t cmd[] = { CMD_COPY,
                                 static_cast<std::uint8_t>(r0.x), static_cast<std::uint8_t>(r0.y),
                                 static_cast<std::uint8_t>(r1.x), static_cast<std::uint8_t>(r1.y),
                                 static_cast<std::uint8_t>(d.x),  static_cast<std::uint8_t>(d.y) };
    write_cmd(cmd, sizeof(cmd));
    gac_done();
  }


  /**
   * Write a block of pixels: the address window is set to the (clipped) block and the pixels are streamed
   * \param rect Destination rect, right and bottom are outside the rect
   * \param pixels Pixel block in ARGB format, row major
   * \param stride Distance between two rows in pixels
   */
  virtual void blit(const rect_type& rect, const color::value_type* pixels, std::size_t stride)
  {
    // clip to the screen
    rect_type r = rect;
    std::size_t skip_x = 0U, skip_y = 0U;
    if (r.left < 0) { skip_x = static_cast<std::size_t>(-r.left); r.left = 0; }
    if (r.top  < 0) { skip_y = static_cast<std::size_t>(-r.top);  r.top  = 0; }
    if (!clip(r)) {
      return;
    }

    // set the window to the block, the address pointer wraps inside the window
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

  /**
   * Enable/disable the display
   * \param enable True to switch the display on, false to switch it off (sleep mode, RAM is retained)
   */
  virtual void display_enable(bool enable = true)
  {
    const std::uint8_t cmd = enable ? CMD_DISPLAY_ON : CMD_DISPLAY_OFF;
    write_cmd(&cmd, 1U);
  }


  /**
   * Set the display brightness (master current, 16 steps)
   * \param level 0: minimum (1/16); 255: maximum (16/16)
   */
  virtual void display_brightness(std::uint8_t level)
  {
    const std::uint8_t cmd[] = { CMD_MASTER_CURRENT, static_cast<std::uint8_t>(level >> 4U) };
    write_cmd(cmd, sizeof(cmd));
  }


  /**
   * Set the display contrast, the color balance of the three segment currents is kept
   * \param level 0: minimum; 255: maximum
   */
  virtual void display_contrast(std::uint8_t level)
  {
    const std::uint8_t cmd[] = { CMD_CONTRAST_A, static_cast<std::uint8_t>((static_cast<std::uint16_t>(CONTRAST_A) * level) / 255U),
                                 CMD_CONTRAST_B, static_cast<std::uint8_t>((static_cast<std::uint16_t>(CONTRAST_B) * level) / 255U),
                                 CMD_CONTRAST_C, static_cast<std::uint8_t>((static_cast<std::uint16_t>(CONTRAST_C) * level) / 255U) };
    write_cmd(cmd, sizeof(cmd));
  }


  /**
   * Inverse display (GS0 <-> GS63), the RAM content is not changed
   * \param inverse True for inverse display
   */
  void display_inverse(bool inverse)
  {
    const std::uint8_t cmd = inverse ? CMD_DISPLAY_INVERSE : CMD_DISPLAY_NORMAL;
    write_cmd(&cmd, 1U);
  }


  ///////////////////////////////////////////////////////////////////////////////

private:

  /**
   * Map a logical screen vertex to the RAM address (column, row): the rotated orientations transpose the
   * axes, the mirrors are done by the remap bits of the controller
   */
  static inline vertex_type to_ram(vertex_type v)
  {
    if (ROTATED) {
      const vertex_type r = { v.y, v.x };
      return r;
    }
    return v;
  }


  /**
   * Clip a rect to the screen
   * \param r Rect to clip, right/bottom are outside
   * \return False if nothing is left
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
   * Set the address window (logical coordinates, inclusive) and the address pointer to its start
   */
  void set_window(std::int16_t x0, std::int16_t x1, std::int16_t y0, std::int16_t y1)
  {
    const vertex_type r0 = to_ram({ x0, y0 });
    const vertex_type r1 = to_ram({ x1, y1 });
    const std::uint8_t cmd[] = { CMD_COLUMN_ADDR, static_cast<std::uint8_t>(r0.x), static_cast<std::uint8_t>(r1.x),
                                 CMD_ROW_ADDR,    static_cast<std::uint8_t>(r0.y), static_cast<std::uint8_t>(r1.y) };
    write_cmd(cmd, sizeof(cmd));
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
   * GAC draw rectangle with fill, RAM coordinates
   */
  void fill_rect(std::uint8_t c0, std::uint8_t r0, std::uint8_t c1, std::uint8_t r1, std::uint16_t pixel)
  {
    const std::uint8_t ca = static_cast<std::uint8_t>((pixel >> 10U) & 0x3EU);   // color C, bits 5:1
    const std::uint8_t cb = static_cast<std::uint8_t>((pixel >>  5U) & 0x3FU);   // color B, bits 5:0
    const std::uint8_t cc = static_cast<std::uint8_t>((static_cast<unsigned>(pixel) <<  1U) & 0x3EU);   // color A, bits 5:1
    const std::uint8_t cmd[] = { CMD_DRAW_RECT, c0, r0, c1, r1, ca, cb, cc, ca, cb, cc };   // outline and fill color
    write_cmd(cmd, sizeof(cmd));
    gac_done();
  }


  /**
   * A GAC command was sent: optional delay, the address pointer is unknown afterwards
   */
  inline void gac_done()
  {
    if (GAC_Delay != 0U) {
      io::delay(GAC_Delay);
    }
    pos_ = { -1, -1 };
  }


  /**
   * Send a command sequence, io option 0
   */
  inline void write_cmd(const std::uint8_t* cmd, std::size_t length) const
  {
    io::write(device_handle_, 0U, cmd, length, nullptr, 0U);
  }


  /**
   * Pixel data stream, io option 1, sent in blocks of STREAM_PIXELS
   */
  inline void stream_begin()
  {
    stream_idx_ = 0U;
  }

  inline void stream_pixel(std::uint16_t pixel)
  {
    stream_[stream_idx_++] = static_cast<std::uint8_t>(pixel >> 8U);     // C4..C0 B5..B3
    stream_[stream_idx_++] = static_cast<std::uint8_t>(pixel);           // B2..B0 A4..A0
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

#endif  // _VIC_HEAD_SSD1331_H_
