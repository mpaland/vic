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
// \brief Bar gauge widget: a linear scale strip with colored bands, ticks, an optional
//        fill bar and up to two triangle markers (pointers) beside the strip
//
// Usage:
// static const vic::widget::band_type _bands[] = { { 0, 10, vic::color::red }, { 10, 20, vic::color::yellow }, { 20, 50, vic::color::green } };
// static const vic::widget::bargauge<>::marker_type _markers[] = { { vic::color::white, vic::widget::bargauge<>::side_before, 4 } };
// vic::widget::bargauge<>::config_type _cfg = { { 20, 100 }, 14, 80, vic::widget::bargauge<>::bottom_to_top, 4,
//                                               0, 50, _bands, 3, 6, _markers, 1,
//                                               vic::color::none, vic::color::gray08, vic::color::white, vic::color::black };
// vic::widget::bargauge<> qty(_head, _cfg);
// qty.init();
// qty.set_value(0, 23);     // move marker 0 to 23
//
// Configuration:
// - origin is the left/top corner, x_size * y_size is the complete widget including the marker lanes
// - the strip (bar_width wide) is centered across the orientation axis, the markers live in the
//   lanes beside it: side_before is left of a vertical / above a horizontal strip, side_after is
//   right / below; marker size is the triangle height (3 - 7 pixel)
// - color_fill: color::none shows the band strip only, any other color fills the strip from
//   the range_lower end up to the value of marker 0 (bar graph style), the bands show in the
//   rest of the strip; the fill is delta drawn
// - the markers are canvas sprites, moving a marker costs only its pixels
// - color_bg: the marker lanes are cleared with it, color::none leaves the background untouched
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_BARGAUGE_H_
#define _VIC_WIDGET_BARGAUGE_H_

#include "widget.h"
#include "../sprite.h"


// maximum number of markers per bar gauge
#define VIC_WIDGET_BARGAUGE_MAX_MARKERS   2U


namespace vic {
namespace widget {


/**
 * Bar gauge widget
 * \param Marker_Pixels Maximum pixels of one marker sprite, a marker of size s needs s * s pixels
 */
template <std::uint16_t Marker_Pixels = 32U>
class bargauge : public base
{
public:
  // orientation of the scale (direction of increasing values)
  typedef enum tag_orientation_type {
    left_to_right = 0,
    right_to_left,
    bottom_to_top,
    top_to_bottom
  } orientation_type;

  // side of the strip where a marker is placed
  typedef enum tag_marker_side_type {
    side_before = 0,    // left of a vertical strip, above a horizontal strip
    side_after          // right of a vertical strip, below a horizontal strip
  } marker_side_type;

  // marker (pointer) definition
  typedef struct tag_marker_type {
    color::value_type color;    // marker color
    marker_side_type  side;     // marker side
    std::uint8_t      size;     // triangle height in pixel (tip to base), 3 - 7
  } marker_type;

  // configuration
  typedef struct tag_config_type {
    vertex_type         origin;         // origin (left/top)
    std::uint16_t       x_size;         // x size of the widget
    std::uint16_t       y_size;         // y size of the widget
    orientation_type    orientation;    // orientation of the scale
    std::uint8_t        bar_width;      // width of the strip across the orientation axis
    std::int16_t        range_lower;    // lower range, e.g. 0
    std::int16_t        range_upper;    // upper range, e.g. 100
    const band_type*    bands;          // colored bands, nullptr for none, must stay valid
    std::uint8_t        band_count;     // number of bands
    std::uint8_t        tick_count;     // number of ticks including both ends, 0 for none
    const marker_type*  markers;        // marker definitions, must stay valid
    std::uint8_t        marker_count;   // number of markers, 0 - VIC_WIDGET_BARGAUGE_MAX_MARKERS
    color::value_type   color_fill;     // fill color (bar graph style), color::none for a band strip only
    color::value_type   color_bar;      // strip color where no band / fill is
    color::value_type   color_tick;     // tick color
    color::value_type   color_bg;       // background color of the marker lanes, color::none to leave it untouched
  } config_type;

private:
  dc                                dc_;                                          // drawing context
  sprite::canvas<Marker_Pixels, 1U> marker_[VIC_WIDGET_BARGAUGE_MAX_MARKERS];     // marker sprites
  config_type                       config_;                                      // configuration
  std::int16_t                      value_[VIC_WIDGET_BARGAUGE_MAX_MARKERS];      // marker values
  bool                              shown_;                                       // markers are rendered


public:
  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  bargauge(drv& head, const config_type& config)
    : base(head)
    , dc_(head)
    , marker_{ { head, 0 }, { head, 0 } }
    , config_(config)
    , shown_(false)
  {
    if (config_.marker_count > VIC_WIDGET_BARGAUGE_MAX_MARKERS) {
      config_.marker_count = VIC_WIDGET_BARGAUGE_MAX_MARKERS;
    }
    for (std::uint_fast8_t m = 0U; m < VIC_WIDGET_BARGAUGE_MAX_MARKERS; ++m) {
      value_[m] = config.range_lower;
    }
  }


