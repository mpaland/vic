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
// \brief SPxxP05 generic matrix LED driver
// This driver is for the STMicroelectronics LED array drivers
// In xsize and ysize set the logical LED mapping
// BYTE_COUNT = 3 (SP24DP05)
// xsize = 8, ysize = 3
//
// Create a SPI device with the following characteristics to drive the SPxxP05:
// mode = 3                 SCK idle high, data captured on rising edge, data propagated on falling edge
// lsb  = false             MSB is transmitted first
// tx_idle_value = 0xFF     output data value during data read (normally 0xFF)
// clock_speed = 10000000   clock speed of the bus in [Hz]
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_DRV_STPXXP05_H_
#define _VGX_DRV_STPXXP05_H_

#include "../drv.h"


// defines the driver name and version
#define VGX_DRV_STPXXP05_VERSION   "STPxxP05 driver 1.02"

namespace vgx {
namespace head {


/**
 * STPxxP05 head is a monochrome LED driver, using 1 bit color depth
 * \param Screen_Size_X Screen (buffer) width
 * \param Screen_Size_Y Screen (buffer) height
 * \param Viewport_Size_X Viewport (window) width
 * \param Viewport_Size_Y Viewport (window) height
 * \param Byte_Count Available channels (in byte) of the chip (1 for STP08xP05, 2 for STP16xP05, 3 for STP24DP05)
*/
template<std::uint16_t Screen_Size_X = 8U,   std::uint16_t Screen_Size_Y = 8U,
         std::uint16_t Viewport_Size_X = 8U, std::uint16_t Viewport_Size_Y = 8U,
         std::uint_fast8_t Byte_Count = 1>
class STPxxP05 : public drv
{
public:
/////////////////////////////////////////////////////////////////////////////
// M A N D A T O R Y   F U N C T I O N S

  /**
   * ctor
   * \param xsize Screen width
   * \param ysize Screen height
   * \param xoffset X offset of the screen, relative to top/left corner
   * \param yoffset Y offset of the screen, relative to top/left corner
   * \param spi_device_id Logical SPI bus device ID
   */
  STPxxP05(orientation_type orientation, device_handle_type device_handle,
           std::uint16_t viewport_x = 0U, std::uint16_t viewport_y = 0U)
    : drv(Screen_Size_X,   Screen_Size_Y,
          Viewport_Size_Y, Viewport_Size_Y,
          viewport_x,      viewport_y,
          orientation)
    , device_handle_(device_handle)
  {
    // clear buffer
    cls();
  }


  /**
   * dtor
   * Shutdown the driver
   */
  ~STPxxP05()
  {
    shutdown();
  }


  // driver init
  virtual void init()
  { }


  // driver shutdown
  virtual void shutdown()
  {
    cls();
  }


  // get driver name and version
  virtual const char* version() const
  {
    return (const char*)VGX_DRV_STPXXP05_VERSION;
  }


  virtual bool is_graphic() const
  {
    // STPxxP05 is a graphic display
    return true;
  }


  // clear display, all pixels off (black)
  virtual void cls()
  {
    // clear display
    for (std::uint_fast8_t i = 0; i < BYTE_COUNT; ++i) {
      buffer_[i] = 0U;
    }
  }


  // rendering done (copy RAM / frame buffer to screen)
  virtual void primitive_done()
  {
    // write buffer to SPI
    write();
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
      buffer_[y] |= static_cast<std::uint8_t>(0x01U << (x & 0x07U));    // set pixel
    }
    else {
      buffer_[y] &= static_cast<std::uint8_t>(~(0x01U << (x & 0x07U))); // clear pixel
    }
  }


  virtual color::value_type pixel_get(std::int16_t x, std::int16_t y) const
  {
    // check limits
    if (x >= screen_width() || y >= screen_height()) {
      // out of bounds
      return color_from_head_L1(0U);
    }

    return color_from_head_L1((buffer_[y] >> (x & 0x07U)) & 0x01U);
  }


private:
  /**
   * Write data to SPI
   * \return True if successful
   */
  bool write()
  {
    if (primitive_lock_) {
      return;
    }

    // copy memory bitmap to screen
    switch (orientation_) {
      case orientation_0 :
        return io::dev_set(spi_device_id_, buffer_, Byte_Count, nullptr, 0U);
      // TBD: all other orientations
      default :
        break;
    }
  }

private:
  device_handle_type  device_handle_;       // Logical SPI device ID
  std::uint8_t  buffer_[Byte_Count];  // display buffer, cause reading data isn't supported
};

} // namespace head
} // namespace vgx

#endif  // _VGX_DRV_STPXXP05_H_
