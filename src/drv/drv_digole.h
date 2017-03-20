///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2014, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// This file is part of the vgx library.
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
// \brief Digole LCD driver for Digole (OLED) graphic displays
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_DRV_DIGOLE_H_
#define _VGX_DRV_DIGOLE_H_

#include "vgx_drv.h"


// defines the driver name and version
#define VGX_DRV_DIGOLE_VERSION   "Digole driver 1.10"

namespace vgx {
namespace head {


  /**
 * Digole head is an RGB color graphics driver
 * \param Screen_Size_X Screen (buffer) width
 * \param Screen_Size_Y Screen (buffer) height
 * \param Viewport_Size_X Viewport (window) width
 * \param Viewport_Size_Y Viewport (window) height
 */
template<std::uint16_t Screen_Size_X = 8U,   std::uint16_t Screen_Size_Y = 8U,
         std::uint16_t Viewport_Size_X = 8U, std::uint16_t Viewport_Size_Y = 8U>
class digole : public drv
{
public:
/////////////////////////////////////////////////////////////////////////////
// M A N D A T O R Y   F U N C T I O N S

  /**
   * ctor
   * \param orientation Screen orientation
   * \param iface Interface type, SPI, I²C or UART are valid
   * \param iface_id Interface id: SPI: device id, I²C: address, UART: port_id
   * \param uart_baudrate Baudrate of the UART interface, unused for SPI or I²C mode
   */
  digole(orientation_type orientation, interface_type iface, std::uint16_t iface_id, std::uint32_t uart_baudrate = 9600U)
    : drv(Screen_Size_X,   Screen_Size_Y,
          Viewport_Size_X, Viewport_Size_Y,
          0U, 0U)
    , orientation_(orientation)
    , interface_(iface)
    , interface_port_(iface_id)
    , uart_baudrate_(uart_baudrate)
  { }


  /**
   * dtor
   * Deinit the driver
   */
  ~digole()
  { deinit(); }


  void init()
  {
    if (interface_ == interface_i2c) {
      // set I²C address
      std::uint8_t i2c_adr = static_cast<std::uint8_t>(interface_port_);
      interface_port_ = 0x27;   // use default I²C address for init
      cmd_[0] = 'S';
      cmd_[1] = 'I';
      cmd_[2] = '2';
      cmd_[3] = 'C';
      cmd_[4] = 'A';
      cmd_[5] =  i2c_adr;
      (void)write(cmd_, 6U);
      interface_port_ = i2c_adr;  // use new address now
    }

    // set orientation
    cmd_[0] = 'S';
    cmd_[1] = 'D';
    cmd_[2] =  static_cast<std::uint8_t>(orientation_);
    (void)write(cmd_, 3U);

    // clear screen
    cls();

    // display on
    brightness_set(255U);

    if (interface_ == interface_uart) {
    // set UART baudrate
      std::uint8_t len = 0U;
      cmd_[0] = 'S';
      cmd_[1] = 'B';
      switch (uart_baudrate_) {
        case    300U : cmd_[2] = '3'; cmd_[3] = '0'; cmd_[4] = '0';                                              len = 5U; break;
        case   1200U : cmd_[2] = '1'; cmd_[3] = '2'; cmd_[4] = '0'; cmd_[5] = '0';                               len = 6U; break;
        case   2400U : cmd_[2] = '2'; cmd_[3] = '4'; cmd_[4] = '0'; cmd_[5] = '0';                               len = 6U; break;
        case   4800U : cmd_[2] = '4'; cmd_[3] = '8'; cmd_[4] = '0'; cmd_[5] = '0';                               len = 6U; break;
        case   9600U : cmd_[2] = '9'; cmd_[3] = '6'; cmd_[4] = '0'; cmd_[5] = '0';                               len = 6U; break;
        case  19200U : cmd_[2] = '1'; cmd_[3] = '9'; cmd_[4] = '2'; cmd_[5] = '0'; cmd_[6] = '0';                len = 7U; break;
        case  28800U : cmd_[2] = '2'; cmd_[3] = '8'; cmd_[4] = '8'; cmd_[5] = '0'; cmd_[6] = '0';                len = 7U; break;
        case  38400U : cmd_[2] = '3'; cmd_[3] = '8'; cmd_[4] = '4'; cmd_[5] = '0'; cmd_[6] = '0';                len = 7U; break;
        case  57600U : cmd_[2] = '5'; cmd_[3] = '7'; cmd_[4] = '6'; cmd_[5] = '0'; cmd_[6] = '0';                len = 7U; break;
        case 115200U : cmd_[2] = '1'; cmd_[3] = '1'; cmd_[4] = '5'; cmd_[5] = '2'; cmd_[6] = '0'; cmd_[7] = '0'; len = 8U; break;
        default: break;
      }
      (void)write(cmd_, len);
    }
  }


