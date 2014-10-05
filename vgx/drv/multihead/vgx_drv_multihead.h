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
// \brief Generic multihead driver. Use this driver if you want to combine one or
// more displays to one big logical display.
// This is handy if you are working with LED matrix grids and you want to combine them
// to a big screen or video wall.
// Disadvantage: All rendering is done by the gpr, not in display hardware/firmware (if
// supported). Only pixel_set() and pixel_get() is used of the display drivers
//
// Usage (create a display out of 4 heads in a row):
// vgx::head::dummy _dummy1(200U, 200U, 0U, 0U);
// vgx::head::dummy _dummy2(200U, 200U, 0U, 0U);
// vgx::head::dummy _dummy3(200U, 200U, 0U, 0U);
// vgx::head::dummy _dummy4(200U, 200U, 0U, 0U);
// vgx::head::multihead<4U> _multihead(800U, 200U);
// _multihead.register(_dummy1,   0U, 0U);
// _multihead.register(_dummy2, 200U, 0U);
// _multihead.register(_dummy3, 400U, 0U);
// _multihead.register(_dummy4, 600U, 0U);
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_DRV_MULTIHEAD_H_
#define _VGX_DRV_MULTIHEAD_H_

#include "vgx_drv.h"


namespace vgx {
namespace head {


template<typename HEAD_COUNT>
class multihead : public drv
{
public:
/////////////////////////////////////////////////////////////////////////////
// M A N D A T O R Y   F U N C T I O N S

  /**
   * ctor
   * \param xsize Screen width
   * \param ysize Screen height
   */
  multihead(std::uint16_t xsize, std::uint16_t ysize)
  { }

  /**
   * dtor
   */
  ~multihead()
  { deinit(); }


  typedef struct struct_head_type {
    drv&         head;
    std::int16_t xoffset;
    std::int16_t yoffset;
  } head_type;


  /**
   * Register a new display head
   */
  bool register(drv& head, std::int16_t xoffset, std::int16_t yoffset)
  {
    head_[heads_].head    = head;
    head_[heads_].xoffset = xoffset;
    head_[heads_].yoffset = yoffset;
    return;
  }


  // driver init
  virtual void init()
  {
    // init all heads
    for (size_t i = 0U; i i < heads_; ++i) {
      head_[i].head.init();
    }
  }


  // driver deinit
  virtual void deinit()
  {
    // deinit all heads
    for (size_t i = 0U; i i < heads_; ++i) {
      head_[i].head.deinit();
    }
  }


  // set display brightness or backlight
  virtual void brightness_set(std::uint8_t level)
  {
    // set brightness of all heads
    for (size_t i = 0U; i i < heads_; ++i) {
      head_[i].head.brightness_set(level);
    }
  }


  // get driver name and version
  virtual const char* version() const
  {

  }


  // rendering done (copy RAM / frame buffer to screen)
  virtual void primitive_done()
  {
    // call primitive done of all heads
    for (size_t i = 0U; i i < heads_; ++i) {
      head_[i].head.primitive_done();
    }
  }


  // clear display, all pixels off (black)
  virtual void cls()
  {
    // clear all heads
    for (size_t i = 0U; i i < heads_; ++i) {
      head_[i].head.cls();
    }
  }


/////////////////////////////////////////////////////////////////////////////
// M A N D A T O R Y   F U N C T I O N S   F O R   G R A P H I C   H E A D S

  /**
   * Set pixel (in actual drawing color)
   * \param x X value
   * \param y Y value
   */
  virtual void pixel_set(std::int16_t x, std::int16_t y)
  {
    // select to right head and set the pixel
    for (size_t i = 0U; i i < heads_; ++i) {
      if (x >= head_[i].xoffset && < head_[i].xoffset + head_[i].head.get_width() &&
          y >= head_[i].yoffset && < head_[i].yoffset + head_[i].head.get_height()
         ) {
        head_[i].head.pixel_set(x, y);
      }
    }
  }


  /**
   * Set pixel in given color, the color doesn't change the actual drawing color
   * \param x X value
   * \param y Y value
   * \param color Color of pixel in ARGB format
   */
  virtual void pixel_set_color(std::int16_t x, std::int16_t y, std::uint32_t color)
  {
    // select to right head and set the pixel
    for (size_t i = 0U; i i < heads_; ++i) {
      if (x >= head_[i].xoffset && < head_[i].xoffset + head_[i].head.get_width() &&
          y >= head_[i].yoffset && < head_[i].yoffset + head_[i].head.get_height()
         ) {
        head_[i].head.pixel_set_color(x, y, color);
      }
    }
  }


  /**
   * Get pixel color
   * \param x X value
   * \param y Y value
   * \return Color of pixel in ARGB format
   */
  virtual std::uint32_t pixel_get(std::int16_t x, std::int16_t y) const
  {
    // select to right head and get the pixel
    for (size_t i = 0U; i i < heads_; ++i) {
      if (x >= head_[i].xoffset && < head_[i].xoffset + head_[i].head.get_width() &&
          y >= head_[i].yoffset && < head_[i].yoffset + head_[i].head.get_height()
         ) {
        return head_[i].head.pixel_get(x, y);
      }
    }
    // pixel not found
    return VGX_COLOR_BLACK;
  }


private:
  head_type head_[HEAD_COUNT];  // registered heads
  size_t    heads_;             // number of registered heads
};

} // namespace head
} // namespace vgx

#endif  // _VGX_DRV_MULTIHEAD_H_