  /**
   * Init: create the marker patterns and draw the widget
   */
  virtual void init() final
  {
    for (std::uint_fast8_t m = 0U; m < config_.marker_count; ++m) {
      draw_marker(m);
    }
    redraw();
  }


  /**
   * Set a new value for a marker, the marker is moved, in fill mode the bar of marker 0 is updated
   * \param marker Marker index
   * \param value New value, clamped to the valid range
   */
  void set_value(std::uint8_t marker, std::int16_t value)
  {
    if (marker >= config_.marker_count) {
      return;
    }
    value = clamp(value);
    if (shown_ && (value == value_[marker])) {
      return;
    }
    const std::int16_t old = value_[marker];
    value_[marker] = value;

    if ((marker == 0U) && (config_.color_fill != color::none)) {
      // fill delta: the strip and the lanes don't overlap, the markers don't need to be hidden
      const std::int16_t length  = bar_length();
      const std::int16_t bar_old = bar_px(old, length);
      const std::int16_t bar_new = bar_px(value, length);
      if (bar_old != bar_new) {
        dc_.present_lock();
        render_strip(bar_old < bar_new ? bar_old : bar_new, bar_old < bar_new ? bar_new : bar_old);
        dc_.present_lock(false);
      }
    }
    marker_[marker].render(marker_pos(marker), 0U);
  }


  /**
   * Returns the actual value of a marker
   * \param marker Marker index
   * \return Actual value
   */
  inline std::int16_t get_value(std::uint8_t marker = 0U) const
  {
    return marker < VIC_WIDGET_BARGAUGE_MAX_MARKERS ? value_[marker] : config_.range_lower;
  }


  /**
   * Set a new range, all values are set to range_lower
   * \param lower Lower range limit
   * \param upper Upper range limit
   */
  void set_range(std::int16_t lower, std::int16_t upper)
  {
    if (lower < upper) {
      config_.range_lower = lower;
      config_.range_upper = upper;
      for (std::uint_fast8_t m = 0U; m < VIC_WIDGET_BARGAUGE_MAX_MARKERS; ++m) {
        value_[m] = lower;
      }
    }
    redraw();
  }


