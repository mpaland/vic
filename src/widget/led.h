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
// \brief LED widget, a round or square indicator with an on and an off color
//
// Usage:
// vic::widget::led::config_type _cfg = { { 20, 20 }, 6, vic::widget::led::shape_round,
//                                        vic::color::green, vic::color::darkgreen, vic::color::gray08 };
// vic::widget::led l(_head, _cfg);
// l.init();
// l.set_state(true);
// l.toggle();
//
// Configuration:
// - center and radius define the LED, a square LED is 2 * radius + 1 wide with slightly rounded corners
// - color_border is a one pixel border, color::none for none
// - anti_aliasing_enable() smooths the round shape
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_LED_H_
#define _VIC_WIDGET_LED_H_

#include "widget.h"


namespace vic {
namespace widget {


class led : public base
{
public:
  // shape of the LED
  typedef enum tag_shape_type {
    shape_round = 0,
    shape_square
  } shape_type;

  // configuration
  typedef struct tag_config_type {
    vertex_type       center;         // center of the LED
    std::uint8_t      radius;         // radius (half size) of the LED
    shape_type        shape;          // round or square
    color::value_type color_on;       // color in on state
    color::value_type color_off;      // color in off state
    color::value_type color_border;   // border color, color::none for no border
  } config_type;

private:
  dc            dc_;        // drawing context
  config_type   config_;    // configuration
  bool          on_;        // actual state


public:
  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  led(drv& head, const config_type& config)
    : base(head)
    , dc_(head)
    , config_(config)
    , on_(false)
  { }


  /**
   * Init: draw the widget (off)
   */
  virtual void init() final
  {
    redraw();
  }


  /**
   * Set the LED state, redrawn only on change
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
   * Toggle the LED state
   */
  inline void toggle()
  {
    set_state(!on_);
  }


  /**
   * Returns the LED state
   * \return True if on
   */
  inline bool is_on() const
  {
    return on_;
  }


  /**
   * Set new on / off colors and redraw
   * \param color_on New on color
   * \param color_off New off color
   */
  void set_color(color::value_type color_on, color::value_type color_off)
  {
    config_.color_on  = color_on;
    config_.color_off = color_off;
    redraw();
  }


  /**
   * Enable anti aliasing for the round shape and the border
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
    const std::uint16_t r = config_.radius;
    dc_.present_lock();
    dc_.set_color(on_ ? config_.color_on : config_.color_off);
    if (config_.shape == shape_round) {
      dc_.disc(config_.center, r);
      if (config_.color_border != color::none) {
        dc_.set_color(config_.color_border);
        dc_.circle(config_.center, r);
      }
    }
    else {
      const rect_type rect = outer_rect();
      const std::uint16_t corner = static_cast<std::uint16_t>(r / 3U);
      dc_.box(rect, corner);
      if (config_.color_border != color::none) {
        dc_.set_color(config_.color_border);
        dc_.rectangle(rect, corner);
      }
    }
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

  // the bounding rect, 2 * radius + 1 wide
  inline rect_type outer_rect() const
  {
    return { static_cast<std::int16_t>(config_.center.x - config_.radius), static_cast<std::int16_t>(config_.center.y - config_.radius),
             static_cast<std::int16_t>(config_.center.x + config_.radius + 1), static_cast<std::int16_t>(config_.center.y + config_.radius + 1) };
  }
};


} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_LED_H_
