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
// \brief Spinbox widget, a numeric value with a minus and a plus button
//
// Usage:
// vic::widget::spinbox::config_type _cfg = { { 10, 10, 90, 30 }, 3, &_font, 0, 100, 5, 0,
//                                            vic::color::white, vic::color::black, vic::color::gray04, vic::color::gray10 };
// vic::widget::spinbox sb(_head, _cfg);
// sb.init();
// sb.set_value(50);
//
// Behavior:
// - the minus and plus cells are squares at the left and right end (as wide as the rect is high),
//   event_tap on them changes the value by step, event_long_tap by 10 * step, then the event
//   handler is called
// - the value is shown with 'decimals' decimal places, e.g. 275 with decimals 1 is "27.5"
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_SPINBOX_H_
#define _VIC_WIDGET_SPINBOX_H_

#include "widget.h"
#include "label.h"    // label::format()


namespace vic {
namespace widget {


class spinbox : public base
{
public:
  // configuration
  typedef struct tag_config_type {
    rect_type               rect;           // widget rect
    std::uint8_t            radius;         // corner radius of the buttons, 0 for sharp corners
    const font::font_type*  font;           // value font, must stay valid
    std::int16_t            range_lower;    // lower range
    std::int16_t            range_upper;    // upper range
    std::int16_t            step;           // step per tap
    std::uint8_t            decimals;       // decimal places of the shown value
    color::value_type       color_text;     // value and sign color
    color::value_type       color_bg;       // background of the value cell
    color::value_type       color_button;   // background of the minus / plus cells
    color::value_type       color_border;   // border color, color::none for no border
  } config_type;

private:
  dc            dc_;        // drawing context
  tc            tc_;        // text context
  config_type   config_;    // configuration
  std::int16_t  value_;     // actual value


public:
  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  spinbox(drv& head, const config_type& config)
    : base(head)
    , dc_(head)
    , tc_(dc_)
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
   * Set a new value, only the value cell is redrawn
   * \param value New value, clamped to the valid range
   */
  void set_value(std::int16_t value)
  {
    value = clamp(value);
    if (value != value_) {
      value_ = value;
      render_value();
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
   * Set a new range, the value is clamped to it
   * \param lower Lower range limit
   * \param upper Upper range limit
   */
  void set_range(std::int16_t lower, std::int16_t upper)
  {
    if (lower < upper) {
      config_.range_lower = lower;
      config_.range_upper = upper;
      value_ = clamp(value_);
    }
    redraw();
  }


  /**
   * Set a new step
   * \param step Step per tap
   */
  inline void set_step(std::int16_t step)
  {
    config_.step = step;
  }


  /**
   * Redraw the control
   */
  void redraw()
  {
    const rect_type    r = config_.rect;
    const std::int16_t w = r.height();    // button width
    const rect_type    minus = { r.left, r.top, static_cast<std::int16_t>(r.left + w), r.bottom };
    const rect_type    plus  = { static_cast<std::int16_t>(r.right - w), r.top, r.right, r.bottom };

    dc_.present_lock();
    // buttons
    dc_.set_color(config_.color_button);
    dc_.box(minus, config_.radius);
    dc_.box(plus, config_.radius);
    if (config_.color_border != color::none) {
      dc_.set_color(config_.color_border);
      dc_.rectangle(minus, config_.radius);
      dc_.rectangle(plus, config_.radius);
    }
    // signs, drawn as lines so no font glyphs are needed
    const std::int16_t len = static_cast<std::int16_t>(w / 3);
    const std::int16_t cy  = static_cast<std::int16_t>(r.top + w / 2);
    const std::int16_t cxm = static_cast<std::int16_t>(minus.left + w / 2);
    const std::int16_t cxp = static_cast<std::int16_t>(plus.left + w / 2);
    dc_.set_color(config_.color_text);
    dc_.box({ static_cast<std::int16_t>(cxm - len / 2), static_cast<std::int16_t>(cy - 1), static_cast<std::int16_t>(cxm + len / 2 + 1), static_cast<std::int16_t>(cy + 1) });
    dc_.box({ static_cast<std::int16_t>(cxp - len / 2), static_cast<std::int16_t>(cy - 1), static_cast<std::int16_t>(cxp + len / 2 + 1), static_cast<std::int16_t>(cy + 1) });
    dc_.box({ static_cast<std::int16_t>(cxp - 1), static_cast<std::int16_t>(cy - len / 2), static_cast<std::int16_t>(cxp + 1), static_cast<std::int16_t>(cy + len / 2 + 1) });
    render_value();
    dc_.present_lock(false);
  }


protected:

  virtual bool is_inside(vertex_type vertex) const final
  {
    return config_.rect.contain(vertex);
  }


  /**
   * Taps on the minus / plus cells change the value
   */
  virtual void on_event(vertex_type vertex, event_type ev) final
  {
    if ((ev != event_tap) && (ev != event_long_tap)) {
      return;
    }
    const std::int16_t w    = config_.rect.height();
    const std::int32_t step = static_cast<std::int32_t>(config_.step) * (ev == event_long_tap ? 10 : 1);
    std::int32_t value = value_;
    if (vertex.x < config_.rect.left + w) {
      value -= step;
    }
    else if (vertex.x >= config_.rect.right - w) {
      value += step;
    }
    else {
      return;
    }
    value = value < -32768 ? -32768 : (value > 32767 ? 32767 : value);
    set_value(static_cast<std::int16_t>(value));
  }


private:

  inline std::int16_t clamp(std::int16_t value) const
  {
    return value < config_.range_lower ? config_.range_lower : (value > config_.range_upper ? config_.range_upper : value);
  }


  // render the value cell between the buttons
  void render_value()
  {
    const rect_type    r = config_.rect;
    const std::int16_t w = r.height();
    const rect_type    cell = { static_cast<std::int16_t>(r.left + w + 1), r.top, static_cast<std::int16_t>(r.right - w - 1), r.bottom };

    dc_.present_lock();
    dc_.set_color(config_.color_bg);
    dc_.box(cell);
    if (!!config_.font) {
      char buf[16];
      buf[label::format(buf, value_, config_.decimals)] = 0;
      tc_.set_font(*config_.font);
      tc_.set_color(config_.color_text);
      rect_type ext;
      tc_.get_extend(ext, reinterpret_cast<const std::uint8_t*>(buf));
      tc_.out({ static_cast<std::int16_t>(cell.left + (cell.width()  - ext.width())  / 2),
                static_cast<std::int16_t>(cell.top  + (cell.height() - ext.height()) / 2) },
              reinterpret_cast<const std::uint8_t*>(buf));
    }
    dc_.present_lock(false);
  }
};


} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_SPINBOX_H_
