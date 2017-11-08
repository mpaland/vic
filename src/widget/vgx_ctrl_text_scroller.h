///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2015-2016, PALANDesign Hannover, Germany
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
// \brief Text scroller control
// This controll draws and moves a test within a given area
// On graphic displays clipping is necessary and must be supported by the driver
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_CTRL_TEXT_SCOLLER_H_
#define _VGX_CTRL_TEXT_SCOLLER_H_

#include "../vgx_ctrl.h"


namespace vgx {


// modes:
// scroll left once
// scroll right once
// scroll left and right
// scroll once
// scroll looping
// start position (0 = text initial shown, width = text invisible, when scrolling left)


class text_scroller : public ctrl
{
public:
  typedef struct struct_config_type {
    std::int16_t        x0;             // left   coordinate (pixel or char pos)
    std::int16_t        y0;             // top    coordinate (pixel or char pos)
    std::int16_t        x1;             // right  coordinate (pixel or char pos)
    std::int16_t        y1;             // bottom coordinate (pixel or char pos)
    const std::uint8_t* text;           // text to show
    font_type*          font;           // font to use (unused on alpha numeric displays)
    bool                scroll_left;    // true to scroll left (normal), false to scroll right
    std::uint16_t       step_size;      // steps the text is scrolled by calling step_it()
    color::value_type   color;          // text color
    color::value_type   bg_color;       // background color
  } config_type;

  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  text_scroller(drv& head, config_type& config)
    : ctrl(head)
    , config_(config)
  { render(true); };


  /*
   * dtor
   */
  ~text_scroller()
  { }


  virtual bool is_inside(std::int16_t x, std::int16_t y)
  { return config_.x <= x && x <= config_.x + config_.width &&
           config_.y <= y && y <= config_.y + config_.height; }

  /**
   * Move the text
   * Has to be called periodically with the text scroll frequency
   */
  void step_it()
  {
    x_ = orientation ? x + config_.step_size : x - config_.step_size;

    if (head_.is_graphic) {
      
    }

    if (right && x_ > config_.x1) {
    if (!head_.is_graphic() && head_.text_string_get_extend(w, h, config_.text))
    }
  }


  void set_text(const std::uint8_t* text)
  {
    config_.text = text;
    redraw(true);
  }


  /**
   * Set new bar color
   * \param color New color for control, control is redrawn
   */
  void set_color(color::value_type color)
  { config_.color = color; redraw(); }



  /**
   * Redraw the control
   */
  void redraw()
  { render(pos_, true); }


private:
  void render(bool refresh = false)
  {
    // lock the output
    head_.primitive_lock();

    // clear the text area
    if (head_.is_graphic()) {
      head_.color_set(config_.bg_color);
      head_.box(config_.x0, config_.y0, config_.x1, config_.y1);
    }
    else {
      for (std::uint16_t _x = config_.x0; _x != config_.x1; ++_x) {
        head_.text_string_pos(_x, y, " ");
      }
    }

    // activate clipping region
    clipping_type area(config_.x0, config_.y0, config_.x1, config_.y1);
    head_.clipping_set(area);
    head_.text_string_pos(x_, config_.y, config_.text)
    head_.clipping_clear();
  }

  config_type   config_;  // configuration
  std::int16_t  x_;       // actual x position
};

} // namespace vgx

#endif  // _VGX_CTRL_TEXT_SCOLLER_H_