  void deinit()
  {
    brightness_set(0U);     // display off
  }


  void brightness_set(std::uint8_t level)
  {
    cmd_[0] = 'B';
    cmd_[1] = 'L';
    cmd_[2] = level == 0U ? 0U : 1U;
    cmd_[3] = 'S';
    cmd_[4] = 'O';
    cmd_[5] = 'O';
    cmd_[6] = level == 0U ? 0U : 1U;
    (void)write(cmd_, 7U);
  }


  const char* digole::version() const
  {
    return (const char*)VGX_DRV_DIGOLE_VERSION;
  }


  virtual bool is_graphic() const
  {
    // Digole LCD is a graphic display
    return true;
  }


  void cls()
  {
    cmd_[0] = 'C';
    cmd_[1] = 'L';
    cmd_[2] = 'T';
    cmd_[3] = 'P';
    cmd_[4] = static_cast<std::uint8_t>(0U);
    cmd_[5] = static_cast<std::uint8_t>(0U);
    (void)write(cmd_, 6U);
  }


  void drv_pixel_set(int16_t x, int16_t y)
  {
    // check limits and clipping
    if (x >= screen_width() || y >= screen_height() || (clipping_ && !clipping_->is_clipping(x, y))) {
      // out of bounds or outside clipping region
      return;
    }

    // assemble and send command
    cmd_[0] = 'D';
    cmd_[1] = 'P';
    cmd_[2] = static_cast<std::uint8_t>(x);
    cmd_[3] = static_cast<std::uint8_t>(y);
    (void)write(cmd_, 4U);
  }


  void drv_pixel_set_color(int16_t x, int16_t y, std::uint32_t color)
  {
    // check limits and clipping
    if (x >= screen_width() || y >= screen_height() || (clipping_ && !clipping_->is_clipping(x, y))) {
      // out of bounds or outside clipping region
      return;
    }

    // assemble and send command
    cmd_[ 0] = 'E';
    cmd_[ 1] = 'S';
    cmd_[ 2] = 'C';
    cmd_[ 3] = color::get_red(color);
    cmd_[ 4] = color::get_green(color);
    cmd_[ 5] = color::get_blue(color);
    cmd_[ 6] = 'D';
    cmd_[ 7] = 'P';
    cmd_[ 8] = static_cast<std::uint8_t>(x);
    cmd_[ 9] = static_cast<std::uint8_t>(y);
    cmd_[10] = 'E';
    cmd_[11] = 'S';
    cmd_[12] = 'C';
    cmd_[13] = color::get_red(color_);
    cmd_[14] = color::get_green(color_);
    cmd_[15] = color::get_blue(color_);
    (void)write(cmd_, 16U);
  }


  // The problem of the Digole displays is that they communicate unidirectional - you can't read anything back.
  // To get the pixel color, a buffer would be necessary to store a display content copy locally, about 60k for 160x128 RGB
  // Therefore this function is not implemented, (font) anti aliasing and fill function don't work correctly.
  std::uint32_t drv_pixel_get(int16_t, int16_t) const
  {
    return color_bg_;
  }


  void color_set(color::value_type color)
  {
    color_ = color;
    cmd_[0] = 'E';
    cmd_[1] = 'S';
    cmd_[2] = 'C';
    cmd_[3] = color::get_red(color);
    cmd_[4] = color::get_green(color);
    cmd_[5] = color::get_blue(color);
    (void)write(cmd_, 6U);
  }


  void drv_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
  {
    // check clipping
    if (clipping_) {
      // Digole displays doesn't support clipping, using the gpr instead
      gpr::drv_line(x0, y0, x1, y1); 
      return;
    }

    // assemble and send command
    cmd_[0] = 'L';
    cmd_[1] = 'N';
    cmd_[2] = static_cast<std::uint8_t>(x0);
    cmd_[3] = static_cast<std::uint8_t>(y0);
    cmd_[4] = static_cast<std::uint8_t>(x1);
    cmd_[5] = static_cast<std::uint8_t>(y1);
    (void)write(cmd_, 6U);
  }


  void drv_line_horz(std::int16_t x0, std::int16_t y0, std::int16_t x1)
  {
    // check clipping
    if (clipping_) {
      // Digole displays doesn't support clipping, using the gpr instead
      gpr::drv_line_horz(x0, y0, x1); 
      return;
    }

    // assemble and send command
    cmd_[0] = 'L';
    cmd_[1] = 'N';
    cmd_[2] = static_cast<std::uint8_t>(x0);
    cmd_[3] = static_cast<std::uint8_t>(y0);
    cmd_[4] = static_cast<std::uint8_t>(x1);
    cmd_[5] = static_cast<std::uint8_t>(y0);
    (void)write(cmd_, 6U);
  }


