///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2018, PALANDesign Hannover, Germany
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
// \brief Text context, used for alpha numeric displays
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_TC_H_
#define _VIC_TC_H_

#include "dc.h"
#include "txr.h"
#include "drv.h"


namespace vic {


// alpha text context
class tc : public txr
{
  drv*        head_;            // alpha numeric head
  dc*         dc_;              // drawing context for graphic display
  std::size_t present_lock_;    // present lock counter, > 0 is locked

  /**
   * Set a pixel in the given color in the given dc
   * \param vertex Vertex to set
   * \param color Color of the pixel
   */
  inline virtual void pixel_set(vertex_type vertex, color::value_type color) final
  {
    // only supported on graphic heads
    if (!!dc_) { dc_->pixel_set(vertex, color); }
  }


public:

  /**
   * ctor - for alpha (head) driver
   * \param head
   */
  tc(drv& head)
    : head_(&head)
    , dc_(nullptr)
    , present_lock_(0U)
  { }


  /**
   * ctor - for dc context (graphic head)
   * \param head 
   */
  tc(dc& _dc)
    : head_(nullptr)
    , dc_(&_dc)
    , present_lock_(0U)
  { }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //


  /**
   * Clear screen, set all pixels off, delete all characters or fill screen with background/blank color
   * \param bg_color Backgound/erase color, defines normally the default color of the display
   */
  inline void cls(color::value_type bg_color = color::none)
  { 
    !!dc_ ? dc_->cls(bg_color) : head_->cls(bg_color);
  }


  /**
   * Primitive rendering is done. Display (frame) buffer is copied to real screen.
   * Not all devices may support this function, especially not, when there's no separate
   * screen / frame buffer
   */
  inline void present()
  {
    if (!present_lock_) {
      !!dc_ ? dc_->present() : head_->present();
    }
  }


  /**
   * Prevent calling present() after a single primitive drawing.
   * Use this to draw complex objects like controls. After all primitives are drawn call
   * present_lock(false) which releases the lock and calls present().
   * This is a nested call, every 'lock' call needs an 'unlock' call.
   * \param lock True for engaging a present() lock and incrementing the lock count
   */
  void present_lock(bool lock = true)
  {
    if (lock) {
      present_lock_++;
    }
    else {
      if (present_lock_ > 0U) {
        present_lock_--;
        if (present_lock_ == 0U) {
          // lock is released, present
          !!dc_ ? dc_->present() : head_->present();
        }
      }
    }
  }


  /**
   * Returns the screen (buffer) width
   * \return Screen width in chars
   */
  inline std::uint16_t screen_width() const
  { return head_->screen_width(); }


  /**
   * Returns the screen (buffer) height
   * \return Screen height in chars
   */
  inline std::uint16_t screen_height() const
  { return head_->screen_height(); }


  ///////////////////////////////////////////////////////////////////////////////

  /**
   * Set the new text position
   * \param pos Position in chars on text displays (0/0 is left/top)
   */
  inline void set_pos(vertex_type pos)
  {
    !!dc_ ? txr::set_pos(pos) : head_->text_set_pos(pos);
  }


  /**
   * Set inverse text mode
   * \param mode Set normal or inverse video
   */
  inline void set_inverse(bool inverse = true)
  {
    // only supported on alpha heads
    if (!!head_) { head_->text_set_inverse(inverse); }
  }


  /**
   * Clear actual line from cursor pos to end of line
   */
  inline void clear_eol()
  {
    // only supported on alpha heads
    if (!!head_) { head_->text_clear_eol(); }
  }


  /**
   * Clear actual line from start to cursor pos
   */
  inline void clear_sol()
  {
    // only supported on alpha heads
    if (!!head_) { head_->text_clear_sol(); }
  }


  /**
   * Clear the actual line
   */
  inline void clear_line()
  {
    // only supported on alpha heads
    if (!!head_) { head_->text_clear_line(); }
  }


  /**
   * Output a single ASCII/UNICODE char at the actual cursor position
   * The cursor position is moved by the char width (distance)
   * \param ch Output character in 16 bit ASCII/UNICODE (NOT UTF-8) format, 00-7F is compatible with ASCII
   */
  inline void out(std::uint16_t ch)
  {
    !!dc_ ? txr::out(ch) : head_->text_out(ch);
  }


  /**
   * Render an UTF-8 / ASCII coded string
   * \param string Output string in UTF-8/ASCII format, zero terminated
   * \return Number of written characters, not bytes (as an UTF-8 character may consist out of more bytes)
   */
  inline std::uint16_t out(const std::uint8_t* string)
  {
    const std::uint16_t cnt = !!dc_ ? txr::out(string) : head_->text_out(string);
    present();
    return cnt;
  }


  /**
   * Output a string at x/y position (combines text_set_pos and text_string)
   * \param pos Position in pixel on graphic displays, position in chars on text displays
   * \param string Output string in ASCII/UTF-8 format, zero terminated
   * \return Number of written characters, not bytes (as an UTF-8 character may consist out of more bytes)
   */
  inline std::uint16_t out(vertex_type pos, const std::uint8_t* string)
  {
    set_pos(pos);
    return out(string);
  }


  /**
   * Returns the width and height the rendered string would take.
   * The string is not rendered on screen
   * \param width Width the rendered string would take
   * \param height Height the rendered string would take
   * \param string String in UTF-8 format, zero terminated
   * \return Number of string characters, not bytes (as a character may consist out of two bytes)
   */
  std::uint16_t get_extend(std::uint16_t& width, std::uint16_t& height, const std::uint8_t* string) const
  {
    if (!!dc_) {
      // get graphic extend
      return txr::get_extend(width, height, string);
    }
    else {
      // alpha numeric head
      std::uint16_t ch, cnt = 0U;
      while (*string) {
        if ((*string & 0x80U) == 0x00U) {
          // 1 byte sequence (ASCII char)
          ch = (std::uint16_t)(*string++ & 0x7FU);
        }
        else if ((*string & 0xE0U) == 0xC0U) {
          // 2 byte UTF-8 sequence
          ch = (((std::uint16_t)*string & 0x001FU) << 6U) | ((std::uint16_t)*(string + 1U) & 0x003FU);
          string += 2U;
        }
        else if ((*string & 0xF0U) == 0xE0U) {
          // 3 byte UTF-8 sequence
          ch = (((std::uint16_t)*string & 0x000FU) << 12U) | (((std::uint16_t)*(string + 1U) & 0x003FU) << 6U) | ((std::uint16_t)*(string + 2U) & 0x003FU);
          string += 3U;
        }
        else {
          // unknown sequence
          string++;
          continue;
        }
        cnt++;
      }
      width  = cnt;
      height = 1U;
      return cnt;
    }
  }
};

} // namespace vic

#endif  // _VIC_TC_H_
