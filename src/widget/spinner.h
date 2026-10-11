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
// \brief Spinner widget, a ring segment which rotates on every step(), a busy indicator
//
// Usage:
// vic::widget::spinner::config_type _cfg = { { 60, 60 }, 16, 4, 90, 10,
//                                            vic::color::white, vic::color::gray04, vic::color::black };
// vic::widget::spinner s(_head, _cfg);
// s.init();
// s.step();     // call periodically, e.g. every 40 ms
//
// Configuration:
// - sweep is the length of the segment in degrees, step the rotation per step() in degrees
//   (positive is clockwise)
// - color_track is the color of the rest of the ring, use the background color for an invisible
//   track (it is needed to erase the segment)
// - a step draws only the changed sectors, with anti_aliasing_enable() the complete ring is
//   redrawn on every step: the ring and two pixel around it are erased with color_bg first
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_SPINNER_H_
#define _VIC_WIDGET_SPINNER_H_

#include "widget.h"


namespace vic {
namespace widget {


class spinner : public base
{
public:
  // configuration
  typedef struct tag_config_type {
    vertex_type       center;         // center of the ring
    std::uint16_t     radius;         // outer radius of the ring
    std::uint8_t      ring_width;     // width of the ring
    std::uint16_t     sweep;          // length of the segment in degrees, 1 - 359
    std::int16_t      step;           // rotation per step() in degrees, positive is clockwise
    color::value_type color_arc;      // segment color
    color::value_type color_track;    // track color (rest of the ring)
    color::value_type color_bg;       // background around the ring, used to erase the anti aliased edges, color::none for none
  } config_type;

private:
  dc            dc_;        // drawing context
  config_type   config_;    // configuration
  std::int16_t  pos_;       // start angle of the segment in compass degrees, 0 - 359
  bool          aa_;        // anti aliasing, full redraw on every step


public:
  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  spinner(drv& head, const config_type& config)
    : base(head)
    , dc_(head)
    , config_(config)
    , pos_(0)
    , aa_(false)
  {
    if ((config_.sweep == 0U) || (config_.sweep > 359U)) {
      config_.sweep = 90U;
    }
  }


  /**
   * Init: draw the widget
   */
  virtual void init() final
  {
    redraw();
  }


  /**
   * Rotate the segment by the configured step, only the changed sectors are drawn
   */
  void step()
  {
    const std::int32_t s     = config_.step;
    const std::int32_t sweep = config_.sweep;
    const std::int32_t old   = pos_;
    pos_ = static_cast<std::int16_t>((((old + s) % 360) + 360) % 360);
    if (s == 0) {
      return;
    }
    if (aa_) {
      redraw();
      return;
    }
    // old segment [old, old + sweep), new segment [old + s, old + s + sweep): erase old \ new, draw new \ old
    const std::uint16_t ri = inner_radius();
    dc_.present_lock();
    if (s > 0) {
      dc_.set_color(config_.color_track);
      sector_cw(dc_, config_.center, ri, config_.radius, old, old + (s < sweep ? s : sweep));
      dc_.set_color(config_.color_arc);
      sector_cw(dc_, config_.center, ri, config_.radius, old + (s > sweep ? s : sweep), old + s + sweep);
    }
    else {
      dc_.set_color(config_.color_track);
      sector_cw(dc_, config_.center, ri, config_.radius, old + sweep + (s > -sweep ? s : -sweep), old + sweep);
      dc_.set_color(config_.color_arc);
      sector_cw(dc_, config_.center, ri, config_.radius, old + s, old + (s < -sweep ? s + sweep : 0));
    }
    dc_.present_lock(false);
  }


  /**
   * Set the segment start angle
   * \param angle Start angle in compass degrees
   */
  void set_pos(std::int16_t angle)
  {
    pos_ = static_cast<std::int16_t>(((angle % 360) + 360) % 360);
    redraw();
  }


  /**
   * Returns the segment start angle
   * \return Start angle in compass degrees, 0 - 359
   */
  inline std::int16_t get_pos() const
  {
    return pos_;
  }


  /**
   * Enable anti aliasing, the ring is then completely redrawn on every step
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
    const std::uint16_t ri = inner_radius();
    dc_.present_lock();
    erase_edges(dc_, config_.center, ri, config_.radius, aa_, config_.color_bg);
    dc_.set_color(config_.color_track);
    sector_cw(dc_, config_.center, ri, config_.radius, pos_ + config_.sweep, pos_ + 360);
    dc_.set_color(config_.color_arc);
    sector_cw(dc_, config_.center, ri, config_.radius, pos_, pos_ + config_.sweep);
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
};


} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_SPINNER_H_
