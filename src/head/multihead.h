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
// \brief Multihead driver, this driver combines multiple single displays (heads)
// to a big screen or video wall.
// Disadvantage: All rendering is done by the gpr, not in display hardware/firmware (if
// supported). Only pixel_set() and pixel_get() is used of the display drivers
//
// Usage (create a display out of 4 heads in a row):
// vic::head::memory<200U, 200U> _head1;
// vic::head::memory<200U, 200U> _head2;
// vic::head::memory<200U, 200U> _head3;
// vic::head::memory<200U, 200U> _head4;
// vic::head::multihead<800U, 200U, 4U> _multihead = {{ _head1, { 0, 0 } }, { _head2, { 200, 0 } }, { _head3, { 400, 0 } }, { _head4, { 600, 0 } }};
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_MULTIHEAD_H_
#define _VIC_HEAD_MULTIHEAD_H_

#include "../drv.h"
#include <initializer_list>


// defines the head name and version
#define VIC_HEAD_MULTIHEAD_VERSION   "Multihead driver 2.1.0"


namespace vic {
namespace head {


/**
 * Multihead driver class
 * \param Screen_Size_X (Virtual) screen width
 * \param Screen_Size_Y (virtual) screen height
 * \param Head_Count Number of used heads, all heads MUST be given in the ctor initializer list
 */
template<std::uint16_t Screen_Size_X, std::uint16_t Screen_Size_Y, std::size_t Head_Count>
class multihead : public drv
{
public:

  /**
   * Head definition for initializer list of ctor
   */
  typedef struct tag_head_type {
    drv&        head;       // reference to head
    vertex_type viewport;   // position of the head in the multihead screen

    tag_head_type(drv& _head, vertex_type _viewport)
      : head(_head), viewport(_viewport)
    { }
  } head_type;

private:
  typedef struct tag_multihead_head_type {
    drv*        head;
    vertex_type viewport;
  } multihead_head_type;

  multihead_head_type head_[Head_Count];  // registered heads
  std::size_t         count_;             // number of registered heads
  bool                is_graphic_;        // true if this is a graphic head, false for alpha numeric multihead

public:

  /**
   * ctor
   * \param il Initializer list of heads, like head::multihead<300U, 100U, 3U> _multihead = {{ _head0, { 0, 0 } }, { _head1, { 100, 0 } }, { _head2, { 200, 0 } }};
   *           Heads beyond Head_Count are ignored
   */
  multihead(const std::initializer_list<head_type>& il)
    : drv(Screen_Size_X, Screen_Size_Y,
          Screen_Size_X, Screen_Size_Y)
    , count_(0U)
    , is_graphic_(false)
  {
    for (typename std::initializer_list<head_type>::iterator it = il.begin(); (it != il.end()) && (count_ < Head_Count); ++it) {
      head_[count_].head     = &it->head;
      head_[count_].viewport = it->viewport;
      // the multihead is graphic if at least one head is graphic
      is_graphic_ |= it->head.is_graphic();
      ++count_;
    }
  }


  /**
   * dtor
   */
  ~multihead()
  {
    shutdown();
  }


  /**
   * Get the number of registered heads
   * \return Head count
   */
  inline std::size_t head_count() const
  {
    return count_;
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   D R I V E R   F U N C T I O N S
  //

  virtual void init()
  {
    for (std::size_t i = 0U; i < count_; ++i) {
      head_[i].head->init();
    }
  }


  virtual void shutdown()
  {
    for (std::size_t i = 0U; i < count_; ++i) {
      head_[i].head->shutdown();
    }
  }


  inline virtual const char* version() const
  {
    return (const char*)VIC_HEAD_MULTIHEAD_VERSION;
  }


  /**
   * Returns the display capability: graphic or alpha numeric
   * \return True if AT LEAST ONE head is graphic
   */
  inline virtual bool is_graphic() const
  {
    return is_graphic_;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

protected:

  inline virtual void cls(color::value_type bg_color = color::none)
  {
    for (std::size_t i = 0U; i < count_; ++i) {
      head_[i].head->cls(bg_color);
    }
  }


  inline virtual void present()
  {
    for (std::size_t i = 0U; i < count_; ++i) {
      head_[i].head->present();
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  // G R A P H I C   F U N C T I O N S

  /**
   * Set pixel in given color
   * Every head gets the vertex in its own coordinates and clips it itself
   * \param vertex Pixel coordinates
   * \param color Color of pixel in ARGB format
   */
  inline virtual void pixel_set(vertex_type vertex, color::value_type color)
  {
    for (std::size_t i = 0U; i < count_; ++i) {
      head_[i].head->pixel_set(vertex - head_[i].viewport, color);
    }
  }


  /**
   * Return the color of the pixel
   * \param vertex Coordinates of the pixel
   * \return Color of pixel in ARGB format, color::none if no head covers the vertex
   */
  inline virtual color::value_type pixel_get(vertex_type vertex)
  {
    for (std::size_t i = 0U; i < count_; ++i) {
      const vertex_type p = vertex - head_[i].viewport;
      if (head_[i].head->screen_is_inside(p)) {
        return head_[i].head->pixel_get(p);
      }
    }
    // pixel not found
    return color::none;
  }


  /////////////////////////////////////////////////////////////////////////////
  // A L P H A   T E X T   F U N C T I O N S

  inline virtual void text_set_pos(vertex_type pos)
  {
    for (std::size_t i = 0U; i < count_; ++i) {
      head_[i].head->text_set_pos(pos - head_[i].viewport);
    }
  }


  inline virtual void text_set_inverse(bool inverse)
  {
    for (std::size_t i = 0U; i < count_; ++i) {
      head_[i].head->text_set_inverse(inverse);
    }
  }


  inline virtual void text_clear_eol()
  {
    for (std::size_t i = 0U; i < count_; ++i) {
      head_[i].head->text_clear_eol();
    }
  }


  inline virtual void text_clear_sol()
  {
    for (std::size_t i = 0U; i < count_; ++i) {
      head_[i].head->text_clear_sol();
    }
  }


  inline virtual void text_clear_line()
  {
    for (std::size_t i = 0U; i < count_; ++i) {
      head_[i].head->text_clear_line();
    }
  }


  inline virtual void text_out(std::uint16_t ch)
  {
    for (std::size_t i = 0U; i < count_; ++i) {
      head_[i].head->text_out(ch);
    }
  }


  /**
   * Render an ASCII/UTF-8 coded string, each head renders its own part
   * \param string Output string in ASCII/UTF-8 format, zero terminated
   * \return Number of written characters, not bytes (as an UTF-8 character may consist out of more bytes)
   */
  inline virtual std::uint16_t text_out(const std::uint8_t* string)
  {
    std::uint16_t cnt = 0U;
    for (std::size_t i = 0U; i < count_; ++i) {
      cnt = head_[i].head->text_out(string);
    }
    return cnt;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // D I S P L A Y   C O N T R O L
  //

public:

  inline virtual void display_enable(bool enable = true)
  {
    for (std::size_t i = 0U; i < count_; ++i) {
      head_[i].head->display_enable(enable);
    }
  }


  inline virtual void display_brightness(std::uint8_t level)
  {
    for (std::size_t i = 0U; i < count_; ++i) {
      head_[i].head->display_brightness(level);
    }
  }


  inline virtual void display_contrast(std::uint8_t level)
  {
    for (std::size_t i = 0U; i < count_; ++i) {
      head_[i].head->display_contrast(level);
    }
  }
};

} // namespace head
} // namespace vic

#endif  // _VIC_HEAD_MULTIHEAD_H_
