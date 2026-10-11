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
// \brief Textbox widget, a multi line text with word wrap, explicit line breaks and line scrolling
//
// Usage:
// vic::font::Montserrat_12_aa2 _font;
// vic::widget::textbox::config_type _cfg = { { 10, 10 }, 120, 60, &_font, vic::widget::textbox::align_left, 1,
//                                            vic::color::white, vic::color::black, vic::color::none };
// vic::widget::textbox tb(_head, _cfg);
// tb.init();
// tb.set_text("The quick brown fox jumps over the lazy dog.\nSecond paragraph.");
// tb.scroll(1);     // scroll one line down
//
// Configuration:
// - origin is the left/top corner, x_size * y_size is the complete cell, the border (if any) is part of it
// - the text is wrapped at spaces to the cell width, '\n' forces a line break, lines are rendered from
//   the top, lines which don't fit into the cell are not drawn, scroll() / set_first_line() select
//   the first visible line
// - the text is NOT copied, the pointer must stay valid as long as the widget shows it
// - a line is at most VIC_WIDGET_TEXTBOX_LINE_LEN bytes, longer words are cut
// - inline color escapes ("\aRRGGBB") are supported, the color is reset to color_text at every redraw
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_TEXTBOX_H_
#define _VIC_WIDGET_TEXTBOX_H_

#include "widget.h"


// maximum line length in bytes (without the terminating zero)
#ifndef VIC_WIDGET_TEXTBOX_LINE_LEN
#define VIC_WIDGET_TEXTBOX_LINE_LEN  127U
#endif


namespace vic {
namespace widget {


class textbox : public base
{
public:
  // horizontal alignment of the lines in the cell
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
    align_type              align;          // horizontal alignment of the lines
    std::uint8_t            line_spacing;   // extra pixel between the lines
    color::value_type       color_text;     // text color
    color::value_type       color_bg;       // background color, color::none to leave the background untouched
    color::value_type       color_border;   // border color, color::none for no border
  } config_type;

private:
  dc            dc_;          // drawing context
  tc            tc_;          // text context
  config_type   config_;      // configuration
  const char*   text_;        // the text, not copied
  std::uint16_t first_line_;  // first visible line
  std::uint16_t line_count_;  // total lines of the text after wrapping


public:
  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  textbox(drv& head, const config_type& config)
    : base(head)
    , dc_(head)
    , tc_(dc_)
    , config_(config)
    , text_(nullptr)
    , first_line_(0U)
    , line_count_(0U)
  { }


  /**
   * Init: draw the (empty) widget
   */
  virtual void init() final
  {
    redraw();
  }


  /**
   * Set a new text and render it from the first line
   * \param utf8 Zero terminated UTF-8 text, not copied, must stay valid; nullptr clears the box
   * \return Number of lines the text takes
   */
  std::uint16_t set_text(const char* utf8)
  {
    text_       = utf8;
    first_line_ = 0U;
    redraw();
    return line_count_;
  }


  /**
   * Set the first visible line and redraw
   * \param line First visible line, clamped to the last line
   */
  void set_first_line(std::uint16_t line)
  {
    if (line_count_ > 0U && line >= line_count_) {
      line = static_cast<std::uint16_t>(line_count_ - 1U);
    }
    if (line != first_line_) {
      first_line_ = line;
      redraw();
    }
  }


  /**
   * Scroll the text by a number of lines
   * \param lines Lines to scroll, positive scrolls down (shows later lines)
   */
  void scroll(std::int16_t lines)
  {
    const std::int32_t line = static_cast<std::int32_t>(first_line_) + lines;
    set_first_line(static_cast<std::uint16_t>(line < 0 ? 0 : line));
  }


  /**
   * Returns the number of lines of the text after wrapping (valid after set_text() / redraw())
   * \return Line count
   */
  inline std::uint16_t get_line_count() const
  {
    return line_count_;
  }


  /**
   * Returns the first visible line
   * \return First visible line
   */
  inline std::uint16_t get_first_line() const
  {
    return first_line_;
  }


