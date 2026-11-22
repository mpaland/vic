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
// \brief VU meter widget: a level meter out of segments, colored by zones or a gradient
//
// Usage:
// static const vic::color::value_type _zones[] = { vic::color::green, vic::color::yellow, vic::color::red };
// vic::widget::vumeter::config_type _cfg = { { 10, 10 }, 120, 8, vic::widget::vumeter::left_to_right, 20, 1,
//                                            0, 100, _zones, 3, true, 64, vic::color::black };
// vic::widget::vumeter vu(_head, _cfg);
// vu.init();
// vu.set_value(70);
//
// Configuration:
// - origin is the left/top corner, x_size * y_size is the complete widget
// - the segments are spread evenly along the orientation axis, segment_gap pixel apart
// - colors is an array of color_count colors from the first to the last segment, it must stay valid:
//   gradient = false: color_count hard zones of equal size, a wider zone is made by repeating its color,
//                     e.g. { green, green, green, yellow, red } is 60 / 20 / 20 %
//   gradient = true:  the colors are stops from the first to the last segment, the segments blend between them
//   one color is a single colored meter in both modes
// - an unlit segment shows its own color dimmed to off_lum (0: black, 255: full color)
// - color_bg: the gaps are cleared with it, color::none leaves them untouched
// - a value change draws only the segments which change their state
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_VUMETER_H_
#define _VIC_WIDGET_VUMETER_H_

#include "widget.h"


namespace vic {
namespace widget {


class vumeter : public base
{
public:
  // orientation of the meter (direction of increasing values)
  typedef enum tag_orientation_type {
    left_to_right = 0,
    right_to_left,
    bottom_to_top,
    top_to_bottom
  } orientation_type;

