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
// \brief MAX7219/21 matrix LED driver
//
///////////////////////////////////////////////////////////////////////////////

#include "vgx_drv_MAX7219.h"

// defines the driver name and version
#define VGX_DRV_VERSION   "MAX7219/21 driver 1.00"


namespace vgx {
namespace head {

// register address map
#define REG_NOOP        0x00U
#define REG_DIGIT0      0x01U
#define REG_DIGIT1      0x02U
#define REG_DIGIT2      0x03U
#define REG_DIGIT3      0x04U
#define REG_DIGIT4      0x05U
#define REG_DIGIT5      0x06U
#define REG_DIGIT6      0x07U
#define REG_DIGIT7      0x08U
#define REG_DECODE      0x09U
#define REG_INTENSITY   0x0AU
#define REG_SCANLIMIT   0x0BU
#define REG_SHUTDOWN    0x0CU
#define REG_TEST        0x0FU


void MAX7219::init()
{
  // clear buffer
  cls();

  // set scan limit according to width or height
  write(REG_SCANLIMIT, (orientation_ == orientation_0) || (orientation_ == orientation_180) ? static_cast<std::uint8_t>(ysize_ - 1U) : static_cast<std::uint8_t>(xsize_ - 1U));

  // start normal operation
  write(REG_SHUTDOWN, 0x01U);
}


void MAX7219::deinit()
{
  cls();

  // shutdown operation
  write(REG_SHUTDOWN, 0x00U);
}


void MAX7219::brightness_set(std::uint8_t level)
{
  // set brightness, use lower nibble of level
  write(REG_INTENSITY, static_cast<std::uint8_t>(level >> 4U));
}


const char* MAX7219::version() const
{
  return (const char*)VGX_DRV_VERSION;
}


void MAX7219::primitive_done()
{
  // copy memory bitmap to screen
  switch (orientation_) {
    case orientation_0 :
      for (std::uint_fast8_t i = 0; i < get_height(); ++i) {
        write(REG_DIGIT0 + i, digit_[i]);
      }
      break;
    case orientation_90 :
      for (std::uint_fast8_t i = 0U; i < get_height(); ++i) {
        std::uint8_t data = 0U;
        for (std::uint_fast8_t n = 0; n < get_width(); ++n) {
          data |= (digit_[n] & (0x01U << i)) ? 0x01U : 0x00U;
          data <<= 1U;
        }
        write(REG_DIGIT0 + i, data);
      }
      break;
    case orientation_180 :
      for (std::uint_fast8_t i = 0U; i < get_height(); ++i) {
        write(REG_DIGIT0 + i, byte_reverse(digit_[get_height() - i - 1U]));
      }
      break;
    case orientation_270 :
      //TBD
      break;
    default:
      break;
  }
}


void MAX7219::cls()
{
  // clear display
  for (std::uint_fast8_t i = 0U; i < 8U; ++i) {
    digit_[i] = 0U;
  }
}


void MAX7219::pixel_set_color(std::int16_t x, std::int16_t y, std::uint32_t color)
{
  // check limits
  if (x >= xsize_ || y >= ysize_ || x + xoffset_ >= 8 || y + yoffset_ >= 8) {
    return;
  }

  // set pixel in display buffer
  if (color_to_head_L1(color)) {
    digit_[y + yoffset_] |= static_cast<std::uint8_t>(0x01U << (x + xoffset_));     // set pixel
  }
  else {
    digit_[y + yoffset_] &= static_cast<std::uint8_t>(~(0x01U << (x + xoffset_)));  // clear pixel
  }
}


std::uint32_t MAX7219::pixel_get(std::int16_t x, std::int16_t y) const
{
  // check limits
  if (x >= xsize_ || y >= ysize_ || x + xoffset_ >= 8 || y + yoffset_ >= 8) {
    return color_from_head_L1(0U);
  }
    return color_from_head_L1(static_cast<std::uint8_t>(digit_[y] >> x) & 0x01U);
}

} // namespace head
} // namespace vgx
