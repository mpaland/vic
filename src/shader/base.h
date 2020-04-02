///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2018, PALANDesign Hannover, Germany
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
  inline virtual color::value_type pixel_get(vertex_type vertex)
  {
    return next_->pixel_get(vertex);
  }
};



/**
 * Output shader
 * This is always the last shader in the pipe and it sends the pixels to the head
 */
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
   * Enable/disable alpha blending support
   * \param enable True to enable alpha blending
   */
  inline void alpha_blending_enable(bool enable = true)
  {
    alpha_blending_ = enable;
  }


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



/**
 * Alpha blending shader
 * This shader alpha blends all pixels
 * Use this for dynamic alpha blending effects when alpha blending is disabled in the output shader
 */
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



/**
 * Clipping region shader
 * This shader draws all vertices which are inside (true) or outside (false) of the defined clipping region
 */
class clipping : public base
{
  rect_type region_;
  bool      inside_;    // true for drawing, false to suppress all vertices inside the region

public:

  /**
   * ctor
   * Create a clipping region, default disabled
   */
  clipping()
    : region_({ 0, 0, 0, 0 })
    , inside_(false)
  { }


  /**
   * ctor
   * Create an enabled clipping region
   * \param region Clipping region
   * \param inside True if the clipping region is INSIDE the given box, so all pixels INSIDE the clipping region are drawn. This is the default.
   */
  clipping(const rect_type& region, bool inside = true)
  {
    set(region, inside);
  }


  /**
   * Set the clipping region
   * \param region Clipping region
   * \param inside True if the clipping region is INSIDE the given box, so all pixels INSIDE the clipping region are drawn. This is the default.
   */
  void set(const rect_type& region, bool inside = true)
  {
    region_ = region;
    inside_ = inside;
  }


  /**
   * Get the actual clipping region
   * \return Clipping region
   */
  inline rect_type get() const
  {
    return region_;
  }


  /**
   * Test if the given vertex is inside the clipping region - and should be drawn
   * \param vertex Vertex to test
   * \return True if given vertex is within the active clipping region and should be drawn
   */
  inline bool is_inside(const vertex_type& vertex) const
  {
    // region_contain  inside_  is_inside
    //       1            1         1
    //       1            0         0
    //       0            1         0
    //       0            0         1
    return !(region_.contain(vertex) ^ inside_);
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



/**
 * Zoom vertex shader
 * This shader zooms all processed vertices
 */
class zoom : public base
{
  std::uint16_t x_level_;
  std::uint16_t y_level_;

public:

  /**
   * ctor
   * Set the zoom level
   * \param x_level Zoom factor for x-axis
   * \param x_level Zoom factor for y-axis
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
    for (std::int_fast16_t y = vertex.y * y_level_, ye = y + y_level_; y < ye; y++) {
      for (std::int_fast16_t x = vertex.x * x_level_, xe = x + x_level_; x < xe; x++) {
        next_->pixel_set({ static_cast<std::int16_t>(x), static_cast<std::int16_t>(y) }, color);
      }
    }
  }


  /**
   * Return the color of the pixel
   * \param vertex Vertex of the pixel
   * \return Color of pixel in ARGB format
   */
  inline virtual color::value_type pixel_get(vertex_type vertex) final
  {
    return next_->pixel_get({ static_cast<std::int16_t>(vertex.x * x_level_), static_cast<std::int16_t>(vertex.y * y_level_) });
  }
};



/**
 * Offset vertex shader
 * This shader adds an offset to all processed vertices
 */
class offset : public base
{
  vertex_type offset_;

public:

  /**
   * ctor
   * \param _offset Offset to set
   */
  offset(vertex_type _offset)
    : offset_(_offset)
  { }


  /**
   * Set the offset
   * \param _offset Offset to set
   */
  inline void set(vertex_type _offset)
  {
    offset_ = _offset;
  }


  /**
   * Get the actual offset
   * \return Actual offset
   */
  inline vertex_type get() const
  {
    return offset_;
  }


  /**
   * Set pixel in given color
   * \param vertex Pixel coordinates
   * \param color Color of pixel in ARGB format
   */
  inline virtual void pixel_set(vertex_type vertex, color::value_type color) final
  {
    next_->pixel_set(vertex + offset_, color);
  }


  /**
   * Return the color of the pixel
   * \param vertex Vertex of the pixel
   * \return Color of pixel in ARGB format
   */
  inline virtual color::value_type pixel_get(vertex_type vertex) final
  {
    return next_->pixel_get(vertex + offset_);
  }
};



/**
 * Flip (mirror) vertex shader class
 */
class flip final : public base
{
  bool        enable_h_;
  bool        enable_v_;
  vertex_type axis_h_;
  vertex_type axis_v_;

public:

  /**
  * ctor
  */
  flip()
    : enable_h_(false)
    , enable_v_(false)
  { }


  /**
  * Enable/disable the horizontal flip effect
  * \param enable Enable/disable the horizontal flip effect
  * \param axis Horizontal flip axis
  */
  void set_horizontal(bool enable, const vertex_type& axis)
  {
    enable_h_ = enable;
    axis_h_   = axis;
  }


  /**
  * Enable/disable the vertical flip effect
  * \param enable Enable/disable the vertical flip effect
  * \param axis Vertical flip axis
  */
  void set_vertical(bool enable, const vertex_type& axis)
  {
    enable_v_ = enable;
    axis_v_   = axis;
  }


  /**
   * Set pixel in given color
   * \param vertex Pixel coordinates
   * \param color Color of pixel in ARGB format
   */
  inline virtual void pixel_set(vertex_type vertex, color::value_type color)
  {
    next_->pixel_set({ enable_h_ ? 2 * axis_h_.x - vertex.x : vertex.x,
                       enable_v_ ? 2 * axis_v_.y - vertex.y : vertex.y }, color);
  }


  /**
  * Return the color of the pixel
  * \param vertex Vertex of the pixel
  * \return Color of pixel in ARGB format
  */
  inline virtual color::value_type pixel_get(vertex_type vertex)
  {
    return next_->pixel_get({ enable_h_ ? 2 * axis_h_.x - vertex.x : vertex.x,
                              enable_v_ ? 2 * axis_v_.y - vertex.y : vertex.y });
  }
};


} // namespace shader
} // namespace vic

#endif  // _VIC_SHADER_H_
