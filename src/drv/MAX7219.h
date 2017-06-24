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
// \brief MAX7219/21 matrix LED driver
// Create a SPI device with the following characteristics
// mode = 0                 data captured on rising edge, clock base value is zero
// lsb  = false             MSB is transmitted first
// tx_idle_value = 0xFF     output data value during data read (normally 0xFF)
// clock_speed = 10000000   clock speed of the bus in [Hz] (10 MHz maximum for MAX7219)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_DRV_MAX7219_H_
#define _VIC_DRV_MAX7219_H_

#include "../drv.h"


// defines the driver name and version
#define VIC_DRV_MAX7219_VERSION   "MAX7219/21 driver 1.40"

namespace vic {
namespace head {


/**
 * MAX7219 head is a monochrome LED / 7-segment driver, using 1 bit color depth
 * \param Screen_Size_X Screen (buffer) width, should be normally 8
 * \param Screen_Size_Y Screen (buffer) height, should be normally 8
 */
template<std::uint16_t Screen_Size_X = 8U, std::uint16_t Screen_Size_Y = 8U>
class MAX7219 : public drv
{
  // register address map
  static const std::uint8_t REG_NOOP       = 0x00U;
  static const std::uint8_t REG_DIGIT0     = 0x01U;
  static const std::uint8_t REG_DIGIT1     = 0x02U;
  static const std::uint8_t REG_DIGIT2     = 0x03U;
  static const std::uint8_t REG_DIGIT3     = 0x04U;
  static const std::uint8_t REG_DIGIT4     = 0x05U;
  static const std::uint8_t REG_DIGIT5     = 0x06U;
  static const std::uint8_t REG_DIGIT6     = 0x07U;
  static const std::uint8_t REG_DIGIT7     = 0x08U;
  static const std::uint8_t REG_DECODE     = 0x09U;
  static const std::uint8_t REG_INTENSITY  = 0x0AU;
  static const std::uint8_t REG_SCANLIMIT  = 0x0BU;
  static const std::uint8_t REG_SHUTDOWN   = 0x0CU;
  static const std::uint8_t REG_TEST       = 0x0FU;

public:

  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S

  /**
   * ctor
   * \param orientation Screen orientation
   * \param spi_device_id Logical SPI bus device ID
   */
  MAX7219(orientation_type orientation, io::dev::handle_type device_handle)
    : drv(Screen_Size_X, Screen_Size_Y,
          Screen_Size_X, Screen_Size_Y,     // no viewport support (screen = viewport)
          0, 0,                             // no viewport support
          orientation)
    , device_handle_(device_handle)
  { }


  /**
   * dtor
   * Shutdown the driver
   */
  ~MAX7219()
  {
    drv_shutdown();
  }


protected:

  virtual void drv_init() final
  {
    // set scan limit according to width or height
    write(REG_SCANLIMIT, (orientation_ == orientation_0)  || (orientation_ == orientation_180) ||
                         (orientation_ == orientation_0m) || (orientation_ == orientation_180m) ?
                         static_cast<std::uint8_t>(screen_height() - 1U) : static_cast<std::uint8_t>(screen_width() - 1U));

    // set no decode
    write(REG_DECODE, 0x00U);

    // start normal operation
    write(REG_SHUTDOWN, 0x01U);

    // no test mode
    write(REG_TEST, 0x00U);

    // clear buffer
    drv_cls();

    // full brightness
    brightness_set(255U);
  }


  virtual void drv_shutdown() final
  {
    // clear buffer
    drv_cls();

    // shutdown operation
    write(REG_SHUTDOWN, 0x00U);
  }


  inline virtual const char* drv_version() const final
  {
    return (const char*)VIC_DRV_MAX7219_VERSION;
  }


  inline virtual bool drv_is_graphic() const final
  {
    // MAX7219 is a graphic display
    return true;
  }


  virtual void drv_cls() final
  {
    // clear display
    for (std::uint_fast8_t i = 0U; i < screen_height(); ++i) {
      digit_[i] = 0U;
    }
    // data to MAX7219
    drv_present();
  }


