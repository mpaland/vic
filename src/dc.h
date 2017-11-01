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


  inline bool shader_is_active() const
  {
    return shader_pipe_ != &shader_output_;
  }


  void shader_register(shader::base* _shader)
  {
    _shader->next_ = shader_pipe_;
    shader_pipe_ = _shader;
  }


  void shader_remove(const shader::base* _shader)
  {
    if (_shader == shader_pipe_) {
      // remove the first shader in pipe
      shader_pipe_ = shader_pipe_->next_;
    }
    else {
      for (shader::base* s = shader_pipe_; !!s; s->next_) {
        if (s == _shader) {
          s = s->next_;
        }
      }
    }
  }


  /**
   * Init / clear the shader pipeline
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
   */
  inline void cls(color::value_type bk_color = color::none)
  { 
    head_.cls(bk_color);
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
   * \return Screen width in pixel or chars
   */
  inline virtual std::uint16_t screen_width() const final
  { return head_.screen_width(); }


  /**
   * Returns the screen (buffer) height
   * \return Screen height in pixel or chars
   */
  inline virtual std::uint16_t screen_height() const final
  { return head_.screen_height(); }


  ///////////////////////////////////////////////////////////////////////////////


  /**
   * Draw a horizontal line, width is one pixel
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, y component is ignored
   */
  void line_horz(vertex_type v0, vertex_type v1)
  {
    if (!shader_is_active()) {
      // no active shaders, let the driver draw the line
      head_.line_horz(v0, v1, get_color());
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
  void line_vert(vertex_type v0, vertex_type v1)
  {
    if (!shader_is_active()) {
      // no active shaders, let the driver draw the line
      head_.line_vert(v0, v1, get_color());
      present();
    }
    else {
      // let the gpr do the work
      gpr::line_vert(v0, v1);
    }
  }


  /**
   * Draw a box (filled rectangle)
   * \param v0 top/left vertex
   * \param v1 bottom/right vertex
   */
  void box(vertex_type v0, vertex_type v1)
  {
    if (!shader_is_active()) {
      // no active shaders, let the driver draw the box
      head_.box(v0, v1, get_color());
      present();
    }
    else {
      // let the gpr do the work
      gpr::box(v0, v1);
    }
  }


  /**
   * Move display area
   * \param source Source top/left vertex
   * \param destination Destination top/left vertex
   * \param width Width of the area
   * \param height Height of the area
   */
  inline void move(vertex_type source, vertex_type destination, std::uint16_t width, std::uint16_t height)
  {
    head_.move(source, destination, width, height);
    present();
  }


  /**
   * move wrapper, vertex based
   * \param orig_top_left Source top/left vertex
   * \param orig_bottom_right Source bottom/right vertex
   * \param dest_top_left Destination top/left vertex
   */
  inline void move(vertex_type orig_top_left, vertex_type orig_bottom_right, vertex_type dest_top_left) {
    move(orig_top_left, dest_top_left, orig_bottom_right.x - orig_top_left.x, orig_bottom_right.y - orig_top_left.y);
  }
};

} // namespace vic

#endif  // _VIC_DC_H_
