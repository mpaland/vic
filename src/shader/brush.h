///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2017, PALANDesign Hannover, Germany
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
// \brief Brush shader class for the pixel shader pipeline
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_SHADER_BRUSH_H_
#define _VIC_SHADER_BRUSH_H_

#include "base.h"


namespace vic {
namespace shader {


/**
 * Brush shader class
 */
class brush : public base
{
public:

  typedef enum tag_style_type {
    style_solid = 0,
    style_dash,
    style_dot,
    style_dashdot
  } style_type;

  typedef struct tag_shape_type {
    const std::uint8_t* alpha;    // brush shape as alpha value array
    std::uint8_t        width;    // width of shape
    std::uint8_t        height;   // height of shape
    style_type          style;    // style
  } shape_type;

  shape_type    shape_;           // actual shape
  std::uint8_t  style_ctl_;       // internal style control, set to 0 on init

  /**
   * ctor
   */
  brush()
    : style_ctl_(0U)
  { }


  /**
   * Render the shape at the given position
   * \param center Pen center position
   */
  virtual void pixel_set(vertex_type vertex, color::value_type color) final
  {
    switch (shape_.style) {
      case style_solid :
        // ***********
        break;
      case style_dot :
        // * * * * * *
        if (style_ctl_ >= shape_.width * 2U) {
          style_ctl_ = 0U;
        }
        if (style_ctl_++ > 0U) {
          return;
        }
        break;
      case style_dash :
        // *** *** ***
        if (style_ctl_ >= shape_.width * 4U) {
          style_ctl_ = 0U;
        }
        if (style_ctl_++ > shape_.width * 2U) {
          return;
        }
        break;
      case style_dashdot :
        // *** * *** * ***
        if (style_ctl_ >= shape_.width * 6U) {
          style_ctl_ = 0U;
        }
        if ((style_ctl_ >  shape_.width * 2U) &&
            (style_ctl_ != shape_.width * 4U )) {
          style_ctl_++;
          return;
        }
        style_ctl_++;
        break;
      default:
        // default is style_solid
        break;
    }
    std::size_t i = 0U;
    for (std::int16_t y = vertex.y - shape_.height / 2, ye = y + shape_.height; y < ye; ++y) {
      for (std::int16_t x = vertex.x - shape_.width / 2, xe = x + shape_.width; x < xe; ++x, ++i) {
        next_->pixel_set({ x, y }, color::set_alpha(color, shape_.alpha[i]));
      }
    }
  }


  /**
   * Reset the style (dash, dot, dashdot)
   */
  inline void reset()
  { style_ctl_ = 0U; }


  /**
   * Select the actual drawing shape
   * \param shape Set the actual pen shape or nullptr for 1 pixel default pen (fastest)
   */
  inline void set_shape(shape_type& shape)
  { shape_ = shape; }


  /**
   * Get a stock shape
   * \param shape Receiving shape struct
   * \param style Brush style 
   * \param width Brush width
   */
  bool get_stock_shape(shape_type& shape, style_type style, std::uint8_t width) const
  {
    // stock pens
    static const std::uint8_t   pen_shape_width_1[1]  = { 255 };                // one pixel pen
    static const std::uint8_t   pen_shape_width_2[4]  = { 255, 255,
                                                          255, 255 };           // two pixel pen
    static const std::uint8_t   pen_shape_width_3[9]  = { 128, 255, 128,
                                                          255, 255, 255,
                                                          128, 255, 128 };      // three pixel pen
    static const std::uint8_t   pen_shape_width_4[16] = {   0, 255, 255,   0,
                                                          255, 255, 255, 255,
                                                          255, 255, 255, 255,
                                                            0, 255, 255,   0 }; // four pixel pen
    static const std::uint8_t* alpha[4] = { pen_shape_width_1, pen_shape_width_2, pen_shape_width_3, pen_shape_width_4 };

    shape = { alpha[width - 1U], width, width, style };

    return true;
  }
};


} // namespace shader
} // namespace vic

#endif  // _VIC_SHADER_BRUSH_H_
