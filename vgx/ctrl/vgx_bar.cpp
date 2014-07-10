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
// \brief Progress bar control
//
///////////////////////////////////////////////////////////////////////////////

#include "vgx_bar.h"


#define MAJOR_MARK_SCALE    1
#define MINOR_MARK_SCALE    1 / 2
#define COLOR_MARK_SCALE    4 / 5
#define MARKER_SCALE        5 / 4


namespace vgx {

void bar::render(std::int16_t pos, bool refresh)
{
  render_color_marks();
  render_tick_marks();
  pos_ = 50;
  render_marker();

/*
  std::int16_t offset_new = static_cast<std::int16_t>((std::int32_t)(((config_.orientation == left_to_right || config_.orientation == right_to_left) ? config_.width : config_.height) *
                            (pos - config_.range_lower)) / (config_.range_upper - config_.range_lower));
  std::int16_t offset_old = static_cast<std::int16_t>((std::int32_t)(((config_.orientation == left_to_right || config_.orientation == right_to_left) ? config_.width : config_.height) *
                            (pos_ - config_.range_lower)) / (config_.range_upper - config_.range_lower));
  std::uint32_t color_old = head_.color_get();

  if (!refresh) {
    // incremental drawing
    if (pos == pos_) {
      // nothing to do
      return;
    }
    head_.color_set(pos > pos_ ? config_.color : config_.bg_color);
    switch (config_.orientation) {
      case left_to_right :
        head_.box(config_.x + offset_old, config_.y, config_.x + offset_new, config_.y + config_.height);
        break;
      case right_to_left :
        head_.box(config_.x + config_.width - offset_old, config_.y, config_.x + config_.width - offset_new, config_.y + config_.height);
        break;
      case bottom_to_top :
        head_.box(config_.x, config_.y + config_.height - offset_old, config_.x + config_.width, config_.y +  config_.height - offset_new);
        break;
      case top_to_bottom :
        head_.box(config_.x, config_.y + offset_old, config_.x + config_.width, config_.y + offset_new);
        break;
      default:
        break;
    }
  }
  else {
    // full refresh
    head_.color_set(config_.bg_color);
    switch (config_.orientation) {
      case left_to_right :
        head_.box(config_.x + offset_new, config_.y, config_.x + config_.width, config_.y + config_.height);
        if (offset_new) {
          head_.color_set(config_.color);
          head_.box(config_.x, config_.y, config_.x + offset_new, config_.y + config_.height);
        }
        break;
      case right_to_left :
        head_.box(config_.x, config_.y, config_.x + config_.width - offset_new, config_.y + config_.height);
        if (offset_new) {
          head_.color_set(config_.color);
          head_.box(config_.x + config_.width - offset_new, config_.y, config_.x + config_.width, config_.y + config_.height);
        }
        break;
      case bottom_to_top :
        head_.box(config_.x, config_.y, config_.x + config_.width, config_.y + config_.height - offset_new);
        if (offset_new) {
          head_.color_set(config_.color);
          head_.box(config_.x, config_.y + config_.height - offset_new, config_.x + config_.width, config_.y + config_.height);
        }
        break;
      case top_to_bottom :
        head_.box(config_.x, config_.y + offset_new, config_.x + config_.width, config_.y + config_.height);
        if (offset_new) {
          head_.color_set(config_.color);
          head_.box(config_.x, config_.y, config_.x + config_.width, config_.y + offset_new);
        }
        break;
      default:
        break;
    }
  }
  head_.color_set(color_old);
*/
  // set new position
  pos_ = pos;
}


void bar::render_color_marks(std::int16_t lower, std::int16_t upper)
{
  for (std::uint16_t i = 0; i < config_.color_mark_count; i++) {
    std::int16_t start = static_cast<std::int16_t>((std::int32_t)(((config_.orientation == horizontal_top || config_.orientation == horizontal_bottom) ? config_.width : config_.height) *
                                                   (config_.color_mark[i].start - config_.range_lower)) / (config_.range_upper - config_.range_lower));
    std::int16_t end   = static_cast<std::int16_t>((std::int32_t)(((config_.orientation == horizontal_top || config_.orientation == horizontal_bottom) ? config_.width : config_.height) *
                                                   (config_.color_mark[i].end   - config_.range_lower)) / (config_.range_upper - config_.range_lower));

    head_.color_set(config_.color_mark[i].color);
    switch (config_.orientation) {
      case horizontal_top :
        head_.box(config_.x + start, config_.y + 1, config_.x + end, config_.y + config_.height * COLOR_MARK_SCALE);
        break;
      case horizontal_bottom :
        head_.box(config_.x + start, config_.y + config_.height - (config_.height * COLOR_MARK_SCALE), config_.x + end, config_.y + config_.height);
        break;
      default :
        break;
    }
  }
}


void bar::render_tick_marks()
{
  head_.color_set(config_.color_scale);
  // render the frame
  switch (config_.orientation) {
    case horizontal_top :
      head_.line_horz(config_.x, config_.y, config_.x + config_.width);
      head_.line_vert(config_.x, config_.y, config_.y + config_.height);
      head_.line_vert(config_.x + config_.width, config_.y, config_.y + config_.height);
      break;
    case horizontal_bottom :
      head_.line_horz(config_.x, config_.y + config_.height, config_.x + config_.width);
      head_.line_vert(config_.x, config_.y, config_.y + config_.height);
      head_.line_vert(config_.x + config_.width, config_.y, config_.y + config_.height);
      break;
  }

  // render the ticks
  for (std::uint16_t i = 0; i < config_.tick_mark_count; i++) {
    std::int16_t pos = static_cast<std::int16_t>((std::int32_t)(((config_.orientation == horizontal_top || config_.orientation == horizontal_bottom) ? config_.width : config_.height) *
                                                 (config_.tick_mark[i].pos - config_.range_lower)) / (config_.range_upper - config_.range_lower));
    switch (config_.orientation) {
      case horizontal_top :
        head_.line_vert(config_.x + pos, config_.y, config_.y + (config_.tick_mark[i].major ? config_.height * MAJOR_MARK_SCALE : config_.height * MINOR_MARK_SCALE));
        break;
      case horizontal_bottom :
        head_.line_vert(config_.x + pos, config_.y + config_.height - (config_.tick_mark[i].major ? config_.height * MAJOR_MARK_SCALE : config_.height * MINOR_MARK_SCALE), config_.y + config_.height);
        break;
      default :
        break;
    }
  }
}


void bar::render_marker()
{
  std::int16_t pos = static_cast<std::int16_t>((std::int32_t)(((config_.orientation == horizontal_top || config_.orientation == horizontal_bottom) ? config_.width : config_.height) *
                                               (pos_ - config_.range_lower)) / (config_.range_upper - config_.range_lower));
  head_.color_set(config_.color_marker);
    switch (config_.orientation) {
      case horizontal_top :
        head_.triangle_solid(config_.x + pos, config_.y + 1,
                             config_.x + pos - ((config_.height * MARKER_SCALE) / 2), config_.y + 1 + config_.height * MARKER_SCALE,
                             config_.x + pos + ((config_.height * MARKER_SCALE) / 2), config_.y + 1 + config_.height * MARKER_SCALE);
        break;
      case horizontal_bottom :
        head_.triangle_solid(config_.x + pos, config_.y - 1 + config_.height,
                             config_.x + pos - ((config_.height * MARKER_SCALE) / 2), config_.y - 1 + config_.height - config_.height * MARKER_SCALE,
                             config_.x + pos + ((config_.height * MARKER_SCALE) / 2), config_.y - 1 + config_.height - config_.height * MARKER_SCALE);
        break;
      default :
        break;
    }
}

} // namespace vgx
