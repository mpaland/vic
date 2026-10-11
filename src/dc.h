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
// \brief Drawing context
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_DC_H_
#define _VIC_DC_H_

#include "gpr.h"
#include "drv.h"


namespace vic {


// drawing context
class dc : public gpr
{
  drv&            head_;              // display driver
  shader::output  shader_output_;     // output shader (is always the end of the shader pipeline)
  std::size_t     present_lock_;      // present lock counter, > 0 is locked

protected:

  shader::base*   shader_pipe_;       // shader pipeline


public:

  /**
   * ctor
   * \param head
   */
  dc(drv& head)
    : head_(head)
    , shader_output_(head)
    , present_lock_(0U)
  {
    // init the shader pipeline
    shader_init();
  }


  ///////////////////////////////////////////////////////////////////////////////
  // S H A D E R   P I P E L I N E   F U N C T I O N S
  //

  // access shader pipeline
  inline shader::base* shader_pipe() const
  {
    return shader_pipe_;
  }


  /**
   * Returns true if the shader pipe contains one or more shaders
   */
  inline bool shader_is_active() const
  {
    return shader_pipe_ != &shader_output_;
  }


  /**
   * Register/add a shader in the shader pipe
   * The new shader is inserted at the FRONT of the pipe, so that it's rendered as FIRST of the pipe
   * \param _shader The new shader to insert
   */
  void shader_register(shader::base* _shader)
  {
    _shader->next_ = shader_pipe_;
    shader_pipe_ = _shader;
  }


  /**
   * Remove a shader out of the shader pipe
   * \param _shader The shader to remove out of the pipe
   * \return True if the shader is removed, false if error (shader not found)
   */
  bool shader_remove(const shader::base* _shader)
  {
    if (_shader == shader_pipe_) {
      // remove the first shader in the pipe
      shader_pipe_ = shader_pipe_->next_;
      return true;
    }
    else {
      for (shader::base* s = shader_pipe_; !!s; s = s->next_) {
        if (_shader == s->next_) {
          s->next_ = _shader->next_;
          return true;
        }
      }
    }
    // error, shader not found
    return false;
  }


