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
// \brief seven-segment driver
// This is the driver for a seven-segment device with a decimal point
// It's a character head which can display 0-9 and A-F
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_DRV_SEVEN_SEGMENT_H_
#define _VGX_DRV_SEVEN_SEGMENT_H_

#include "vgx_drv.h"


namespace vgx {
namespace head {


/**
 * 7-segment driver, using 1 bit color depth
 */
template<std::uint_fast8_t XSIZE>
class seven_segment : public drv
{
public:
/////////////////////////////////////////////////////////////////////////////
// M A N D A T O R Y   F U N C T I O N S

typedef struct stuct_segment_type {
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
  seven_segment(const segment_type& seg_id, std::uint16_t* device_select
               )
    : drv(XSIZE, 1, 0, 0)
    , seg_id_(seg_id)
  { }

  /**
   * dtor
   * Deinit the driver
   */
  ~seven_segment()
  { deinit(); }

  // mandatory driver functions
  virtual void init();                                    // driver init
  virtual void deinit();                                  // driver deinit
  virtual void brightness_set(std::uint8_t level);        // set display brightness/backlight
  virtual const char* version() const;                    // get driver name and version
  virtual void primitive_done();                          // rendering done (copy RAM / frame buffer to screen)
  virtual void cls();                                     // clear display, all pixels off (black)

  /**
   * Output one character
   * \param ch Output character in ASCII format
   */
  virtual void drv_text_char(std::uint16_t ch)
  {
    // handling of special chars
    if (((char)ch == '\n') || ((char)ch == '\r')) {
      // X = 0
      text_x_act_ = 0;
      return;
    }
    // check for valid chars
    if (((char)ch >= '0') && ((char)ch <= '9') ||
        ((char)ch >= 'A') && ((char)ch <= 'F') ||
        ((char)ch == '.')) {
      if ((char)ch == '.') {
        point_[text_x_act_ ? text_x_act_ - 1U : 0U] = 1U;
      }
      else {
        digit_[text_x_act_]   = static_cast<std::uint8_t>(ch);
        point_[text_x_act_++] = 0U;
      }
    }

  }



  void digit_out() {
    // digits 0 - 9, A - F, encoding is 0 a b c d e f g
    const std::uint8_t seg[16] = { 0×7E, 0×30, 0×6D, 0×79, 0×33, 0×5B, 0×5F, 0×70, 0×7F, 0×7B,
                                   0×77, 0×1F, 0×4E, 0×3D, 0×4F, 0×47 };

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

  std::uint8_t      digit_[XSIZE];    // digit buffer
  std::uint8_t      point_[XSIZE];    // point buffer
};

} // namespace head
} // namespace vgx

#endif  // _VGX_DRV_SEVEN_SEGMENT_H_
