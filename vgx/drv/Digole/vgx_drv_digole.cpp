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

#include "vgx_drv_digole.h"

// defines the driver name and version
#define VGX_DRV_VERSION   "Digole driver 1.00"


namespace vgx {


void drv_digole::init()
{
  if (interface_ == interface_i2c) {
    // set I²C address
    std::uint8_t i2c_adr = interface_port_;
    interface_port_ = 0x27;   // use default I²C address for init
    cmd_[0] = 'S';
    cmd_[1] = 'I';
    cmd_[2] = '2';
    cmd_[3] = 'C';
    cmd_[4] = 'A';
    cmd_[5] =  i2c_adr;
    send(cmd_, 6U);
    interface_port_ = i2c_adr;  // use new address now
  }

  // set orientation
  cmd_[0] = 'S';
  cmd_[1] = 'D';
  cmd_[2] =  static_cast<std::uint8_t>(orientation_);
  send(cmd_, 3U);

  // clear screen
  cls();

  // display on
  brightness_set(255U);

  if (interface_ == interface_uart) {
  // set UART baudrate
    cmd_[0] = 'S';
    cmd_[1] = 'B';
    switch (uart_baudrate_) {
      case    300 : cmd_[2] = '3'; cmd_[3] = '0'; cmd_[4] = '0'; send(cmd_, 5U); break;
      case   1200 : cmd_[2] = '1'; cmd_[3] = '2'; cmd_[4] = '0'; cmd_[5] = '0'; send(cmd_, 6U); break;
      case   2400 : cmd_[2] = '2'; cmd_[3] = '4'; cmd_[4] = '0'; cmd_[5] = '0'; send(cmd_, 6U); break;
      case   4800 : cmd_[2] = '4'; cmd_[3] = '8'; cmd_[4] = '0'; cmd_[5] = '0'; send(cmd_, 6U); break;
      case   9600 : cmd_[2] = '9'; cmd_[3] = '6'; cmd_[4] = '0'; cmd_[5] = '0'; send(cmd_, 6U); break;
      case  19200 : cmd_[2] = '1'; cmd_[3] = '9'; cmd_[4] = '2'; cmd_[5] = '0'; cmd_[6] = '0'; send(cmd_, 7U); break;
      case  28800 : cmd_[2] = '2'; cmd_[3] = '8'; cmd_[4] = '8'; cmd_[5] = '0'; cmd_[6] = '0'; send(cmd_, 7U); break;
      case  38400 : cmd_[2] = '3'; cmd_[3] = '8'; cmd_[4] = '4'; cmd_[5] = '0'; cmd_[6] = '0'; send(cmd_, 7U); break;
      case  57600 : cmd_[2] = '5'; cmd_[3] = '7'; cmd_[4] = '6'; cmd_[5] = '0'; cmd_[6] = '0'; send(cmd_, 7U); break;
      case 115200 : cmd_[2] = '1'; cmd_[3] = '1'; cmd_[4] = '5'; cmd_[5] = '2'; cmd_[6] = '0'; cmd_[7] = '0'; send(cmd_, 8U); break;
      default:
        break;
    }
  }
}


void drv_digole::deinit()
{
  brightness_set(0U);     // display off
}


void drv_digole::brightness_set(std::uint8_t level)
{
  cmd_[0] = 'B';
  cmd_[1] = 'L';
  cmd_[2] = level == 0U ? 0U : 1U;
  cmd_[3] = 'S';
  cmd_[4] = 'O';
  cmd_[5] = 'O';
  cmd_[6] = level == 0U ? 0U : 1U;
  send(cmd_, 7U);
}


const char* drv_digole::version() const
{
  return (const char*)VGX_DRV_VERSION;
}


void drv_digole::cls()
{
  cmd_[0] = 'C';
  cmd_[1] = 'L';
  cmd_[2] = 'T';
  cmd_[3] = 'P';
  cmd_[4] = static_cast<std::uint8_t>(0);
  cmd_[5] = static_cast<std::uint8_t>(0);
  send(cmd_, 6U);
}


void drv_digole::pixel_set(int16_t x, int16_t y)
{
  cmd_[0] = 'D';
  cmd_[1] = 'P';
  cmd_[2] = static_cast<std::uint8_t>(x);
  cmd_[3] = static_cast<std::uint8_t>(y);
  send(cmd_, 4U);
}


void drv_digole::pixel_set_color(int16_t x, int16_t y, std::uint32_t color)
{
  cmd_[ 0] = 'E';
  cmd_[ 1] = 'S';
  cmd_[ 2] = 'C';
  cmd_[ 3] = color_get_red(color);
  cmd_[ 4] = color_get_green(color);
  cmd_[ 5] = color_get_blue(color);
  cmd_[ 6] = 'D';
  cmd_[ 7] = 'P';
  cmd_[ 8] = static_cast<std::uint8_t>(x);
  cmd_[ 9] = static_cast<std::uint8_t>(y);
  cmd_[10] = 'E';
  cmd_[11] = 'S';
  cmd_[12] = 'C';
  cmd_[13] = color_get_red(color_);
  cmd_[14] = color_get_green(color_);
  cmd_[15] = color_get_blue(color_);
  send(cmd_, 16U);
}


// The problem f the Digole displays is that they communicate unidirectional - you can't read anything back.
// To get the pixel color, a buffer would be necessary to store a display content copy locally, about 60k for 160x128 RGB
// Therefore this function is not implemented, (font) anti aliasing and fill function don't work correctly.
std::uint32_t drv_digole::pixel_get(int16_t, int16_t) const
{
  return color_bg_;
}


void drv_digole::color_set(std::uint32_t color)
{
  color_ = color;
  cmd_[0] = 'E';
  cmd_[1] = 'S';
  cmd_[2] = 'C';
  cmd_[3] = color_get_red(color);
  cmd_[4] = color_get_green(color);
  cmd_[5] = color_get_blue(color);
  send(cmd_, 6U);
}


void drv_digole::drv_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
  cmd_[0] = 'L';
  cmd_[1] = 'N';
  cmd_[2] = static_cast<std::uint8_t>(x0);
  cmd_[3] = static_cast<std::uint8_t>(y0);
  cmd_[4] = static_cast<std::uint8_t>(x1);
  cmd_[5] = static_cast<std::uint8_t>(y1);
  send(cmd_, 6U);
}


