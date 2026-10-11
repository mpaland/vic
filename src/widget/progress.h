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
// \brief Progress bar widget
//
// Usage:
// vic::widget::progress::config_type _cfg = { { 10, 10 }, 100, 13, vic::widget::progress::left_to_right,
//                                             0, 100,
//                                             vic::color::red,
//                                             vic::color::darkgray,
//                                             vic::color::white
//                                           };
// vic::widget::progress prog(_head, _cfg);
// prog.init();
// prog.set_pos(50);
//
// Configuration:
// - origin is always the left/top corner of the widget
// - x_size is always the complete size in x-direction
// - y_size is always the complete size in y-direction
// - if border color is set, the border is one pixel wide and part of x_size * y_size
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_PROGRESS_H_
#define _VIC_WIDGET_PROGRESS_H_

#include "widget.h"


namespace vic {
namespace widget {


class progress : public base
{
public:
  // orientation of the bar
  typedef enum tag_oriantation_type {
    left_to_right = 0,
    right_to_left,
    bottom_to_top,
    top_to_bottom
  } orientation_type;

  // configuration
  typedef struct tag_config_type {
    vertex_type       origin;         // origin (left/top)
    std::uint16_t     x_size;         // x size of the bar
    std::uint16_t     y_size;         // y size of the bar
    orientation_type  orientation;    // orientation of the bar, see orientation_type
    std::int16_t      range_lower;    // initial lower range, e.g. 0
    std::int16_t      range_upper;    // initial upper range, e.g. 100
    color::value_type color_bar;      // bar color
    color::value_type color_bg;       // background color, color::none to leave the background untouched
    color::value_type color_border;   // border color, color::none for no border
  } config_type;

private:
  dc            dc_;        // drawing context
  config_type   config_;    // configuration
  std::int16_t  pos_;       // actual position, valid from range_lower to range_upper
  std::int16_t  step_;      // step increment


public:
  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  progress(drv& head, const config_type& config)
    : base(head)
    , dc_(head)
    , config_(config)
    , pos_(config.range_lower)
    , step_(1)
  { }


  /**
   * Init: draw the widget
   */
  virtual void init() final
  {
    redraw();
  }


  /**
   * Set new bar color
   * Redraw is called to reflect the changes
   * \param color New color for control, control is redrawn
   */
  void set_color(color::value_type color)
  {
    config_.color_bar = color;
    redraw();
  }


  /**
   * Set the new minimum and maximum range, pos is set to range_lower
   * Redraw is called to reflect the changes
   * \param lower Lower range limit including 'lower' value
   * \param upper Upper range limit including 'upper' value
   */
  void set_range(std::int16_t lower, std::int16_t upper)
  {
    if (lower < upper) {
      config_.range_lower = pos_ = lower;
      config_.range_upper        = upper;
    }
    redraw();
  }


  /**
   * Set the new position of the progress bar control and render it
   * \param pos New position, clamped to the valid range
   */
  void set_pos(std::int16_t pos)
  {
    render(clamp(pos));
  }


  /**
   * Reset the progress bar to lower range position (no progress)
   */
  inline void reset()
  {
    render(config_.range_lower);
  }


  /**
   * Returns the current position of the progress bar control
   * \return Current position
   */
  inline std::int16_t get_pos() const
  {
    return pos_;
  }


  /**
   * Specifies the step increment for the step_it() function
   * \param step New step increment, can be negative, default is 1
   */
  inline void set_step(std::int16_t step = 1)
  {
    step_ = step;
  }


  /**
   * Advances the current position by the step increment and redraws the bar
   * to reflect the new position
   */
  void step_it()
  {
    render(clamp(static_cast<std::int16_t>(pos_ + step_)));
  }


  /**
   * Redraw the control
   */
  void redraw()
  {
    dc_.present_lock();
    // render the border
    if (config_.color_border != color::none) {
      dc_.set_color(config_.color_border);
      dc_.rectangle(outer_rect());
    }
    // render the bar
    render(pos_, true);
    dc_.present_lock(false);
  }


protected:

