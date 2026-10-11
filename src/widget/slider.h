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
// \brief Slider widget, a track with a knob, the value is set by tapping on the track
//
// Usage:
// vic::widget::slider::config_type _cfg = { { 10, 10 }, 100, 16, vic::widget::slider::horizontal, 0, 100, 4, 6,
//                                           vic::color::gray04, vic::color::green, vic::color::white, vic::color::black };
// vic::widget::slider sl(_head, _cfg);
// sl.init();
// sl.set_value(50);
//
// Behavior:
// - event_down and event_tap set the value to the event position (the knob jumps there), then the
//   event handler is called; a touch driver which sends event_down on every move gives a dragging knob
// - the track is centered in the widget rect, the knob radius defines the hit margin at the track ends
// - color_bg is used to erase the knob, use the background color of the display
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_SLIDER_H_
#define _VIC_WIDGET_SLIDER_H_

#include "widget.h"


namespace vic {
namespace widget {


class slider : public base
{
public:
  // orientation of the slider
  typedef enum tag_orientation_type {
    horizontal = 0,   // range_lower at the left
    vertical          // range_lower at the bottom
  } orientation_type;

  // configuration
  typedef struct tag_config_type {
    vertex_type       origin;         // origin (left/top)
    std::uint16_t     x_size;         // x size of the widget
    std::uint16_t     y_size;         // y size of the widget
    orientation_type  orientation;    // orientation
    std::int16_t      range_lower;    // lower range, e.g. 0
    std::int16_t      range_upper;    // upper range, e.g. 100
    std::uint8_t      track_width;    // thickness of the track
    std::uint8_t      knob_radius;    // radius of the knob
    color::value_type color_track;    // track color
    color::value_type color_fill;     // track color between range_lower and the knob, color::none for none
    color::value_type color_knob;     // knob color
    color::value_type color_bg;       // background color of the widget rect (erases the knob)
  } config_type;

private:
  dc            dc_;        // drawing context
  config_type   config_;    // configuration
  std::int16_t  value_;     // actual value


public:
  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  slider(drv& head, const config_type& config)
    : base(head)
    , dc_(head)
    , config_(config)
    , value_(config.range_lower)
  { }


  /**
   * Init: draw the widget
   */
  virtual void init() final
  {
    redraw();
  }


  /**
   * Set a new value and move the knob
   * \param value New value, clamped to the valid range
   */
  void set_value(std::int16_t value)
  {
    value = clamp(value);
    if (value != value_) {
      value_ = value;
      redraw();
    }
  }


  /**
   * Returns the actual value
   * \return Actual value
   */
  inline std::int16_t get_value() const
  {
    return value_;
  }


  /**
   * Set a new range, the value is set to range_lower
   * \param lower Lower range limit
   * \param upper Upper range limit
   */
  void set_range(std::int16_t lower, std::int16_t upper)
  {
    if (lower < upper) {
      config_.range_lower = value_ = lower;
      config_.range_upper = upper;
    }
    redraw();
  }


  /**
   * Enable anti aliasing for the knob
   * \param enable True to enable anti aliasing, default is off
   */
  void anti_aliasing_enable(bool enable = true)
  {
    dc_.anti_aliasing_enable(enable);
    redraw();
  }


  /**
   * Redraw the control
   */
  void redraw()
  {
    const rect_type   r    = outer_rect();
    const vertex_type knob = knob_pos();

    dc_.present_lock();
    dc_.set_color(config_.color_bg);
    dc_.box(r);
    // track
    dc_.set_color(config_.color_track);
    dc_.box(track_rect());
    // filled part
    if (config_.color_fill != color::none) {
      rect_type f = track_rect();
      if (config_.orientation == horizontal) { f.right = knob.x; } else { f.top = knob.y; }
      dc_.set_color(config_.color_fill);
      dc_.box(f);
    }
    // knob
    dc_.set_color(config_.color_knob);
    dc_.disc(knob, config_.knob_radius);
    dc_.present_lock(false);
  }


protected:

  virtual bool is_inside(vertex_type vertex) const final
  {
    return outer_rect().contain(vertex);
  }


  /**
   * Down and tap set the value to the event position
   */
  virtual void on_event(vertex_type vertex, event_type ev) final
  {
    if ((ev == event_down) || (ev == event_tap)) {
      set_value(pos_to_value(vertex));
    }
  }


private:

  inline rect_type outer_rect() const
  {
    return { config_.origin.x, config_.origin.y,
             static_cast<std::int16_t>(config_.origin.x + config_.x_size), static_cast<std::int16_t>(config_.origin.y + config_.y_size) };
  }


  // the track rect, centered across the orientation axis, the knob radius is kept free at the ends
  rect_type track_rect() const
  {
    const rect_type    r  = outer_rect();
    const std::int16_t kr = config_.knob_radius;
    if (config_.orientation == horizontal) {
      const std::int16_t top = static_cast<std::int16_t>(r.top + (r.height() - config_.track_width) / 2);
      return { static_cast<std::int16_t>(r.left + kr), top, static_cast<std::int16_t>(r.right - kr), static_cast<std::int16_t>(top + config_.track_width) };
    }
    const std::int16_t left = static_cast<std::int16_t>(r.left + (r.width() - config_.track_width) / 2);
    return { left, static_cast<std::int16_t>(r.top + kr), static_cast<std::int16_t>(left + config_.track_width), static_cast<std::int16_t>(r.bottom - kr) };
  }


  inline std::int16_t clamp(std::int16_t value) const
  {
    return value < config_.range_lower ? config_.range_lower : (value > config_.range_upper ? config_.range_upper : value);
  }


  // length of the track along the orientation axis (knob travel)
  inline std::int16_t travel() const
  {
    const rect_type t = track_rect();
    const std::int16_t len = static_cast<std::int16_t>((config_.orientation == horizontal ? t.width() : t.height()) - 1);
    return len > 0 ? len : 0;
  }


  // knob center for the actual value
  vertex_type knob_pos() const
  {
    const rect_type    t     = track_rect();
    const std::int32_t range = static_cast<std::int32_t>(config_.range_upper) - config_.range_lower;
    const std::int32_t px    = range > 0 ? (static_cast<std::int32_t>(travel()) * (value_ - config_.range_lower) + range / 2) / range : 0;
    if (config_.orientation == horizontal) {
      return { static_cast<std::int16_t>(t.left + px), static_cast<std::int16_t>(t.top + t.height() / 2) };
    }
    return { static_cast<std::int16_t>(t.left + t.width() / 2), static_cast<std::int16_t>(t.bottom - 1 - px) };
  }


  // value for a position on the widget
  std::int16_t pos_to_value(vertex_type v) const
  {
    const rect_type    t     = track_rect();
    const std::int32_t len   = travel();
    const std::int32_t range = static_cast<std::int32_t>(config_.range_upper) - config_.range_lower;
    if (len <= 0) {
      return config_.range_lower;
    }
    std::int32_t px = (config_.orientation == horizontal) ? (v.x - t.left) : (t.bottom - 1 - v.y);
    px = px < 0 ? 0 : (px > len ? len : px);
    return static_cast<std::int16_t>(config_.range_lower + (px * range + len / 2) / len);
  }
};


} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_SLIDER_H_