  // configuration
  typedef struct tag_config_type {
    vertex_type               origin;         // origin (left/top)
    std::uint16_t             x_size;         // x size of the widget
    std::uint16_t             y_size;         // y size of the widget
    orientation_type          orientation;    // orientation of the meter, see orientation_type
    std::uint8_t              segment_count;  // number of segments
    std::uint8_t              segment_gap;    // gap between two segments in pixel
    std::int16_t              range_lower;    // lower range, e.g. 0
    std::int16_t              range_upper;    // upper range, e.g. 100
    const color::value_type*  colors;         // zone colors from the first to the last segment, must stay valid
    std::uint8_t              color_count;    // number of colors, 1 for a single colored meter
    bool                      gradient;       // true: the colors blend from zone to zone, false: hard zones
    std::uint8_t              off_lum;        // luminance of the unlit segments, 0: black, 255: full color
    color::value_type         color_bg;       // gap color, color::none to leave the gaps untouched
  } config_type;

private:
  dc            dc_;        // drawing context
  config_type   config_;    // configuration
  std::int16_t  value_;     // actual value, valid from range_lower to range_upper


public:
  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  vumeter(drv& head, const config_type& config)
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
   * Set a new value, only the segments which change their state are drawn
   * \param value New value, clamped to the valid range
   */
  void set_value(std::int16_t value)
  {
    value = clamp(value);
    const std::uint_fast8_t lit_old = lit_count(value_);
    const std::uint_fast8_t lit_new = lit_count(value);
    value_ = value;
    if (lit_old != lit_new) {
      dc_.present_lock();
      render(lit_old < lit_new ? lit_old : lit_new, lit_old < lit_new ? lit_new : lit_old);
      dc_.present_lock(false);
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
   * Redraw is called to reflect the changes
   * \param lower Lower range limit
   * \param upper Upper range limit
   */
  void set_range(std::int16_t lower, std::int16_t upper)
  {
    if (lower < upper) {
      config_.range_lower = value_ = lower;
      config_.range_upper          = upper;
    }
    redraw();
  }


  /**
   * Redraw the control
   */
  void redraw()
  {
    dc_.present_lock();
    if (config_.color_bg != color::none) {
      dc_.set_color(config_.color_bg);
      dc_.box(outer_rect());
    }
    render(0U, config_.segment_count);
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


  // clamp a value to the valid range
  inline std::int16_t clamp(std::int16_t value) const
  {
    return value < config_.range_lower ? config_.range_lower : (value > config_.range_upper ? config_.range_upper : value);
  }


  /**
   * Calculate the number of lit segments for a value
   * \param value Value within the range
   * \return Lit segments, 0 to segment_count
   */
  std::uint_fast8_t lit_count(std::int16_t value) const
  {
    const std::int32_t range = static_cast<std::int32_t>(config_.range_upper) - config_.range_lower;
    if (range <= 0) {
      return 0U;
    }
    return static_cast<std::uint_fast8_t>((static_cast<std::int32_t>(config_.segment_count) * (value - config_.range_lower) + range / 2) / range);
  }


  /**
   * Get the rect of a segment, the rounding is spread evenly so the last segment ends at the widget edge
   * \param segment Segment index, 0 is the segment of the lowest value
   * \return The rect
   */
  rect_type segment_rect(std::uint_fast8_t segment) const
  {
    const rect_type    out    = outer_rect();
    const std::int32_t length = (horizontal() ? out.width() : out.height()) + config_.segment_gap;
    const std::int16_t from   = static_cast<std::int16_t>(length * static_cast<std::int32_t>(segment) / config_.segment_count);
    const std::int16_t to     = static_cast<std::int16_t>(length * static_cast<std::int32_t>(segment + 1U) / config_.segment_count - config_.segment_gap);
    switch (config_.orientation) {
      case left_to_right : return { static_cast<std::int16_t>(out.left + from), out.top, static_cast<std::int16_t>(out.left + to), out.bottom };
      case right_to_left : return { static_cast<std::int16_t>(out.right - to), out.top, static_cast<std::int16_t>(out.right - from), out.bottom };
      case bottom_to_top : return { out.left, static_cast<std::int16_t>(out.bottom - to), out.right, static_cast<std::int16_t>(out.bottom - from) };
      case top_to_bottom : return { out.left, static_cast<std::int16_t>(out.top + from), out.right, static_cast<std::int16_t>(out.top + to) };
      default            : return { 0, 0, 0, 0 };
    }
  }


  /**
   * Get the color of a lit segment
   * \param segment Segment index
   * \return The color out of its zone or blended between the two closest colors
   */
  color::value_type segment_color(std::uint_fast8_t segment) const
  {
    const std::uint32_t segments = config_.segment_count;
    const std::uint32_t colors   = config_.color_count;
    if (!config_.gradient) {
      // hard zones of equal size
      return config_.colors[segment * colors / segments];
    }
    if ((segments < 2U) || (colors < 2U)) {
      return config_.colors[0];
    }
    // the colors are stops from the first to the last segment, 255 steps from stop to stop
    const std::uint32_t pos  = segment * (colors - 1U) * 255U / (segments - 1U);
    const std::uint32_t stop = pos / 255U;
    if (stop >= colors - 1U) {
      return config_.colors[colors - 1U];
    }
    return color::mix(config_.colors[stop + 1U], config_.colors[stop], static_cast<std::uint8_t>(pos % 255U));
  }


  /**
   * Render the segments in their actual state, lit or dimmed
   * \param from First segment (inclusive)
   * \param to Last segment (exclusive)
   */
  void render(std::uint_fast8_t from, std::uint_fast8_t to)
  {
    if (!config_.colors || !config_.color_count) {
      return;
    }
    const std::uint_fast8_t lit = lit_count(value_);
    for (std::uint_fast8_t n = from; n < to; ++n) {
      const color::value_type color = segment_color(n);
      dc_.set_color(n < lit ? color : color::dim(color, config_.off_lum));
      dc_.box(segment_rect(n));
    }
  }
};


} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_VUMETER_H_
