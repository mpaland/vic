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
// \brief STPxxP05 head, LED array driver (STP08xP05, STP16xP05, STP24DP05)
// The LEDs are organized as a bit array, the pixel (x, y) is LED number
// y * Screen_Size_X + x. For the typical usage an 8 x 1 LED bar is:
// xsize = 8, ysize = 1
//
// Create a SPI device with the following characteristics to drive the SPxxP05:
// mode = 3                 SCK idle high, data captured on rising edge, data propagated on falling edge
// lsb  = false             MSB is transmitted first
// tx_idle_value = 0xFF     output data value during data read (normally 0xFF)
// clock_speed = 10000000   clock speed of the bus in [Hz]
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_STPXXP05_H_
#define _VIC_HEAD_STPXXP05_H_

#include "../drv.h"


// defines the head name and version
#define VIC_HEAD_STPXXP05_VERSION   "STPxxP05 head 1.10"

namespace vic {
namespace head {


/**
 * STPxxP05 head is a monochrome LED driver, using 1 bit color depth
 * \param Screen_Size_X Screen (buffer) width
 * \param Screen_Size_Y Screen (buffer) height
 * \param Byte_Count Available channels (in byte) of the chip (1 for STP08xP05, 2 for STP16xP05, 3 for STP24DP05)
 */
template<std::uint16_t Screen_Size_X = 8U, std::uint16_t Screen_Size_Y = 1U,
         std::uint_fast8_t Byte_Count = 1U
>
class STPxxP05 : public drv
{
  // vars
  const io::handle_type device_handle_;       // logical SPI device handle
  std::uint8_t          buffer_[Byte_Count];  // display buffer, cause reading data isn't supported

public:

  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S

  /**
   * ctor
   * \param device_handle Logical SPI device handle
   */
  STPxxP05(io::handle_type device_handle)
    : drv(Screen_Size_X, Screen_Size_Y,
          Screen_Size_X, Screen_Size_Y)
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
  {
    io::init(device_handle_);
    cls();
    present();
  }


  // driver shutdown
  virtual void shutdown()
  {
    cls();
    present();
  }


  // get driver name and version
  virtual const char* version() const
  {
    return (const char*)VIC_HEAD_STPXXP05_VERSION;
  }


  virtual bool is_graphic() const
  {
    // STPxxP05 is a graphic display
    return true;
  }


  /**
   * Access the raw LED buffer
   * \return Pointer to Byte_Count bytes
   */
  inline const std::uint8_t* buffer() const
  {
    return buffer_;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

protected:

  /**
   * Clear screen, set all LEDs off
   * \param bg_color Unused, LEDs are off
   */
  virtual void cls(color::value_type bg_color = color::none)
  {
    (void)bg_color;
    for (std::uint_fast8_t i = 0U; i < Byte_Count; ++i) {
      buffer_[i] = 0U;
    }
  }


  /**
   * Primitive rendering is done, write the buffer to the chip
   */
  virtual void present()
  {
    io::write(device_handle_, 0U, buffer_, Byte_Count, nullptr, 0U);
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
    const std::uint32_t led = static_cast<std::uint32_t>(vertex.y) * Screen_Size_X + static_cast<std::uint32_t>(vertex.x);
    if (led >= Byte_Count * 8U) {
      // no channel for this pixel
      return;
    }

    if (color::color_to_L1(color)) {
      buffer_[led >> 3U] |= static_cast<std::uint8_t>(0x01U << (led & 0x07U));    // LED on
    }
    else {
      buffer_[led >> 3U] &= static_cast<std::uint8_t>(~(0x01U << (led & 0x07U))); // LED off
    }
  }


  virtual color::value_type pixel_get(vertex_type vertex)
  {
    // check limits
    if (!screen_is_inside(vertex)) {
      // out of bounds
      return color::none;
    }
    const std::uint32_t led = static_cast<std::uint32_t>(vertex.y) * Screen_Size_X + static_cast<std::uint32_t>(vertex.x);
    if (led >= Byte_Count * 8U) {
      return color::none;
    }
    return color::L1_to_color(static_cast<std::uint8_t>((buffer_[led >> 3U] >> (led & 0x07U)) & 0x01U));
  }
};

} // namespace head
} // namespace vic

#endif  // _VIC_HEAD_STPXXP05_H_
