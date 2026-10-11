///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2018, PALANDesign Hannover, Germany
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
// \brief ILI9325 head, 240 x 320 TFT controller with 18 bit GRAM
// The head writes directly into the GRAM of the controller (no framebuffer),
// the GRAM window and the address counter direction (entry mode) are set up
// so that every vic orientation is rendered with native auto increment.
// See doc/ILI9325.md for the programming summary of the controller.
//
// Interface modes:
//   0 : device (SPI) via io::write(), option 0 = index register, 1 = data (start byte is built by the io layer)
//   8 : 8 bit memory mapped bus (DB17..DB10)
//   9 : 9 bit memory mapped bus (DB17..DB9), pixels are always 18 bit
//  16 : 16 bit memory mapped bus
//  18 : 18 bit memory mapped bus (32 bit access), pixels are always 18 bit
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_ILI9325_H_
#define _VIC_HEAD_ILI9325_H_

#include "../drv.h"


// defines the driver name and version
#define VIC_HEAD_ILI9325_VERSION   "ILI9325 driver 0.30"


namespace vic {
namespace head {


/**
 * ILI9325 display controller
 * The GRAM is always 240 columns x 320 lines. The screen size is the LOGICAL size and must match the orientation:
 * 240 x 320 for orientation_0 / 180 (and the mirrored ones), 320 x 240 for orientation_90 / 270.
 * \param Screen_Size_X Logical screen width in pixel
 * \param Screen_Size_Y Logical screen height in pixel
 * \param Orientation Hardware orientation of the display
 * \param Interface_Mode 0: device (SPI), 8: mem 8 bit, 9: mem 9 bit, 16: mem 16 bit, 18: mem 18 bit
 * \param Color_256k false: 64k colors (RGB565), true: 256k colors (RGB666). The 9 and 18 bit buses are always 256k.
 *                   Don't use true with the SPI device interface, the SPI 18 bit format (TRI = 1, DFM = 1) is not supported.
 * \param RGB_to_BGR Swap R and B color components, some displays need that
 */
template<std::uint16_t Screen_Size_X = 240U, std::uint16_t Screen_Size_Y = 320U,  // X, Y size, must match the orientation
         drv::orientation_type Orientation = drv::orientation_0,                  // hardware orientation of the display
         std::uint8_t Interface_Mode = 0U,                                        // 0: device, 8: mem 8bit, 9: mem 9bit, 16: mem 16 bit, 18: mem 18bit
         bool Color_256k = false,                                                 // false: 64k, true: 256k color mode. Used in 'device', '8 or 16 bit' mode. Use false for SPI device interface!
         bool RGB_to_BGR = false                                                  // swap R and B color components, so displays need that
>
class ILI9325 final : public drv
{
  // register address map
  static const std::uint8_t REG_ID_CODE                = 0x00U;
  static const std::uint8_t REG_OSC_CTRL               = 0x00U;
  static const std::uint8_t REG_DRV_OUTPUT_CTRL        = 0x01U;
  static const std::uint8_t REG_DRV_WAVE_CTRL          = 0x02U;
  static const std::uint8_t REG_ENTRY_MODE             = 0x03U;
  static const std::uint8_t REG_RESIZE_CTRL            = 0x04U;
  static const std::uint8_t REG_DISP_CTRL1             = 0x07U;
  static const std::uint8_t REG_DISP_CTRL2             = 0x08U;
  static const std::uint8_t REG_DISP_CTRL3             = 0x09U;
  static const std::uint8_t REG_DISP_CTRL4             = 0x0AU;
  static const std::uint8_t REG_RGB_DISPLAY_IF_CTRL1   = 0x0CU;
  static const std::uint8_t REG_FRM_MARKER_POS         = 0x0DU;
  static const std::uint8_t REG_RGB_DISPLAY_IF_CTRL2   = 0x0FU;
  static const std::uint8_t REG_POW_CTRL1              = 0x10U;
  static const std::uint8_t REG_POW_CTRL2              = 0x11U;
  static const std::uint8_t REG_POW_CTRL3              = 0x12U;
  static const std::uint8_t REG_POW_CTRL4              = 0x13U;
  static const std::uint8_t REG_GRAM_HOR_ADDR          = 0x20U;
  static const std::uint8_t REG_GRAM_VER_ADDR          = 0x21U;
  static const std::uint8_t REG_GRAM_DATA              = 0x22U;
  static const std::uint8_t REG_POW_CTRL7              = 0x29U;
  static const std::uint8_t REG_FRM_RATE_COLOR_CTRL    = 0x2BU;
  static const std::uint8_t REG_GAMMA_CTRL1            = 0x30U;
  static const std::uint8_t REG_GAMMA_CTRL2            = 0x31U;
  static const std::uint8_t REG_GAMMA_CTRL3            = 0x32U;
  static const std::uint8_t REG_GAMMA_CTRL4            = 0x35U;
  static const std::uint8_t REG_GAMMA_CTRL5            = 0x36U;
  static const std::uint8_t REG_GAMMA_CTRL6            = 0x37U;
  static const std::uint8_t REG_GAMMA_CTRL7            = 0x38U;
  static const std::uint8_t REG_GAMMA_CTRL8            = 0x39U;
  static const std::uint8_t REG_GAMMA_CTRL9            = 0x3CU;
  static const std::uint8_t REG_GAMMA_CTRL10           = 0x3DU;
  static const std::uint8_t REG_HOR_ADDR_START         = 0x50U;
  static const std::uint8_t REG_HOR_ADDR_END           = 0x51U;
  static const std::uint8_t REG_VER_ADDR_START         = 0x52U;
  static const std::uint8_t REG_VER_ADDR_END           = 0x53U;
  static const std::uint8_t REG_GATE_SCAN_CTRL1        = 0x60U;
  static const std::uint8_t REG_GATE_SCAN_CTRL2        = 0x61U;
  static const std::uint8_t REG_GATE_SCAN_CTRL3        = 0x6AU;
  static const std::uint8_t REG_PART_IMG1_DISP_POS     = 0x80U;
  static const std::uint8_t REG_PART_IMG1_START        = 0x81U;
  static const std::uint8_t REG_PART_IMG1_END          = 0x82U;
  static const std::uint8_t REG_PART_IMG2_DISP_POS     = 0x83U;
  static const std::uint8_t REG_PART_IMG2_START        = 0x84U;
  static const std::uint8_t REG_PART_IMG2_END          = 0x85U;
  static const std::uint8_t REG_PANEL_IF_CTRL1         = 0x90U;
  static const std::uint8_t REG_PANEL_IF_CTRL2         = 0x92U;
  static const std::uint8_t REG_PANEL_IF_CTRL3         = 0x93U;
  static const std::uint8_t REG_PANEL_IF_CTRL4         = 0x95U;
  static const std::uint8_t REG_PANEL_IF_CTRL5         = 0x97U;
  static const std::uint8_t REG_OTP_VCM_PRG_CTRL       = 0xA1U;
  static const std::uint8_t REG_OTP_VCM_STATUS         = 0xA2U;
  static const std::uint8_t REG_OTP_PRG_ID_KEY         = 0xA5U;