  /**
   * Init / clear the shader pipe
   */
  inline void shader_init()
  {
    // set shader pipeline to output shader
    shader_pipe_ = &shader_output_;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

  /**
   * Clear screen, set all pixels off, delete all characters or fill screen with background/blank color
   * \param bg_color Backgound/erase color, defines normally the default color of the display
   */
  inline void cls(color::value_type bg_color = color::none)
  {
    head_.cls(bg_color);
  }


  /**
   * Set the background color of the context for heads which can't read their pixels back
   * Such heads return color::none from pixel_get(). With a background color set, alpha blending and
   * anti aliasing blend against this color instead (exact on uniform backgrounds, needs no RAM).
   * Sprites use it to restore their background. Default is color::none (no fallback).
   * \param bg_color Background color, color::none disables the fallback
   */
  inline void set_bg_color(color::value_type bg_color)
  {
    shader_output_.set_bg_color(bg_color);
  }


  /**
   * Get the background color of the context
   * \return Background color
   */
  inline color::value_type get_bg_color() const
  {
    return shader_output_.get_bg_color();
  }


  /**
   * Primitive rendering is done. Display (frame) buffer is copied to real screen.
   * Not all devices may support this function, especially not, when there's no separate
   * screen / frame buffer
   */
  inline virtual void present()
  {
    if (!present_lock_) {
      head_.present();
    }
  }


  /**
   * Prevent calling present() after a single primitive drawing.
   * Use this to draw complex objects like controls. After all primitives are drawn call
   * present_lock(false) which releases the lock and calls present().
   * This is a nested call, every 'lock' call needs an 'unlock' call.
   * \param lock True for engaging a present() lock and incrementing the lock count
   */
  virtual void present_lock(bool lock = true) final
  {
    if (lock) {
      present_lock_++;
    }
    else {
      if (present_lock_ > 0U) {
        present_lock_--;
        if (present_lock_ == 0U) {
          // lock is released, present it
          present();
        }
      }
    }
  }


  /**
   * Returns the screen (buffer) width
   * \return Screen width in pixel
   */
  inline virtual std::uint16_t screen_width() const final
  {
    return head_.screen_width();
  }


  /**
   * Returns the screen (buffer) height
   * \return Screen height in pixel
   */
  inline virtual std::uint16_t screen_height() const final
  {
    return head_.screen_height();
  }


  /**
   * Enable/disable alpha blending support
   * \param enable True to enable alpha blending
   */
  inline void alpha_blending_enable(bool enable = true)
  {
    shader_output_.alpha_blending_enable(enable);
  }


  ///////////////////////////////////////////////////////////////////////////////

  // make the gpr overloads (box with border radius) visible, they would be hidden by the overrides below
  using gpr::box;


  /**
   * Draw a horizontal line, width is one pixel
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, y component is ignored
   */
  virtual void line_horz(vertex_type v0, vertex_type v1) override
  {
    if (!shader_is_active() && (color::is_opaque(get_color()))) {
      // no active shaders, opaque color: let the driver draw the line, clipped to the screen
      util::min_max_swap(v0.x, v1.x);
      if ((v0.y >= 0) && (v0.y < static_cast<std::int16_t>(screen_height())) && (v1.x >= 0) && (v0.x < static_cast<std::int16_t>(screen_width()))) {
        if (v0.x < 0) {
          v0.x = 0;
        }
        if (v1.x >= static_cast<std::int16_t>(screen_width())) {
          v1.x = static_cast<std::int16_t>(screen_width() - 1U);
        }
        head_.line_horz(v0, v1, get_color());
      }
      present();
    }
    else {
      // let the gpr do the work
      gpr::line_horz(v0, v1);
    }
  }


  /**
   * Draw a vertical line, width is one pixel
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, x component is ignored
   */
  virtual void line_vert(vertex_type v0, vertex_type v1) override
  {
    if (!shader_is_active() && (color::is_opaque(get_color()))) {
      // no active shaders, opaque color: let the driver draw the line, clipped to the screen
      util::min_max_swap(v0.y, v1.y);
      if ((v0.x >= 0) && (v0.x < static_cast<std::int16_t>(screen_width())) && (v1.y >= 0) && (v0.y < static_cast<std::int16_t>(screen_height()))) {
        if (v0.y < 0) {
          v0.y = 0;
        }
        if (v1.y >= static_cast<std::int16_t>(screen_height())) {
          v1.y = static_cast<std::int16_t>(screen_height() - 1U);
        }
        head_.line_vert(v0, v1, get_color());
      }
      present();
    }
    else {
      // let the gpr do the work
      gpr::line_vert(v0, v1);
    }
  }


  /**
   * Draw a box (filled rectangle)
   * \param rect Box bounding
   */
  virtual void box(const rect_type& rect) override
  {
    if ((!shader_is_active()) && (color::is_opaque(get_color()))) {
      // no active shaders, opaque color: let the driver draw the box, clipped to the screen
      rect_type r = rect;
      if (r.left < 0) {
        r.left = 0;
      }
      if (r.top < 0) {
        r.top = 0;
      }
      if (r.right > static_cast<std::int16_t>(screen_width())) {
        r.right = static_cast<std::int16_t>(screen_width());
      }
      if (r.bottom > static_cast<std::int16_t>(screen_height())) {
        r.bottom = static_cast<std::int16_t>(screen_height());
      }
      if ((r.left < r.right) && (r.top < r.bottom)) {
        head_.box(r, get_color());
      }
      present();
    }
    else {
      // let the gpr do the work
      gpr::box(rect);
    }
  }


  /**
   * Draw a box (filled rectangle), vertex to rect wrapper
   * \param v0 First corner vertex
   * \param v1 Second corner vertex
   */
  inline void box(vertex_type v0, vertex_type v1)
  {
    rect_type r;
    r.normalize(v0, v1);
    box(r);
  }


  /**
   * Write a block of pixels (bit block transfer), e.g. a bitmap
   * Without an active shader and without translucent pixels (or alpha blending disabled) the block is
   * handed to the head in one piece, otherwise every pixel takes the shader pipe
   * \param rect Destination rect, right and bottom are outside the rect
   * \param pixels Pixel block in ARGB format, row major, first pixel is the left/top pixel of rect
   * \param stride Distance between two rows in pixels, normally rect.width()
   */
  void blit(const rect_type& rect, const color::value_type* pixels, std::size_t stride)
  {
    if ((rect.right <= rect.left) || (rect.bottom <= rect.top)) {
      return;
    }
    bool native = !shader_is_active();
    if (native && shader_output_.alpha_blending_is_enabled()) {
      // the head can't blend, so all pixels have to be opaque
      const color::value_type* row = pixels;
      for (std::int16_t y = rect.top; native && (y < rect.bottom); ++y, row += stride) {
        for (std::size_t x = 0U; x < static_cast<std::size_t>(rect.width()); ++x) {
          if (!color::is_opaque(row[x])) {
            native = false;
            break;
          }
        }
      }
    }
    if (native) {
      head_.blit(rect, pixels, stride);
    }
    else {
      shader::base* const pipe = shader_pipe();
      const color::value_type* row = pixels;
      for (std::int16_t y = rect.top; y < rect.bottom; ++y, row += stride) {
        const color::value_type* p = row;
        for (std::int16_t x = rect.left; x < rect.right; ++x) {
          pipe->pixel_set({ x, y }, *p++);
        }
      }
    }
    present();
  }


  /**
   * Move display area
   * \param source Source rect
   * \param destination Destination top/left vertex
   */
  inline void move(const rect_type& source, vertex_type destination)
  {
    head_.move(source, destination);
    present();
  }


  /**
   * Move display area, wrapper
   * \param source Source top/left vertex
   * \param destination Destination top/left vertex
   * \param width Width of the area
   * \param height Height of the area
   */
  inline void move(vertex_type source, vertex_type destination, std::uint16_t width, std::uint16_t height)
  {
    const rect_type src = { source.x, source.y, static_cast<std::int16_t>(source.x + width), static_cast<std::int16_t>(source.y + height) };
    move(src, destination);
  }

};

} // namespace vic

#endif  // _VIC_DC_H_