void drv_digole::drv_line_horz(std::int16_t x0, std::int16_t y0, std::int16_t x1)
{
  cmd_[0] = 'L';
  cmd_[1] = 'N';
  cmd_[2] = static_cast<std::uint8_t>(x0);
  cmd_[3] = static_cast<std::uint8_t>(y0);
  cmd_[4] = static_cast<std::uint8_t>(x1);
  cmd_[5] = static_cast<std::uint8_t>(y0);
  send(cmd_, 6U);
}


void drv_digole::drv_line_vert(std::int16_t x0, std::int16_t y0, std::int16_t y1)
{
  cmd_[0] = 'L';
  cmd_[1] = 'N';
  cmd_[2] = static_cast<std::uint8_t>(x0);
  cmd_[3] = static_cast<std::uint8_t>(y0);
  cmd_[4] = static_cast<std::uint8_t>(x0);
  cmd_[5] = static_cast<std::uint8_t>(y1);
  send(cmd_, 6U);
}


void drv_digole::drv_rect(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
{
  std::int16_t tmp;
  if (x0 > x1) { tmp = x0; x0 = x1; x1 = tmp; }
  if (y0 > y1) { tmp = y0; y0 = y1; y1 = tmp; }

  cmd_[0] = 'D';
  cmd_[1] = 'R';
  cmd_[2] = static_cast<std::uint8_t>(x0);
  cmd_[3] = static_cast<std::uint8_t>(y0);
  cmd_[4] = static_cast<std::uint8_t>(x1);
  cmd_[5] = static_cast<std::uint8_t>(y1);
  send(cmd_, 6U);
}


void drv_digole::drv_box(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
{
  std::int16_t tmp;
  if (x0 > x1) { tmp = x0; x0 = x1; x1 = tmp; }
  if (y0 > y1) { tmp = y0; y0 = y1; y1 = tmp; }

  cmd_[0] = 'F';
  cmd_[1] = 'R';
  cmd_[2] = static_cast<std::uint8_t>(x0);
  cmd_[3] = static_cast<std::uint8_t>(y0);
  cmd_[4] = static_cast<std::uint8_t>(x1);
  cmd_[5] = static_cast<std::uint8_t>(y1);
  send(cmd_, 6U);
}


void drv_digole::drv_circle(std::int16_t x, std::int16_t y, std::uint16_t r)
{
  cmd_[0] = 'C';
  cmd_[1] = 'C';
  cmd_[2] = static_cast<std::uint8_t>(x);
  cmd_[3] = static_cast<std::uint8_t>(y);
  cmd_[4] = static_cast<std::uint8_t>(r);
  cmd_[5] = static_cast<std::uint8_t>(0);
  send(cmd_, 6U);
}


void drv_digole::drv_disc(std::int16_t x, std::int16_t y, std::uint16_t r)
{
  cmd_[0] = 'C';
  cmd_[1] = 'C';
  cmd_[2] = static_cast<std::uint8_t>(x);
  cmd_[3] = static_cast<std::uint8_t>(y);
  cmd_[4] = static_cast<std::uint8_t>(r);
  cmd_[5] = static_cast<std::uint8_t>(1);
  send(cmd_, 6U);
}


void drv_digole::drv_move(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::uint16_t width, std::uint16_t height)
{
  cmd_[0] = 'M';
  cmd_[1] = 'A';
  cmd_[2] = static_cast<std::uint8_t>(x0);
  cmd_[3] = static_cast<std::uint8_t>(y0);
  cmd_[4] = static_cast<std::uint8_t>(width);
  cmd_[5] = static_cast<std::uint8_t>(height);
  cmd_[6] = static_cast<std::uint8_t>(x1);
  cmd_[7] = static_cast<std::uint8_t>(y1);
  send(cmd_, 8U);
}


void drv_digole::send(std::uint8_t* buffer, std::uint8_t length)
{
  switch (interface_) {
    case interface_spi :
      VGX_SPI(interface_port_, buffer, length, nullptr, 0);
      break;
    case interface_i2c :
      VGX_I2C(interface_port_, buffer, length, nullptr, 0);
      break;
    case interface_uart :
      VGX_UART_TX(interface_port_, buffer, length);
      break;
    default:
      break;
  }
}

} // namespace vgx
