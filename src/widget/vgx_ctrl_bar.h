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

#include "vgx_ctrl.h"


namespace vgx {

class bar : public ctrl
{
public:

  // this base class is used as reference to the templated instance
  struct mark {
    virtual color::value_type mix(std::int16_t pos) const = 0;
    virtual color::value_type solid(std::int16_t pos) const = 0;
  };

  /**
   * Color mark structure
   * Construct a color mark with 3 colors like
   * vgx::bar::color_mark<3U> cm = { {0, 60, vgx::color::green}, {60, 80, vgx::color::yellow}, {80, 100, vgx::color::red} };
                                     green                       yellow                        red
   */
  template<std::size_t N>
  struct color_mark : public mark
  {
    // array of the colors marks
    typedef struct struct_mark_type {
      std::int16_t      start;      // start position, must be within valid range
      std::int16_t      end;        // end position, must be within valid range
      color::value_type color;      // ARGB value, A = 255 (color::none) doesn't draw the segment
    } mark_type;

    color_mark(const std::initializer_list<mark_type>& il) {
      std::size_t n = 0U;
      for (std::initializer_list<mark_type>::const_iterator it = il.begin(); it != il.end(); ++it) {
        mark_[n++] = *it;
      }
    }

    /**
     * Return the mixed color at the given position
     * \param pos The position in per mille [1/1000]
     * \return The mixed color at the given position
     */
    virtual color::value_type mix(std::int16_t pos) const {
      std::uint_fast16_t i;
      for (i = 1U; i < N && pos > mark_[i].start; ++i);
      if (i == N) {
        return mark_[i - 1U].color;
      }
      return color::mix(mark_[i - 1U].color, mark_[i].color,
                        static_cast<std::uint8_t>((std::uint32_t)(mark_[i].start - pos) * 255UL / (mark_[i].start - mark_[i - 1U].start)));
    }

    /**
     * Returns the solid color at the given position
     * \param pos The position within the range
     * \return The solid color at the given position
     */
    virtual color::value_type solid(std::int16_t pos) const {
      std::uint_fast16_t i;
      for (i = 0U; i < N; ++i) {
        if (pos >= mark_[i].start && pos <= mark_[i].end) {
          return mark_[i].color;
        }
      }
      // not found
      return color::none;
    }

  private:
    // array of the gradient colors and the according start values
    mark_type mark_[N];
  };


  typedef enum enum_oriantation_type {
    horizontal_top = 0,   // horizontal orientation, scale/frame on top
    horizontal_bottom,    // horizontal orientation, scale/frame on bottom
    vertical_left,        // vertical   orientation, scale/frame on left side
    vertical_right        // vertical   orientation, scale/frame on right side
  } orientation_type;

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
    std::int16_t      range_lower;        // lower range
    std::int16_t      range_upper;        // upper range
    orientation_type  orientation;        // orientation of the bar, see orientation_type
    color::value_type color_scale;        // scale bar (tick) color
    color::value_type color_marker;       // marker color
    color::value_type color_bg;           // background color
    mark*             color_mark;         // array of color marks
    tick_mark_type*   tick_mark;          // array of tick marks
    std::uint16_t     tick_mark_count;    // number of tick marks
    const font::font_type*  font_tick;    // font for the ticks
    const font::font_type*  font_marker;  // font for the marker character
  } config_type;

  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  bar(drv& head, config_type& config)
    : ctrl(head)
    , config_(config)
    , marker_pos_(config.range_lower)
    , marker_chr_(0)
    , marker_l_(0)
    , marker_u_(0)
  { redraw(); };

  /**
   * Set new marker color
   * \param color New color for the marker, marker is redrawn
   */
  void set_marker_color(color::value_type color)
  {
    config_.color_marker = color;
    redraw();
  }

  /**
   * Set the new minimum and maximum range
   * \param lower Lower range limit including 'lower' value
   * \param upper Upper range limit including 'upper' value
   */
  void set_range(std::int16_t lower, std::int16_t upper)
  {
    config_.range_lower = lower;
    config_.range_upper = upper;
    if (marker_pos_ < lower || marker_pos_ > upper) marker_pos_ = lower;
    redraw();
  }

  /**
   * Set marker to position
   * \param pos New position - must be within range
   * \param ch Character on the marker
   */
  void set_marker(std::int16_t pos, char chr = 0)
  {
    marker_pos_ = pos;
    marker_chr_ = chr;
    render();
  }

  /**
   * Returns the current position of the marker
   * \return Current position
   */
  std::int16_t get_marker()
  { return marker_pos_; }

  /**
   * Redraw the control
   */
  void redraw()
  { render(true); }

  virtual bool is_inside(std::int16_t x, std::int16_t y)
  { return config_.x <= x && x <= config_.x + config_.width &&
           config_.y <= y && y <= config_.y + config_.height; }

private:

  // return x or y coordinate for the given position
  inline std::int16_t pos_to_coord(std::int16_t pos) {
    switch (config_.orientation) {
      case horizontal_top :
      case horizontal_bottom :
        // return x value
        return config_.x + config_.width * (pos - config_.range_lower) / (config_.range_upper - config_.range_lower);
      case vertical_left :
      case vertical_right :
        // return y value
        return config_.y + config_.height * (pos - config_.range_lower) / (config_.range_upper - config_.range_lower);
      default :
        return 0;
    }
  }

  // return the position for the given x or y coordinate
  inline std::int16_t coord_to_pos(std::int16_t coord) {
    switch (config_.orientation) {
      case horizontal_top :
      case horizontal_bottom :
        // return x value
        return config_.range_lower + (coord - config_.x) * (config_.range_upper - config_.range_lower) / (config_.width);
      case vertical_left :
      case vertical_right :
        // return y value
        return config_.range_lower + (coord - config_.y) * (config_.range_upper - config_.range_lower) / (config_.height);
      default :
        return 0;
    }
  }


  void render(bool refresh = false);
  void render_color_marks();
  void render_tick_marks();
  void render_marker();
  void erase_marker();

  config_type   config_;      // configuration
  std::int16_t  marker_pos_;    // actual position, valid from range_lower to range_upper
  char          marker_chr_;    // character on the marker
  std::int16_t  marker_l_;      // internal marker redraw helper
  std::int16_t  marker_u_;      // internal marker redraw helper
};

} // namespace vgx

#endif  // _VGX_CTRL_BAR_H_
