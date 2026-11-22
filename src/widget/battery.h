///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
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
// \brief Battery widget: a battery symbol with a level from 0 to 100 %, like in the status bar of a phone
//
// Usage:
// vic::widget::battery::config_type _cfg = { { 290, 4 }, 24, 11, vic::widget::battery::left_to_right, 0, 20,
//                                            vic::color::green, vic::color::red, vic::color::white,
//                                            vic::color::white, vic::color::black };
// vic::widget::battery bat(_head, _cfg);
// bat.init();
// bat.set_level(75);        // 0 = empty, 100 = full
// bat.set_charging(true);
//
// Configuration:
// - origin is the left/top corner, x_size * y_size is the complete widget including the terminal
// - orientation is the fill direction, the terminal is at the full end: left_to_right is the phone style
//   battery with the terminal on the right, bottom_to_top is an upright battery
// - the body has a one pixel outline, the level is drawn with one pixel clearance inside of it
// - segment_count 0 or 1 is a continuous level bar, any other count shows the level as blocks
// - any level above 0 shows at least one pixel / one block, only level 0 is empty
// - level_low: the level is drawn in color_low if it is equal or below this threshold, 0 for none
// - set_charging() draws a bolt in color_charge over the level
// - color_bg: the inside of the body is cleared with it, color::none leaves it untouched
// - a change of the level draws only the changed part of the level
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_BATTERY_H_
#define _VIC_WIDGET_BATTERY_H_

#include "widget.h"


namespace vic {
namespace widget {


class battery : public base
{
public:
  // orientation of the battery (fill direction, the terminal is at the full end)
  typedef enum tag_orientation_type {
    left_to_right = 0,
    right_to_left,
    bottom_to_top,
    top_to_bottom
  } orientation_type;

  // configuration
  typedef struct tag_config_type {
    vertex_type       origin;         // origin (left/top)
    std::uint16_t     x_size;         // x size of the widget including the terminal
    std::uint16_t     y_size;         // y size of the widget including the terminal
    orientation_type  orientation;    // orientation of the battery, see orientation_type
    std::uint8_t      segment_count;  // 0 or 1: continuous level bar, else the number of level blocks
    std::uint8_t      level_low;      // low level threshold in percent, 0 for none
    color::value_type color_level;    // level color
    color::value_type color_low;      // level color if the level is equal or below level_low
    color::value_type color_frame;    // color of the body outline and the terminal
    color::value_type color_charge;   // color of the charging symbol
    color::value_type color_bg;       // background color inside the body, color::none to leave it untouched
  } config_type;

private:
  dc            dc_;          // drawing context
  config_type   config_;      // configuration
  std::uint8_t  level_;       // actual level in percent, 0 to 100
  bool          charging_;    // charging symbol is shown


public:
  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  battery(drv& head, const config_type& config)
    : base(head)
    , dc_(head)
    , config_(config)
    , level_(0U)
    , charging_(false)
  { }


  /**
   * Init: draw the widget (empty)
   */
  virtual void init() final
  {
    redraw();
  }


  /**
   * Set the level, only the changed part of the level is drawn
   * \param level New level in percent, 0 is empty, 100 is full, bigger values are clamped to 100
   */
  void set_level(std::uint8_t level)
  {
    level = level > 100U ? static_cast<std::uint8_t>(100U) : level;
    if (level != level_) {
      const std::int16_t      length_old = level_length();
      const color::value_type color_old  = level_color();
      level_ = level;
      const std::int16_t      length_new = level_length();
      if (color_old != level_color()) {
        // the low level threshold is crossed: the complete level changes its color
        render(0, length_old > length_new ? length_old : length_new);
      }
      else if (length_old != length_new) {
        render(length_old < length_new ? length_old : length_new, length_old < length_new ? length_new : length_old);
      }
    }
  }


  /**
   * Returns the actual level
   * \return Level in percent
   */
  inline std::uint8_t get_level() const
  {
    return level_;
  }


  /**
   * Set a new level color, drawn only on change
   * \param color New level color
   */
  void set_color(color::value_type color)
  {
    if (color != config_.color_level) {
      const color::value_type color_old = level_color();
      config_.color_level = color;
      if (color_old != level_color()) {
        render(0, level_length());
      }
    }
  }


