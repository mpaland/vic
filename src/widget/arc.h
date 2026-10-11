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
// \brief Arc widget, a ring (or a part of it) which fills with the value, like a round progress bar
//
// Usage:
// vic::widget::arc::config_type _cfg = { { 60, 60 }, 30, 8, -135, 135, 0, 100,
//                                        vic::color::green, vic::color::gray04, vic::color::black };
// vic::widget::arc a(_head, _cfg);
// a.init();
// a.set_value(42);
//
// Configuration:
// - angles are "compass" degrees: 0 is 12 o'clock, positive is clockwise, angle_start < angle_end,
//   the arc fills clockwise from angle_start, a full ring is -180 .. 180 (or 0 .. 360)
// - the ring spans from radius - ring_width to radius
// - color_track is the color of the unfilled part of the ring, use the background color for an
//   invisible track (it is needed to erase the arc when the value decreases)
// - a value change draws only the changed sector, with anti_aliasing_enable() the complete ring is
//   redrawn on every change (the anti aliased edges can't be delta drawn): the ring and two pixel
//   around it are erased with color_bg first, so keep two pixel clearance to other content
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_ARC_H_
#define _VIC_WIDGET_ARC_H_

#include "widget.h"


namespace vic {
namespace widget {


class arc : public base
{
public:
  // configuration
  typedef struct tag_config_type {
    vertex_type       center;         // center of the ring
    std::uint16_t     radius;         // outer radius of the ring
    std::uint8_t      ring_width;     // width of the ring
    std::int16_t      angle_start;    // angle of range_lower in compass degrees (0 = 12 o'clock, clockwise)
    std::int16_t      angle_end;      // angle of range_upper, must be greater than angle_start
    std::int16_t      range_lower;    // lower range, e.g. 0
    std::int16_t      range_upper;    // upper range, e.g. 100
    color::value_type color_arc;      // arc (value) color
    color::value_type color_track;    // track color (unfilled part of the ring)
    color::value_type color_bg;       // background around the ring, used to erase the anti aliased edges, color::none for none
  } config_type;

private:
  dc            dc_;        // drawing context
  config_type   config_;    // configuration
  std::int16_t  value_;     // actual value
  bool          aa_;        // anti aliasing, full redraw on every change


public:
  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  arc(drv& head, const config_type& config)
    : base(head)
    , dc_(head)
    , config_(config)
    , value_(config.range_lower)
    , aa_(false)
  { }


  /**
   * Init: draw the widget
   */
  virtual void init() final
  {
    redraw();
  }


  /**
   * Set a new value, only the changed part of the ring is drawn
   * \param value New value, clamped to the valid range
   */
  void set_value(std::int16_t value)
  {
    value = clamp(value);
    if (value == value_) {
      return;
    }
    const std::int16_t a_old = value_to_angle(value_);
    const std::int16_t a_new = value_to_angle(value);
    value_ = value;
    if (aa_) {
      redraw();
      return;
    }
    if (a_new > a_old) {
      dc_.set_color(config_.color_arc);
      sector_cw(dc_, config_.center, inner_radius(), config_.radius, a_old, a_new);
    }
    else if (a_new < a_old) {
      dc_.set_color(config_.color_track);
      sector_cw(dc_, config_.center, inner_radius(), config_.radius, a_new, a_old);
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
   * Set a new arc color and redraw
   * \param color New arc color
   */
  void set_color(color::value_type color)
  {
    config_.color_arc = color;
    redraw();
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
   * Enable anti aliasing, the ring is then completely redrawn on every value change
   * \param enable True to enable anti aliasing, default is off
   */
  void anti_aliasing_enable(bool enable = true)
  {
    aa_ = enable;
    dc_.anti_aliasing_enable(enable);
    redraw();
  }


  /**
   * Redraw the control
   */
  void redraw()
  {
    const std::int16_t a = value_to_angle(value_);
    dc_.present_lock();
    erase_edges(dc_, config_.center, inner_radius(), config_.radius, aa_, config_.color_bg);
    dc_.set_color(config_.color_track);
    sector_cw(dc_, config_.center, inner_radius(), config_.radius, a, config_.angle_end);
    dc_.set_color(config_.color_arc);
    sector_cw(dc_, config_.center, inner_radius(), config_.radius, config_.angle_start, a);
    dc_.present_lock(false);
  }


protected:

  /**
   * Check if the vertex is inside the widget (the ring disc)
   * \param vertex Vertex to check
   * \return true if vertex is inside
   */
  virtual bool is_inside(vertex_type vertex) const final
  {
    return util::distance_squared(vertex, config_.center) <= static_cast<std::uint32_t>(config_.radius) * config_.radius;
  }


private:

  inline std::uint16_t inner_radius() const
  {
    return static_cast<std::uint16_t>(config_.radius > config_.ring_width ? config_.radius - config_.ring_width : 0U);
  }


  inline std::int16_t clamp(std::int16_t value) const
  {
    return value < config_.range_lower ? config_.range_lower : (value > config_.range_upper ? config_.range_upper : value);
  }


  inline std::int16_t value_to_angle(std::int16_t value) const
  {
    return widget::value_to_angle(value, config_.range_lower, config_.range_upper, config_.angle_start, config_.angle_end);
  }
};


} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_ARC_H_
