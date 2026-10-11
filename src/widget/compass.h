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
// \brief Compass widget, a heading up compass rose: the rose rotates below a fixed pointer
//
// Usage:
// vic::widget::compass::config_type _cfg = { { 60, 60 }, 40, &_font,
//                                            vic::color::black, vic::color::gray08, vic::color::white, vic::color::white, vic::color::magenta };
// vic::widget::compass cp(_head, _cfg);
// cp.init();
// cp.set_heading(215);
//
// Configuration:
// - the rose has ticks every 10 degree (longer every 30, longest every 90) and the cardinal letters
//   N E S W (if a font is set) inside the ring, the pointer is a fixed triangle at the top
// - the heading is the direction at the pointer, the rose turns counter clockwise when it increases
// - every heading change redraws the whole disc (within a present lock), so there is no sprite RAM,
//   but it costs the disc pixels on slow displays
// - anti_aliasing_enable() smooths the ring, the ticks and the pointer
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_COMPASS_H_
#define _VIC_WIDGET_COMPASS_H_

#include "widget.h"


namespace vic {
namespace widget {


class compass : public base
{
public:
  // configuration
  typedef struct tag_config_type {
    vertex_type             center;         // center of the rose
    std::uint16_t           radius;         // outer radius of the rose
    const font::font_type*  font;           // font of the cardinal letters, nullptr for no letters
    color::value_type       color_bg;       // disc background
    color::value_type       color_ring;     // outer ring
    color::value_type       color_tick;     // ticks
    color::value_type       color_letter;   // cardinal letters
    color::value_type       color_pointer;  // heading pointer
  } config_type;

private:
  dc            dc_;        // drawing context
  tc            tc_;        // text context
  config_type   config_;    // configuration
  std::int16_t  heading_;   // heading in degree, 0 - 359


public:
  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  compass(drv& head, const config_type& config)
    : base(head)
    , dc_(head)
    , tc_(dc_)
    , config_(config)
    , heading_(0)
  { }


  /**
   * Init: draw the widget
   */
  virtual void init() final
  {
    redraw();
  }


  /**
   * Set a new heading, the rose is redrawn on change
   * \param heading Heading in degree, any value, normalized to 0 - 359
   */
  void set_heading(std::int16_t heading)
  {
    heading = static_cast<std::int16_t>(((heading % 360) + 360) % 360);
    if (heading != heading_) {
      heading_ = heading;
      redraw();
    }
  }


  /**
   * Returns the heading
   * \return Heading in degree, 0 - 359
   */
  inline std::int16_t get_heading() const
  {
    return heading_;
  }


  /**
   * Enable anti aliasing for the ring, the ticks and the pointer
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
    const vertex_type  c = config_.center;
    const std::int16_t r = static_cast<std::int16_t>(config_.radius);

    dc_.present_lock();
    // disc and ring
    dc_.set_color(config_.color_bg);
    dc_.disc(c, config_.radius);
    dc_.set_color(config_.color_ring);
    dc_.circle(c, config_.radius);

    // ticks every 10 degree, the rose is rotated by -heading (heading up)
    dc_.set_color(config_.color_tick);
    for (std::int16_t a = 0; a < 360; a = static_cast<std::int16_t>(a + 10)) {
      const std::int16_t len   = (a % 90 == 0) ? static_cast<std::int16_t>(r / 5) : ((a % 30 == 0) ? static_cast<std::int16_t>(r / 8) : static_cast<std::int16_t>(r / 14));
      const std::int16_t angle = static_cast<std::int16_t>(a - heading_);
      dc_.line(util::vertex_rotate({ c.x, static_cast<std::int16_t>(c.y - r + 2) },       c, angle),
               util::vertex_rotate({ c.x, static_cast<std::int16_t>(c.y - r + 2 + len) }, c, angle));
    }

    // cardinal letters inside the ticks
    if (!!config_.font) {
      static const char* const letters[4] = { "N", "E", "S", "W" };
      tc_.set_font(*config_.font);
      tc_.set_color(config_.color_letter);
      const std::int16_t lr = static_cast<std::int16_t>(r - r / 5 - 2 - config_.font->info().ysize / 2);
      for (std::int16_t n = 0; n < 4; ++n) {
        const vertex_type p = util::vertex_rotate({ c.x, static_cast<std::int16_t>(c.y - lr) }, c, static_cast<std::int16_t>(n * 90 - heading_));
        rect_type ext;
        tc_.get_extend(ext, reinterpret_cast<const std::uint8_t*>(letters[n]));
        tc_.out({ static_cast<std::int16_t>(p.x - ext.width() / 2), static_cast<std::int16_t>(p.y - ext.height() / 2) },
                reinterpret_cast<const std::uint8_t*>(letters[n]));
      }
    }

    // fixed pointer at the top and the center dot
    dc_.set_color(config_.color_pointer);
    dc_.triangle_solid({ static_cast<std::int16_t>(c.x - 4), static_cast<std::int16_t>(c.y - r - 1) },
                       { static_cast<std::int16_t>(c.x + 4), static_cast<std::int16_t>(c.y - r - 1) },
                       { c.x, static_cast<std::int16_t>(c.y - r + 6) });
    dc_.disc(c, 2U);
    dc_.present_lock(false);
  }


protected:

  virtual bool is_inside(vertex_type vertex) const final
  {
    return util::distance_squared(vertex, config_.center) <= static_cast<std::uint32_t>(config_.radius) * config_.radius;
  }
};


} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_COMPASS_H_
