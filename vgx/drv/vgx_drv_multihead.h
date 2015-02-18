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
// \brief Generic multihead driver. Use this driver if you want to combine one or
// more displays to one big logical display.
// This is handy if you are working with LED matrix grids and you want to combine them
// to a big screen or video wall.
// Disadvantage: All rendering is done by the gpr, not in display hardware/firmware (if
// supported). Only pixel_set() and pixel_get() is used of the display drivers
//
// Usage (create a display out of 4 heads in a row):
// vgx::head::dummy<200U, 200U> _dummy1;
// vgx::head::dummy<200U, 200U> _dummy2;
// vgx::head::dummy<200U, 200U> _dummy3;
// vgx::head::dummy<200U, 200U> _dummy4;
// vgx::head::multihead<4U> _multihead = {{ _dummy1, 0U, 0U }, { _dummy2, 200U, 0U }, { _dummy3, 400U, 0U }, { _dummy4, 600U, 0U }};
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_DRV_MULTIHEAD_H_
#define _VGX_DRV_MULTIHEAD_H_

#include "vgx_drv.h"
#include <initializer_list>


// defines the driver name and version
#define VGX_DRV_MULTIHEAD_VERSION   "Multihead driver 1.10"


namespace vgx {
namespace head {

/**
 * Multihead class
 * \param HEAD_COUNT Number of used heads, all heads MUST be initialized in ctor!
 */
template<std::size_t HEAD_COUNT>
class multihead : public drv
{
public:
/////////////////////////////////////////////////////////////////////////////
// M A N D A T O R Y   F U N C T I O N S

  /**
   * Head definition for initializer list of ctor
   */
  typedef struct struct_head_type {
    drv&          head;         // reference to head
    std::uint16_t viewport_x;   // viewport x offset
    std::uint16_t viewport_y;   // viewport y offset

    struct_head_type(drv& _head, std::uint16_t _viewport_x, std::uint16_t _viewport_y)
      : head(_head), viewport_x(_viewport_x), viewport_y(_viewport_y) { }
    struct_head_type& operator=( const struct_head_type& ) { return *this; }
  } head_type;


  /**
   * ctor
   * \param il Initializer list of heads, like head::multihead<3U> _multihead = {{ _head0, 0U, 0U }, { _head1, 100U, 0U }, { _head2, 200U, 0U }};
   */
  multihead(const std::initializer_list<head_type>& il)
    : drv(0U, 0U, 0U, 0U)
  {
    std::size_t n = 0U;
    for (typename std::initializer_list<head_type>::iterator it = il.begin(); it != il.end(); ++it) {
      head_[n  ].head = reinterpret_cast<multihead*>(&it->head);
      head_[n  ].viewport_x = it->viewport_x;
      head_[n++].viewport_y = it->viewport_y;
    }
  }


  /**
   * dtor
   */
  ~multihead()
  { deinit(); }


  // driver init
  virtual void init()
  {
    // init all heads
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      head_[i].head->init();
    }
  }


  // driver deinit
  virtual void deinit()
  {
    // deinit all heads
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      head_[i].head->deinit();
    }
  }


  // get driver name and version
  virtual const char* version() const
  {
    return (const char*)VGX_DRV_MULTIHEAD_VERSION;
  }


