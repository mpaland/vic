///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2020, PALANDesign Hannover, Germany
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
// \brief SSD1306 / SSD1309 / SH1106 head, monochrome OLED controller (128 x 64 typical)
// The controller RAM is organized in pages of 8 vertical pixels. With the (default)
// framebuffer the pixels are rendered in RAM and the whole buffer is sent with
// present(), which is the fastest option on a serial bus. Without framebuffer every
// pixel is a read-modify-write of one page byte on the display. The GDDRAM can only
// be read on the parallel buses (not on SPI / I2C) and the controller needs a dummy
// read first, both is the job of the io layer.
// See doc/SSD1306.md for the programming summary of the controller.
//
// Interface option values of io::write(): 0 = command, 1 = data (D/C# line on SPI).
// The head emits plain 8 bit command/data bytes for all interfaces: in 3-wire SPI mode
// the io layer has to build the 9 bit frames (D/C# bit + byte), in I2C mode the control
// byte is part of the stream. Panels which are not mapped to RAM column 0 (SH1106 with
// 128 pixels at column 2, 64 x 48 SSD1306 panels at column 32) use Column_Offset.
// The SH1106 has no horizontal addressing mode, every page is addressed and sent
// separately (see doc/SSD1306.md).
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_SSD1306_H_
#define _VIC_HEAD_SSD1306_H_

#include "../drv.h"


// defines the head name and version
#define VIC_HEAD_SSD1306_VERSION   "SSD1306 head 1.00"

namespace vic {
namespace head {


// interface type
typedef enum tag_SSD1306_interface_type {
  SSD1306_SPI3,  // 3-wire SPI
  SSD1306_SPI4,  // 4-wire SPI (D/C# line is the io option)
  SSD1306_I2C,   // I²C
  SSD1306_MEM    // 8-bit memory interface
} SSD1306_interface_type;

// controller type
typedef enum tag_SSD1306_controller_type {
  SSD1306_CTRL_SSD1306,   // SSD1306: 128 columns, horizontal addressing mode, charge pump 8Dh
  SSD1306_CTRL_SSD1309,   // SSD1309: like the SSD1306 without charge pump
  SSD1306_CTRL_SH1106     // SH1106: 132 columns (128 pixel panels at column 2), page addressing only, charge pump ADh
} SSD1306_controller_type;


/**
 * SSD1306 display controller
 * \param Screen_Size_X Screen width in pixel, 128 typical
 * \param Screen_Size_Y Screen height in pixel, 64 or 32 typical, must be a multiple of 8
 * \param Orientation Hardware orientation of the display (0, 0m, 180, 180m); orientation_0 is the datasheet default,
 *                    the reference drivers (Adafruit_SSD1306, u8g2) use segment remap + COM scan remap (A1h, C8h) as
 *                    their rotation 0, which is orientation_180 here
 * \param Interface_Mode Interface mode
 * \param Framebuffer True to render into a RAM framebuffer which is sent with present() (recommended)
 * \param Charge_Pump True to enable the internal charge pump (SSD1306 8Dh 14h, SH1106 ADh 8Bh), false for an
 *                    external VCC supply; the SSD1309 has no charge pump
 * \param Controller SSD1306 (default), SSD1309 or SH1106
 * \param Column_Offset First RAM column of the panel: 2 for 128 pixel SH1106 panels (132 column RAM), 32 for 64 x 48 SSD1306 panels
 */
template<std::uint16_t Screen_Size_X = 128U, std::uint16_t Screen_Size_Y = 64U,   // X, Y size, must match the orientation
         drv::orientation_type Orientation = drv::orientation_0,                  // hardware orientation of the display
         SSD1306_interface_type Interface_Mode = SSD1306_I2C,                     // interface mode
         bool Framebuffer = true,                                                 // use framebuffer for fast rendering and pixel read
         bool Charge_Pump = true,                                                 // enable the internal charge pump, false for external VCC
         SSD1306_controller_type Controller = SSD1306_CTRL_SSD1306,               // controller variant
         std::uint8_t Column_Offset = (Controller == SSD1306_CTRL_SH1106) ? 2U : 0U   // first RAM column of the panel
>
class SSD1306 final : public drv
{
  static_assert(Screen_Size_Y % 8U == 0U, "SSD1306: Screen_Size_Y must be a multiple of 8 (page height)");