  /**
   * Redraw the control
   */
  void redraw()
  {
    // hide the markers, the lanes below are redrawn
    if (shown_) {
      for (std::uint_fast8_t m = 0U; m < config_.marker_count; ++m) {
        marker_[m].render(marker_[m].get_position(), 0xFFFFU);
      }
      shown_ = false;
    }

    dc_.present_lock();
    if (config_.color_bg != color::none) {
      dc_.set_color(config_.color_bg);
      dc_.box(outer_rect());
    }
    render_strip(0, bar_length());
    dc_.present_lock(false);

    for (std::uint_fast8_t m = 0U; m < config_.marker_count; ++m) {
      marker_[m].render(marker_pos(m), 0U);
    }
    shown_ = true;
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


  // the strip rect, centered across the orientation axis
  rect_type strip_rect() const
  {
    const rect_type out = outer_rect();
    if (horizontal()) {
      const std::int16_t top = static_cast<std::int16_t>(out.top + (out.height() - config_.bar_width) / 2);
      return { out.left, top, out.right, static_cast<std::int16_t>(top + config_.bar_width) };
    }
    const std::int16_t left = static_cast<std::int16_t>(out.left + (out.width() - config_.bar_width) / 2);
    return { left, out.top, static_cast<std::int16_t>(left + config_.bar_width), out.bottom };
  }


  // length of the strip along the orientation axis in pixel
  inline std::int16_t bar_length() const
  {
    const rect_type s = strip_rect();
    return horizontal() ? s.width() : s.height();
  }


  // clamp a value to the valid range
  inline std::int16_t clamp(std::int16_t value) const
  {
    return value < config_.range_lower ? config_.range_lower : (value > config_.range_upper ? config_.range_upper : value);
  }


  /**
   * Convert a value to a pixel length along the strip
   * \param value Value within the range
   * \param length Scale length in pixel
   * \return 0 to length
   */
  std::int16_t bar_px(std::int16_t value, std::int16_t length) const
  {
    const std::int32_t range = static_cast<std::int32_t>(config_.range_upper) - config_.range_lower;
    if (range <= 0) {
      return 0;
    }
    std::int32_t px = (static_cast<std::int32_t>(length) * (static_cast<std::int32_t>(value) - config_.range_lower) + range / 2) / range;
    px = px < 0 ? 0 : (px > length ? length : px);
    return static_cast<std::int16_t>(px);
  }


  /**
   * Get the strip rect between two lengths along the orientation axis (exclusive end)
   * \param from Start length in pixel (smaller)
   * \param to End length in pixel (bigger)
   * \return The rect
   */
  rect_type bar_rect(std::int16_t from, std::int16_t to) const
  {
    const rect_type s = strip_rect();
    switch (config_.orientation) {
      case left_to_right : return { static_cast<std::int16_t>(s.left + from), s.top, static_cast<std::int16_t>(s.left + to), s.bottom };
      case right_to_left : return { static_cast<std::int16_t>(s.right - to), s.top, static_cast<std::int16_t>(s.right - from), s.bottom };
      case bottom_to_top : return { s.left, static_cast<std::int16_t>(s.bottom - to), s.right, static_cast<std::int16_t>(s.bottom - from) };
      case top_to_bottom : return { s.left, static_cast<std::int16_t>(s.top + from), s.right, static_cast<std::int16_t>(s.top + to) };
      default            : return { 0, 0, 0, 0 };
    }
  }


  /**
   * Render the strip (base color, bands, fill and ticks) between two lengths
   * \param from Start length in pixel (inclusive)
   * \param to End length in pixel (exclusive)
   */
  void render_strip(std::int16_t from, std::int16_t to)
  {
    const std::int16_t length = bar_length();
    if (from >= to) {
      return;
    }

    // base
    dc_.set_color(config_.color_bar);
    dc_.box(bar_rect(from, to));

    // bands
    for (std::uint_fast8_t n = 0U; (n < config_.band_count) && !!config_.bands; ++n) {
      const std::int16_t b0 = bar_px(clamp(config_.bands[n].from), length);
      const std::int16_t b1 = bar_px(clamp(config_.bands[n].to),   length);
      const std::int16_t p0 = b0 > from ? b0 : from;
      const std::int16_t p1 = b1 < to   ? b1 : to;
      if (p0 < p1) {
        dc_.set_color(config_.bands[n].color);
        dc_.box(bar_rect(p0, p1));
      }
    }

    // fill
    if (config_.color_fill != color::none) {
      const std::int16_t f1 = bar_px(value_[0], length);
      const std::int16_t p1 = f1 < to ? f1 : to;
      if (from < p1) {
        dc_.set_color(config_.color_fill);
        dc_.box(bar_rect(from, p1));
      }
    }

    // ticks
    if (config_.tick_count > 1U) {
      dc_.set_color(config_.color_tick);
      for (std::uint_fast8_t n = 0U; n < config_.tick_count; ++n) {
        const std::int16_t t = static_cast<std::int16_t>(static_cast<std::int32_t>(length - 1) * static_cast<std::int32_t>(n) / static_cast<std::int32_t>(config_.tick_count - 1U));
        if ((t >= from) && (t < to)) {
          dc_.box(bar_rect(t, static_cast<std::int16_t>(t + 1)));
        }
      }
    }
  }


  /**
   * Draw the marker pattern (a solid triangle pointing to the strip, tip at the sprite origin)
   * \param m Marker index
   */
  void draw_marker(std::uint_fast8_t m)
  {
    const marker_type& mk = config_.markers[m];
    const std::int16_t dir = (mk.side == side_before) ? -1 : 1;
    marker_[m].cls();
    marker_[m].set_color(mk.color);
    for (std::int16_t i = 0; i < static_cast<std::int16_t>(mk.size); ++i) {
      const std::int16_t d = static_cast<std::int16_t>(i * dir);   // distance from the tip, away from the strip
      if (horizontal()) {
        marker_[m].box({ static_cast<std::int16_t>(-i), d, static_cast<std::int16_t>(i + 1), static_cast<std::int16_t>(d + 1) });
      }
      else {
        marker_[m].box({ d, static_cast<std::int16_t>(-i), static_cast<std::int16_t>(d + 1), static_cast<std::int16_t>(i + 1) });
      }
    }
  }


  /**
   * Get the screen position of the marker tip for its actual value
   * \param m Marker index
   * \return Tip position
   */
  vertex_type marker_pos(std::uint_fast8_t m) const
  {
    const rect_type    s      = strip_rect();
    const std::int16_t length = bar_length();
    // the tip points at the pixel row/column of the value, 0 .. length - 1
    const std::int16_t px     = bar_px(value_[m], static_cast<std::int16_t>(length > 0 ? length - 1 : 0));
    const bool         before = config_.markers[m].side == side_before;
    switch (config_.orientation) {
      case left_to_right : return { static_cast<std::int16_t>(s.left + px),      static_cast<std::int16_t>(before ? s.top - 1 : s.bottom) };
      case right_to_left : return { static_cast<std::int16_t>(s.right - 1 - px), static_cast<std::int16_t>(before ? s.top - 1 : s.bottom) };
      case bottom_to_top : return { static_cast<std::int16_t>(before ? s.left - 1 : s.right), static_cast<std::int16_t>(s.bottom - 1 - px) };
      case top_to_bottom : return { static_cast<std::int16_t>(before ? s.left - 1 : s.right), static_cast<std::int16_t>(s.top + px) };
      default            : return { 0, 0 };
    }
  }
};


} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_BARGAUGE_H_
