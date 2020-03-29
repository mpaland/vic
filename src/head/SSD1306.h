///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2018, PALANDesign Hannover, Germany
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
// \brief SSD1306 driver (128 x 64 monochrome display driver)
//        I²C mode: the bus address is 0x3C (default) or 0x3D
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_DRV_SSD1306_H_
#define _VIC_DRV_SSD1306_H_

#include "../drv.h"
#include "../dc.h"    // include drawing context for alpha numeric driver


// defines the driver name and version
#define VIC_DRV_SSD1306_VERSION   "SSD1306 driver 0.01"

namespace vic {
namespace head {


// interface type
typedef enum tag_SSD1306_interface_type {
  SSD1306_SPI3,  // 3-wire SPI
  SSD1306_SPI4,  // 4 wire SPI
  SSD1306_I2C,   // I²C
  SSD1306_MEM    // 8-bit memory
} SSD1306_interface_type;


/**
 * SSD1306 display controller
 * Default is 128 pixel per line and 64 lines
 */
template<std::uint16_t Screen_Size_X = 128U, std::uint16_t Screen_Size_Y = 64U,   // X, Y size, must match the orientation
         drv::orientation_type Orientation = drv::orientation_0,                  // hardware orientation of the display
         SSD1306_interface_type Interface_Mode = SSD1306_I2C,                     // interface mode
         bool Framebuffer = false                                                 // use framebuffer for fast pixel read
>
class SSD1306 final : public drv
{
private:

  // I²C control
  // bit 7 : Co : Continuation bit - If 0, then all the next bytes are data (no more control bytes)
  //       :      You can send a stream of data, ie: gRAM dump - if Co=0
  //       :      For a command, set this - one at a time. Hence, Co=1 for commands
  //       :      For Data stream, Co=0
  // bit 6 : D/C# : Data/Command Selection bit, Data=1/Command=0
  // bit [5-0] : lower 6 bits have to be 0
  static const std::uint8_t I2C_CONTROL_CMD_SINGLE  = 0x80U;    // send a single command
  static const std::uint8_t I2C_CONTROL_CMD_STREAM  = 0x00U;    // send a cmd stream
  static const std::uint8_t I2C_CONTROL_DATA_STREAM = 0x40U;    // send a data stream


  // Fundamental Commands
  static const std::uint8_t CMD_CONTRAST            = 0x81U;    // set contrast
  static const std::uint8_t CMD_TEST_OFF            = 0xA4U;    // test mode off, display RAM content
  static const std::uint8_t CMD_TEST_ON             = 0xA5U;    // test mode on, all pixel on
  static const std::uint8_t CMD_INVERSE_OFF         = 0xA6U;    // normal video mode
  static const std::uint8_t CMD_INVERSE_ON          = 0xA7U;    // inverse video mode
  static const std::uint8_t CMD_DISPLAY_OFF         = 0xAEU;    // sleep mode
  static const std::uint8_t CMD_DISPLAY_ON          = 0xAFU;    // normal mode

  // Addressing Setting Commands
  static const std::uint8_t CMD_MEMORY_ADDR_MODE    = 0x20U;    // memory addressing mode
  static const std::uint8_t CMD_COLUMN_ADDR         = 0x21U;    // column start and end address
  static const std::uint8_t CMD_PAGE_ADDR           = 0x22U;    // page start and end address
  static const std::uint8_t CMD_PAGE_START          = 0xB0U;    // GDDRAM Page Start Address

  // Hardware Configuration Commands
  static const std::uint8_t CMD_DISPLAY_START_LINE  = 0x40U;    // set RAM display start line
  static const std::uint8_t CMD_SEGMENT_REMAP_OFF   = 0xA0U;    // column address 0 is mapped to SEG0
  static const std::uint8_t CMD_SEGMENT_REMAP_ON    = 0xA1U;    // column address 127 is mapped to SEG0
  static const std::uint8_t CMD_MUX_RATIO           = 0xA8U;    // set MUX ratio to N+1 MUX
  static const std::uint8_t CMD_SCAN_DIR_REMAP_OFF  = 0xC0U;    // normal mode (RESET) Scan from COM0 to COM[N –1]
  static const std::uint8_t CMD_SCAN_DIR_REMAP_ON   = 0xC8U;    // remapped mode. Scan from COM[N-1] to COM0 where N is the Multiplex ratio
  static const std::uint8_t CMD_DISPLAY_OFFSET      = 0xD3U;    // set vertical shift by COM from 0d~63d
  static const std::uint8_t CMD_COM_PIN_REMAP       = 0xDAU;    //