  static const bool SH1106 = (Controller == SSD1306_CTRL_SH1106);

  // I²C control byte
  // bit 7 : Co : Continuation bit - 0: all following bytes are data/commands (stream), 1: single command
  // bit 6 : D/C# : Data/Command Selection bit, Data=1/Command=0
  static const std::uint8_t I2C_CONTROL_CMD_STREAM  = 0x00U;    // send a cmd stream
  static const std::uint8_t I2C_CONTROL_DATA_STREAM = 0x40U;    // send a data stream

  // Fundamental Commands
  static const std::uint8_t CMD_CONTRAST            = 0x81U;    // set contrast
  static const std::uint8_t CMD_TEST_OFF            = 0xA4U;    // test mode off, display RAM content
  static const std::uint8_t CMD_INVERSE_OFF         = 0xA6U;    // normal video mode
  static const std::uint8_t CMD_DISPLAY_OFF         = 0xAEU;    // sleep mode
  static const std::uint8_t CMD_DISPLAY_ON          = 0xAFU;    // normal mode

  // Addressing Setting Commands
  static const std::uint8_t CMD_MEMORY_ADDR_MODE    = 0x20U;    // memory addressing mode
  static const std::uint8_t CMD_COLUMN_ADDR         = 0x21U;    // column start and end address
  static const std::uint8_t CMD_PAGE_ADDR           = 0x22U;    // page start and end address

  // Hardware Configuration Commands
  static const std::uint8_t CMD_DISPLAY_START_LINE  = 0x40U;    // set RAM display start line
  static const std::uint8_t CMD_SEGMENT_REMAP_OFF   = 0xA0U;    // column address 0 is mapped to SEG0
  static const std::uint8_t CMD_SEGMENT_REMAP_ON    = 0xA1U;    // column address 127 is mapped to SEG0
  static const std::uint8_t CMD_MUX_RATIO           = 0xA8U;    // set MUX ratio to N+1 MUX
  static const std::uint8_t CMD_SCAN_DIR_REMAP_OFF  = 0xC0U;    // normal mode (RESET) Scan from COM0 to COM[N-1]
  static const std::uint8_t CMD_SCAN_DIR_REMAP_ON   = 0xC8U;    // remapped mode. Scan from COM[N-1] to COM0
  static const std::uint8_t CMD_DISPLAY_OFFSET      = 0xD3U;    // set vertical shift by COM from 0d~63d
  static const std::uint8_t CMD_COM_PIN_REMAP       = 0xDAU;    // COM pins hardware configuration

  // Timing & Driving Scheme Setting Commands
  static const std::uint8_t CMD_DISPLAY_CLK_DIV     = 0xD5U;    // define the divide ratio and set the Oscillator Frequency
  static const std::uint8_t CMD_PRECHARGE_PERIOD    = 0xD9U;    // Phase 1 period, Phase 2 period
  static const std::uint8_t CMD_VCOMH_DESELECT      = 0xDBU;    // VCOMH Deselect

  // Charge Pump Commands
  static const std::uint8_t CMD_CHARGE_PUMP         = 0x8DU;    // charge pump setting (SSD1306)
  static const std::uint8_t CMD_SH1106_DCDC         = 0xADU;    // DC-DC control mode set (SH1106), 8Ah off, 8Bh on

  // page addressing commands (SH1106)
  static const std::uint8_t CMD_COLUMN_LOW          = 0x00U;    // lower nibble of the column address
  static const std::uint8_t CMD_COLUMN_HIGH         = 0x10U;    // upper nibble of the column address
  static const std::uint8_t CMD_PAGE_START          = 0xB0U;    // page start address

  // IO buffer for SPI and I²C: 32 data/cmd bytes + one control byte as max
  static const std::size_t IO_BUFFER_SIZE = 33U;
  static const std::size_t PAGES          = Screen_Size_Y / 8U;
  static const std::size_t FB_SIZE        = Framebuffer ? PAGES * Screen_Size_X : 1U;

