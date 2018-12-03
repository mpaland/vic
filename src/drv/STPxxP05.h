///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2014, PALANDesign Hannover, Germany
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

#ifndef _VIC_DRV_STPXXP05_H_
#define _VIC_DRV_STPXXP05_H_

#include "../drv.h"
#include "../dc.h"    // include drawing context for alpha numeric driver


// defines the driver name and version
#define VIC_DRV_STPXXP05_VERSION   "STPxxP05 driver 1.02"

namespace vic {
namespace head {


/**
 * STPxxP05 head is a monochrome LED driver, using 1 bit color depth
 * \param Screen_Size_X Screen (buffer) width
 * \param Screen_Size_Y Screen (buffer) height
 * \param Viewport_Size_X Viewport (window) width
 * \param Viewport_Size_Y Viewport (window) height
 * \param Byte_Count Available channels (in byte) of the chip (1 for STP08xP05, 2 for STP16xP05, 3 for STP24DP05)
*/
template<std::uint16_t Screen_Size_X   = 8U, std::uint16_t Screen_Size_Y   = 8U,
         std::uint16_t Viewport_Size_X = 8U, std::uint16_t Viewport_Size_Y = 8U,
         drv::orientation_type Orientation = drv::orientation_0,                  // hardware orientation of the display
         std::uint_fast8_t Byte_Count = 1U
>
class STPxxP05 : public drv
{
  // vars
  const io::dev::handle_type  device_handle_;       // Logical SPI device ID
  std::uint8_t                buffer_[Byte_Count];  // display buffer, cause reading data isn't supported

public:

  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S

  /**
   * ctor
   * \param device_handle Logical SPI device ID
   * \param xoffset X offset of the screen, relative to top/left corner
   * \param yoffset Y offset of the screen, relative to top/left corner
   */
  STPxxP05(io::dev::handle_type device_handle,   // device handle
           std::uint16_t viewport_x = 0U, std::uint16_t viewport_y = 0U)
    : drv(Screen_Size_X,   Screen_Size_Y,
          Viewport_Size_Y, Viewport_Size_Y,
          viewport_x,      viewport_y,
          Orientation)
    , device_handle_(device_handle)
  {
    // clear buffer
    cls();
  }


  /**
   * dtor
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
    return (const char*)VIC_DRV_STPXXP05_VERSION;
  }


  virtual bool is_graphic() const
  {
    // STPxxP05 is a graphic display
    return true;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

  /**
   * Clear screen, set all pixels off, delete all characters or fill screen with background/blank color
   */
  virtual void cls(color::value_type)
  {
    // clear display
    for (std::uint_fast8_t i = 0; i < BYTE_COUNT; ++i) {
      buffer_[i] = 0U;
    }
  }


  /**
   * Primitive rendering is done. May be overridden by driver to update display,
   * frame buffer or something else (like copy RAM / rendering buffer to screen)
   */
  virtual void present()
  {
    // write buffer to SPI
    write();
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

    // set pixel in display buffer
    if (color::color_to_L1(color)) {
      buffer_[y] |= static_cast<std::uint8_t>(0x01U << (x & 0x07U));    // set pixel
    }
    else {
      buffer_[y] &= static_cast<std::uint8_t>(~(0x01U << (x & 0x07U))); // clear pixel
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

    return color::L1_to_color((buffer_[y] >> (x & 0x07U)) & 0x01U);
  }


  ///////////////////////////////////////////////////////////////////////////////

private:
  /**
   * Write data to SPI
   * \return True if successful
   */
  bool write()
  {
    // copy memory bitmap to screen
    switch (orientation_) {
      case orientation_0 :
        return io::dev_set(spi_device_id_, buffer_, Byte_Count, nullptr, 0U);
      // TBD: all other orientations
      default :
        break;
    }
  }

};

} // namespace head
} // namespace vic

#endif  // _VIC_DRV_STPXXP05_H_
