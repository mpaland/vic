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
// \brief Round gauge widget: an arc scale with colored bands, major ticks and a needle
//
// Usage:
// static const vic::widget::band_type _bands[] = { { 800, 2500, vic::color::green },
//                                                   { 2500, 2700, vic::color::yellow },
//                                                   { 2700, 3000, vic::color::red } };
// vic::widget::gauge<>::config_type _cfg = { { 60, 50 }, 34, 6, -120, 120, 0, 3000,
//                                            _bands, 3, 7, 28, 5,
//                                            vic::color::gray08, vic::color::white, vic::color::white,
//                                            vic::color::gray12, vic::color::black };
// vic::widget::gauge<> rpm(_head, _cfg);
// rpm.init();
// rpm.set_value(2100);
//
// Configuration:
// - angles are "compass" degrees: 0 is 12 o'clock, positive is clockwise, so a scale from the
//   lower left to the lower right over the top is angle_start = -120, angle_end = 120
// - the ring spans from radius - ring_width to radius, bands are drawn on the ring
// - tick_count major ticks are spread evenly from angle_start to angle_end (0 or 1 for none)
// - the needle is a canvas sprite (Needle_Pixels is its pattern size), it restores the
//   background by itself, so a value change costs only the needle pixels
// - anti_aliasing_enable() switches anti aliasing on for the scale, the needle is always anti aliased
// - color_bg: the disc inside the ring is filled with it, color::none leaves it untouched
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_GAUGE_H_
#define _VIC_WIDGET_GAUGE_H_

#include "widget.h"
#include "../sprite.h"


namespace vic {
namespace widget {


/**
 * Round gauge widget
 * \param Needle_Pixels Maximum pixels of the needle sprite, an anti aliased needle of length L and width W needs about L * (W + 3) pixels
 */
template <std::uint16_t Needle_Pixels = 256U>
class gauge : public base
{
public:
  // configuration
  typedef struct tag_config_type {
    vertex_type       center;         // center of the gauge
    std::uint16_t     radius;         // outer radius of the ring
    std::uint8_t      ring_width;     // width of the ring (scale)
    std::int16_t      angle_start;    // angle of range_lower in compass degrees (0 = 12 o'clock, clockwise)
    std::int16_t      angle_end;      // angle of range_upper, must be greater than angle_start
    std::int16_t      range_lower;    // lower range, e.g. 0
    std::int16_t      range_upper;    // upper range, e.g. 100
    const band_type*  bands;          // colored bands on the ring, nullptr for none, must stay valid
    std::uint8_t      band_count;     // number of bands
    std::uint8_t      tick_count;     // number of major ticks including both ends, 0 for none
    std::uint8_t      needle_length;  // needle length in pixel from the center
    std::uint8_t      needle_width;   // needle width at the hub in pixel (tapers to the tip), 2 - 8
    color::value_type color_ring;     // ring color where no band is
    color::value_type color_tick;     // tick color
    color::value_type color_needle;   // needle color
    color::value_type color_hub;      // hub (needle center) color, color::none for no hub
    color::value_type color_bg;       // background disc color, color::none to leave the background untouched
  } config_type;

private:
  dc                                dc_;        // drawing context
  sprite::canvas<Needle_Pixels, 1U> needle_;    // needle sprite
  config_type                       config_;    // configuration
  std::int16_t                      value_;     // actual value
  bool                              shown_;     // needle is rendered


public:
  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  gauge(drv& head, const config_type& config)
    : base(head)
    , dc_(head)
    , needle_(head, 0)
    , config_(config)
    , value_(config.range_lower)
    , shown_(false)
  { }


  /**
   * Init: draw the widget
   */
  virtual void init() final
  {
    redraw();
  }