  /**
   * Check if the vertex is inside the widget
   * \param vertex Vertex to check
   * \return true if vertex is inside
   */
  virtual bool is_inside(vertex_type vertex) const final
  {
    return outer_rect().contain(vertex);
  }


private:

  // the complete widget rect
  inline rect_type outer_rect() const
  {
    return { config_.origin.x, config_.origin.y,
             static_cast<std::int16_t>(config_.origin.x + config_.x_size), static_cast<std::int16_t>(config_.origin.y + config_.y_size) };
  }


  // the bar rect (inside the border)
  inline rect_type inner_rect() const
  {
    rect_type r = outer_rect();
    if (config_.color_border != color::none) {
      r.left++; r.top++; r.right--; r.bottom--;
      if (r.right < r.left) { r.right = r.left; }
      if (r.bottom < r.top) { r.bottom = r.top; }
    }
    return r;
  }


  inline bool horizontal() const
  {
    return (config_.orientation == left_to_right) || (config_.orientation == right_to_left);
  }


  // clamp a position to the valid range
  inline std::int16_t clamp(std::int16_t pos) const
  {
    return pos < config_.range_lower ? config_.range_lower : (pos > config_.range_upper ? config_.range_upper : pos);
  }


  /**
   * Calculate the bar length in pixel for a position
   * \param pos Position within the range
   * \param length Length of the complete bar in pixel
   * \return Bar length in pixel, 0 to length
   */
  std::int16_t bar_length(std::int16_t pos, std::int16_t length) const
  {
    const std::int32_t range = static_cast<std::int32_t>(config_.range_upper) - config_.range_lower;
    if (range <= 0) {
      return 0;
    }
    std::int32_t bar = (static_cast<std::int32_t>(length) * (pos - config_.range_lower) + range / 2) / range;
    bar = bar < 0 ? 0 : (bar > length ? length : bar);
    return static_cast<std::int16_t>(bar);
  }


  /**
   * Get the rect of the bar part between two bar lengths (exclusive right/bottom)
   * \param from Start length in pixel (smaller)
   * \param to End length in pixel (bigger)
   * \return The rect
   */
  rect_type bar_rect(std::int16_t from, std::int16_t to) const
  {
    const rect_type in = inner_rect();
    switch (config_.orientation) {
      case left_to_right : return { static_cast<std::int16_t>(in.left + from), in.top, static_cast<std::int16_t>(in.left + to), in.bottom };
      case right_to_left : return { static_cast<std::int16_t>(in.right - to), in.top, static_cast<std::int16_t>(in.right - from), in.bottom };
      case bottom_to_top : return { in.left, static_cast<std::int16_t>(in.bottom - to), in.right, static_cast<std::int16_t>(in.bottom - from) };
      case top_to_bottom : return { in.left, static_cast<std::int16_t>(in.top + from), in.right, static_cast<std::int16_t>(in.top + to) };
      default            : return { 0, 0, 0, 0 };
    }
  }


  /**
   * Render the bar for the new position
   * \param pos New bar position
   * \param refresh True if a complete redraw is done, false for delta drawing
   */
  void render(std::int16_t pos, bool refresh = false)
  {
    const rect_type    in     = inner_rect();
    const std::int16_t length = horizontal() ? in.width() : in.height();
    const std::int16_t bar    = bar_length(pos, length);

    dc_.present_lock();
    if (refresh) {
      // complete redraw: background and bar
      if (config_.color_bg != color::none) {
        dc_.set_color(config_.color_bg);
        dc_.box(bar_rect(bar, length));
      }
      dc_.set_color(config_.color_bar);
      dc_.box(bar_rect(0, bar));
    }
    else if (pos != pos_) {
      // delta redraw, only the changed area
      const std::int16_t bar_old = bar_length(pos_, length);
      if (bar > bar_old) {
        dc_.set_color(config_.color_bar);
        dc_.box(bar_rect(bar_old, bar));
      }
      else if ((bar < bar_old) && (config_.color_bg != color::none)) {
        dc_.set_color(config_.color_bg);
        dc_.box(bar_rect(bar, bar_old));
      }
    }
    dc_.present_lock(false);

    // set new position
    pos_ = pos;
  }
};


} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_PROGRESS_H_
