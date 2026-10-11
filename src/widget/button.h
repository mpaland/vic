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
// \brief Button widget, a push button with a caption
//
// Usage:
// vic::widget::button::config_type _cfg = { { 10, 10, 70, 30 }, 4, &_font, "OK",
//                                           vic::color::white, vic::color::gray04, vic::color::gray08, vic::color::gray10 };
// vic::widget::button btn(_head, _cfg);
// btn.init();
// btn.set_event_handler(on_button, nullptr);    // called with event_tap when the button is tapped
//
// Behavior:
// - event_down shows the button pressed (color_bg_pressed), event_up releases it, the tap itself
//   is reported to the event handler, the button has no state of its own
// - the caption is centered, nullptr for a blank button
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_BUTTON_H_
#define _VIC_WIDGET_BUTTON_H_

#include "widget.h"


namespace vic {
namespace widget {


class button : public base
{
public:
  // configuration
  typedef struct tag_config_type {
    rect_type               rect;               // button rect
    std::uint8_t            radius;             // corner radius, 0 for sharp corners
    const font::font_type*  font;               // caption font, must stay valid
    const char*             caption;            // caption (UTF-8), nullptr for none, must stay valid
    color::value_type       color_text;         // caption color
    color::value_type       color_bg;           // button color
    color::value_type       color_bg_pressed;   // button color while pressed
    color::value_type       color_border;       // border color, color::none for no border
  } config_type;

private:
  dc            dc_;        // drawing context
  tc            tc_;        // text context
  config_type   config_;    // configuration
  bool          pressed_;   // pressed state


public:
  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  button(drv& head, const config_type& config)
    : base(head)
    , dc_(head)
    , tc_(dc_)
    , config_(config)
    , pressed_(false)
  { }


  /**
   * Init: draw the widget
   */
  virtual void init() final
  {
    redraw();
  }


  /**
   * Set a new caption and redraw
   * \param caption New caption, nullptr for none, must stay valid
   */
  void set_caption(const char* caption)
  {
    config_.caption = caption;
    redraw();
  }


  /**
   * Returns the pressed state
   * \return True while the button is pressed (between event_down and event_up)
   */
  inline bool is_pressed() const
  {
    return pressed_;
  }


  /**
   * Redraw the control
   */
  void redraw()
  {
    dc_.present_lock();
    dc_.set_color(pressed_ ? config_.color_bg_pressed : config_.color_bg);
    dc_.box(config_.rect, config_.radius);
    if (config_.color_border != color::none) {
      dc_.set_color(config_.color_border);
      dc_.rectangle(config_.rect, config_.radius);
    }
    if (!!config_.caption && !!config_.font) {
      tc_.set_font(*config_.font);
      tc_.set_color(config_.color_text);
      rect_type ext;
      tc_.get_extend(ext, reinterpret_cast<const std::uint8_t*>(config_.caption));
      tc_.out({ static_cast<std::int16_t>(config_.rect.left + (config_.rect.width()  - ext.width())  / 2),
                static_cast<std::int16_t>(config_.rect.top  + (config_.rect.height() - ext.height()) / 2) },
              reinterpret_cast<const std::uint8_t*>(config_.caption));
    }
    dc_.present_lock(false);
  }


protected:

  virtual bool is_inside(vertex_type vertex) const final
  {
    return config_.rect.contain(vertex);
  }


  /**
   * Pressed look on down, released on up
   */
  virtual void on_event(vertex_type vertex, event_type ev) final
  {
    (void)vertex;
    if ((ev == event_down) && !pressed_) {
      pressed_ = true;
      redraw();
    }
    else if ((ev == event_up) && pressed_) {
      pressed_ = false;
      redraw();
    }
  }
};


} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_BUTTON_H_
