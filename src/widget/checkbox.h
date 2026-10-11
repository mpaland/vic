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
// \brief Checkbox widget, a box with a check mark and a caption at the right
//
// Usage:
// vic::widget::checkbox::config_type _cfg = { { 10, 10 }, 12, &_font, "Sound",
//                                             vic::color::white, vic::color::green, vic::color::gray04, vic::color::gray10 };
// vic::widget::checkbox cb(_head, _cfg);
// cb.init();
// cb.set_checked(true);
//
// Behavior:
// - event_tap toggles the state, then the event handler is called
// - the caption is vertically centered to the box, the hit area is box and caption
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_CHECKBOX_H_
#define _VIC_WIDGET_CHECKBOX_H_

#include "widget.h"


namespace vic {
namespace widget {


class checkbox : public base
{
public:
  // configuration
  typedef struct tag_config_type {
    vertex_type             origin;         // origin (left/top) of the box
    std::uint8_t            size;           // box size in pixel (square), 8 or more
    const font::font_type*  font;           // caption font, must stay valid
    const char*             caption;        // caption (UTF-8), nullptr for none, must stay valid
    color::value_type       color_text;     // caption color
    color::value_type       color_check;    // check mark color
    color::value_type       color_box;      // box fill color
    color::value_type       color_border;   // box border color, color::none for no border
  } config_type;

private:
  dc            dc_;              // drawing context
  tc            tc_;              // text context
  config_type   config_;          // configuration
  bool          checked_;         // state
  std::int16_t  caption_width_;   // width of the rendered caption


public:
  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  checkbox(drv& head, const config_type& config)
    : base(head)
    , dc_(head)
    , tc_(dc_)
    , config_(config)
    , checked_(false)
    , caption_width_(0)
  { }


  /**
   * Init: draw the widget
   */
  virtual void init() final
  {
    redraw();
  }


  /**
   * Set the state, redrawn only on change
   * \param checked True for checked
   */
  void set_checked(bool checked)
  {
    if (checked != checked_) {
      checked_ = checked;
      render_box();
    }
  }


  /**
   * Toggle the state
   */
  inline void toggle()
  {
    set_checked(!checked_);
  }


  /**
   * Returns the state
   * \return True if checked
   */
  inline bool is_checked() const
  {
    return checked_;
  }


  /**
   * Redraw the control
   */
  void redraw()
  {
    dc_.present_lock();
    render_box();
    if (!!config_.caption && !!config_.font) {
      tc_.set_font(*config_.font);
      tc_.set_color(config_.color_text);
      rect_type ext;
      tc_.get_extend(ext, reinterpret_cast<const std::uint8_t*>(config_.caption));
      caption_width_ = ext.width();
      tc_.out({ static_cast<std::int16_t>(config_.origin.x + config_.size + 4),
                static_cast<std::int16_t>(config_.origin.y + (config_.size - ext.height()) / 2) },
              reinterpret_cast<const std::uint8_t*>(config_.caption));
    }
    dc_.present_lock(false);
  }


protected:

  virtual bool is_inside(vertex_type vertex) const final
  {
    const rect_type r = { config_.origin.x, config_.origin.y,
                          static_cast<std::int16_t>(config_.origin.x + config_.size + (caption_width_ > 0 ? 4 + caption_width_ : 0)),
                          static_cast<std::int16_t>(config_.origin.y + config_.size) };
    return r.contain(vertex);
  }


  /**
   * A tap toggles the state
   */
  virtual void on_event(vertex_type vertex, event_type ev) final
  {
    (void)vertex;
    if (ev == event_tap) {
      toggle();
    }
  }


private:

  // render the box with the check mark
  void render_box()
  {
    const std::int16_t s = config_.size;
    const rect_type box  = { config_.origin.x, config_.origin.y, static_cast<std::int16_t>(config_.origin.x + s), static_cast<std::int16_t>(config_.origin.y + s) };
    dc_.present_lock();
    dc_.set_color(config_.color_box);
    dc_.box(box);
    if (config_.color_border != color::none) {
      dc_.set_color(config_.color_border);
      dc_.rectangle(box);
    }
    if (checked_) {
      // check mark: two strokes, two pixel thick
      const std::int16_t x0 = static_cast<std::int16_t>(box.left + s / 4),       y0 = static_cast<std::int16_t>(box.top + s / 2);
      const std::int16_t x1 = static_cast<std::int16_t>(box.left + s * 5 / 12),  y1 = static_cast<std::int16_t>(box.bottom - 1 - s / 4);
      const std::int16_t x2 = static_cast<std::int16_t>(box.right - 1 - s / 5),  y2 = static_cast<std::int16_t>(box.top + s / 4);
      dc_.set_color(config_.color_check);
      dc_.line({ x0, y0 }, { x1, y1 });
      dc_.line({ x1, y1 }, { x2, y2 });
      dc_.line({ x0, static_cast<std::int16_t>(y0 - 1) }, { x1, static_cast<std::int16_t>(y1 - 1) });
      dc_.line({ x1, static_cast<std::int16_t>(y1 - 1) }, { x2, static_cast<std::int16_t>(y2 - 1) });
    }
    dc_.present_lock(false);
  }
};


} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_CHECKBOX_H_
