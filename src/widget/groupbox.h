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
// \brief Group box widget, a (rounded) frame with a title cut into the top border
//
// Usage:
// vic::font::Montserrat_10_aa2 _font;
// vic::widget::groupbox::config_type _cfg = { { 2, 84, 150, 196 }, 4, &_font, "FUEL",
//                                             vic::color::gray, vic::color::white, vic::color::black };
// vic::widget::groupbox grp(_head, _cfg);
// grp.init();
//
// Configuration:
// - rect is the frame rect (right/bottom exclusive), the title is centered on the top border line
// - the title area is cleared with color_bg, color::none leaves the background (and the border
//   under the title) untouched
// - the group box is static, draw it before the widgets inside it
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_GROUPBOX_H_
#define _VIC_WIDGET_GROUPBOX_H_

#include "widget.h"


namespace vic {
namespace widget {


class groupbox : public base
{
public:
  // configuration
  typedef struct tag_config_type {
    rect_type               rect;           // frame rect
    std::uint8_t            radius;         // corner radius, 0 for sharp corners
    const font::font_type*  font;           // title font, must stay valid during the lifetime of the widget
    const char*             title;          // title text (UTF-8), nullptr for no title, must stay valid
    color::value_type       color_border;   // frame color
    color::value_type       color_title;    // title text color
    color::value_type       color_bg;       // background color of the frame interior, color::none to leave it untouched
  } config_type;

private:
  dc            dc_;        // drawing context
  tc            tc_;        // text context
  config_type   config_;    // configuration


public:
  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  groupbox(drv& head, const config_type& config)
    : base(head)
    , dc_(head)
    , tc_(dc_)
    , config_(config)
  { }


  /**
   * Init: draw the widget
   */
  virtual void init() final
  {
    redraw();
  }


  /**
   * Set a new title and redraw
   * \param title New title, nullptr for none, must stay valid
   */
  void set_title(const char* title)
  {
    config_.title = title;
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
      dc_.box(config_.rect, config_.radius);
    }
    dc_.set_color(config_.color_border);
    dc_.rectangle(config_.rect, config_.radius);

    if (!!config_.title && !!config_.font && (config_.title[0] != 0)) {
      tc_.set_font(*config_.font);
      tc_.set_color(config_.color_title);
      rect_type ext;
      tc_.get_extend(ext, reinterpret_cast<const std::uint8_t*>(config_.title));
      const std::int16_t x = static_cast<std::int16_t>(config_.rect.left + config_.radius + 4);
      const std::int16_t y = static_cast<std::int16_t>(config_.rect.top - ext.height() / 2);
      if (config_.color_bg != color::none) {
        // cut the title into the border
        dc_.set_color(config_.color_bg);
        dc_.box({ static_cast<std::int16_t>(x - 2), y, static_cast<std::int16_t>(x + ext.width() + 2), static_cast<std::int16_t>(y + ext.height()) });
      }
      tc_.out({ x, y }, reinterpret_cast<const std::uint8_t*>(config_.title));
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
    return config_.rect.contain(vertex);
  }
};


} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_GROUPBOX_H_