  // the 9 and 18 bit buses transfer 18 bit pixels only
  static const bool COLOR_18 = Color_256k || (Interface_Mode == 9U) || (Interface_Mode == 18U);

  // orientation: the GRAM is always 240 x 320 (columns x lines), the screen size is the logical size
  static const bool ROTATED = (Orientation == orientation_90)  || (Orientation == orientation_270) ||
                              (Orientation == orientation_90m) || (Orientation == orientation_270m);
  static const std::uint16_t GRAM_SIZE_X = ROTATED ? Screen_Size_Y : Screen_Size_X;   // GRAM columns, 240
  static const std::uint16_t GRAM_SIZE_Y = ROTATED ? Screen_Size_X : Screen_Size_Y;   // GRAM lines, 320

  // entry mode (R03h) for the orientation: AM (D3) selects the auto increment axis of logical x, I/D (D5:D4) the
  // directions, so that a logical row major stream is written with native auto increment
  static const std::uint16_t ENTRY_MODE = (Orientation == orientation_0)    ? 0x0030U :   // AM = 0, I/D = 11
                                          (Orientation == orientation_0m)   ? 0x0010U :   // AM = 0, I/D = 01
                                          (Orientation == orientation_180)  ? 0x0000U :   // AM = 0, I/D = 00
                                          (Orientation == orientation_180m) ? 0x0020U :   // AM = 0, I/D = 10
                                          (Orientation == orientation_90)   ? 0x0028U :   // AM = 1, I/D = 10
                                          (Orientation == orientation_90m)  ? 0x0008U :   // AM = 1, I/D = 00
                                          (Orientation == orientation_270)  ? 0x0018U :   // AM = 1, I/D = 01
                                                                              0x0038U;    // AM = 1, I/D = 11 (270m)

