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
// \brief Shader base class of the pixel shader pipeline
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_SHADER_H_
#define _VIC_SHADER_H_

#include "../drv.h"


namespace vic {
namespace shader {


/**
 * Abstract shader base class
 */
class base
{
protected:

  /**
   * default ctor
   */
  base()
    : next_(nullptr)
  { }

public:

  base* next_;   // pointer to next shader in pipe, nullptr if the current shader is the last one

  /**
   * Set pixel in given color
   * \param vertex Pixel coordinates
   * \param color Color of pixel in ARGB format
   */
  virtual void pixel_set(vertex_type vertex, color::value_type color) = 0;


  /**
   * Return the color of the pixel
   * \param point Vertex of the pixel
   * \return Color of pixel in ARGB format
   */
  virtual color::value_type pixel_get(vertex_type vertex)
  {
    return next_->pixel_get(vertex);
  }
};


class output : public base
{
  drv& head_;             // actual head instance
  bool alpha_blending_;   // alpha blending active (default)

public:

  output(drv& head)
    : head_(head)
    , alpha_blending_(true)
  { }

  /**
   * Set pixel in given color
   * \param vertex Pixel coordinates
   * \param color Color of pixel in ARGB format
   */
  inline virtual void pixel_set(vertex_type vertex, color::value_type color) final
  {
    head_.pixel_set(vertex, alpha_blending_ ? color::alpha_blend(color, head_.pixel_get(vertex)) : color);
  }


  /**
   * Return the color of the pixel
   * \param point Vertex of the pixel
   * \return Color of pixel in ARGB format
   */
  inline virtual color::value_type pixel_get(vertex_type vertex) final
  {
    return head_.pixel_get(vertex);
  }
};



class alpha_blend : public base
{
public:

  /**
   * Set pixel in given color
   * \param vertex Pixel coordinates
   * \param color Color of pixel in ARGB format
   */
  inline virtual void pixel_set(vertex_type vertex, color::value_type color) final
  {
    next_->pixel_set(vertex, color::alpha_blend(color, next_->pixel_get(vertex)));
  }
};



// clipping region shader
class clipping : public base
{
  rect_type region_;
  bool      inside_;
  bool      active_;

public:
  /**
   * ctor
   * Create a clipping region, default disabled
   */
  clipping()
    : inside_(false)
    , active_(false)
  { }


  /**
   * ctor
   * Create an enabled clipping region
   * \param region Clipping region
   * \param inside True if the clipping region is INSIDE the given box, so all pixels inside the clipping region are drawn. This is the default.
   */
  clipping(const rect_type& region, bool inside = true)
  {
    set(region, inside);
  }


  /**
   * Set the clipping region
   * \param region Clipping region
   * \param inside True if the clipping region is INSIDE the given box, so all pixels inside the clipping region are drawn. This is the default.
   */
  void set(const rect_type& region, bool inside = true)
  {
    region_ = region;
    inside_ = inside;
    active_ = true;
  }


  /**
   * Enable the clipping function
   * \param enable True to enable
   */
  inline void enable(bool _enable = true)
  {
    active_ = _enable;
  }


  /**
   * Test if clipping is active and if the given vertex is inside the clipping region
   * \param v Vertex to test
   * \return True if given vertex is within the active clipping region and should be drawn
   */
  inline bool is_inside(const vertex_type& v) const
  {
    return !active_ || (region_.contain(v) ? inside_ : !inside_);
  }


  /**
   * Return the clipping status
   * \return True if clipping is enabled
   */
  inline bool is_enabled() const
  {
    return active_;
  }


  /**
   * Set pixel in given color
   * \param vertex Pixel coordinates
   * \param color Color of pixel in ARGB format
   */
  inline virtual void pixel_set(vertex_type vertex, color::value_type color) final
  {
    if (is_inside(vertex)) {
      // process vertex
      next_->pixel_set(vertex, color);
    }
  }
};



// zoom vertex shader
class zoom : public base
{
  std::uint16_t x_level_;
  std::uint16_t y_level_;

public:

  /**
   * ctor
   * Create a clipping region, default disabled
   */
  zoom(std::uint16_t x_level, std::uint16_t y_level)
    : x_level_(x_level)
    , y_level_(y_level)
  { }


  /**
   * Set pixel in given color
   * \param vertex Pixel coordinates
   * \param color Color of pixel in ARGB format
   */
  inline virtual void pixel_set(vertex_type vertex, color::value_type color) final
  {
    for (std::int16_t y = vertex.y * y_level_, ye = y + y_level_; y < ye; ++y) {
      for (std::int16_t x = vertex.x * x_level_, xe = x + x_level_; x < xe; ++x) {
        next_->pixel_set({ x, y }, color);
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
    return next_->pixel_get({ static_cast<std::int16_t>(vertex.x * x_level_), static_cast<std::int16_t>(vertex.y * y_level_) });
  }
};



// offset vertex shader
class offset : public base
{
  std::int16_t x_offset_;
  std::int16_t y_offset_;

public:

  /**
   * ctor
   * Create a clipping region, default disabled
   */
  offset(std::uint16_t x_offset, std::int16_t y_offset)
    : x_offset_(x_offset)
    , y_offset_(y_offset)
  { }


  /**
   * Set pixel in given color
   * \param vertex Pixel coordinates
   * \param color Color of pixel in ARGB format
   */
  inline virtual void pixel_set(vertex_type vertex, color::value_type color) final
  {
    next_->pixel_set({ static_cast<std::int16_t>(vertex.x + x_offset_), static_cast<std::int16_t>(vertex.y + y_offset_) }, color);
  }


  /**
   * Return the color of the pixel
   * \param point Vertex of the pixel
   * \return Color of pixel in ARGB format
   */
  inline virtual color::value_type pixel_get(vertex_type vertex) final
  {
    return next_->pixel_get({ static_cast<std::int16_t>(vertex.x + x_offset_), static_cast<std::int16_t>(vertex.y + y_offset_) });
  }
};


} // namespace shader
} // namespace vic

#endif  // _VIC_SHADER_H_
