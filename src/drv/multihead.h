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
// \brief Generic multihead driver. Use this driver if you want to combine one or
// more displays to one big logical display.
// This is handy if you are working with LED matrix grids and you want to combine them
// to a big screen or video wall.
// Disadvantage: All rendering is done by the gpr, not in display hardware/firmware (if
// supported). Only pixel_set() and pixel_get() is used of the display drivers
//
// Usage (create a display out of 4 heads in a row):
// vic::head::dummy<200U, 200U> _head1;
// vic::head::dummy<200U, 200U> _head2;
// vic::head::dummy<200U, 200U> _head3;
// vic::head::dummy<200U, 200U> _head4;
// vic::head::multihead<800U, 200U, 4U> _multihead = {{ _head1, 0U, 0U }, { _head2, 200U, 0U }, { _head3, 400U, 0U }, { _head4, 600U, 0U }};
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_DRV_MULTIHEAD_H_
#define _VIC_DRV_MULTIHEAD_H_

#include "../drv.h"
#include <initializer_list>


// defines the driver name and version
#define VIC_DRV_MULTIHEAD_VERSION   "Multihead driver 2.0.0"


namespace vic {
namespace head {


/**
 * Multihead driver class
 * \param Screen_Size_X (Virtual) screen width
 * \param Screen_Size_Y (virtual) screen height
 * \param HEAD_COUNT Number of used heads, all heads MUST be initialized in ctor!
 */
template<std::uint16_t Screen_Size_X, std::uint16_t Screen_Size_Y,
         std::size_t HEAD_COUNT>
  class multihead : public drv
{
  typedef struct tag_multihead_head_type {
    multihead*    head;
    vertex_type   viewport;
  } multihead_head_type;

  multihead_head_type head_[HEAD_COUNT];  // registered heads
  bool                is_graphic_;        // true if this is a graphic head, false for alpha numeric multihead

public:

  /**
   * Head definition for initializer list of ctor
   */
  typedef struct tag_head_type {
    drv&        head;       // reference to head
    vertex_type viewport;   // viewport in multihead screen

    tag_head_type(drv& _head, vertex_type _viewport)
      : head(_head), viewport(_viewport)
    { }
  } head_type;


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S

  /**
   * ctor
   * \param il Initializer list of heads, like head::multihead<3U> _multihead = {{ _head0, 0U, 0U }, { _head1, 100U, 0U }, { _head2, 200U, 0U }};
   */
  multihead(const std::initializer_list<head_type>& il)
    : drv(Screen_Size_X, Screen_Size_Y,
          Screen_Size_X, Screen_Size_Y)
  {
    std::size_t n = 0U;
    for (typename std::initializer_list<head_type>::iterator it = il.begin(); it != il.end(); ++it) {
      head_[n].head       = reinterpret_cast<multihead*>(&it->head);
      head_[n++].viewport = it->viewport;
    }

    // eval graphic/text info of all heads
    is_graphic_ = false;
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      is_graphic_ |= head_[i].head->is_graphic();
    }
  }


  /**
   * dtor
   */
  ~multihead()
  {
    shutdown();
  }


protected:

  // driver init
  virtual void init()
  {
    // init all heads
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      head_[i].head->init();
    }
  }


  // driver shutdown
  virtual void shutdown()
  {
    // shutdown all heads
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      head_[i].head->shutdown();
    }
  }


  // get driver name and version
  inline virtual const char* version() const
  {
    return (const char*)VIC_DRV_MULTIHEAD_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    return is_graphic_;
  }


  // clear display, all pixels off (black)
  inline virtual void cls()
  {
    // clear all heads
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      head_[i].head->cls();
    }
  }


  // rendering done (copy RAM / frame buffer to screen)
  inline virtual void present()
  {
    // call present of all heads
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      head_[i].head->present();
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S   F O R   G R A P H I C   H E A D S

  /**
   * Set pixel in given color, the color doesn't change the actual drawing color
   * \param point Pixel coordinates
   * \param color Color of pixel in ARGB format
   */
  inline virtual void pixel_set(vertex_type point, color::value_type color)
  {
    // select to right head and set the pixel
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      head_[i].head->pixel_set({ static_cast<std::int16_t>(point.x - head_[i].viewport.x), static_cast<std::int16_t>(point.y - head_[i].viewport.y) }, color);
    }
  }


  /**
   * Return the color of the pixel
   * \param point Coordinates of the pixel
   * \return Color of pixel in ARGB format
   */
  inline virtual color::value_type pixel_get(vertex_type point)
  {
    // select to right head and read the pixel
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      const vertex_type p = { static_cast<std::int16_t>(point.x - head_[i].viewport.x), static_cast<std::int16_t>(point.y - head_[i].viewport.y) };
      if (head_[i].head->screen_is_inside(p)) {
        return head_[i].head->pixel_get(p);
      }
    }
    // pixel not found
    return color::none;
  }


  /////////////////////////////////////////////////////////////////////////////
  // F U N C T I O N S   F O R   A L P H A   H E A D S

  /**
   * Set the new text position
   * \param pos Position in pixel on graphic displays, position in chars on text displays
   */
  inline virtual void text_pos(std::int16_t x, std::int16_t y)
  {
    // set the individual position on every head
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      head_[i].head->text_pos(static_cast<std::int16_t>(x - head_[i].viewport.x), static_cast<std::int16_t>(y - head_[i].viewport.y));
    }
  }


 /**
  * Set inverse text mode
  * \param mode Set normal or inverse video
  */
  inline virtual void text_set_inverse(bool inverse)
  {
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      head_[i].head->text_set_inverse(inverse);
    }
  }


  /**
   * Output one character
   * \param ch Output character in 16 bit UNICODE (NOT UTF-8) format
   * \return The x size (distance) of the rendered char
   */
  inline virtual void text_char(std::uint16_t ch)
  {
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      head_[i].head->text_char(ch);
    }
  }


  /**
   * Render an ASCII/UTF-8 coded string
   * \param string Output string in ASCII/UTF-8 format, zero terminated
   * \return Number of written characters, not bytes (as an UTF-8 character may consist out of more bytes)
   */
  virtual std::uint16_t text_out(const std::uint8_t* string)
  {
    // each graphic head renders its own string
    std::uint16_t cnt;
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      cnt = head_[i].head->text_out(string);
    }
    return cnt;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // D I S P L A Y   C O N T R O L
  //

  /**
   * Enable / disable the display
   * \param enable True to switch the display on, false to switch it off
   */
  virtual void display_enable(bool enable = true)
  {
    // enable/disable all heads
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      head_[i].head->display_enable(enable);
    }
  }


  /**
   * Set display or backlight brightness
   * \param level 0: dark, backlight off; 255: maximum brightness, backlight full on
   */
  virtual void display_brightness(std::uint8_t level)
  {
    // set brightness of all heads
    for (std::size_t i = 0U; i < HEAD_COUNT; ++i) {
      head_[i].head->display_brightness(level);
    }
  }
};

} // namespace head
} // namespace vic

#endif  // _VIC_DRV_MULTIHEAD_H_