  // vars
  const io::handle_type     device_handle_;
  void*                     mem_reg_addr_;
  void*                     mem_data_addr_;
  const bool                spi_id_bit_;        // level of IM0/ID hardware pin
  vertex_type               gram_pos_;          // logical position of the address counter, invalid if out of screen

public:

  /**
   * ctor
   * \param device_handle Device handle for the device (SPI) interface
   * \param mem_reg_addr Memory address of the index register (memory interfaces)
   * \param mem_data_addr Memory address of the data register (memory interfaces)
   * \param spi_id_bit Level of the IM0/ID hardware pin (SPI device ID)
   */
  ILI9325(io::handle_type device_handle,        // device handle
          void* mem_reg_addr,                   // memory register access address
          void* mem_data_addr,                  // memory data access address
          bool spi_id_bit)                      // level of IM0/ID hardware pin
    : drv(Screen_Size_X, Screen_Size_Y,
          Screen_Size_X, Screen_Size_Y,         // no viewport support (screen = viewport)
          0, 0,                                 // no viewport support
          Orientation)
    , device_handle_(device_handle)
    , mem_reg_addr_(mem_reg_addr)
    , mem_data_addr_(mem_data_addr)
    , spi_id_bit_(spi_id_bit)
    , gram_pos_({ 0x7FFF, 0x7FFF })
  { }