  // vars
  const io::handle_type device_handle_;
  struct tag_framebuffer_type {
    std::uint8_t ctrl;              // control byte, needed for I²C data streams, so that ctrl and data are sent as one block
    std::uint8_t data[FB_SIZE];     // page organized pixel data: data[page * Screen_Size_X + x], bit (y & 7)
  } framebuffer_;
  std::uint8_t          io_buffer_[IO_BUFFER_SIZE];
  std::size_t           io_buffer_idx_;
  bool                  io_data_;     // true if the current sequence is a data sequence

public:

  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S

  /**
   * ctor
   * \param device_handle Device handle
   */
  SSD1306(io::handle_type device_handle)
    : drv(Screen_Size_X, Screen_Size_Y,
          Screen_Size_X, Screen_Size_Y,         // no viewport support (screen = viewport)
          0, 0,                                 // no viewport support
          Orientation)
    , device_handle_(device_handle)
    , io_buffer_idx_(0U)
    , io_data_(false)
  {
    framebuffer_.ctrl = I2C_CONTROL_DATA_STREAM;
    for (std::size_t i = 0U; i < FB_SIZE; ++i) {
      framebuffer_.data[i] = 0U;
    }
  }


  /**
   * dtor
   * Shutdown the driver
   */
  ~SSD1306()
  {
    shutdown();
  }


  virtual void init()
  {
    io::init(device_handle_);

    // start command sequence
    write_begin(false);

    // display off
    write_cmd(CMD_DISPLAY_OFF);

    // set mux ratio to the number of rows
    write_cmd(CMD_MUX_RATIO, static_cast<std::uint8_t>(Screen_Size_Y - 1U));

    // set the display offset to 0
    write_cmd(CMD_DISPLAY_OFFSET, 0x00U);

    // set the display start line to 0
    write_cmd(CMD_DISPLAY_START_LINE);

    // orientation
    switch (Orientation) {
      case orientation_0 :
        write_cmd(CMD_SEGMENT_REMAP_OFF);   // x-axis: normal
        write_cmd(CMD_SCAN_DIR_REMAP_OFF);  // y-axis: normal
        break;
      case orientation_0m :
        write_cmd(CMD_SEGMENT_REMAP_OFF);   // x-axis: normal
        write_cmd(CMD_SCAN_DIR_REMAP_ON);   // y-axis: mirror
        break;
      case orientation_180m :
        write_cmd(CMD_SEGMENT_REMAP_ON);    // x-axis: mirror
        write_cmd(CMD_SCAN_DIR_REMAP_OFF);  // y-axis: normal
        break;
      case orientation_180 :
        write_cmd(CMD_SEGMENT_REMAP_ON);    // x-axis: mirror
        write_cmd(CMD_SCAN_DIR_REMAP_ON);   // y-axis: mirror
        break;
      default :
        // 90 and 270 degree need a rotated framebuffer, not supported yet
        write_cmd(CMD_SEGMENT_REMAP_OFF);
        write_cmd(CMD_SCAN_DIR_REMAP_OFF);
        break;
    }

    // set COM pin map: alternative configuration for 64 lines, sequential for 32 lines
    write_cmd(CMD_COM_PIN_REMAP, Screen_Size_Y > 32U ? 0x12U : 0x02U);

    // set default contrast
    write_cmd(CMD_CONTRAST, 0x7FU);

    // set display to enable rendering from GDDRAM, no test mode
    write_cmd(CMD_TEST_OFF);

    // use normal mode
    write_cmd(CMD_INVERSE_OFF);

    // default oscillator clock
    write_cmd(CMD_DISPLAY_CLK_DIV, 0x80U);

    // enable the charge pump (must be set before display on), the SSD1309 has none
    if (Charge_Pump && (Controller != SSD1306_CTRL_SSD1309)) {
      if (SH1106) {
        write_cmd(CMD_SH1106_DCDC, 0x8BU);
      }
      else {
        write_cmd(CMD_CHARGE_PUMP, 0x14U);
      }
    }

    // set the precharge cycles to high cap type
    write_cmd(CMD_PRECHARGE_PERIOD, 0x22U);

    // set the V_COMH deselect voltage to max
    write_cmd(CMD_VCOMH_DESELECT, 0x30U);

    // use horizontal addressing mode as default (the SH1106 has page addressing only)
    if (!SH1106) {
      write_cmd(CMD_MEMORY_ADDR_MODE, 0x00U);
    }

    // turn the display on
    write_cmd(CMD_DISPLAY_ON);

    write_end();

    cls();
    present();
  }


