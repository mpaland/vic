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
// \brief Label widget, a text or numeric readout in a fixed cell
//
// Usage:
// vic::font::Montserrat_16_aa4 _font;
// vic::widget::label::config_type _cfg = { { 10, 10 }, 60, 20, &_font, vic::widget::label::align_center,
//                                          vic::color::white, vic::color::black, vic::color::none };
// vic::widget::label lbl(_head, _cfg);
// lbl.init();
// lbl.set_text("RPM");
// lbl.set_value(2100);              // "2100"
// lbl.set_value(270, 1, " PSI");    // "27.0 PSI"
//
// Configuration:
// - origin is the left/top corner, x_size * y_size is the complete cell, the border (if any) is part of it
// - the text is vertically centered in the cell, horizontally aligned by 'align'
// - the cell is cleared with color_bg before the text is rendered, color::none leaves the background untouched
//   (then the text is drawn over the old one, use it for static texts only)
// - inline color escapes ("\aRRGGBB") in the text are supported
// - the text is redrawn only when it changes, so set_text()/set_value() can be called every frame
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_LABEL_H_
#define _VIC_WIDGET_LABEL_H_

#include "widget.h"


// maximum text length in bytes (without the terminating zero), inline color escapes take 7 bytes each
#ifndef VIC_WIDGET_LABEL_MAX_LEN
#define VIC_WIDGET_LABEL_MAX_LEN  63U
#endif


namespace vic {
namespace widget {


class label : public base
{
public:
  // horizontal alignment of the text in the cell
  typedef enum tag_align_type {
    align_left = 0,
    align_center,
    align_right
  } align_type;

  // configuration
  typedef struct tag_config_type {
    vertex_type             origin;         // origin (left/top)
    std::uint16_t           x_size;         // x size of the cell
    std::uint16_t           y_size;         // y size of the cell
    const font::font_type*  font;           // font, must stay valid during the lifetime of the widget
    align_type              align;          // horizontal text alignment
    color::value_type       color_text;     // text color
    color::value_type       color_bg;       // background color, color::none to leave the background untouched
    color::value_type       color_border;   // border color, color::none for no border
  } config_type;

private:
  dc            dc_;        // drawing context
  tc            tc_;        // text context
  config_type   config_;    // configuration
  char          text_[VIC_WIDGET_LABEL_MAX_LEN + 1U];   // actual text


public:
  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  label(drv& head, const config_type& config)
    : base(head)
    , dc_(head)
    , tc_(dc_)
    , config_(config)
  {
    text_[0] = 0;
  }


  /**
   * Init: draw the (empty) widget
   */
  virtual void init() final
  {
    redraw();
  }


  /**
   * Set a new text, the widget is redrawn if the text differs from the actual one
   * \param utf8 Zero terminated UTF-8 text, truncated to VIC_WIDGET_LABEL_MAX_LEN bytes
   */
  void set_text(const char* utf8)
  {
    if (!utf8) {
      utf8 = "";
    }
    // compare and copy in one pass
    bool same = true;
    std::size_t n = 0U;
    for (; (n < VIC_WIDGET_LABEL_MAX_LEN) && (utf8[n] != 0); ++n) {
      if (text_[n] != utf8[n]) {
        same = false;
      }
      text_[n] = utf8[n];
    }
    if (text_[n] != 0) {
      same = false;
    }
    text_[n] = 0;
    if (!same) {
      redraw();
    }
  }


  /**
   * Set a numeric value as text, no floating point is used
   * \param value The value, e.g. 270 with decimals = 1 is rendered as "27.0"
   * \param decimals Number of decimal places (0 - 9)
   * \param suffix Optional text appended to the number, e.g. " PSI", nullptr for none
   */
  void set_value(std::int32_t value, std::uint8_t decimals = 0U, const char* suffix = nullptr)
  {
    char buf[VIC_WIDGET_LABEL_MAX_LEN + 1U];
    std::size_t len = format(buf, value, decimals);
    if (suffix) {
      for (std::size_t i = 0U; (len < VIC_WIDGET_LABEL_MAX_LEN) && (suffix[i] != 0); ++i) {
        buf[len++] = suffix[i];
      }
    }
    buf[len] = 0;
    set_text(buf);
  }


  /**
   * Set a new text color
   * \param color New text color, the widget is redrawn
   */
  void set_color(color::value_type color)
  {
    config_.color_text = color;
    redraw();
  }


  /**
   * Returns the actual text
   * \return Zero terminated text
   */
  inline const char* get_text() const
  {
    return text_;
  }


  /**
   * Redraw the control
   */
  void redraw()
  {
    const rect_type in = inner_rect();

    dc_.present_lock();
    if (config_.color_bg != color::none) {
      dc_.set_color(config_.color_bg);
      dc_.box(in);
    }
    if (config_.color_border != color::none) {
      dc_.set_color(config_.color_border);
      dc_.rectangle(outer_rect());
    }
    if ((text_[0] != 0) && !!config_.font) {
      tc_.set_font(*config_.font);
      tc_.set_color(config_.color_text);
      rect_type ext;
      tc_.get_extend(ext, reinterpret_cast<const std::uint8_t*>(text_));
      std::int16_t x = in.left;
      switch (config_.align) {
        case align_center : x = static_cast<std::int16_t>(in.left + (in.width() - ext.width()) / 2); break;
        case align_right  : x = static_cast<std::int16_t>(in.right - ext.width());                   break;
        default           : break;
      }
      const std::int16_t y = static_cast<std::int16_t>(in.top + (in.height() - ext.height()) / 2);
      tc_.out({ x, y }, reinterpret_cast<const std::uint8_t*>(text_));
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
    return outer_rect().contain(vertex);
  }


private:

  // the complete widget rect
  inline rect_type outer_rect() const
  {
    return { config_.origin.x, config_.origin.y,
             static_cast<std::int16_t>(config_.origin.x + config_.x_size), static_cast<std::int16_t>(config_.origin.y + config_.y_size) };
  }


  // the text cell (inside the border)
  inline rect_type inner_rect() const
  {
    rect_type r = outer_rect();
    if (config_.color_border != color::none) {
      r.left++; r.top++; r.right--; r.bottom--;
      if (r.right < r.left) { r.right = r.left; }
      if (r.bottom < r.top) { r.bottom = r.top; }
    }
    return r;
  }


public:
  /**
   * Format a value as decimal number, like "-27.05", used by other widgets as well
   * \param buf Output buffer, at least 14 chars
   * \param value Value to format
   * \param decimals Decimal places, 0 - 9
   * \return Length of the string in buf (not terminated)
   */
  static std::size_t format(char* buf, std::int32_t value, std::uint8_t decimals)
  {
    // digits in reverse order
    char tmp[12];
    std::size_t n = 0U;
    std::uint32_t v = value < 0 ? 0U - static_cast<std::uint32_t>(value) : static_cast<std::uint32_t>(value);
    if (decimals > 9U) {
      decimals = 9U;
    }
    do {
      tmp[n++] = static_cast<char>('0' + (v % 10U));
      v /= 10U;
    } while ((v != 0U) || (n <= decimals));   // at least one digit before the point

    std::size_t len = 0U;
    if (value < 0) {
      buf[len++] = '-';
    }
    while (n > 0U) {
      --n;
      if (n + 1U == decimals) {
        buf[len++] = '.';
      }
      buf[len++] = tmp[n];
    }
    return len;
  }
};


} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_LABEL_H_
