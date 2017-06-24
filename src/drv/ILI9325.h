///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2017, PALANDesign Hannover, Germany
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
// \brief ILI9325 driver
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_DRV_ILI9325_H_
#define _VIC_DRV_ILI9325_H_

#include "../drv.h"


// defines the driver name and version
#define VIC_DRV_ILI9325_VERSION   "ILI9325 driver 0.10"

namespace vic {
namespace head {


/**
 * ILI9325 display controller
 * Default is 320 lines and 240 pixel per line. If the display mounting is different, set the size and orientation
 * accordingly, like: Screen_Size_X = 320U, std::uint16_t Screen_Size_Y = 240U, orientation = drv::orientation_90
 */
template<std::uint16_t Screen_Size_X = 240U, std::uint16_t Screen_Size_Y = 320U,  // X, Y size, must match the orientation
         drv::orientation_type orientation = drv::orientation_0,  // hardware orientation of the display
         std::uint8_t interface_mode = 0U,              // 0: device, 8: mem 8bit, 9: mem 9bit, 16: mem 16 bit, 18: mem 18bit
         bool color_256k = false,                       // false: 64k, true: 256k color mode. Used in 'device', '8 or 16 bit' mode. Use false for SPI device interface!
         bool RGB_to_BGR = false                        // swap R and B color components, so displays need that
>
class ILI9325 : public drv
{
private:
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

  const io::dev::handle_type  device_handle_;
  void*                       mem_reg_addr_;
  void*                       mem_data_addr_;
  const bool                  spi_id_bit_;           // level of IM0/ID hardware pin
  vertex_type                 gram_pos_; 

public:

  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S

  /**
   * ctor
   * \param orientation Screen orientation
   * \param spi_device_id Logical SPI bus device ID
   */
  ILI9325(io::dev::handle_type device_handle,   // device handle
          void* mem_reg_addr,                   // memory register access address
          void* mem_data_addr,                  // memory data access address
          bool spi_id_bit)                      // level of IM0/ID hardware pin
    : drv(Screen_Size_X, Screen_Size_Y,
          Screen_Size_X, Screen_Size_Y,         // no viewport support (screen = viewport)
          0, 0,                                 // no viewport support
          orientation)
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
    drv_shutdown();
  }


protected:

  virtual void drv_init() final
  {
    // undocumented timings in app note - seem not to be necessary
//  write_reg(0xE3,                     0x3008);  // internal timing
//  write_reg(0xE7,                     0x0012);  // internal timing
//  write_reg(0xEF,                     0x1231);  // internal timing

    const bool TRI = (interface_mode !=  9U) && (interface_mode != 18U) && (color_256k);   // set if 256k colors
    const bool DFM = (interface_mode == 16U) && (color_256k); // set if 16 bit and 256k colors
    const bool BGR = RGB_to_BGR;

    io::delay(200);    // wait 200 ms for display power stabalization

    // power supply initial setting sequence
    write_reg(REG_OSC_CTRL,             0x0001);  // start osc
    write_reg(REG_DRV_OUTPUT_CTRL,      0x0100);  // set SS
    write_reg(REG_DRV_WAVE_CTRL,        0x0700);  // set B/C and EOR
    write_reg(REG_ENTRY_MODE,           (TRI ? 0x8000U : 0x0000U) |  // TRI = 0/1
                                        (DFM ? 0x4000U : 0x0000U) |  // DFM = 0/1
                                        (BGR ? 0x1000U : 0x0000U) |  // BGR = 0/1
                                        0x0030);                     // BGR = 0
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

    write_reg(REG_HOR_ADDR_START,       0x0000);
    write_reg(REG_HOR_ADDR_END,         0x00EF);
    write_reg(REG_VER_ADDR_START,       0x0000);
    write_reg(REG_VER_ADDR_END,         0x013F);
    write_reg(REG_GATE_SCAN_CTRL1,      0xA700);  // start scan pos = 0, # of lines = 320
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

    drv_cls();
  }