  virtual void shutdown()
  {
    // switch off the display
    display_enable(false);
  }


  inline virtual const char* version() const
  {
    return (const char*)VIC_HEAD_SSD1306_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    // SSD1306 is a true graphic display
    return true;
  }


  /**
   * Access the framebuffer (page organized), only meaningful with Framebuffer = true
   * \return Pointer to PAGES * Screen_Size_X bytes
   */
  inline const std::uint8_t* framebuffer() const
  {
    return framebuffer_.data;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

protected:

  /**
   * Clear screen, set all pixels off
   * \param bg_color Background color, every color which is not black switches the pixels on
   */
  virtual void cls(color::value_type bg_color = color::none)
  {
    const std::uint8_t fill = color::color_to_L1(bg_color) ? 0xFFU : 0x00U;
    if (Framebuffer) {
      for (std::size_t i = 0U; i < FB_SIZE; ++i) {
        framebuffer_.data[i] = fill;
      }
    }
    else {
      // stream the whole GDDRAM page by page
      for (std::uint8_t page = 0U; page < static_cast<std::uint8_t>(PAGES); ++page) {
        set_address(0U, static_cast<std::uint8_t>(Screen_Size_X - 1U), page, page);
        write_begin(true);
        for (std::size_t i = 0U; i < Screen_Size_X; ++i) {
          write_cmd(fill);
        }
        write_end();
      }
    }
  }


  /**
   * Primitive rendering is done, copy the framebuffer to the display
   */
  virtual void present()
  {
    if (Framebuffer) {
      if (SH1106) {
        // page addressing: every page is addressed and sent separately
        for (std::uint8_t page = 0U; page < static_cast<std::uint8_t>(PAGES); ++page) {
          set_address(0U, static_cast<std::uint8_t>(Screen_Size_X - 1U), page, page);
          write_begin(true);
          for (std::size_t i = 0U; i < Screen_Size_X; ++i) {
            write_cmd(framebuffer_.data[page * Screen_Size_X + i]);
          }
          write_end();
        }
      }
      else {
        set_address(0U, static_cast<std::uint8_t>(Screen_Size_X - 1U), 0U, static_cast<std::uint8_t>(PAGES - 1U));
        write_framebuffer();
      }
    }
  }


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

    const std::uint8_t page = static_cast<std::uint8_t>(vertex.y >> 3U);
    const std::uint8_t mask = static_cast<std::uint8_t>(0x01U << (vertex.y & 0x07U));

    if (Framebuffer) {
      std::uint8_t& data = framebuffer_.data[page * Screen_Size_X + vertex.x];
      data = static_cast<std::uint8_t>(color::color_to_L1(color) ? (data | mask) : (data & ~mask));
    }
    else {
      // read-modify-write of the page byte on the display
      set_address(static_cast<std::uint8_t>(vertex.x), static_cast<std::uint8_t>(vertex.x), page, page);
      std::uint8_t data = 0U;
      read_data(data);
      data = static_cast<std::uint8_t>(color::color_to_L1(color) ? (data | mask) : (data & ~mask));
      set_address(static_cast<std::uint8_t>(vertex.x), static_cast<std::uint8_t>(vertex.x), page, page);
      write_begin(true);
      write_cmd(data);
      write_end();
    }
  }


  virtual color::value_type pixel_get(vertex_type vertex)
  {
    // check limits
    if (!screen_is_inside(vertex)) {
      // out of bounds
      return color::none;
    }

    const std::uint8_t page = static_cast<std::uint8_t>(vertex.y >> 3U);
    std::uint8_t data = 0U;
    if (Framebuffer) {
      data = framebuffer_.data[page * Screen_Size_X + vertex.x];
    }
    else {
      set_address(static_cast<std::uint8_t>(vertex.x), static_cast<std::uint8_t>(vertex.x), page, page);
      read_data(data);
    }
    return color::L1_to_color(static_cast<std::uint8_t>((data >> (vertex.y & 0x07U)) & 0x01U));
  }


  ///////////////////////////////////////////////////////////////////////////////
  // D I S P L A Y   C O N T R O L
  //

public:

  /**
   * Enable/disable the display
   * \param enable True to switch the display on, false to switch it off (standby, powersave)
   */
  virtual void display_enable(bool enable = true)
  {
    write_begin(false);
    write_cmd(enable ? CMD_DISPLAY_ON : CMD_DISPLAY_OFF);
    write_end();
  }