  /**
   * Returns the number of lines which fit into the cell
   * \return Visible lines
   */
  std::uint16_t get_visible_lines() const
  {
    if (!config_.font) {
      return 0U;
    }
    const std::int16_t h = static_cast<std::int16_t>(config_.font->info().ysize + config_.line_spacing);
    return h > 0 ? static_cast<std::uint16_t>((inner_rect().height() + config_.line_spacing) / h) : 0U;
  }


  /**
   * Set a new text color and redraw
   * \param color New text color
   */
  void set_color(color::value_type color)
  {
    config_.color_text = color;
    redraw();
  }


  /**
   * Redraw the control: clear the cell, wrap and render the text
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

    line_count_ = 0U;
    if (!!text_ && !!config_.font) {
      tc_.set_font(*config_.font);
      tc_.set_color(config_.color_text);
      const std::int16_t line_height = static_cast<std::int16_t>(config_.font->info().ysize + config_.line_spacing);
      std::int16_t y = in.top;

      char line[VIC_WIDGET_TEXTBOX_LINE_LEN + 1U];
      std::size_t line_len = 0U;
      const char* p = text_;
      for (;;) {
        // end of text or explicit line break: flush the line
        if ((*p == 0) || (*p == '\n')) {
          line[line_len] = 0;
          render_line(line, in, y, line_height);
          line_len = 0U;
          if (*p == 0) {
            break;
          }
          ++p;
          continue;
        }
        // next word including the trailing spaces
        const char* w = p;
        while ((*p != 0) && (*p != ' ') && (*p != '\n')) { ++p; }
        while (*p == ' ') { ++p; }
        std::size_t word_len = static_cast<std::size_t>(p - w);
        if (word_len > VIC_WIDGET_TEXTBOX_LINE_LEN) {
          word_len = VIC_WIDGET_TEXTBOX_LINE_LEN;    // cut overlong words
        }
        if (line_len + word_len > VIC_WIDGET_TEXTBOX_LINE_LEN) {
          // the line buffer is full: flush and start a new line with the word
          line[line_len] = 0;
          render_line(line, in, y, line_height);
          line_len = 0U;
        }
        // does the line still fit with the word?
        for (std::size_t i = 0U; i < word_len; ++i) {
          line[line_len + i] = w[i];
        }
        line[line_len + word_len] = 0;
        rect_type ext;
        tc_.get_extend(ext, reinterpret_cast<const std::uint8_t*>(line));
        if ((ext.width() > in.width()) && (line_len > 0U)) {
          // no: flush the line without the word and start a new line with it
          line[line_len] = 0;
          render_line(line, in, y, line_height);
          for (std::size_t i = 0U; i < word_len; ++i) {
            line[i] = w[i];
          }
          line_len = word_len;
        }
        else {
          line_len += word_len;
        }
      }
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


  /**
   * Render one wrapped line if it is visible, count it and advance y
   * \param line Zero terminated line, trailing spaces are removed
   * \param in The text cell
   * \param y Top of the line, advanced by the line height when the line is visible
   * \param line_height Line height incl. spacing
   */
  void render_line(char* line, const rect_type& in, std::int16_t& y, std::int16_t line_height)
  {
    // trim trailing spaces
    std::size_t len = 0U;
    while (line[len] != 0) { ++len; }
    while ((len > 0U) && (line[len - 1U] == ' ')) { line[--len] = 0; }

    const std::uint16_t n = line_count_++;
    if ((n < first_line_) || (y + static_cast<std::int16_t>(config_.font->info().ysize) > in.bottom)) {
      // scrolled out or below the cell
      return;
    }
    rect_type ext;
    tc_.get_extend(ext, reinterpret_cast<const std::uint8_t*>(line));
    std::int16_t x = in.left;
    switch (config_.align) {
      case align_center : x = static_cast<std::int16_t>(in.left + (in.width() - ext.width()) / 2); break;
      case align_right  : x = static_cast<std::int16_t>(in.right - ext.width());                   break;
      default           : break;
    }
    tc_.out({ x, y }, reinterpret_cast<const std::uint8_t*>(line));
    y = static_cast<std::int16_t>(y + line_height);
  }
};


} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_TEXTBOX_H_