  virtual void drv_shutdown() final
  {
    // TBD: switch off display

    // clear buffer
    drv_cls();
  }


  inline virtual const char* drv_version() const final
  {
    return (const char*)VIC_DRV_ILI9325_VERSION;
  }


  inline virtual bool drv_is_graphic() const final
  {
    // ILI9325 is a true graphic display
    return true;
  }


  virtual void drv_cls() final
  { 
    for (std::int_fast16_t y = 0; y < Screen_Size_Y; ++y) {
      // set y GRAM position
      write_reg(REG_GRAM_HOR_ADDR, static_cast<std::uint16_t>(0));
      write_reg(REG_GRAM_VER_ADDR, static_cast<std::uint16_t>(y));
      write_idx(REG_GRAM_DATA);

      // set pixel to black
      for (std::int_fast16_t x = 0; x < Screen_Size_X; ++x) {
        if (!color_256k) {
          // 64k color mode
          write_data(0U, 2U);
        }
        else {
          // 256k color mode
          write_data(0U, 3U);
        }
      }
    }
  }


  virtual void drv_pixel_set_color(vertex_type point, color::value_type color) final
  {
    // check limits and clipping
    if (!screen_is_inside(point) || (!clipping_.is_inside(point))) {
      // out of bounds or outside clipping region
      return;
    }

    // set GRAM position
    if (gram_pos_ != point) {
      write_reg(REG_GRAM_HOR_ADDR, static_cast<std::uint16_t>(point.x));
      write_reg(REG_GRAM_VER_ADDR, static_cast<std::uint16_t>(point.y));
    }
    gram_pos_ = { ++point.x, point.y };

    if (!color_256k) {
      // 64k color mode
      write_idx(REG_GRAM_DATA);
      write_data(color_to_head_RGB565(color), 2U);
    }
    else {
      // 256k color mode
      write_idx(REG_GRAM_DATA);
      write_data(color_to_head_RGB666(color), 3U);
    }
  }


  virtual color::value_type drv_pixel_get(vertex_type point) final
  {
    // check limits
    if (!screen_is_inside(point)) {
      // out of bounds
      return color_from_head_L1(0U);
    }

    // set GRAM position
    if (gram_pos_ != point) {
      write_reg(REG_GRAM_HOR_ADDR, static_cast<std::uint16_t>(point.x));
      write_reg(REG_GRAM_VER_ADDR, static_cast<std::uint16_t>(point.y));
    }
    gram_pos_ = { ++point.x, point.y };

    if (!color_256k) {
      std::uint8_t data[2];
      //read_data(data, 2U);
      return color_from_head_RGB565(static_cast<std::uint16_t>(data[0] << 8U) |
                                    static_cast<std::uint16_t>(data[1]));
    }
    else {
      std::uint8_t data[3];
      //read_data();
      return color_from_head_RGB666(static_cast<std::uint32_t>(data[0] << 16U) |
                                    static_cast<std::uint32_t>(data[1] <<  8U) |
                                    static_cast<std::uint32_t>(data[2]));
    }
  }


  // unused - data is written directly to the dislay
  virtual void drv_present() final
  { }


  ///////////////////////////////////////////////////////////////////////////////
  // overwritten gpr functions for faster rendering
  //
 
  /**
   * Draw a horizontal line, width is one pixel, no pen support
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, y component is ignored
   * \return true if successful
   */
  virtual void line_horz(vertex_type v0, vertex_type v1) final
  {
    // set y GRAM position
    write_reg(REG_GRAM_HOR_ADDR, static_cast<std::uint16_t>(0));
    write_reg(REG_GRAM_VER_ADDR, static_cast<std::uint16_t>(v0.y));
    write_idx(REG_GRAM_DATA);

    // swap x
    vertex_min_x(v0, v1);

    if (!color_256k) {
      // 64k color mode
      if (color_pen_is_function()) {
        for (; v0.x <= v1.x; ++v0.x) {
          write_data(color_to_head_RGB565(color_pen_get(v0)), 2U);
        }
      }
      else {
        const std::uint16_t color = color_to_head_RGB565(color_pen_get());
        for (; v0.x <= v1.x; ++v0.x) {
          write_data(color, 2U);
        }
      }
    }
    else {
      // 256k color mode
      if (color_pen_is_function()) {
        for (; v0.x <= v1.x; ++v0.x) {
          write_data(color_to_head_RGB666(color_pen_get(v0)), 3U);
        }
      }
      else {
        const std::uint32_t color = color_to_head_RGB666(color_pen_get());
        for (; v0.x <= v1.x; ++v0.x) {
          write_data(color, 3U);
        }
      }
    }
  }