  /**
   * Set display contrast brightness
   * \param level 0: minimum; 255: maximum
   */
  virtual void display_contrast(std::uint8_t level)
  {
    write_begin(false);
    write_cmd(CMD_CONTRAST, level);
    write_end();
  }


  ///////////////////////////////////////////////////////////////////////////////

private:

  /**
   * Set the GDDRAM address window (horizontal addressing mode) or the page and column (SH1106, page addressing),
   * the columns are panel columns, the RAM column offset of the panel is added
   */
  void set_address(std::uint8_t col_start, std::uint8_t col_end, std::uint8_t page_start, std::uint8_t page_end)
  {
    write_begin(false);
    if (SH1106) {
      (void)col_end; (void)page_end;
      const std::uint8_t col = static_cast<std::uint8_t>(col_start + Column_Offset);
      write_cmd(static_cast<std::uint8_t>(CMD_PAGE_START  | (page_start & 0x0FU)));
      write_cmd(static_cast<std::uint8_t>(CMD_COLUMN_LOW  | (col & 0x0FU)));
      write_cmd(static_cast<std::uint8_t>(CMD_COLUMN_HIGH | (col >> 4U)));
    }
    else {
      write_cmd(CMD_MEMORY_ADDR_MODE, 0x00U);
      write_cmd(CMD_COLUMN_ADDR, static_cast<std::uint8_t>(col_start + Column_Offset), static_cast<std::uint8_t>(col_end + Column_Offset));
      write_cmd(CMD_PAGE_ADDR, page_start, page_end);
    }
    write_end();
  }


  /**
   * Start a command or data sequence
   * \param data True for data, false for commands
   */
  inline void write_begin(bool data)
  {
    io_data_       = data;
    io_buffer_idx_ = 0U;
    if (Interface_Mode == SSD1306_I2C) {
      io_buffer_[io_buffer_idx_++] = data ? I2C_CONTROL_DATA_STREAM : I2C_CONTROL_CMD_STREAM;
    }
  }


  /**
   * Finish the sequence and send it, the io option is 0 for commands and 1 for data
   */
  inline void write_end()
  {
    const std::size_t header = (Interface_Mode == SSD1306_I2C) ? 1U : 0U;
    if (io_buffer_idx_ > header) {
      io::write(device_handle_, io_data_ ? 1U : 0U, io_buffer_, io_buffer_idx_, nullptr, 0U);
    }
    io_buffer_idx_ = 0U;
  }


  // write one byte to the sequence, the sequence is flushed and continued if the buffer is full
  inline void write_cmd(std::uint8_t cmd)
  {
    if (io_buffer_idx_ >= IO_BUFFER_SIZE) {
      write_end();
      write_begin(io_data_);
    }
    io_buffer_[io_buffer_idx_++] = cmd;
  }

  inline void write_cmd(std::uint8_t cmd, std::uint8_t param1)
  {
    write_cmd(cmd);
    write_cmd(param1);
  }

  inline void write_cmd(std::uint8_t cmd, std::uint8_t param1, std::uint8_t param2)
  {
    write_cmd(cmd);
    write_cmd(param1);
    write_cmd(param2);
  }


  /**
   * Write the whole framebuffer as one data block
   */
  inline void write_framebuffer()
  {
    if (Interface_Mode == SSD1306_I2C) {
      // control byte and data in one block
      io::write(device_handle_, 1U, &framebuffer_.ctrl, 1U + FB_SIZE, nullptr, 0U);
    }
    else {
      io::write(device_handle_, 1U, framebuffer_.data, FB_SIZE, nullptr, 0U);
    }
  }


  /**
   * Read one data byte from the GDDRAM at the actual address
   * \param data Receives the byte, unchanged if the interface can't read
   * \return True if successful
   */
  inline bool read_data(std::uint8_t& data)
  {
    std::size_t length = 1U;
    std::uint8_t byte = 0U;
    if (io::read(device_handle_, 1U, &byte, length, 0U) && (length == 1U)) {
      data = byte;
      return true;
    }
    return false;
  }

};

} // namespace head
} // namespace vic

#endif  // _VIC_HEAD_SSD1306_H_