  /**
   * Show or hide the charging symbol, drawn only on change
   * \param charging True to show the charging symbol
   */
  void set_charging(bool charging = true)
  {
    if (charging != charging_) {
      charging_ = charging;
      // on: only the symbol is drawn, off: the part of the level below the symbol is restored
      const std::int16_t mid   = static_cast<std::int16_t>(level_area() / 2);
      const std::int16_t reach = static_cast<std::int16_t>(charging ? 0 : bolt_reach());
      render(static_cast<std::int16_t>(mid - reach), static_cast<std::int16_t>(mid + reach));
    }
  }


  /**
   * Returns the charging state
   * \return True if the charging symbol is shown
   */
  inline bool is_charging() const
  {
    return charging_;
  }


  /**
   * Redraw the control
   */
  void redraw()
  {
    const std::int16_t across = size_across();
    const std::int16_t body   = body_length();
    // the terminal is half as wide as the body and centered
    std::int16_t width = static_cast<std::int16_t>(across / 2);
    width = static_cast<std::int16_t>(width + ((across - width) & 1));
    const std::int16_t start = static_cast<std::int16_t>((across - width) / 2);

    dc_.present_lock();
    if (config_.color_bg != color::none) {
      // the one pixel clearance between the outline and the level
      dc_.set_color(config_.color_bg);
      dc_.rectangle(axis_rect(1, static_cast<std::int16_t>(body - 1), 1, static_cast<std::int16_t>(across - 1)));
    }
    dc_.set_color(config_.color_frame);
    dc_.rectangle(axis_rect(0, body, 0, across), radius());
    dc_.box(axis_rect(body, size_along(), start, static_cast<std::int16_t>(start + width)));
    render(0, level_area());
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


  inline bool horizontal() const
  {
    return (config_.orientation == left_to_right) || (config_.orientation == right_to_left);
  }


  // size of the widget along the fill axis
  inline std::int16_t size_along() const
  {
    return static_cast<std::int16_t>(horizontal() ? config_.x_size : config_.y_size);
  }


  // size of the widget across the fill axis
  inline std::int16_t size_across() const
  {
    return static_cast<std::int16_t>(horizontal() ? config_.y_size : config_.x_size);
  }


  // length of the body along the fill axis, the rest is the terminal (a fifth of the size across)
  inline std::int16_t body_length() const
  {
    const std::int16_t terminal = static_cast<std::int16_t>(size_across() / 5);
    return static_cast<std::int16_t>(size_along() - (terminal > 0 ? terminal : 1));
  }


  // corner radius of the body outline
  inline std::uint16_t radius() const
  {
    const std::int16_t r = static_cast<std::int16_t>(size_across() / 6);
    return static_cast<std::uint16_t>(r > 3 ? 3 : r);
  }


  /**
   * Get a rect out of lengths along the fill axis and across it
   * \param from Start length along the fill axis in pixel, 0 is the empty end
   * \param to End length along the fill axis in pixel (exclusive)
   * \param a0 Start across the fill axis in pixel, 0 is left / top
   * \param a1 End across the fill axis in pixel (exclusive)
   * \return The rect
   */
  rect_type axis_rect(std::int16_t from, std::int16_t to, std::int16_t a0, std::int16_t a1) const
  {
    const rect_type out = outer_rect();
    switch (config_.orientation) {
      case left_to_right : return { static_cast<std::int16_t>(out.left + from), static_cast<std::int16_t>(out.top + a0), static_cast<std::int16_t>(out.left + to), static_cast<std::int16_t>(out.top + a1) };
      case right_to_left : return { static_cast<std::int16_t>(out.right - to), static_cast<std::int16_t>(out.top + a0), static_cast<std::int16_t>(out.right - from), static_cast<std::int16_t>(out.top + a1) };
      case bottom_to_top : return { static_cast<std::int16_t>(out.left + a0), static_cast<std::int16_t>(out.bottom - to), static_cast<std::int16_t>(out.left + a1), static_cast<std::int16_t>(out.bottom - from) };
      case top_to_bottom : return { static_cast<std::int16_t>(out.left + a0), static_cast<std::int16_t>(out.top + from), static_cast<std::int16_t>(out.left + a1), static_cast<std::int16_t>(out.top + to) };
      default            : return { 0, 0, 0, 0 };
    }
  }


  // length of the level area along the fill axis: two pixel inside the body at both ends
  inline std::int16_t level_area() const
  {
    return static_cast<std::int16_t>(body_length() - 4);
  }


  // the level color of the actual level
  inline color::value_type level_color() const
  {
    return level_ <= config_.level_low ? config_.color_low : config_.color_level;
  }


  /**
   * Calculate the lit length of the level area for the actual level
   * The length is rounded up, any level above 0 is visible
   * \return Lit length in pixel, with blocks the end of the last lit block
   */
  std::int16_t level_length() const
  {
    const std::int32_t length = level_area();
    if (config_.segment_count > 1U) {
      const std::int32_t lit = (level_ * config_.segment_count + 99) / 100;
      return static_cast<std::int16_t>(lit > 0 ? lit * (length + 1) / config_.segment_count - 1 : 0);
    }
    return static_cast<std::int16_t>((length * level_ + 99) / 100);
  }


  // half height of the charging symbol
  inline std::int16_t bolt_height() const
  {
    return static_cast<std::int16_t>(((body_length() < size_across() ? body_length() : size_across()) - 5) / 2);
  }


  // half width of the charging symbol
  inline std::int16_t bolt_width() const
  {
    const std::int16_t hw = static_cast<std::int16_t>(bolt_height() * 2 / 3);
    return static_cast<std::int16_t>(hw > 0 ? hw : 1);
  }


  // reach of the charging symbol along the fill axis, seen from the middle of the level area
  inline std::int16_t bolt_reach() const
  {
    return static_cast<std::int16_t>((horizontal() ? bolt_width() : bolt_height()) + 2);
  }


  /**
   * Render the level area between two lengths along the fill axis: level, background and charging symbol
   * \param from Start length in pixel (inclusive), 0 is the empty end of the level area
   * \param to End length in pixel (exclusive)
   */
  void render(std::int16_t from, std::int16_t to)
  {
    const std::int16_t length = level_area();
    const std::int16_t a1     = static_cast<std::int16_t>(size_across() - 2);
    if ((length <= 0) || (a1 <= 2)) {
      return;
    }
    const std::int32_t lit   = level_length();
    const std::int32_t count = config_.segment_count > 1U ? config_.segment_count : 1;
    const std::int32_t end   = to < length ? to : length;

    dc_.present_lock();
    for (std::int32_t n = 0; n < count; ++n) {
      // a block and the one pixel gap behind it, a continuous level is one block
      std::int32_t c0 = n * (length + 1) / count;         // start of the block
      std::int32_t c1 = (n + 1) * (length + 1) / count;   // end of the gap
      std::int32_t l1 = c1 - 1 < lit ? c1 - 1 : lit;      // end of the lit part
      c0 = c0 > from ? c0 : from;
      c1 = c1 < end  ? c1 : end;
      l1 = l1 < c1   ? l1 : c1;
      if (c0 < l1) {
        dc_.set_color(level_color());
        dc_.box(axis_rect(static_cast<std::int16_t>(2 + c0), static_cast<std::int16_t>(2 + l1), 2, a1));
      }
      l1 = l1 > c0 ? l1 : c0;
      if ((l1 < c1) && (config_.color_bg != color::none)) {
        dc_.set_color(config_.color_bg);
        dc_.box(axis_rect(static_cast<std::int16_t>(2 + l1), static_cast<std::int16_t>(2 + c1), 2, a1));
      }
    }

    if (charging_ && (from <= length / 2 + bolt_reach()) && (to >= length / 2 - bolt_reach())) {
      // the rendered part touches the symbol: an upright bolt out of two triangles, point symmetric to the center of the body
      const rect_type    b  = axis_rect(0, body_length(), 0, size_across());
      const std::int16_t cx = static_cast<std::int16_t>((b.left + b.right - 1) / 2);
      const std::int16_t cy = static_cast<std::int16_t>((b.top + b.bottom - 1) / 2);
      const std::int16_t hh = bolt_height();
      const std::int16_t hw = bolt_width();
      const std::int16_t ax = static_cast<std::int16_t>((hw + 1) / 2);    // x offset of the tips
      dc_.set_color(config_.color_charge);
      dc_.triangle_solid({ static_cast<std::int16_t>(cx + ax), static_cast<std::int16_t>(cy - hh) }, { static_cast<std::int16_t>(cx - hw), cy }, { cx, cy });
      dc_.triangle_solid({ static_cast<std::int16_t>(cx - ax), static_cast<std::int16_t>(cy + hh) }, { static_cast<std::int16_t>(cx + hw), cy }, { cx, cy });
    }
    dc_.present_lock(false);
  }
};


} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_BATTERY_H_