  virtual bool is_graphic() const
  {
    // get info of all heads
    bool _is_graphic = false;
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      _is_graphic |= head_[i].head->is_graphic();
    }
    return _is_graphic;
  }


  // clear display, all pixels off (black)
  virtual void cls()
  {
    // clear all heads
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      head_[i].head->cls();
    }
  }


  // set display brightness or backlight
  virtual void brightness_set(std::uint8_t level)
  {
    // set brightness of all heads
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      head_[i].head->brightness_set(level);
    }
  }


  // rendering done (copy RAM / frame buffer to screen)
  virtual void drv_primitive_done()
  {
    // call primitive done of all heads
    if (!primitive_lock_) {
      for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
        head_[i].head->drv_primitive_done();
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O L O R   F U N C T I O N S

  /**
   * Set the drawing color
   * \param color New drawing color in ARGB format
   */
  virtual inline void color_set(color::value_type color)
  {
    color_ = color;
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      head_[i].head->color_set(color);
    }
  }

  /**
   * Set the background color (e.g. for cls)
   * \param color_background New background color in ARGB format
   */
  virtual inline void color_set_bg(color::value_type color_background)
  {
    color_bg_ = color_background;
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      head_[i].head->color_set_bg(color_background);
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S   F O R   G R A P H I C   H E A D S

  /**
   * Set pixel (in actual drawing color)
   * \param x X value
   * \param y Y value
   */
  virtual void drv_pixel_set(std::int16_t x, std::int16_t y)
  {
    if (clipping_ && !clipping_->is_clipping(x, y)) {
      // outside clipping region
      return;
    }

    // select to right head and set the pixel
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      if (x >= head_[i].viewport_x && x < head_[i].viewport_x + head_[i].head->viewport_width() &&
          y >= head_[i].viewport_y && y < head_[i].viewport_y + head_[i].head->viewport_height()
         ) {
        head_[i].head->drv_pixel_set(x - head_[i].viewport_x, y - head_[i].viewport_y);
      }
    }
  }


  /**
   * Set pixel in given color, the color doesn't change the actual drawing color
   * \param x X value
   * \param y Y value
   * \param color Color of pixel in ARGB format
   */
  virtual void drv_pixel_set_color(std::int16_t x, std::int16_t y, color::value_type color)
  {
    if (clipping_ && !clipping_->is_clipping(x, y)) {
      // outside clipping region
      return;
    }

    // select to right head and set the pixel
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      if (x >= head_[i].viewport_x && x < head_[i].viewport_x + head_[i].head->viewport_width() &&
          y >= head_[i].viewport_y && y < head_[i].viewport_y + head_[i].head->viewport_height()) {
        head_[i].head->drv_pixel_set_color(x - head_[i].viewport_x, y - head_[i].viewport_y, color);
      }
    }
  }


  /**
   * Get pixel color
   * \param x X value
   * \param y Y value
   * \return Color of pixel in ARGB format
   */
  virtual color::value_type drv_pixel_get(std::int16_t x, std::int16_t y) const
  {
    // select to right head and get the pixel
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      if (x >= head_[i].viewport_x && x < head_[i].viewport_x + head_[i].head->viewport_width() &&
          y >= head_[i].viewport_y && y < head_[i].viewport_y + head_[i].head->viewport_height()) {
        return head_[i].head->drv_pixel_get(x - head_[i].viewport_x, y - head_[i].viewport_y);
      }
    }
    // pixel not found
    return color::black;
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S   F O R   A L P H A   H E A D S

  /**
   * Select the font
   * \param Reference to font to use
   * \return true if font set successfully
   */
  virtual bool drv_text_set_font(const font_type& font)
  {
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      head_[i].head->drv_text_set_font(font);
    }
    return true;
  }


  /**
   * Set the new text position
   * \param x X value in pixel on graphic displays, char pos on alpha displays
   * \param y Y value in pixel on graphic displays, char pos on alpha displays
   * \return true if position is set successfully
   */
  virtual bool drv_text_set_pos(std::int16_t x, std::int16_t y)
  {
    if (is_graphic()) {
      // let the base class render the text on the graphics heads
      gpr::drv_text_set_pos(x, y);
    }
    else {
      for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
        head_[i].head->drv_text_set_pos(x - head_[i].viewport_x, y - head_[i].viewport_y);
      }
    }
    return true;
  }


 /**
  * Set the text mode
  * \param mode Set normal or inverse video
  * \return true if successful
  */
  //virtual bool drv_text_set_mode(text_mode_type mode);


  /**
  * Output one character
  * \param ch Output character in ASCII/UNICODE (NOT UTF-8) format
  */
  virtual void drv_text_char(std::uint16_t ch)
  {
    if (is_graphic()) {
      // let the base class render the text on the graphics heads
      gpr::drv_text_char(ch);
    }
    else {
      // select to right head
      for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
        if (text_x_act_ >= head_[i].viewport_x && text_x_act_ < head_[i].viewport_x + head_[i].head->viewport_width() &&
            text_y_act_ >= head_[i].viewport_y && text_y_act_ < head_[i].viewport_y + head_[i].head->viewport_height()) {
          return head_[i].head->drv_text_char(ch);
        }
      }
    }
  }


private:
  typedef struct struct_heads_type {
    multihead*    head;
    std::uint16_t viewport_x;
    std::uint16_t viewport_y;
  } heads_type;

  heads_type head_[HEAD_COUNT];  // registered heads
};

} // namespace head
} // namespace vgx

#endif  // _VGX_DRV_MULTIHEAD_H_
