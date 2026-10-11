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
// \brief Digole head, serial graphic display adapters (SPI, I²C or UART)
// The display has native line, box and move commands, so most primitives are
// rendered by the display firmware. Coordinates are 8 bit, so the display must
// not be bigger than 256 x 256 pixel.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_DIGOLE_H_
#define _VIC_HEAD_DIGOLE_H_

#include "../drv.h"


// defines the head name and version
#define VIC_HEAD_DIGOLE_VERSION   "Digole head 2.2.0"

namespace vic {
namespace head {


// interface type
typedef enum tag_digole_interface_type {
  DIGOLE_SPI,
  DIGOLE_I2C,
  DIGOLE_UART
} digole_interface_type;


/**
 * Digole head is an RGB color graphics driver
 * \param Screen_Size_X Screen (buffer) width
 * \param Screen_Size_Y Screen (buffer) height
 * \param Viewport_Size_X Viewport (window) width
 * \param Viewport_Size_Y Viewport (window) height
 * \param Orientation Hardware orientation of the display
 * \param Interface_Mode Interface type, SPI, I²C or UART are valid
 */
template<std::uint16_t Screen_Size_X   = 8U, std::uint16_t Screen_Size_Y   = 8U,
         std::uint16_t Viewport_Size_X = Screen_Size_X, std::uint16_t Viewport_Size_Y = Screen_Size_Y,
         drv::orientation_type Orientation = drv::orientation_0,
         digole_interface_type Interface_Mode = DIGOLE_UART
>
class digole final : public drv
{
  io::handle_type         device_handle_;   // device handle, SPI: device id, I²C: address, UART: port
  std::uint32_t           uart_baudrate_;   // baudrate for UART interface mode
  color::value_type       color_;           // actual pixel color of the display

public:

  /**
   * ctor
   * \param device_handle SPI: device id, I²C: address, UART: port_id
   * \param uart_baudrate Baudrate of the UART interface, unused for SPI or I²C mode
   */
  digole(io::handle_type device_handle,
         std::uint32_t uart_baudrate = 9600U)
    : drv(Screen_Size_X,   Screen_Size_Y,
          Viewport_Size_X, Viewport_Size_Y,
          0, 0,
          Orientation)
    , device_handle_(device_handle)
    , uart_baudrate_(uart_baudrate)
    , color_(color::none)
  { }


