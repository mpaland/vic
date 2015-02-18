///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2015, PALANDesign Hannover, Germany
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
// Create a SPI device with the following characteristics
// mode = 3                 SCK idle high, data captured on rising edge, data propagated on falling edge
// lsb  = false             MSB is transmitted first
// tx_idle_value = 0xFF     output data value during data read (normally 0xFF)
// clock_speed = 10000000   clock speed of the bus in [Hz] (10 MHz maximum for MAX7219)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_DRV_MAX7219_H_
#define _VGX_DRV_MAX7219_H_

#include "vgx_drv.h"


// defines the driver name and version
#define VGX_DRV_MAX7219_VERSION   "MAX7219/21 driver 1.00"

namespace vgx {
namespace head {


/**
 * MAX7219 head is a monochrome LED / 7-segment driver, using 1 bit color depth
 * \param Screen_Size_X Screen (buffer) width
 * \param Screen_Size_Y Screen (buffer) height
 * \param Viewport_Size_X Viewport (window) width
 * \param Viewport_Size_Y Viewport (window) height
 */
template<std::uint16_t Screen_Size_X = 8U,   std::uint16_t Screen_Size_Y = 8U,
         std::uint16_t Viewport_Size_X = 8U, std::uint16_t Viewport_Size_Y = 8U>
class MAX7219 : public drv
{
public:
/////////////////////////////////////////////////////////////////////////////
// M A N D A T O R Y   F U N C T I O N S

  /**
   * ctor
   * \param orientation Screen orientation
   * \param spi_device_id Logical SPI bus device ID
   * \param viewport_x X offset of the viewport, relative to top/left corner
   * \param viewport_y Y offset of the viewport, relative to top/left corner
   */
  MAX7219(orientation_type orientation, std::uint16_t spi_device_id,
          std::uint16_t viewport_x = 0U, std::uint16_t viewport_y = 0U)
    : drv(8U, Screen_Size_Y,
          8U, Viewport_Size_Y,
          viewport_x, viewport_y,
          orientation)
    , spi_device_id_(spi_device_id)
  { }


  /**
   * dtor
   * Deinit the driver
   */
  ~MAX7219()
  { deinit(); }


  virtual void init()
  {
    // set scan limit according to width or height
    write(REG_SCANLIMIT, (orientation_ == orientation_0)  || (orientation_ == orientation_180) ||
                         (orientation_ == orientation_0m) || (orientation_ == orientation_180m) ?
                         static_cast<std::uint8_t>(viewport_height() - 1U) : static_cast<std::uint8_t>(viewport_width() - 1U));

    // set no decode
    write(REG_DECODE, 0x00);

    // clear buffer
    cls();
    brightness_set(255U);   // full brightness

    // start normal operation
    write(REG_SHUTDOWN, 0x01U);
  }


  virtual void deinit()
  {
    // clear buffer
    cls();

    // shutdown operation
    write(REG_SHUTDOWN, 0x00U);
  }


  virtual void brightness_set(std::uint8_t level)
  {
    // set brightness, use lower nibble of level
    write(REG_INTENSITY, static_cast<std::uint8_t>(level >> 4U));
  }


  virtual const char* version() const
  {
    return (const char*)VGX_DRV_MAX7219_VERSION;
  }


  virtual bool is_graphic() const
  {
    // MAX7219 is a graphic display
    return true;
  }


  virtual void cls()
  {
    // clear display
    for (std::uint_fast8_t i = 0U; i < screen_height(); ++i) {
      digit_[i] = 0U;
    }
    // data to MAX7219
    drv_primitive_done();
  }


  virtual void drv_pixel_set_color(std::int16_t x, std::int16_t y, color::value_type color)
  {
    // check limits and clipping
    if (x >= screen_width() || y >= screen_height() || (clipping_ && !clipping_->is_clipping(x, y))) {
      // out of bounds or outside clipping region
      return;
    }

    // set pixel in display buffer
    if (color_to_head_L1(color)) {
      digit_[y] |= static_cast<std::uint8_t>(0x01U << (x & 0x07U));     // set pixel
    }
    else {
      digit_[y] &= static_cast<std::uint8_t>(~(0x01U << (x & 0x07U)));  // clear pixel
    }
  }


  virtual color::value_type drv_pixel_get(std::int16_t x, std::int16_t y) const
  {
    // check limits
    if (x >= screen_width() || y >= screen_height()) {
      // out of bounds
      return color_from_head_L1(0U);
    }
    return color_from_head_L1(static_cast<std::uint8_t>(digit_[y] >> x) & 0x01U);
  }


