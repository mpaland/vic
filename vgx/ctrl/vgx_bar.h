///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2014, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// This file is part of the vgx library.
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
// \brief Universal bar control
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_CTRL_BAR_H_
#define _VGX_CTRL_BAR_H_

#include "../vgx_ctrl.h"
#include "../vgx_fonts.h"


namespace vgx {

class bar : public ctrl
{
public:
  typedef enum enum_oriantation_type {
    horizontal_top = 0,   // horizontal orientation, scale/frame on top
    horizontal_bottom,    // horizontal orientation, scale/frame on bottom
    vertical_left,        // vertical orientation, scale/frame on left side
    vertical_right        // vertical orientation, scale/frame on right side
  } orientation_type;

  typedef struct struct_color_mark_type {
    std::uint32_t color;                  // ARGB value, A = 255 doesn't draw the segment
    std::int16_t  start;                  // start position, must be within valid range
    std::int16_t  end;                    // end position, must be within valid range
  } color_mark_type;

  typedef struct struct_tick_mark_type {
    bool          major;                  // true for major tick, false for minor tick mark
    std::int16_t  pos;                    // position of the tick, must be within valid range
    const std::uint8_t* label;            // label string in ASCII/UTF-8 format, nullptr if no label
  } tick_mark_type;

  typedef struct struct_config_type {
    std::int16_t      x;                  // left coordinate
    std::int16_t      y;                  // top  coordinate
    std::uint16_t     width;              // width of the bar
    std::uint16_t     height;             // height of the bar
    orientation_type  orientation;        // orientation of the bar, see orientation_type
    std::int16_t      range_lower;        // lower range
    std::int16_t      range_upper;        // upper range
    std::uint32_t     color_scale;        // scale bar (tick) color
    std::uint32_t     color_marker;       // marker color
    std::uint32_t     color_bg;           // background color
    color_mark_type*  color_mark;         // array of color marks
    std::uint16_t     color_mark_count;   // number of color marks
    tick_mark_type*   tick_mark;          // array of tick marks
    std::uint16_t     tick_mark_count;    // number of tick marks
    const font_type*        font_tick;          // font for the ticks
    const font_type*        font_marker;        // font for the marker
  } config_type;

  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  bar(drv& head, config_type& config)
    : ctrl(head)
    , config_(config)
    , pos_(config.range_lower)
    , marker_l_(0)
    , marker_u_(0)
  { render(config.range_lower, true); };

  /**
   * Set new marker color
   * \param color New color for the marker, marker is redrawn
   */
  void set_marker_color(std::uint32_t color)
  { config_.color_marker = color; redraw(); }

  /**
   * Set the new minimum and maximum range
   * \param lower Lower range limit including 'lower' value
   * \param upper Upper range limit including 'upper' value
   */
  void set_range(std::int16_t lower, std::int16_t upper)
  { config_.range_lower = pos_ = lower; config_.range_upper = upper; redraw(); }

  /**
   * Set marker to position
   * \param pos New position - must be within range
   * \param ch Character on the marker
   */
  void set_marker(std::int16_t pos, char ch = 0)
  { if (pos != pos_) render(pos); }

  /**
   * Returns the current position of the marker
   * \return Current position
   */
  std::int16_t get_marker()
  { return pos_; }

  /**
   * Redraw the control
   */
  void redraw()
  { render(pos_, true); }

  virtual bool is_inside(std::int16_t x, std::int16_t y)
  { return config_.x <= x && x <= config_.x + config_.width &&
           config_.y <= y && y <= config_.y + config_.height; }

private:
  void render(std::int16_t pos, bool refresh = false);
  void render_color_marks();
  void render_tick_marks();
  void render_marker();
  void erase_marker();

  config_type   config_;    // configuration
  std::int16_t  pos_;       // actual position, valid from range_lower to range_upper
  std::int16_t  marker_l_;  // internal marker var
  std::int16_t  marker_u_;  // internal marker var
};

} // namespace vgx

#endif  // _VGX_CTRL_BAR_H_