  /**
   * dtor
   * Shutdown the driver
   */
  ~digole()
  {
    shutdown();
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   D R I V E R   F U N C T I O N S
  //

  virtual void init()
  {
    std::uint8_t cmd[8];

    io::init(device_handle_);

    if (Interface_Mode == DIGOLE_I2C) {
      // set the I²C address: the display listens on the default address ($27) first
      const std::uint8_t i2c_adr = static_cast<std::uint8_t>(reinterpret_cast<std::uintptr_t>(device_handle_));
      device_handle_ = reinterpret_cast<io::handle_type>(static_cast<std::uintptr_t>(0x27U));
      cmd[0] = 'S';
      cmd[1] = 'I';
      cmd[2] = '2';
      cmd[3] = 'C';
      cmd[4] = 'A';
      cmd[5] = i2c_adr;
      write(cmd, 6U);
      device_handle_ = reinterpret_cast<io::handle_type>(static_cast<std::uintptr_t>(i2c_adr));  // use the new address now
    }

    // set orientation
    cmd[0] = 'S';
    cmd[1] = 'D';
    cmd[2] = static_cast<std::uint8_t>(orientation_);
    write(cmd, 3U);

    // cursor off
    cmd[0] = 'C';
    cmd[1] = 'S';
    cmd[2] = 0U;
    write(cmd, 3U);

    // clear screen
    cls();

    // display and backlight on
    display_enable();
    display_brightness(255U);

    if (Interface_Mode == DIGOLE_UART) {
      // set UART baudrate
      std::uint8_t len = 0U;
      cmd[0] = 'S';
      cmd[1] = 'B';
      switch (uart_baudrate_) {
        case    300U : cmd[2] = '3'; cmd[3] = '0'; cmd[4] = '0';                                           len = 5U; break;
        case   1200U : cmd[2] = '1'; cmd[3] = '2'; cmd[4] = '0'; cmd[5] = '0';                             len = 6U; break;
        case   2400U : cmd[2] = '2'; cmd[3] = '4'; cmd[4] = '0'; cmd[5] = '0';                             len = 6U; break;
        case   4800U : cmd[2] = '4'; cmd[3] = '8'; cmd[4] = '0'; cmd[5] = '0';                             len = 6U; break;
        case   9600U : cmd[2] = '9'; cmd[3] = '6'; cmd[4] = '0'; cmd[5] = '0';                             len = 6U; break;
        case  19200U : cmd[2] = '1'; cmd[3] = '9'; cmd[4] = '2'; cmd[5] = '0'; cmd[6] = '0';               len = 7U; break;
        case  28800U : cmd[2] = '2'; cmd[3] = '8'; cmd[4] = '8'; cmd[5] = '0'; cmd[6] = '0';               len = 7U; break;
        case  38400U : cmd[2] = '3'; cmd[3] = '8'; cmd[4] = '4'; cmd[5] = '0'; cmd[6] = '0';               len = 7U; break;
        case  57600U : cmd[2] = '5'; cmd[3] = '7'; cmd[4] = '6'; cmd[5] = '0'; cmd[6] = '0';               len = 7U; break;
        case 115200U : cmd[2] = '1'; cmd[3] = '1'; cmd[4] = '5'; cmd[5] = '2'; cmd[6] = '0'; cmd[7] = '0'; len = 8U; break;
        default: break;
      }
      if (len) {
        write(cmd, len);
      }
    }
  }


  virtual void shutdown()
  {
    cls();
    display_enable(false);    // display off
    display_brightness(0U);   // backlight off
  }


  inline virtual const char* version() const
  {
    return (const char*)VIC_HEAD_DIGOLE_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    // Digole LCD is a graphic display
    return true;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

protected:

  virtual void cls(color::value_type bg_color = color::none)
  {
    (void)bg_color;
    const std::uint8_t cmd[2] = { 'C', 'L' };
    write(cmd, 2U);
  }


  ///////////////////////////////////////////////////////////////////////////////
  // G R A P H I C   F U N C T I O N S
  //

  virtual void pixel_set(vertex_type vertex, color::value_type color)
  {
    // check limits and clipping
    if (!screen_is_inside(vertex)) {
      // out of bounds
      return;
    }

    std::uint8_t cmd[10];
    std::uint8_t len = set_color(cmd, color);
    cmd[len++] = 'D';
    cmd[len++] = 'P';
    cmd[len++] = static_cast<std::uint8_t>(vertex.x);
    cmd[len++] = static_cast<std::uint8_t>(vertex.y);
    write(cmd, len);
  }


  virtual void line_horz(vertex_type v0, vertex_type v1, color::value_type color)
  {
    std::uint8_t cmd[12];
    std::uint8_t len = set_color(cmd, color);
    cmd[len++] = 'L';
    cmd[len++] = 'N';
    cmd[len++] = static_cast<std::uint8_t>(v0.x);
    cmd[len++] = static_cast<std::uint8_t>(v0.y);
    cmd[len++] = static_cast<std::uint8_t>(v1.x);
    cmd[len++] = static_cast<std::uint8_t>(v0.y);     // the y of v1 is ignored
    write(cmd, len);
  }


  virtual void line_vert(vertex_type v0, vertex_type v1, color::value_type color)
  {
    std::uint8_t cmd[12];
    std::uint8_t len = set_color(cmd, color);
    cmd[len++] = 'L';
    cmd[len++] = 'N';
    cmd[len++] = static_cast<std::uint8_t>(v0.x);
    cmd[len++] = static_cast<std::uint8_t>(v0.y);
    cmd[len++] = static_cast<std::uint8_t>(v0.x);     // the x of v1 is ignored
    cmd[len++] = static_cast<std::uint8_t>(v1.y);
    write(cmd, len);
  }


  virtual void box(const rect_type& rect, color::value_type color)
  {
    if ((rect.right <= rect.left) || (rect.bottom <= rect.top)) {
      return;
    }
    // the FR command takes the inclusive corners
    std::uint8_t cmd[12];
    std::uint8_t len = set_color(cmd, color);
    cmd[len++] = 'F';
    cmd[len++] = 'R';
    cmd[len++] = static_cast<std::uint8_t>(rect.left);
    cmd[len++] = static_cast<std::uint8_t>(rect.top);
    cmd[len++] = static_cast<std::uint8_t>(rect.right - 1);
    cmd[len++] = static_cast<std::uint8_t>(rect.bottom - 1);
    write(cmd, len);
  }


  virtual void move(const rect_type& source, vertex_type destination)
  {
    const std::uint8_t cmd[8] = {
      'M', 'A',
      static_cast<std::uint8_t>(source.left),                    // left
      static_cast<std::uint8_t>(source.top),                     // top
      static_cast<std::uint8_t>(source.width()),                 // width
      static_cast<std::uint8_t>(source.height()),                // height
      static_cast<std::uint8_t>(destination.x - source.left),    // x offset
      static_cast<std::uint8_t>(destination.y - source.top)      // y offset
    };
    write(cmd, 8U);
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
    const std::uint8_t cmd[4] = { 'S', 'O', 'O', static_cast<std::uint8_t>(enable ? 1U : 0U) };
    write(cmd, 4U);
  }


  /**
   * Set display or backlight brightness
   * \param level 0: dark, backlight off; 255: maximum brightness, backlight full on
   */
  virtual void display_brightness(std::uint8_t level)
  {
    const std::uint8_t cmd[3] = { 'B', 'L', static_cast<std::uint8_t>(level == 0U ? 0U : 1U) };
    write(cmd, 3U);
  }


private:

  /**
   * Put the color set command in front of a drawing command if the color changed
   * \param cmd Command buffer
   * \param color New drawing color
   * \return Number of bytes written to cmd (0 or 6)
   */
  inline std::uint8_t set_color(std::uint8_t* cmd, color::value_type color)
  {
    if (color == color_) {
      return 0U;
    }
    color_ = color;
    // use enhanced color setup
    cmd[0] = 'E';
    cmd[1] = 'S';
    cmd[2] = 'C';
    cmd[3] = color::get_red(color);
    cmd[4] = color::get_green(color);
    cmd[5] = color::get_blue(color);
    return 6U;
  }


  inline void write(const std::uint8_t* buffer, std::uint8_t length)
  {
    io::write(device_handle_, 0U, buffer, length, nullptr, 0U);
  }

};

} // namespace head
} // namespace vic

#endif  // _VIC_HEAD_DIGOLE_H_