  virtual void drv_primitive_done()
  {
    if (primitive_lock_) {
      return;
    }

    // copy memory bitmap to screen
    switch (orientation_) {
      case orientation_0 :
        for (std::uint_fast8_t y = 0U; y < static_cast<uint_fast8_t>(viewport_height()); ++y) {
          write(REG_DIGIT0 + y, digit_[viewport_y_ + y]);
        }
        break;
      case orientation_90 :
        for (std::uint_fast8_t y = 0U; y < static_cast<uint_fast8_t>(viewport_height()); ++y) {
          std::uint8_t data = 0U;
          for (std::uint_fast8_t x = 0U; x < static_cast<uint_fast8_t>(viewport_width()); ++x) {
            data >>= 1U;
            data |= (digit_[viewport_y_ + x] & (0x80U >> y)) ? 0x80U : 0x00U;
          }
          write(REG_DIGIT0 + y, data);
        }
        break;
      case orientation_180 :
        for (std::uint_fast8_t y = 0U; y < static_cast<uint_fast8_t>(viewport_height()); ++y) {
          write(REG_DIGIT0 + y, byte_reverse(digit_[viewport_y_ + viewport_height() - y - 1U]));
        }
        break;
      case orientation_270 :
        for (std::uint_fast8_t y = 0U; y < static_cast<uint_fast8_t>(viewport_height()); ++y) {
          std::uint8_t data = 0U;
          for (std::uint_fast8_t x = 0U; x < static_cast<uint_fast8_t>(viewport_width()); ++x) {
            data <<= 1U;
            data |= (digit_[viewport_y_ + x] & (0x01U << y)) ? 0x01U : 0x00U;
          }
          write(REG_DIGIT0 + y, data);
        }
        break;
      case orientation_0m :
        for (std::uint_fast8_t y = 0U; y < static_cast<uint_fast8_t>(viewport_height()); ++y) {
          write(REG_DIGIT0 + y, digit_[viewport_y_ + viewport_height() - y - 1U]);
        }
        break;
      case orientation_90m :
        for (std::uint_fast8_t y = 0U; y < static_cast<uint_fast8_t>(viewport_height()); ++y) {
          std::uint8_t data = 0U;
          for (std::uint_fast8_t x = 0U; x < static_cast<uint_fast8_t>(viewport_width()); ++x) {
            data >>= 1U;
            data |= (digit_[viewport_y_ + x] & (0x01U << y)) ? 0x80U : 0x00U;
          }
          write(REG_DIGIT0 + y, data);
        }
        break;
      case orientation_180m :
        for (std::uint_fast8_t y = 0U; y < static_cast<uint_fast8_t>(viewport_height()); ++y) {
          write(REG_DIGIT0 + y, byte_reverse(digit_[viewport_y_ + y]));
        }
        break;
      case orientation_270m :
        for (std::uint_fast8_t y = 0U; y < static_cast<uint_fast8_t>(viewport_height()); ++y) {
          std::uint8_t data = 0U;
          for (std::uint_fast8_t x = 0U; x < static_cast<uint_fast8_t>(viewport_width()); ++x) {
            data <<= 1U;
            data |= (digit_[viewport_y_ + x] & (0x80U >> y)) ? 0x01U : 0x00U;
          }
          write(REG_DIGIT0 + y, data);
        }
        break;
      default:
        break;
    }
  }

private:
  /**
   * Write data to SPI
   * \param addr Register address
   * \param data Data
   * \return True if successful
   */
  inline bool write(std::uint8_t addr, std::uint8_t data)
  {
    std::uint8_t data_out[2] = { addr, data };
    return io::dev_set(interface_spi, spi_device_id_, data_out, 2U, nullptr, 0U);
  }

  /**
   * Helper function for fast byte reverse (e.g. 0x80 to 0x01)
   * \param data Input byte
   * \return Reversed byte
   */
  inline std::uint8_t byte_reverse(std::uint8_t data) const
  {
    return static_cast<std::uint8_t>(((data * 0x0802LU & 0x22110LU) | (data * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16U);
  }

private:
  // register address map
  const std::uint8_t REG_NOOP       = 0x00U;
  const std::uint8_t REG_DIGIT0     = 0x01U;
  const std::uint8_t REG_DIGIT1     = 0x02U;
  const std::uint8_t REG_DIGIT2     = 0x03U;
  const std::uint8_t REG_DIGIT3     = 0x04U;
  const std::uint8_t REG_DIGIT4     = 0x05U;
  const std::uint8_t REG_DIGIT5     = 0x06U;
  const std::uint8_t REG_DIGIT6     = 0x07U;
  const std::uint8_t REG_DIGIT7     = 0x08U;
  const std::uint8_t REG_DECODE     = 0x09U;
  const std::uint8_t REG_INTENSITY  = 0x0AU;
  const std::uint8_t REG_SCANLIMIT  = 0x0BU;
  const std::uint8_t REG_SHUTDOWN   = 0x0CU;
  const std::uint8_t REG_TEST       = 0x0FU;

private:
  std::uint16_t     spi_device_id_;           // Logical SPI device ID
  std::uint8_t      digit_[Screen_Size_Y];    // display buffer, cause reading data isn't supported
};

} // namespace head
} // namespace vgx

#endif  // _VGX_DRV_MAX7219_H_