  virtual void display_enable(bool enable = true) final
  { 
    write_reg(REG_DISP_CTRL1, enable ? 0x0133U : 0x0000U);  // DTE = 1, GON = 1, D[1:0] = 11, BASEE = 1
  }


private:
  /**
   * Write index
   * \param idx Register index
   * \param data Data to write
   */
  inline void write_idx(std::uint8_t idx) const
  {
    switch (interface_mode) {
      case 0U :
      {
        // write to index register with option = 0
        const std::uint8_t data_out[2] = { 0U, idx };
        io::dev::write(device_handle_, 0U, data_out, 2U, nullptr, 0U);
        break;
      }
      case 8U :
        // 8 bit memory interface
        io::mem::write<std::uint8_t>(mem_reg_addr_,  0x00U);
        io::mem::write<std::uint8_t>(mem_reg_addr_,  idx);
        break;
      case 9U :
        io::mem::write<std::uint16_t>(mem_reg_addr_, 0x00U);
        io::mem::write<std::uint16_t>(mem_reg_addr_, static_cast<std::uint16_t>(idx) << 1U);
        break;
      case 16U :
        // 16 bit memory interface_
        io::mem::write<std::uint16_t>(mem_reg_addr_, idx);
        break;
      case 18U :
        // 18 (32) bit memory interface
        io::mem::write<std::uint32_t>(mem_reg_addr_, static_cast<std::uint32_t>(idx) << 1U);
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
   * \param length: 2: 16 bit, 3: 18 bit data
   */
  inline void write_data(std::uint32_t data, std::size_t length) const
  {
    switch (interface_mode) {
      case 0U : {
        // device interface
        const std::uint8_t data_out[3] = { static_cast<std::uint8_t>(data >> 16U), static_cast<std::uint8_t>(data >> 8U), static_cast<std::uint8_t>(data) };
        // write to GRAM with option = 1
        io::dev::write(device_handle_, 1U, &data_out[3U - length], length, nullptr, 0U);
        break;
      }
      case 8U : {
        // 8 bit memory interface
        const std::uint8_t data_out[3] = { static_cast<std::uint8_t>(data >> 16U), static_cast<std::uint8_t>(data >> 8U), static_cast<std::uint8_t>(data) };
        for (std::size_t i = 3U - length; i < length; ++i) {
          io::mem::write<std::uint8_t>(mem_data_addr_, data_out[i]);
        }
        break;
      }
      case 9U :
        // 9 bit memory interface
        io::mem::write<std::uint16_t>(mem_data_addr_, static_cast<std::uint16_t>(data >> 9U));
        io::mem::write<std::uint16_t>(mem_data_addr_, static_cast<std::uint16_t>(data));
        break;
      case 16U :
        // 16 bit memory interface
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
   * Read register
   */
  inline std::uint16_t read_reg(std::uint8_t idx) const
  {
    write_idx(idx);                                   // write index
    return static_cast<std::uint16_t>(read_data());   // read data
  }


  inline std::uint32_t read_data() const
  {
    switch (interface_mode) {
      case 0U :
        // read data from GRAM with option = 1U
//        io::dev::read(device_handle_, 1U, data, length);
        break;
      default :
        break;
    }
    return 0U;
  }

};

} // namespace head
} // namespace vic

#endif  // _VIC_DRV_ILI9325_H_
