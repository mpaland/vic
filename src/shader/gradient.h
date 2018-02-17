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
// \brief Shader of the pixel shader pipeline
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_SHADER_GRADIENT_H_
#define _VIC_SHADER_GRADIENT_H_

#include "base.h"
#include <initializer_list>


namespace vic {
namespace shader {


/**
 * Abstract color gradient shader base class
 */
template<std::size_t Max_Pixel>
class gradient_base : public base
{
protected:

  pixel_type  gradient_color_[Max_Pixel];  // array of the gradient colors and the according color values
  std::size_t size_;                       // actual pixel count

public:
  /**
   * Initializer list ctor
   * \param il Initializer list of N pixels
   */
  gradient_base(const std::initializer_list<pixel_type>& il)
    : size_(0U)
  {
    for (std::initializer_list<pixel_type>::const_iterator it = il.begin(); (it != il.end()) && (size_ < Max_Pixel); ++it) {
      gradient_color_[size_++] = *it;
    }
  }


  /**
   * Array ctor
   * \param gradient_pixels Array of exact Max_Pixel pixels
   */
  gradient_base(const pixel_type* gradient_pixels)
  {
    set(gradient_pixels, Max_Pixel);
  }


  /**
   * Set the actual gradient pixels
   * \param gradient_pixels Array of pixels of size 'pixel_count'
   * \param pixel_count Count of the given pixels
   */
  void set(const pixel_type* gradient_pixels, std::size_t pixel_count)
  {
    size_ = pixel_count < Max_Pixel ? pixel_count : Max_Pixel;
    for (std::size_t n = 0U; n < size_; ++n) {
      gradient_color_[n] = gradient_pixels[n];
    }
  }


  /**
   * Return the mixed color at the given position
   * \param pos The position of the vertex
   * \return The mixed color at the given vertex position
   */
  virtual color::value_type mix(vertex_type pos) = 0;


  /**
   * Set pixel in given color
   * \param vertex Pixel coordinates
   * \param color Color of pixel in ARGB format
   */
  inline virtual void pixel_set(vertex_type vertex, color::value_type color) final
  {
    const color::value_type gc = mix(vertex);
    next_->pixel_set(vertex, color::set_alpha(gc, color::get_alpha(color) * color::get_alpha(gc) / 255U));
  }
};



/**
 * Color gradient shader
 * The gradient shader sets the pixel color to an interpolated color for a given vertex
 * Construct a gradient with 4 pixels like:
 * vic::shader::gradient_horizontal<4> gr = { { 0, 0, vic::color::darkred }, { 100, 20, vic::color::darkorange }, { 250, 50, vic::color::brown }, { 300, 80, vic::color::green } };
 * dc_.shader_register(&gr);
 */
template<std::size_t Max_Pixel>
class gradient : public gradient_base<Max_Pixel>
{
public:
  /**
   * Initializer list ctor
   * \param il Initializer list of N pixels
   */
  gradient(const std::initializer_list<pixel_type>& il)
    : gradient_base(il)
  { }


  /**
   * Array ctor
   * \param gradient_pixels Array of exact Max_Pixel pixels
   */
  gradient(const pixel_type* gradient_pixels)
    : gradient_base(gradient_pixels)
  { }


  /**
   * Return the mixed color at the given position
   * \param pos The position of the vertex
   * \return The mixed color at the given vertex position
   */
  virtual color::value_type mix(vertex_type pos) final
  {
    std::uint32_t dist_[Max_Pixel];           // distance calculation

    // calculate the sum of all inverse squared distances from pos to all points
    const std::uint32_t factor = 10000000UL;  // use 10M as factor
    std::uint32_t sd_sum = 0U;
    for (std::size_t n = 0U; n < size_; ++n) {
      dist_[n] = factor / (1U + util::distance_squared(pos, gradient_color_[n].vertex));
      sd_sum += dist_[n];
    }
    // assemble the mixed color at given pos
    color::value_type c = color::none;
    for (std::size_t n = 0U; n < size_; ++n) {
      c += (color::dim(gradient_color_[n].color, static_cast<std::uint8_t>(0x100UL * dist_[n] / sd_sum)) & 0x00FFFFFF) |            // RGB
                       ((static_cast<color::value_type>(color::get_alpha(gradient_color_[n].color)) * dist_[n] / sd_sum) << 24U);   // alpha
    }
    return c;
  }
};



/**
 * The gradient_horizontal shader sets the pixel color to an interpolated color for a given vertex, y is ignored
 * Construct a gradient with 3 pixels like:
 * vic::shader::gradient_horizontal<3> gr = { { 0, 0, vic::color::darkred }, { 100, 20, vic::color::darkorange }, { 250, 50, vic::color::brown } };
 * dc_.shader_register(&gr);
 */
template<std::size_t Max_Pixel>
class gradient_horizontal : public gradient_base<Max_Pixel>
{
public:
  /**
   * Initializer list ctor
   * \param il Initializer list of N pixels
   */
  gradient_horizontal(const std::initializer_list<pixel_type>& il)
    : gradient_base(il)
  { }


  /**
   * Array ctor
   * \param gradient_pixels Array of exact Max_Pixel pixels
   */
  gradient_horizontal(const pixel_type* gradient_pixels)
    : gradient_base(gradient_pixels)
  { }


