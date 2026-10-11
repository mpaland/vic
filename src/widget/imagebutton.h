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
// \brief Image button widget, an XPM icon with a caption below, a soft key or icon label
//
// Usage:
// vic::widget::imagebutton::config_type _cfg = { { 10, 10, 50, 50 }, 3, xpm_icon, &_font, "MAP",
//                                                vic::color::white, vic::color::black, vic::color::gray06, vic::color::darkblue, vic::color::gray08 };
// vic::widget::imagebutton ib(_head, _cfg);
// ib.init();
// ib.set_active(true);    // selected / latched look
//
// Behavior:
// - event_down shows the button pressed, event_up releases it, the tap is reported to the event handler
// - set_active() switches the latched look (color_bg_active), e.g. for the selected page of a soft key bar
// - without an icon it is a plain caption key, without a caption a plain icon, the icon is centered
//   horizontally, the caption below it (or centered without icon)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_IMAGEBUTTON_H_
#define _VIC_WIDGET_IMAGEBUTTON_H_

#include "widget.h"
#include "bitmap.h"


namespace vic {
namespace widget {


class imagebutton : public base
{
public:
  // configuration
  typedef struct tag_config_type {
    rect_type               rect;               // button rect
    std::uint8_t            radius;             // corner radius, 0 for sharp corners
    const char**            xpm;                // icon in XPM3 format, nullptr for none, must stay valid
    const font::font_type*  font;               // caption font, must stay valid
    const char*             caption;            // caption (UTF-8), nullptr for none, must stay valid
    color::value_type       color_text;         // caption color
    color::value_type       color_bg;           // button color
    color::value_type       color_bg_pressed;   // button color while pressed
    color::value_type       color_bg_active;    // button color in active (latched) state
    color::value_type       color_border;       // border color, color::none for no border
  } config_type;

private:
  dc            dc_;        // drawing context
  tc            tc_;        // text context
  bitmap        icon_;      // icon renderer, not part of the event routing
  config_type   config_;    // configuration
  bool          pressed_;   // pressed state
  bool          active_;    // latched state


public:
  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  imagebutton(drv& head, const config_type& config)
    : base(head)
    , dc_(head)
    , tc_(dc_)
    , icon_(head)
    , config_(config)
    , pressed_(false)
    , active_(false)
  {
    icon_.event_enable(false);
  }


  /**
   * Init: draw the widget
   */
  virtual void init() final
  {
    redraw();
  }


  /**
   * Set the active (latched) state, redrawn on change
   * \param active True for the active look
   */
  void set_active(bool active)
  {
    if (active != active_) {
      active_ = active;
      redraw();
    }
  }


  /**
   * Returns the active state
   * \return True if active
   */
  inline bool is_active() const
  {
    return active_;
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
   * Set a new caption and redraw
   * \param caption New caption, nullptr for none, must stay valid
   */
  void set_caption(const char* caption)
  {
    config_.caption = caption;
    redraw();
  }


  /**
   * Redraw the control
   */
  void redraw()
  {
    const rect_type r = config_.rect;
    dc_.present_lock();
    dc_.set_color(pressed_ ? config_.color_bg_pressed : (active_ ? config_.color_bg_active : config_.color_bg));
    dc_.box(r, config_.radius);
    if (config_.color_border != color::none) {
      dc_.set_color(config_.color_border);
      dc_.rectangle(r, config_.radius);
    }

    // icon and caption are stacked and centered as a block
    std::int16_t icon_w = 0, icon_h = 0, text_w = 0, text_h = 0;
    rect_type ext;
    if (!!config_.xpm) {
      xpm_size(config_.xpm[0], icon_w, icon_h);
    }
    if (!!config_.caption && !!config_.font) {
      tc_.set_font(*config_.font);
      tc_.get_extend(ext, reinterpret_cast<const std::uint8_t*>(config_.caption));
      text_w = ext.width();
      text_h = ext.height();
    }
    const std::int16_t gap   = ((icon_h > 0) && (text_h > 0)) ? 2 : 0;
    std::int16_t       y     = static_cast<std::int16_t>(r.top + (r.height() - icon_h - gap - text_h) / 2);
    if (icon_h > 0) {
      icon_.render_xpm({ static_cast<std::int16_t>(r.left + (r.width() - icon_w) / 2), y }, config_.xpm);
      y = static_cast<std::int16_t>(y + icon_h + gap);
    }
    if (text_h > 0) {
      tc_.set_color(config_.color_text);
      tc_.out({ static_cast<std::int16_t>(r.left + (r.width() - text_w) / 2), y }, reinterpret_cast<const std::uint8_t*>(config_.caption));
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


private:

  // read width and height out of the XPM header line "w h colors cpp"
  static void xpm_size(const char* header, std::int16_t& w, std::int16_t& h)
  {
    std::int16_t v[2] = { 0, 0 };
    std::size_t  n = 0U;
    for (const char* p = header; (*p != 0) && (n < 2U); ) {
      while (*p == ' ') { ++p; }
      std::int16_t val = 0;
      bool digit = false;
      while ((*p >= '0') && (*p <= '9')) {
        val = static_cast<std::int16_t>(val * 10 + (*p - '0'));
        digit = true;
        ++p;
      }
      if (!digit) {
        break;
      }
      v[n++] = val;
    }
    w = v[0];
    h = v[1];
  }
};


} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_IMAGEBUTTON_H_
