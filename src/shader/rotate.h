///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2017-2017, PALANDesign Hannover, Germany
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
// \brief Rotation shader for the pixel shader pipeline
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_SHADER_ROTATE_H_
#define _VIC_SHADER_ROTATE_H_

#include "base.h"


namespace vic {
namespace shader {


/**
 * Rotation vertex shader class
 */
class rotate : public base
{
  vertex_type  center_;
  std::int16_t angle_;

public:

  /**
   * ctor
   * \param center Rotation center
   * \param angle Rotation angle in degree, direction is math positive (clockwise in screen coords system)
   */
  rotate(vertex_type center, std::int16_t angle)
    : center_(center)
    , angle_(angle)
  { }


  /**
   * Set rotation parameter
   * \param center Rotation center
   * \param angle Rotation angle in degree, direction is math positive (clockwise in screen coords system)
   */
  void set_param(vertex_type center, std::int16_t angle)
  {
    center_ = center;
    angle_  = angle;
  }


  /**
   * Set pixel in given color
   * \param vertex Pixel coordinates
   * \param color Color of pixel in ARGB format
   */
  inline virtual void pixel_set(vertex_type vertex, color::value_type color) final
  {
    // get the destination pixel
    const vertex_type d = util::vertex_rotate(vertex, center_, angle_);
    // reverse iterate to match all pixels of the source vertex
    for (std::int16_t x = -1; x < 2; x++) {
      for (std::int16_t y = -1; y < 2; y++) {
        if (util::vertex_rotate({ d.x + x, d.y + y}, center_, -angle_) == vertex) {
          if (x != y || (x == 0 && y == 0))
            next_->pixel_set({ d.x + x, d.y + y}, color);
        }
      }
    }
  }


  /**
   * Return the color of the pixel
   * \param point Vertex of the pixel
   * \return Color of pixel in ARGB format
   */
  inline virtual color::value_type pixel_get(vertex_type vertex) final
  {
    return next_->pixel_get(util::vertex_rotate(vertex, center_, angle_));
  }
};


} // namespace shader
} // namespace vic

#endif  // _VIC_SHADER_ROTATE_H_