  // Timing & Driving Scheme Setting Commands
  static const std::uint8_t CMD_DISPLAY_CLK_DIV     = 0xD5U;    // define the divide ratio and set the Oscillator Frequency
  static const std::uint8_t CMD_PRECHARGE_PERIOD    = 0xD9U;    // Phase 1 period, Phase 2 period
  static const std::uint8_t CMD_VCOMH_DESELECT      = 0xDBU;    // VCOMH Deselect
  static const std::uint8_t CMD_NOP                 = 0xE3U;    // no operation

  // Charge Pump Commands
  static const std::uint8_t CMD_CHARGE_PUMP         = 0x8DU;    // charge pump setting


  // IO buffer for SPI and I²C, 32 data/cmd byte + one control byte as max
  #define SSD1306_IO_BUFFER_SIZE  33U

  // vars
  const io::dev::handle_type  device_handle_;
  struct framebuffer_ {
    std::uint8_t              ctrl;   // ctrl byte, needed for I²C interface
    std::uint8_t              data[Framebuffer ? Screen_Size_Y / 8U : 1U][Framebuffer ? Screen_Size_X / 8U];
  };
  std::uint8_t                io_buffer_[SSD1306_IO_BUFFER_SIZE];
  std::uint8_t                io_buffer_idx_;

public:

  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S

  /**
   * ctor
   * \param device_handle Device handle
   */
  SSD1306(io::dev::handle_type device_handle)   // device handle
    : drv(Screen_Size_X, Screen_Size_Y,
          Screen_Size_X, Screen_Size_Y,         // no viewport support (screen = viewport)
          0, 0,                                 // no viewport support
          Orientation)
    , device_handle_(device_handle)
  { }


  /**
   * dtor
   * Shutdown the driver
   */
  ~SSD1306()
  { }


protected:

  virtual void init()
  {
    // start command sequence, needed for SPI and I²C
    write_begin(false);

    // display off
    write_cmd(CMD_DISPLAY_OFF);

    // set mux ration tp select max number of rows - 64
    write_cmd(CMD_MUX_RATIO, 0x3FU);

    // set the display offset to 0
    write_cmd(CMD_DISPLAY_OFFSET, 0x00U);

    // set the display start line to 0
    write_cmd(CMD_DISPLAY_START_LINE);

    // TBD: set the orientation
    switch (Orientation) {
      case orientation_0 :
        write_cmd(CMD_SEGMENT_REMAP_OFF);   // x-axis: normal
        write_cmd(CMD_SCAN_DIR_REMAP_OFF);  // y-axis: normal
        break;
      case orientation_90 :
        write_cmd(CMD_SEGMENT_REMAP_OFF);   // x-axis: normal
        write_cmd(CMD_SCAN_DIR_REMAP_ON);   // y-axis: mirror
        break;
      case orientation_180 :
        write_cmd(CMD_SEGMENT_REMAP_ON);    // x-axis: mirror
        write_cmd(CMD_SCAN_DIR_REMAP_OFF);  // y-axis: normal
        break;
      case orientation_270 :
        write_cmd(CMD_SEGMENT_REMAP_ON);    // x-axis: mirror
        write_cmd(CMD_SCAN_DIR_REMAP_ON);   // y-axis: mirror
        break;
      default :
        break;
    }

    // set COM pin map - maybe the must be in config for different displays
    write_cmd(CMD_COM_PIN_REMAP), 0x12U);

    // set default contrast
    write_cmd(CMD_CONTRAST, 0x7FU);

    // set display to enable rendering from GDDRAM, no test mode
    write_cmd(CMD_TEST_OFF);

    // use normal mode
    write_cmd(CMD_INVERSE_OFF);

    // default oscillator clock
    write_cmd(CMD_DISPLAY_CLK_DIV, 0x80U);

    // enable the charge pump
    write_cmd(CMD_CHARGE_PUMP, 0x14U);

    // set the precharge cycles to high cap type
    write_cmd(CMD_PRECHARGE_PERIOD, 0x22U);

    // aet the V_COMH deselect voltage to max
    write_cmd(CMD_VCOMH_DESELECT, 0x30U);

    // use horizonatal addressing mode as default
    // TBD: maybe this can be used for fast vertical line drawing
    write_cmd(CMD_MEMORY_ADDR_MODE, 0x00U);