  /**
   * Set a new value and move the needle
   * \param value New value, clamped to the valid range
   */
  void set_value(std::int16_t value)
  {
    value = clamp(value);
    if (shown_ && (value == value_)) {
      return;
    }
    value_ = value;
    render_needle();
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
   * Enable anti aliasing for the scale (ring, bands and ticks), the needle is always anti aliased.
   * The scale is redrawn if the widget is already initialized. Like dc::anti_aliasing_enable() this
   * needs a head which can read its pixels back or a background color set in the drawing context.
   * \param enable True to enable anti aliasing, default is off
   */
  void anti_aliasing_enable(bool enable = true)
  {
    dc_.anti_aliasing_enable(enable);
    if (shown_) {
      redraw();
    }
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
   * Redraw the control (scale and needle)
   */
  void redraw()
  {
    const vertex_type   c  = config_.center;
    const std::uint16_t ro = config_.radius;
    const std::uint16_t ri = static_cast<std::uint16_t>(ro > config_.ring_width ? ro - config_.ring_width : 0U);

    // hide the needle, the background below is redrawn
    if (shown_) {
      needle_.render(c, 0xFFFFU);
      shown_ = false;
    }

    dc_.present_lock();
    // background
    if (config_.color_bg != color::none) {
      dc_.set_color(config_.color_bg);
      dc_.disc(c, ro);
    }
    // ring
    dc_.set_color(config_.color_ring);
    draw_sector(config_.angle_start, config_.angle_end, ri, ro);
    // bands
    for (std::uint_fast8_t n = 0U; (n < config_.band_count) && !!config_.bands; ++n) {
      const std::int16_t a0 = value_to_angle(clamp(config_.bands[n].from));
      const std::int16_t a1 = value_to_angle(clamp(config_.bands[n].to));
      if (a1 > a0) {
        dc_.set_color(config_.bands[n].color);
        draw_sector(a0, a1, ri, ro);
      }
    }
    // ticks
    if (config_.tick_count > 1U) {
      dc_.set_color(config_.color_tick);
      const std::int16_t tick_in  = static_cast<std::int16_t>(ri > 2U ? ri - 2U : 0U);
      const std::int16_t tick_out = static_cast<std::int16_t>(ro + 1U);
      for (std::uint_fast8_t n = 0U; n < config_.tick_count; ++n) {
        const std::int16_t a = static_cast<std::int16_t>(config_.angle_start + static_cast<std::int32_t>(config_.angle_end - config_.angle_start) * static_cast<std::int32_t>(n) / static_cast<std::int32_t>(config_.tick_count - 1U));
        dc_.line(util::vertex_rotate({ c.x, static_cast<std::int16_t>(c.y - tick_in) },  c, a),
                 util::vertex_rotate({ c.x, static_cast<std::int16_t>(c.y - tick_out) }, c, a));
      }
    }
    dc_.present_lock(false);

    render_needle();
  }


protected:

  /**
   * Check if the vertex is inside the widget (the gauge disc)
   * \param vertex Vertex to check
   * \return true if vertex is inside
   */
  virtual bool is_inside(vertex_type vertex) const final
  {
    return util::distance_squared(vertex, config_.center) <= static_cast<std::uint32_t>(config_.radius) * config_.radius;
  }


private:

  // clamp a value to the valid range
  inline std::int16_t clamp(std::int16_t value) const
  {
    return value < config_.range_lower ? config_.range_lower : (value > config_.range_upper ? config_.range_upper : value);
  }


  /**
   * Convert a value to the compass angle on the scale
   * \param value Value within the range
   * \return Angle in compass degrees
   */
  inline std::int16_t value_to_angle(std::int16_t value) const
  {
    return widget::value_to_angle(value, config_.range_lower, config_.range_upper, config_.angle_start, config_.angle_end);
  }


  /**
   * Draw a ring sector from compass angle a0 to a1 (clockwise)
   */
  inline void draw_sector(std::int16_t a0, std::int16_t a1, std::uint16_t ri, std::uint16_t ro)
  {
    sector_cw(dc_, config_.center, ri, ro, a0, a1);
  }


  /**
   * Draw the needle for the actual value into the sprite and render it
   */
  void render_needle()
  {
    const std::int16_t a   = value_to_angle(value_);
    const std::int16_t len = static_cast<std::int16_t>(config_.needle_length);

    // tapered needle: base of needle_width behind the center, pointed tip
    const std::int16_t hw  = static_cast<std::int16_t>(config_.needle_width > 1U ? (config_.needle_width + 1U) / 2U : 1U);

    needle_.cls();
    needle_.anti_aliasing_enable(true);
    needle_.set_color(config_.color_needle);
    needle_.triangle_solid(util::vertex_rotate({ static_cast<std::int16_t>(-hw), 4 }, { 0, 0 }, a),
                           util::vertex_rotate({ hw, 4 }, { 0, 0 }, a),
                           util::vertex_rotate({ 0, static_cast<std::int16_t>(-len) }, { 0, 0 }, a));
    if (config_.color_hub != color::none) {
      needle_.set_color(config_.color_hub);
      needle_.disc({ 0, 0 }, static_cast<std::uint16_t>(hw + 1));
    }
    needle_.render(config_.center, 0U);
    shown_ = true;
  }
};


} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_GAUGE_H_