  /**
   * dtor
   * Shutdown the driver
   */
  ~ILI9325()
  {
    shutdown();
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   D R I V E R   F U N C T I O N S
  //

  virtual void init()
  {
    // undocumented timings in app note - seem not to be necessary
//  write_reg(0xE3,                     0x3008);  // internal timing
//  write_reg(0xE7,                     0x0012);  // internal timing
//  write_reg(0xEF,                     0x1231);  // internal timing

    const bool TRI = COLOR_18 && (Interface_Mode != 9U) && (Interface_Mode != 18U);   // 18 bit pixels on an 8 or 16 bit bus (or SPI)
    const bool DFM = (Interface_Mode == 16U) && COLOR_18;                             // 16 bit bus: 2 + 16 bit transfers
    const bool BGR = RGB_to_BGR;

    io::init(device_handle_);
    io::delay(200);    // wait 200 ms for display power stabalization

    // power supply initial setting sequence
    write_reg(REG_OSC_CTRL,             0x0001);  // start osc (vendor init sequence, not in the register table of the datasheet)
    write_reg(REG_DRV_OUTPUT_CTRL,      0x0100);  // set SS
    write_reg(REG_DRV_WAVE_CTRL,        0x0700);  // set B/C and EOR
    write_reg(REG_ENTRY_MODE,           static_cast<std::uint16_t>((TRI ? 0x8000U : 0x0000U) |  // TRI = 0/1
                                                                   (DFM ? 0x4000U : 0x0000U) |  // DFM = 0/1
                                                                   (BGR ? 0x1000U : 0x0000U) |  // BGR = 0/1
                                                                   ENTRY_MODE));                // AM and I/D for the orientation
    write_reg(REG_RESIZE_CTRL,          0x0000);  // disable resize
    write_reg(REG_DISP_CTRL1,           0x0000);  // DTE = 0, GON = 0, D[1:0] = 00
    write_reg(REG_DISP_CTRL2,           0x0202);  // front porch = 2 lines, back porch 2 lines
    write_reg(REG_DISP_CTRL3,           0x0000);
    write_reg(REG_DISP_CTRL4,           0x0000);
    write_reg(REG_RGB_DISPLAY_IF_CTRL1, 0x0000);
    write_reg(REG_FRM_MARKER_POS,       0x0000);
    write_reg(REG_RGB_DISPLAY_IF_CTRL2, 0x0000);
    write_reg(REG_POW_CTRL1,            0x0000);  // BT = 0
    write_reg(REG_POW_CTRL2,            0x0000);  // Step-up circuits = max freq, VCx = 0
    write_reg(REG_POW_CTRL3,            0x0000);  // VRH = 0, PON = 0
    write_reg(REG_POW_CTRL4,            0x0000);  // VDV = 0
    write_reg(REG_POW_CTRL7,            0x0000);  // VCM = 0
    io::delay(50);    // wait for 50 ms

    // power supply operation sequence
    write_reg(REG_POW_CTRL1,            0x0490);  // SAP = 0, BT (step-up factor) = 4, APE = 1, driver amplifiers = 1.00
    write_reg(REG_POW_CTRL2,            0x0227);  // VC = 1.0 x Vci, DC0, DC1 = 2
    write_reg(REG_POW_CTRL3,            0x001F);  // VRH = F (Vci * 1.95), PON = 1, use external reference voltage
    write_reg(REG_POW_CTRL4,            0x1500);  // set VCOM amplitude = VREG1OUT x 1.04
    write_reg(REG_POW_CTRL7,            0x0027);  // set VCOMH = VREG1OUT x 0.880
    io::delay(80);    // wait for 80 ms

    // display ON sequence
    write_reg(REG_POW_CTRL1,            0x1490);  // SAP = 1, BT (step-up factor) = 4, APE = 1, driver amplifiers = 1.00
    write_reg(REG_DISP_CTRL1,           0x0001);  // DTE = 0, GON = 0, D[1:0] = 01
    io::delay(20);    // wait for 20 ms
    write_reg(REG_DISP_CTRL1,           0x0021);  // DTE = 0, GON = 1, D[1:0] = 01
    io::delay(20);    // wait for 20 ms
    write_reg(REG_DISP_CTRL1,           0x0023);  // DTE = 0, GON = 1, D[1:0] = 11
    io::delay(20);    // wait for 20 ms
    write_reg(REG_DISP_CTRL1,           0x0133);  // DTE = 1, GON = 1, D[1:0] = 11, BASEE = 1
    io::delay(20);    // wait for 20 ms

    // init misc registers
    write_reg(REG_FRM_RATE_COLOR_CTRL,  0x000C);  // set frame rate to 80 Hz
    write_reg(REG_GRAM_HOR_ADDR,        0x0000);
    write_reg(REG_GRAM_VER_ADDR,        0x0000);
                                                  // ALT1   ALT2
    write_reg(REG_GAMMA_CTRL1,          0x0000);  // 0x0007 0x0000
    write_reg(REG_GAMMA_CTRL2,          0x0707);  // 0x0303 0x0204
    write_reg(REG_GAMMA_CTRL3,          0x0307);  // 0x0003 0x0200
    write_reg(REG_GAMMA_CTRL4,          0x0200);  // 0x0206 0x0007
    write_reg(REG_GAMMA_CTRL5,          0x0008);  // 0x0008 0x1404
    write_reg(REG_GAMMA_CTRL6,          0x0004);  // 0x0406 0x0705
    write_reg(REG_GAMMA_CTRL7,          0x0000);  // 0x0304 0x0305
    write_reg(REG_GAMMA_CTRL8,          0x0707);  // 0x0007 0x0707
    write_reg(REG_GAMMA_CTRL9,          0x0002);  // 0x0602 0x0701
    write_reg(REG_GAMMA_CTRL10,         0x1D04);  // 0x0008 0x000E

    // GRAM window = whole GRAM
    set_window(0U, static_cast<std::uint16_t>(GRAM_SIZE_X - 1U), 0U, static_cast<std::uint16_t>(GRAM_SIZE_Y - 1U));
    write_reg(REG_GATE_SCAN_CTRL1,      static_cast<std::uint16_t>(0x8000U | ((GRAM_SIZE_Y / 8U - 1U) << 8U)));  // GS = 1, scan start pos = 0, NL = number of lines
    write_reg(REG_GATE_SCAN_CTRL2,      0x0001);  // REV = 1 (grayscale inversion), NDL, VLE = 0 (no scroll)
    write_reg(REG_GATE_SCAN_CTRL3,      0x0000);

    write_reg(REG_PART_IMG1_DISP_POS,   0x0000);  // disable partial display
    write_reg(REG_PART_IMG1_START,      0x0000);
    write_reg(REG_PART_IMG1_END,        0x0000);
    write_reg(REG_PART_IMG2_DISP_POS,   0x0000);
    write_reg(REG_PART_IMG2_START,      0x0000);
    write_reg(REG_PART_IMG2_END,        0x0000);

    write_reg(REG_PANEL_IF_CTRL1,       0x0010);  // fosc / 1 with 16 clocks/line
    write_reg(REG_PANEL_IF_CTRL2,       0x0600);  // gate non-overlap period: 6 clocks

    cls();
  }


  /**
   * Shutdown: display off sequence and sleep mode (datasheet 12.4), init() is needed to wake up again
   */
  virtual void shutdown()
  {
    write_reg(REG_DISP_CTRL1, 0x0000);  // DTE = 0, GON = 0, D[1:0] = 00: display off
    write_reg(REG_POW_CTRL1,  0x0000);  // SAP = 0, APE = 0, AP = 000: halt power supply
    write_reg(REG_POW_CTRL3,  0x0000);  // PON = 0
    write_reg(REG_POW_CTRL1,  0x0002);  // SLP = 1: sleep mode, only the RC oscillator keeps running
  }


  inline virtual const char* version() const
  {
    return (const char*)VIC_HEAD_ILI9325_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    // ILI9325 is a true graphic display
    return true;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

protected:

  /**
   * Clear the screen: one stream over the whole GRAM window from logical (0,0)
   * \param bg_color Background color
   */
  virtual void cls(color::value_type bg_color = color::none)
  {
    set_gram_pos({ 0, 0 });
    write_idx(REG_GRAM_DATA);
    const std::uint32_t pixel = COLOR_18 ? color::color_to_RGB666(bg_color) : color::color_to_RGB565(bg_color);
    for (std::uint32_t i = 0U; i < static_cast<std::uint32_t>(GRAM_SIZE_X) * GRAM_SIZE_Y; ++i) {
      write_pixel(pixel);
    }
    gram_pos_ = { 0x7FFF, 0x7FFF };   // the address counter wrapped, don't rely on it
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

    // set the address counter, it auto increments along logical x
    if (gram_pos_ != vertex) {
      set_gram_pos(vertex);
    }
    gram_pos_ = { static_cast<std::int16_t>(vertex.x + 1), vertex.y };

    write_idx(REG_GRAM_DATA);
    write_pixel(COLOR_18 ? color::color_to_RGB666(color) : color::color_to_RGB565(color));
  }


  virtual color::value_type pixel_get(vertex_type vertex)
  {
    // check limits
    if (!screen_is_inside(vertex)) {
      // out of bounds
      return color::L1_to_color(0U);
    }

    // set the address counter and read the pixel (the io layer has to handle the dummy read of the controller)
    set_gram_pos(vertex);
    write_idx(REG_GRAM_DATA);
    gram_pos_ = { 0x7FFF, 0x7FFF };   // the address counter position after a read is not reliable

    color::value_type color = color::none;
    read_pixel(color);    // color stays 'unknown' if the interface can't read
    return color;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // overwritten drv functions for faster rendering
  //

  /**
   * Write a block of pixels: the GRAM window is set to the (clipped) block and the pixels are streamed,
   * the address counter wraps inside the window
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
    if (r.right  > static_cast<std::int16_t>(Screen_Size_X)) { r.right  = static_cast<std::int16_t>(Screen_Size_X); }
    if (r.bottom > static_cast<std::int16_t>(Screen_Size_Y)) { r.bottom = static_cast<std::int16_t>(Screen_Size_Y); }
    if ((r.left >= r.right) || (r.top >= r.bottom)) {
      return;
    }

    // set the GRAM window to the block, the address counter wraps inside the window
    const vertex_type g0 = to_gram({ r.left, r.top });
    const vertex_type g1 = to_gram({ static_cast<std::int16_t>(r.right - 1), static_cast<std::int16_t>(r.bottom - 1) });
    set_window(static_cast<std::uint16_t>(g0.x < g1.x ? g0.x : g1.x), static_cast<std::uint16_t>(g0.x < g1.x ? g1.x : g0.x),
               static_cast<std::uint16_t>(g0.y < g1.y ? g0.y : g1.y), static_cast<std::uint16_t>(g0.y < g1.y ? g1.y : g0.y));
    set_gram_pos({ r.left, r.top });
    write_idx(REG_GRAM_DATA);

    // stream the block
    const color::value_type* row = pixels + skip_y * stride + skip_x;
    for (std::int16_t y = r.top; y < r.bottom; ++y, row += stride) {
      const color::value_type* p = row;
      for (std::int16_t x = r.left; x < r.right; ++x, ++p) {
        write_pixel(COLOR_18 ? color::color_to_RGB666(*p) : color::color_to_RGB565(*p));
      }
    }

    // restore the full GRAM window
    set_window(0U, static_cast<std::uint16_t>(GRAM_SIZE_X - 1U), 0U, static_cast<std::uint16_t>(GRAM_SIZE_Y - 1U));
    gram_pos_ = { 0x7FFF, 0x7FFF };   // the GRAM pointer is undefined now
  }


  /**
   * Draw a horizontal line, width is one pixel, no pen support
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, y component is ignored
   * \param color Line color - only RGB, no alpha channel
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

    // set the address counter, it auto increments along logical x
    if (gram_pos_ != v0) {
      set_gram_pos(v0);
    }
    write_idx(REG_GRAM_DATA);
    gram_pos_ = { static_cast<std::int16_t>(v1.x + 1), v0.y };

    const std::uint32_t pixel = COLOR_18 ? color::color_to_RGB666(color) : color::color_to_RGB565(color);
    for (std::int_fast16_t x = v0.x; x <= v1.x; ++x) {
      write_pixel(pixel);
    }
  }


public:

  /**
   * Enable/disable the display, the GRAM content is retained
   * \param enable True to switch the display on, false to switch it off
   */
  virtual void display_enable(bool enable = true)
  {
    write_reg(REG_DISP_CTRL1, enable ? 0x0133U : 0x0000U);  // DTE = 1, GON = 1, D[1:0] = 11, BASEE = 1
  }


private:

  /**
   * Map a logical screen vertex to the GRAM address (column, line) according to the orientation
   * The GRAM is 240 x 320, orientation_0 is portrait with (0,0) at GRAM (0,0), the 'm' orientations
   * mirror the physical screen vertically (top/bottom) after the rotation
   */
  static inline vertex_type to_gram(vertex_type v)
  {
    const std::int16_t mx = static_cast<std::int16_t>(GRAM_SIZE_X - 1U);
    const std::int16_t my = static_cast<std::int16_t>(GRAM_SIZE_Y - 1U);
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
   * Set the GRAM address counter to the logical position
   */
  inline void set_gram_pos(vertex_type v)
  {
    const vertex_type g = to_gram(v);
    write_reg(REG_GRAM_HOR_ADDR, static_cast<std::uint16_t>(g.x));
    write_reg(REG_GRAM_VER_ADDR, static_cast<std::uint16_t>(g.y));
  }


  /**
   * Set the GRAM window (physical GRAM addresses, inclusive)
   */
  inline void set_window(std::uint16_t hsa, std::uint16_t hea, std::uint16_t vsa, std::uint16_t vea)
  {
    write_reg(REG_HOR_ADDR_START, hsa);
    write_reg(REG_HOR_ADDR_END,   hea);
    write_reg(REG_VER_ADDR_START, vsa);
    write_reg(REG_VER_ADDR_END,   vea);
  }


  /**
   * Write one pixel to the GRAM (index register must be set to REG_GRAM_DATA)
   * \param pixel RGB666 (COLOR_18) or RGB565 pixel value
   */
  inline void write_pixel(std::uint32_t pixel) const
  {
    write_data(pixel, COLOR_18 ? 3U : 2U);
  }


  /**
   * Write index
   * \param idx Register index
   */
  inline void write_idx(std::uint8_t idx) const
  {
    // Interface_Mode is a constant template parameter, so the switch case is optimized away
    switch (Interface_Mode) {
      case 0U :
      {
        // write to index register with option = 0
        const std::uint8_t data_out[2] = { 0U, idx };
        io::write(device_handle_, 0U, data_out, 2U, nullptr, 0U);
        break;
      }
      case 8U :
        // 8 bit memory interface: upper byte (00h), lower byte on DB17..DB10
        io::mem::write<std::uint8_t>(mem_reg_addr_,  0x00U);
        io::mem::write<std::uint8_t>(mem_reg_addr_,  idx);
        break;
      case 9U :
        // 9 bit memory interface: upper byte (00h), lower byte on DB17..DB10, DB9 is unused
        io::mem::write<std::uint16_t>(mem_reg_addr_, 0x00U);
        io::mem::write<std::uint16_t>(mem_reg_addr_, static_cast<std::uint16_t>(idx << 1U));
        break;
      case 16U :
        // 16 bit memory interface
        io::mem::write<std::uint16_t>(mem_reg_addr_, idx);
        break;
      case 18U :
        // 18 (32) bit memory interface: register bits D15..D0 are on DB15..DB0
        io::mem::write<std::uint32_t>(mem_reg_addr_, static_cast<std::uint32_t>(idx));
        break;
      default :
        break;
    }
  }


  /**
   * Write data to register
   * \param idx Register index
   * \param data Data to write
   */
  inline void write_reg(std::uint8_t idx, std::uint16_t data) const
  {
    write_idx(idx);         // write index
    write_data(data, 2U);   // write data
  }


  /**
   * Write data
   * \param data data to write
   * \param length: 2: 16 bit register or RGB565 pixel data, 3: 18 bit RGB666 pixel data
   */
  inline void write_data(std::uint32_t data, std::uint8_t length) const
  {
    // Interface_Mode is a constant template parameter, so the switch case is optimized away
    switch (Interface_Mode) {
      case 0U : {
        // device interface
        const std::uint8_t data_out[3] = { static_cast<std::uint8_t>(data >> 16U), static_cast<std::uint8_t>(data >> 8U), static_cast<std::uint8_t>(data) };
        // write to GRAM with option = 1
        io::write(device_handle_, 1U, &data_out[3U - length], length, nullptr, 0U);
        break;
      }
      case 8U : {
        // 8 bit memory interface
        const std::uint8_t data_out[3] = { static_cast<std::uint8_t>(data >> 16U), static_cast<std::uint8_t>(data >> 8U), static_cast<std::uint8_t>(data) };
        for (std::uint_fast8_t i = static_cast<std::uint_fast8_t>(3U - length); i < 3U; ++i) {
          io::mem::write<std::uint8_t>(mem_data_addr_, data_out[i]);
        }
        break;
      }
      case 9U :
        // 9 bit memory interface
        if (length == 3U) {
          // 18 bit pixel: DB17..DB9, then DB8..DB0
          io::mem::write<std::uint16_t>(mem_data_addr_, static_cast<std::uint16_t>(data >> 9U));
          io::mem::write<std::uint16_t>(mem_data_addr_, static_cast<std::uint16_t>(data & 0x01FFU));
        }
        else {
          // 16 bit register: upper byte, then lower byte on DB17..DB10, DB9 is unused
          io::mem::write<std::uint16_t>(mem_data_addr_, static_cast<std::uint16_t>((data >> 7U) & 0x01FEU));
          io::mem::write<std::uint16_t>(mem_data_addr_, static_cast<std::uint16_t>((data << 1U) & 0x01FEU));
        }
        break;
      case 16U :
        // 16 bit memory interface, 18 bit pixels are transferred as 2 + 16 bits (DFM = 1)
        if (length == 3U) {
          io::mem::write<std::uint16_t>(mem_data_addr_, static_cast<std::uint16_t>(data >> 16U));
        }
        io::mem::write<std::uint16_t>(mem_data_addr_, static_cast<std::uint16_t>(data));
        break;
      case 18U :
        // 18 (32) bit memory interface
        io::mem::write<std::uint32_t>(mem_data_addr_, data);
        break;
      default :
        break;
    }
  }


  /**
   * Read the pixel at the actual GRAM position (index register must be set to REG_GRAM_DATA)
   * The 8/16 bit buses and SPI return the 16 upper bits of the pixel (RD17..RD2 = RGB565), the 9 and 18 bit buses the full 18 bits
   * \param color Receives the pixel color, unchanged if the interface can't read
   * \return True if successful
   */
  inline bool read_pixel(color::value_type& color) const
  {
    // Interface_Mode is a constant template parameter, so the switch case is optimized away
    switch (Interface_Mode) {
      case 0U : {
        // device interface: read data from GRAM with option = 1 (the io layer handles the dummy bytes)
        std::uint8_t data[2] = { 0U, 0U };
        std::size_t len = 2U;
        if (!io::read(device_handle_, 1U, data, len, 0U) || (len != 2U)) {
          return false;
        }
        color = color::RGB565_to_color(static_cast<std::uint16_t>((static_cast<std::uint16_t>(data[0]) << 8U) | data[1]));
        return true;
      }
      case 8U : {
        const std::uint8_t hi = io::mem::read<std::uint8_t>(mem_data_addr_);
        const std::uint8_t lo = io::mem::read<std::uint8_t>(mem_data_addr_);
        color = color::RGB565_to_color(static_cast<std::uint16_t>((static_cast<std::uint16_t>(hi) << 8U) | lo));
        return true;
      }
      case 9U : {
        const std::uint16_t hi = io::mem::read<std::uint16_t>(mem_data_addr_);
        const std::uint16_t lo = io::mem::read<std::uint16_t>(mem_data_addr_);
        color = color::RGB666_to_color((static_cast<std::uint32_t>(hi & 0x01FFU) << 9U) | (lo & 0x01FFU));
        return true;
      }
      case 16U :
        color = color::RGB565_to_color(io::mem::read<std::uint16_t>(mem_data_addr_));
        return true;
      case 18U :
        color = color::RGB666_to_color(io::mem::read<std::uint32_t>(mem_data_addr_) & 0x0003FFFFUL);
        return true;
      default :
        return false;
    }
  }

};

} // namespace head
} // namespace vic

#endif  // _VIC_HEAD_ILI9325_H_
