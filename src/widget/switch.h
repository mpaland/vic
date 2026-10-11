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
// \brief Switch widget, an on / off toggle with a sliding knob
//
// Usage:
// vic::widget::toggle::config_type _cfg = { { 10, 10 }, 30, 14,
//                                           vic::color::green, vic::color::gray04, vic::color::white, vic::color::gray10 };
// vic::widget::toggle sw(_head, _cfg);
// sw.init();
// sw.set_state(true);
//
// Behavior:
// - event_tap toggles the state, then the event handler is called
// - the track is a rounded box, the knob a disc at the left (off) or right (on) end
// - the class is named toggle, as switch is a C++ keyword
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_SWITCH_H_
#define _VIC_WIDGET_SWITCH_H_

#include "widget.h"


namespace vic {
namespace widget {


class toggle : public base
{
public:
  // configuration
  typedef struct tag_config_type {
    vertex_type       origin;         // origin (left/top)
    std::uint16_t     x_size;         // width of the track, at least 2 * y_size
    std::uint16_t     y_size;         // height of the track
    color::value_type color_on;       // track color in on state
    color::value_type color_off;      // track color in off state
    color::value_type color_knob;     // knob color
    color::value_type color_border;   // track border color, color::none for no border
  } config_type;

private:
  dc            dc_;        // drawing context
  config_type   config_;    // configuration
  bool          on_;        // state


public:
  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  toggle(drv& head, const config_type& config)
    : base(head)
    , dc_(head)
    , config_(config)
    , on_(false)
  { }


  /**
   * Init: draw the widget
   */
  virtual void init() final
  {
    redraw();
  }


  /**
   * Set the state, redrawn only on change
   * \param on True for on
   */
  void set_state(bool on)
  {
    if (on != on_) {
      on_ = on;
      redraw();
    }
  }


  /**
   * Toggle the state
   */
  inline void toggle_state()
  {
    set_state(!on_);
  }


  /**
   * Returns the state
   * \return True if on
   */
  inline bool is_on() const
  {
    return on_;
  }


  /**
   * Enable anti aliasing for the rounded track and the knob
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
    const rect_type     r      = outer_rect();
    const std::uint16_t radius = static_cast<std::uint16_t>(config_.y_size / 2U);
    const std::uint16_t knob   = static_cast<std::uint16_t>(radius > 2U ? radius - 2U : 1U);
    const vertex_type   center = { static_cast<std::int16_t>(on_ ? r.right - 1 - radius : r.left + radius), static_cast<std::int16_t>(r.top + radius) };

    dc_.present_lock();
    dc_.set_color(on_ ? config_.color_on : config_.color_off);
    dc_.box(r, radius);
    if (config_.color_border != color::none) {
      dc_.set_color(config_.color_border);
      dc_.rectangle(r, radius);
    }
    dc_.set_color(config_.color_knob);
    dc_.disc(center, knob);
    dc_.present_lock(false);
  }


protected:

  virtual bool is_inside(vertex_type vertex) const final
  {
    return outer_rect().contain(vertex);
  }


  /**
   * A tap toggles the state
   */
  virtual void on_event(vertex_type vertex, event_type ev) final
  {
    (void)vertex;
    if (ev == event_tap) {
      toggle_state();
    }
  }


private:

  inline rect_type outer_rect() const
  {
    return { config_.origin.x, config_.origin.y,
             static_cast<std::int16_t>(config_.origin.x + config_.x_size), static_cast<std::int16_t>(config_.origin.y + config_.y_size) };
  }
};


} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_SWITCH_H_