  /**
   * Return the mixed color at the given position
   * \param pos The position of the vertex
   * \return The mixed color at the given vertex position
   */
  virtual color::value_type mix(vertex_type pos) final
  {
    std::uint32_t dist_[Max_Pixel];           // distance calculation

    // calculate the sum of all inverse squared distances from pos to all points
    const std::uint32_t factor = 10000000UL;  // use 10M as factor
    std::uint32_t sd_sum = 0U;
    for (std::size_t n = 0U; n < size_; n++) {
      // ignore y
      const std::int16_t dx = gradient_color_[n].vertex.x - pos.x;
      dist_[n] = factor / (1U + dx * dx);
      sd_sum += dist_[n];
    }
    // assemble the mixed color at given pos
    color::value_type c = color::none;
    for (std::size_t n = 0U; n < size_; n++) {
      c += (color::dim(gradient_color_[n].color, static_cast<std::uint8_t>(0x100UL * dist_[n] / sd_sum)) & 0x00FFFFFF) |            // RGB
                       ((static_cast<color::value_type>(color::get_alpha(gradient_color_[n].color)) * dist_[n] / sd_sum) << 24U);   // alpha
    }
    return c;
  }
};



/**
 * The gradient_vertical shader sets the pixel color to an interpolated color for a given vertex, x is ignored
 * Construct a gradient with 3 pixels like:
 * vic::shader::gradient_horizontal<3> gr = { { 0, 0, vic::color::darkred }, { 100, 20, vic::color::darkorange }, { 250, 50, vic::color::brown } };
 * dc_.shader_register(&gr);
 */
template<std::size_t Max_Pixel>
class gradient_vertical : public gradient_base<Max_Pixel>
{
public:
  /**
   * Initializer list ctor
   * \param il Initializer list of N pixels
   */
  gradient_vertical(const std::initializer_list<pixel_type>& il)
    : gradient_base(il)
  { }


  /**
   * Array ctor
   * \param gradient_pixels Array of exact Max_Pixel pixels
   */
  gradient_vertical(const pixel_type* gradient_pixels)
    : gradient_base(gradient_pixels)
  { }


  /**
   * Return the mixed color at the given position
   * \param pos The position of the vertex
   * \return The mixed color at the given vertex position
   */
  virtual color::value_type mix(vertex_type pos) final
  {
    std::uint32_t dist_[Max_Pixel];           // distance calculation

    // calculate the sum of all inverse squared distances from pos to all points
    const std::uint32_t factor = 10000000UL;  // use 10M as factor
    std::uint32_t sd_sum = 0U;
    for (std::size_t n = 0U; n < size_; n++) {
      // ignore x
      const std::int16_t dy = gradient_color_[n].vertex.y - pos.y;
      dist_[n] = factor / (1U + dy * dy);
      sd_sum += dist_[n];
    }
    // assemble the mixed color at given pos
    color::value_type c = color::none;
    for (std::size_t n = 0U; n < size_; n++) {
      c += (color::dim(gradient_color_[n].color, static_cast<std::uint8_t>(0x100UL * dist_[n] / sd_sum)) & 0x00FFFFFF) |            // RGB
                       ((static_cast<color::value_type>(color::get_alpha(gradient_color_[n].color)) * dist_[n] / sd_sum) << 24U);   // alpha
    }
    return c;
  }
};



/**
 * The gradient_solid shader sets the pixel color to the color of the nearest vertex
 * Construct a gradient with 3 pixels like
 * vic::shader::gradient_solid<3> gr = { { 0, 0, vic::color::darkred }, { 100, 20, vic::color::darkorange }, { 250, 50, vic::color::brown } };
 * dc_.shader_register(&gr);
 */
template<std::size_t Max_Pixel>
class gradient_solid : public gradient_base<Max_Pixel>
{
public:
  /**
   * Initializer list ctor
   * \param il Initializer list of N pixels
   */
  gradient_solid(const std::initializer_list<pixel_type>& il)
    : gradient_base(il)
  { }


  /**
   * Array ctor
   * \param gradient_pixels Array of exact Max_Pixel pixels
   */
  gradient_solid(const pixel_type* gradient_pixels)
    : gradient_base(gradient_pixels)
  { }


  /**
   * Return the solid base color at the given position
   * \param pos The position of the vertex
   * \return The solid base color at the given position
   */
  virtual color::value_type mix(vertex_type pos) final
  {
    std::uint32_t dist_[Max_Pixel];           // distance calculation

    // calculate the sum of all inverse squared distances from pos to all points
    const std::uint32_t factor = 10000000UL;  // use 10M as factor
    std::uint32_t sd_sum = 0U;
    for (std::size_t n = 0U; n < size_; ++n) {
      dist_[n] = factor / (1U + util::distance_squared(pos, gradient_color_[n].vertex));
      sd_sum += dist_[n];
    }

    // get the solid color at given pos
    std::uint32_t lum_max = 0U;
    std::size_t   n_max   = 0U;
    for (std::size_t n = 0U; n < size_; ++n) {
      const std::uint32_t lum = 0x100UL * dist_[n] / sd_sum;
      if (lum > lum_max) {
        lum_max = lum;
        n_max   = n;
      }
    }
    return gradient_color_[n_max].color;
  }
};


} // namespace shader
} // namespace vic

#endif  // _VIC_SHADER_GRADIENT_H_