  void drv_line_vert(std::int16_t x0, std::int16_t y0, std::int16_t y1)
  {
    // check clipping
    if (clipping_) {
      // Digole displays doesn't support clipping, using the gpr instead
      gpr::drv_line_vert(x0, y0, y1); 
      return;
    }

    // assemble and send command
    cmd_[0] = 'L';
    cmd_[1] = 'N';
    cmd_[2] = static_cast<std::uint8_t>(x0);
    cmd_[3] = static_cast<std::uint8_t>(y0);
    cmd_[4] = static_cast<std::uint8_t>(x0);
    cmd_[5] = static_cast<std::uint8_t>(y1);
    (void)write(cmd_, 6U);
  }


  void drv_rect(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
  {
    // check clipping
    if (clipping_) {
      // Digole displays doesn't support clipping, using the gpr instead
      gpr::drv_rect(x0, y0, x1, y1); 
      return;
    }

    std::int16_t tmp;
    if (x0 > x1) { tmp = x0; x0 = x1; x1 = tmp; }
    if (y0 > y1) { tmp = y0; y0 = y1; y1 = tmp; }

    cmd_[0] = 'D';
    cmd_[1] = 'R';
    cmd_[2] = static_cast<std::uint8_t>(x0);
    cmd_[3] = static_cast<std::uint8_t>(y0);
    cmd_[4] = static_cast<std::uint8_t>(x1);
    cmd_[5] = static_cast<std::uint8_t>(y1);
    (void)write(cmd_, 6U);
  }


  void drv_box(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
  {
    // check clipping
    if (clipping_) {
      // Digole displays doesn't support clipping, using the gpr instead
      gpr::drv_box(x0, y0, x1, y1); 
      return;
    }

    std::int16_t tmp;
    if (x0 > x1) { tmp = x0; x0 = x1; x1 = tmp; }
    if (y0 > y1) { tmp = y0; y0 = y1; y1 = tmp; }

    cmd_[0] = 'F';
    cmd_[1] = 'R';
    cmd_[2] = static_cast<std::uint8_t>(x0);
    cmd_[3] = static_cast<std::uint8_t>(y0);
    cmd_[4] = static_cast<std::uint8_t>(x1);
    cmd_[5] = static_cast<std::uint8_t>(y1);
    (void)write(cmd_, 6U);
  }


  void drv_circle(std::int16_t x, std::int16_t y, std::uint16_t r)
  {
    // check clipping
    if (clipping_) {
      // Digole displays doesn't support clipping, using the gpr instead
      gpr::drv_circle(x, y, r);
      return;
    }

    cmd_[0] = 'C';
    cmd_[1] = 'C';
    cmd_[2] = static_cast<std::uint8_t>(x);
    cmd_[3] = static_cast<std::uint8_t>(y);
    cmd_[4] = static_cast<std::uint8_t>(r);
    cmd_[5] = static_cast<std::uint8_t>(0U);
    (void)write(cmd_, 6U);
  }


  void drv_disc(std::int16_t x, std::int16_t y, std::uint16_t r)
  {
    // check clipping
    if (clipping_) {
      // Digole displays doesn't support clipping, using the gpr instead
      gpr::drv_disc(x, y, r);
      return;
    }

    cmd_[0] = 'C';
    cmd_[1] = 'C';
    cmd_[2] = static_cast<std::uint8_t>(x);
    cmd_[3] = static_cast<std::uint8_t>(y);
    cmd_[4] = static_cast<std::uint8_t>(r);
    cmd_[5] = static_cast<std::uint8_t>(1U);
    (void)write(cmd_, 6U);
  }


  void drv_move(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::uint16_t width, std::uint16_t height)
  {
    // check clipping
    if (clipping_) {
      // Digole displays doesn't support clipping, using the gpr instead
      gpr::drv_move(x0, y0, x1, y1, width, height);
      return;
    }

    cmd_[0] = 'M';
    cmd_[1] = 'A';
    cmd_[2] = static_cast<std::uint8_t>(x0);
    cmd_[3] = static_cast<std::uint8_t>(y0);
    cmd_[4] = static_cast<std::uint8_t>(width);
    cmd_[5] = static_cast<std::uint8_t>(height);
    cmd_[6] = static_cast<std::uint8_t>(x1);
    cmd_[7] = static_cast<std::uint8_t>(y1);
    (void)write(cmd_, 8U);
  }


  inline bool write(std::uint8_t* buffer, std::uint16_t length)
  {
    return io::dev_set(interface_, interface_port_, buffer, length, nullptr, 0U);
  }

private:
  interface_type    interface_;         // interface type
  std::uint32_t     uart_baudrate_;     // baudrate for UART interface mode
  std::uint16_t     interface_port_;    // interface port
  std::uint8_t      cmd_[16U];          // command buffer
};

} // namespace head
} // namespace vgx

#endif  // _VGX_DRV_DIGOLE_H_
