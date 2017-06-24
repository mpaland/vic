///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2017, PALANDesign Hannover, Germany
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
// \brief seven-segment driver
// This is the driver for a seven-segment device with a decimal point
// It's a alpha numeric head which can display 0-9 and A-F
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_DRV_SEVEN_SEGMENT_H_
#define _VGX_DRV_SEVEN_SEGMENT_H_

#include "../drv.h"


// defines the driver name and version
#define VGX_DRV_SEVEN_SEGMENT_VERSION   "Seven segment driver 1.00"


namespace vgx {
namespace head {


/**
 * 7-segment driver, using 1 bit color depth
 */
template<std::uint16_t COLUMNS>
class seven_segment : public drv
{
public:

  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S

  typedef struct tag_segment_type {
    std::uint16_t a;
    std::uint16_t b;
    std::uint16_t c;
    std::uint16_t d;
    std::uint16_t e;
    std::uint16_t f;
    std::uint16_t g;
    std::uint16_t dp;
  } segment_type;


  /**
   * ctor
   * \param xsize Screen width
   * \param ysize Screen height
   * \param xoffset X offset of the screen, relative to top/left corner
   * \param yoffset Y offset of the screen, relative to top/left corner
   * \param orientation Screen orientation
   * \param spi_device_id Logical SPI bus device ID
   */
  seven_segment(const segment_type& seg_id, std::uint16_t* device_select)
    : drv(COLUMNS, 1,
          0, 0)
    , seg_id_(seg_id)
  { }


  /**
   * dtor
   * Shutdown the driver
   */
  ~seven_segment()
  { drv_shutdown(); }


protected:

  virtual void drv_init()
  { }


  virtual void drv_shutdown()
  { }


  virtual inline const char* drv_version() const
  {
    // return the driver version, like
    return (const char*)VGX_DRV_SEVEN_SEGMENT_VERSION;
  }


  virtual inline bool drv_is_graphic() const
  {
    // alpha numeric display
    return false;
  }


  virtual void drv_cls()
  {
    // clear the entire screen / buffer
  }

 
  /**
   * Set pixel in given color, the color doesn't change the actual drawing color
   * \param x X value
   * \param y Y value
   * \param color Color of pixel in ARGB format
   */
  virtual inline void drv_pixel_set_color(vertex_type point, color::value_type color)
  { }


  /**
   * Get pixel color
   * \param x X value
   * \param y Y value
   * \return Color of pixel in ARGB format
   */
  virtual inline color::value_type drv_pixel_get(vertex_type point) const
  { return vgx::color::black; }


  /**
   * Rendering is done (copy RAM / frame buffer to screen)
   */
  virtual void drv_present()
  { }

 
  /**
   * Output a single ASCII/UNICODE char at the actual cursor position
   * \param ch Output character in 16 bit ASCII/UNICODE (NOT UTF-8) format, 00-7F is compatible with ASCII
   * \return 1 if the char is rendered, 0 if error/not rendered
   */
  virtual std::uint16_t text_char(std::uint16_t ch)
  {
    if (ch < 0x20U) {
      // ignore non characters
      return 0U;
    }

    // check limits
    if (screen_is_inside({ text_x_act_, text_y_act_ })) {

      // check for valid chars
      if (((char)ch >= '0') && ((char)ch <= '9') ||
        ((char)ch >= 'A') && ((char)ch <= 'F') ||
        ((char)ch == '.')) {
        if ((char)ch == '.') {
          point_[text_x_act_ ? text_x_act_ - 1U : 0U] = 1U;
        }
        else {
          digit_[text_x_act_] = static_cast<std::uint8_t>(ch);
          point_[text_x_act_++] = 0U;
        }
      }
    }

    return 1U;
  }


  void digit_out() {
    // digits 0 - 9, A - F, encoding is 0 a b c d e f g
    const std::uint8_t seg[16] = { 0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x7B,
                                   0x77, 0x1F, 0x4E, 0x3D, 0x4F, 0x47 };
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
    return out::spi(spi_device_id_, data_out, 2U, nullptr, 0U);
  }


private:

  std::uint8_t  digit_[COLUMNS];    // digit buffer
  std::uint8_t  point_[COLUMNS];    // point buffer
};

} // namespace head
} // namespace vgx

#endif  // _VGX_DRV_SEVEN_SEGMENT_H_
