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
// \brief Progress bar widget
//
// - origin is always the left/top corner of the widget
// - x_size is always the complete size in x-direction
// - y_size is always the complete size in y-direction
// - if border color is set, the border is x-size * y-size
//
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
    color::value_type color_bg;       // background color
    color::value_type color_border;   // border color
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
   * Init
   */
  virtual void init() final
  {
    // reduce the size by one, because rectangle() and box() always include the end vertex
    config_.x_size -= 1U;
    config_.y_size -= 1U;
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
   * \param pos New position - must be within the valid range
   */
  void set_pos(std::int16_t pos)
  {
    if (pos >= config_.range_lower && pos <= config_.range_upper) {
      render(pos);
    }
  }


  /**
   * Reset the progress bar to lower range position (no progress)
   */
  void reset()
  {
    render(config_.range_lower);
  }


  /**
   * Returns the current position of the progress bar control
   * \return Current position
   */
  std::int16_t get_pos() const
  {
    return pos_;
  }


  /**
   * Specifies the step increment for the step_it() function
   * \param step New step increment, can be negative, default is 1
   */
  void set_step(std::int16_t step = 1)
  {
    step_ = step;
  }


  /**
   * Advances the current position by the step increment and redraws the bar
   * to reflect the new position
   */
  void step_it()
  {
    std::int16_t pos = pos_ + step_;
    if (pos < config_.range_lower) {
      pos = config_.range_lower;
    }
    else if (pos > config_.range_upper) {
      pos = config_.range_upper;
    }

    // reflect the new position
    render(pos);
  }


  /**
   * Redraw the control
   */
  inline void redraw()
  {
    // render the border
    if (config_.color_border != color::none) {
      dc_.set_color(config_.color_border);
      dc_.rectangle(config_.origin, { static_cast<std::int16_t>(config_.origin.x + config_.x_size), static_cast<std::int16_t>(config_.origin.y + config_.y_size) });
    }
    // render the bar
    render(pos_, true);
  }


protected:

  /**
   * Check if the vertex is inside the widget
   * \param vertex Vertex to check
   * \return true if vertex is inside
   */
  virtual bool is_inside(vertex_type vertex) const final
  { return config_.origin.x <= vertex.x && vertex.x <= config_.origin.x + config_.x_size &&
           config_.origin.y <= vertex.y && vertex.y <= config_.origin.y + config_.y_size; }


private:

  /**
   * Render the bar for the new position
   * \param pos New bar position
   * \param refresh True if a complete redraw is done, false for delta drawing
   */
  void render(std::int16_t pos, bool refresh = false)
  {
    rect_type rect_bar;
    rect_type rect_bkg;

    const vertex_type   origin = (config_.color_border == color::none) ? config_.origin : config_.origin + vertex_type({ 1, 1 });
    const std::uint16_t x_size = (config_.color_border == color::none) ? config_.x_size : config_.x_size - 2U;
    const std::uint16_t y_size = (config_.color_border == color::none) ? config_.y_size : config_.y_size - 2U;

    // calculate the bar sizes in screen units
    std::uint16_t bar_size = static_cast<std::uint16_t>(util::div_round_closest(((config_.orientation == left_to_right || config_.orientation == right_to_left) ? x_size + 1U : y_size + 1U) * (pos - config_.range_lower),
                                                                                (config_.range_upper - config_.range_lower)));
    std::uint16_t bkg_size = static_cast<std::uint16_t>(((config_.orientation == left_to_right || config_.orientation == right_to_left) ? x_size + 1U : y_size + 1U) - bar_size);
    if (bar_size) {
      bar_size--;
    }
    if (bkg_size) {
      bkg_size--;
    }

    if (refresh) {
      // complete redraw

//  left_to_right                                                100 = xsize
//  |----------------|--------|       0  1  2  3  4  5  6  7  8  9  0
// -200        0    -120     -100
//      bar_color        bg
//
      switch (config_.orientation) {
        case left_to_right :
          rect_bar = { origin.x,                     origin.y, origin.x + bar_size, origin.y + y_size };
          rect_bkg = { origin.x + x_size - bkg_size, origin.y, origin.x + x_size,   origin.y + y_size };
          break;
        case right_to_left :
          rect_bar = { origin.x + x_size - bar_size, origin.y, origin.x + x_size,   origin.y + y_size };
          rect_bkg = { origin.x,                     origin.y, origin.x + bkg_size, origin.y + y_size };
          break;
        case bottom_to_top :
          rect_bar = { origin.x, origin.y + y_size - bar_size, origin.x + x_size, origin.y + y_size };
          rect_bkg = { origin.x, origin.y,                     origin.x + x_size, origin.y + bkg_size };
          break;
        case top_to_bottom :
          rect_bar = { origin.x, origin.y,                     origin.x + x_size, origin.y + bar_size };
          rect_bkg = { origin.x, origin.y + y_size - bkg_size, origin.x + x_size, origin.y + y_size };
          break;
        default:
          rect_bar.clear();
          rect_bkg.clear();
          break;
      }
      // draw bar and background
      if (pos != config_.range_upper) {
        dc_.set_color(config_.color_bg);
        dc_.box(rect_bkg);
      }
      if (pos - config_.range_lower) {
        dc_.set_color(config_.color_bar);
        dc_.box(rect_bar);
      }
    }
    else {
      // delta redraw, only the changed area
      if (pos == pos_) {
        // no change, nothing to do
        return;
      }

      // calculate the last bar size in screen units
      std::uint16_t bar_size_old = static_cast<std::uint16_t>(util::div_round_closest(((config_.orientation == left_to_right || config_.orientation == right_to_left) ? x_size + 1U : y_size + 1U) * (pos_ - config_.range_lower),
                                                                                      (config_.range_upper - config_.range_lower)));
//      std::uint16_t bkg_size = static_cast<std::uint16_t>(((config_.orientation == left_to_right || config_.orientation == right_to_left) ? x_size + 1U : y_size + 1U) - bar_size);
      if (bar_size_old) {
        bar_size_old--;
      }
      // make sure, that offset is smaller than offset_old
      util::min_max_swap(bar_size, bar_size_old);

//  left_to_right
//  |------------n---o--------|
//      bar_color        bg
//
      switch (config_.orientation) {
        case left_to_right :
          rect_bar = { origin.x + bar_size, origin.y, origin.x + bar_size_old, origin.y + y_size };
          break;
        case right_to_left :
          rect_bar = { origin.x + x_size - bar_size_old, origin.y, origin.x + x_size - bar_size, origin.y + y_size };
          break;
        case bottom_to_top :
          rect_bar = { origin.x, origin.y + y_size - bar_size_old, origin.x + x_size, origin.y + y_size - bar_size };
          break;
        case top_to_bottom :
          rect_bar = { origin.x, origin.y + bar_size, origin.x + x_size, origin.y + bar_size_old };
          break;
        default:
          rect_bar.clear();
          break;
      }
      // draw delta rect
      dc_.set_color((pos > pos_)  ? config_.color_bar : config_.color_bg);
      dc_.box(rect_bar);
    }
    // set new position
    pos_ = pos;
  }
};


} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_PROGRESS_H_