  virtual void drv_pixel_set_color(vertex_type point, color::value_type color) final
  {
    // check limits and clipping
    if (!screen_is_inside(point) || (!clipping_.is_inside(point))) {
      // out of bounds or outside clipping region
      return;
    }

    // set pixel in display buffer
    if (color_to_head_L1(color)) {
      digit_[point.y] |= static_cast<std::uint8_t>(0x01U << (point.x & 0x07U));     // set pixel
    }
    else {
      digit_[point.y] &= static_cast<std::uint8_t>(~(0x01U << (point.x & 0x07U)));  // clear pixel
    }
  }


  virtual inline color::value_type drv_pixel_get(vertex_type point) final 
  {
    // check limits
    if (!screen_is_inside(point)) {
      // out of bounds
      return color_from_head_L1(0U);
    }
    return color_from_head_L1(static_cast<std::uint8_t>(digit_[point.y] >> point.x) & 0x01U);
  }


  virtual void drv_present() final
  {
    // copy memory bitmap to screen
    switch (orientation_) {
      case orientation_0 :
        for (std::int16_t y = 0; y < viewport_height(); ++y) {
          write(REG_DIGIT0 + y, digit_[viewport_get().y + y]);
        }
        break;
      case orientation_90 :
        for (std::int16_t y = 0; y < viewport_height(); ++y) {
          std::uint8_t data = 0U;
          for (std::int16_t x = 0; x < viewport_width(); ++x) {
            data >>= 1U;
            data |= (digit_[viewport_get().y + x] & (0x80U >> y)) ? 0x80U : 0x00U;
          }
          write(REG_DIGIT0 + y, data);
        }
        break;
      case orientation_180 :
        for (std::int16_t y = 0; y < viewport_height(); ++y) {
          write(REG_DIGIT0 + y, util::byte_reverse(digit_[viewport_get().y + viewport_height() - y - 1]));
        }
        break;
      case orientation_270 :
        for (std::int16_t y = 0; y < viewport_height(); ++y) {
          std::uint8_t data = 0U;
          for (std::int16_t x = 0; x < viewport_width(); ++x) {
            data <<= 1U;
            data |= (digit_[viewport_get().y + x] & (0x01U << y)) ? 0x01U : 0x00U;
          }
          write(REG_DIGIT0 + y, data);
        }
        break;
      case orientation_0m :
        for (std::int16_t y = 0; y < viewport_height(); ++y) {
          write(REG_DIGIT0 + y, digit_[viewport_get().y + viewport_height() - y - 1]);
        }
        break;
      case orientation_90m :
        for (std::int16_t y = 0U; y < viewport_height(); ++y) {
          std::uint8_t data = 0U;
          for (std::int16_t x = 0; x < viewport_width(); ++x) {
            data >>= 1U;
            data |= (digit_[viewport_get().y + x] & (0x01U << y)) ? 0x80U : 0x00U;
          }
          write(REG_DIGIT0 + y, data);
        }
        break;
      case orientation_180m :
        for (std::int16_t y = 0; y < viewport_height(); ++y) {
          write(REG_DIGIT0 + y, util::byte_reverse(digit_[viewport_get().y + y]));
        }
        break;
      case orientation_270m :
        for (std::int16_t y = 0; y < viewport_height(); ++y) {
          std::uint8_t data = 0U;
          for (std::int16_t x = 0; x < viewport_width(); ++x) {
            data <<= 1U;
            data |= (digit_[viewport_get().y + x] & (0x80U >> y)) ? 0x01U : 0x00U;
          }
          write(REG_DIGIT0 + y, data);
        }
        break;
      default:
        break;
    }
  }


  virtual inline void brightness_set(std::uint8_t level) final
  {
    // set brightness, use upper nibble of level
    write(REG_INTENSITY, static_cast<std::uint8_t>(level >> 4U));
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
    return io::dev::write(device_handle_, 0U, data_out, 2U, nullptr, 0U);
  }

private:
  const io::dev::handle_type  device_handle_;           // (SPI) device handle
  std::uint8_t                digit_[Screen_Size_Y];    // display buffer, cause MAX7219 doesn't support reading data back
};

} // namespace head
} // namespace vic

#endif  // _VIC_DRV_MAX7219_H_