    // turn the display on
    write_cmd(CMD_DISPLAY_ON);

    write_end();

    cls();
  }


  virtual void shutdown()
  {
    // switch off the display
    display_enable(false);
  }


  inline virtual const char* version() const
  {
    return (const char*)VIC_DRV_SSD1306_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    // SSD1306 is a true graphic display
    return true;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

protected:

  /**
   * Clear screen, set all pixels off, delete all characters or fill screen with background/blank color
   */
  virtual void cls(color::value_type)
  { 
  }


  /**
   * Primitive rendering is done, copy framebuffer to the display
   */
  virtual void present()
  {
    if (Framebuffer) {
      // start cmd sequence
      write_begin(false);
      // use horizontal addressing mode
      write_cmd(CMD_MEMORY_ADDR_MODE, 0x00U);
      // set column start and end address (this sets the column pointer to 0)
      write_cmd(CMD_COLUMN_ADDR, 0x00U, Screen_Size_X);
      // set page start and end address (this sets the page pointer to 0)
      write_cmd(CMD_PAGE_ADDR, 0x00U, Screen_Size_Y / 8U);
      // finish sequence
      write_end();

      // write the framebuffer
      write_framebuffer();
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  // G R A P H I C   F U N C T I O N S
  //

  /**
   * Set pixel in given color
   * \param vertex Pixel coordinates
   * \param color Color of pixel in ARGB format, alpha channel is/maybe ignored
   */
  virtual void pixel_set(vertex_type vertex, color::value_type color)
  {
    // check limits
    if (!screen_is_inside(vertex)) {
      // out of bounds
      return;
    }

    if (Framebuffer) {
      // set pixel in framebuffer
      if (color::color_to_L1(color)) {
        framebuffer_.data[vertex.y >> 3U][vertex.x] |= static_cast<std::uint8_t>(  0x01U << (vertex.y & 0x07U));   // set pixel
      }
      else {
        framebuffer_.data[vertex.y >> 3U][vertex.x] &= static_cast<std::uint8_t>(~(0x01U << (vertex.y & 0x07U)));  // clear pixel
      }
    }
    else {
      // start cmd sequence
      write_begin(false);
      // use page addressing mode
      write_cmd(CMD_MEMORY_ADDR_MODE, 0x02U);
      // set column start and end address (this sets the column pointer to 0)
      write_cmd(CMD_COLUMN_ADDR, vertex.x, Screen_Size_X);
      // set page start and end address (this sets the page pointer to 0)
      write_cmd(CMD_PAGE_ADDR, vertex.y >> 8U, Screen_Size_Y / 8U);
      // finish sequence
      write_end();

      std::uint8 gdd_data;
      read_data(gdd_data);

      // start cmd sequence
      write_begin(false);
      // set column start and end address (this sets the column pointer to 0)
      write_cmd(CMD_COLUMN_ADDR, vertex.x, Screen_Size_X);
      // set page start and end address (this sets the page pointer to 0)
      write_cmd(CMD_PAGE_ADDR, vertex.y >> 8U, Screen_Size_Y / 8U);
      // finish sequence
      write_end();

      gdd_data &= static_cast<std::uint8_t>(~(0x01U << (vertex.y & 0x07U));
      gdd_data |= static_cast<std::uint8_t>((color::color_to_L1(color) & 0x01U) << (vertex.y & 0x07U));

      // start data sequence
      write_begin(true);
      write_cmd(&gdd_data);
      write_end();
    }
  }


  /**
   * Return the color of the pixel
   * \param vertex Vertex of the pixel
   * \return Color of pixel in ARGB format, alpha channel must be set to opaque if unsupported (default)
   */
  virtual color::value_type pixel_get(vertex_type vertex)
  {
    // check limits
    if (!screen_is_inside(vertex)) {
      // out of bounds
      return color::L1_to_color(0U);
    }

    if (Framebuffer) {
      // read pixel out of framebuffer
      return color::L1_to_color((framebuffer_.data[vertex.y][vertex.x >> 3U] >> (vertex.y & 0x07U)) & 0x01U);
    }
    else {
      // start cmd sequence
      write_begin(false);
      // use page addressing mode
      write_cmd(CMD_MEMORY_ADDR_MODE, 0x02U);
      // set column start and end address (this sets the column pointer to 0)
      write_cmd(CMD_COLUMN_ADDR, vertex.x, Screen_Size_X);
      // set page start and end address (this sets the page pointer to 0)
      write_cmd(CMD_PAGE_ADDR, vertex.y >> 8U, Screen_Size_Y / 8U);
      // finish sequence
      write_end();

      // read data
      std::uint8 gdd_data;
      // read one byte
      read_data(&gdd_data);

      return color::L1_to_color((gdd_data >> (vertex.y & 0x07U)) & 0x01U);
    }
  }


  /**
   * Draw a horizontal line in the given color, width is one pixel
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, y component is ignored
   * \param color Line color
   */
  virtual void line_horz(vertex_type v0, vertex_type v1, color::value_type color)
  {
  }


  ///////////////////////////////////////////////////////////////////////////////
  // D I S P L A Y   C O N T R O L
  //

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
   * Write command, no param
   * \param cmd Command
   */
  inline void write_cmd(std::uint8_t cmd)
  {
    io_buffer_[io_buffer_idx_++] = cmd;
  }


  /**
   * Write command, 1 param
   * \param cmd Command
   * \param param1 Data
   */
  inline void write_cmd(std::uint8_t cmd, std::uint8_t param1)
  {
    io_buffer_[io_buffer_idx_++] = cmd;
    io_buffer_[io_buffer_idx_++] = param1;
  }


  /**
   * Write command, 2 param
   * \param cmd Command
   * \param param1 Data
   * \param param2 Data
   */
  inline void write_cmd(std::uint8_t cmd, std::uint8_t param1, std::uint8_t param2)
  {
    io_buffer_[io_buffer_idx_++] = cmd;
    io_buffer_[io_buffer_idx_++] = param1;
    io_buffer_[io_buffer_idx_++] = param2;
  }


  // start sequence
  // \param data_cmd True for data, false for command
  inline void write_begin(bool data_cmd)
  {
    // Interface_Mode is a constant template parameter, so the switch case is optimized away
    switch (Interface_Mode) {
      case SSD1306_SPI3 :   // 3-wire SPI
        break;
      case SSD1306_SPI4 :   // 4 wire SPI
        break;
      case SSD1306_I2C :    // I²C
        io_buffer_idx_ = 0U;
        io_buffer_[io_buffer_idx_++] = data_cmd ? I2C_CONTROL_DATA_STREAM : I2C_CONTROL_CMD_STREAM;
        break;
      case SSD1306_MEM :    // 8-bit memory
        break;
      default :
        break;
    }
  }


  inline void write_end(std::uint8_t cmd)
  {
    // Interface_Mode is a constant template parameter, so the switch case is optimized away
    switch (Interface_Mode) {
      case SSD1306_SPI3 :   // 3-wire SPI
        break;
      case SSD1306_SPI4 :   // 4 wire SPI
        break;
      case SSD1306_I2C :    // I²C
        io::write(device_handle_, 1U, io_buffer_, io_buffer_idx_, nullptr, 0U);
        break;
      case SSD1306_MEM :    // 8-bit memory
        break;
      default :
    }
  }


  /**
   * Write framebuffer
   */
  inline void write_framebuffer()
  {
    // Interface_Mode is a constant template parameter, so the switch case is optimized away
    switch (Interface_Mode) {
      case SSD1306_SPI3 :   // 3-wire SPI
        break;
      case SSD1306_SPI4 :   // 4 wire SPI
        break;
      case SSD1306_I2C :    // I²C
        framebuffer_.ctrl = I2C_CONTROL_DATA_STREAM;
        io::write(device_handle_, 1U, std::uint_8_t*>(framebuffer_), 1U + (Screen_Size_Y / 8U) * Screen_Size_X, nullptr, 0U);
        break;
      case SSD1306_MEM :    // 8-bit memory
        break;
      default :
        break;
    }
  }


  inline bool read_data(std::uint8_t* data)
  {
    std::size_t length = 1U;
    // Interface_Mode is a constant template parameter, so the switch case is optimized away
    switch (Interface_Mode) {
      case SSD1306_SPI3 :   // 3-wire SPI
        break;
      case SSD1306_SPI4 :   // 4 wire SPI
        break;
      case SSD1306_I2C :    // I²C
        io::read(device_handle_, 1U, data, length, 0U);
        break;
      case SSD1306_MEM :    // 8-bit memory
        break;
      default :
        break;
    }
  }

};

} // namespace head
} // namespace vic

#endif  // _VIC_DRV_SSD1306_H_
