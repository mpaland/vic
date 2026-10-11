///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2020, PALANDesign Hannover, Germany
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
// \brief Color gradient shaders
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
    : size_(0U)
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
   * \param color Color of pixel in ARGB format, only the alpha channel is used and multiplied with the gradient alpha
   */
  inline virtual void pixel_set(vertex_type vertex, color::value_type color) final
  {
    const color::value_type gc = mix(vertex);
    next_->pixel_set(vertex, color::set_alpha(gc, static_cast<std::uint8_t>(static_cast<std::uint16_t>(color::get_alpha(color)) * color::get_alpha(gc) / 255U)));
  }


protected:

  /**
   * Inverse distance weighting of all gradient colors
   * \param dist Array of the inverse (squared) distances to the gradient points, size_ elements
   * \return The weighted color
   */
  color::value_type weighted_color(const std::uint32_t* dist) const
  {
    // sum of all inverse distances
    std::uint32_t sd_sum = 0U;
    for (std::size_t n = 0U; n < size_; ++n) {
      sd_sum += dist[n];
    }
    if (sd_sum == 0U) {
      return color::none;
    }

    // accumulate the channels separately, the weights are scaled to 0 - 255 and
    // sum up to exactly 255 (the last point gets the rounding remainder), so no
    // channel can overflow or carry into its neighbor
    std::uint32_t a = 0U, r = 0U, g = 0U, b = 0U;
    std::uint32_t lum_sum = 0U;
    for (std::size_t n = 0U; n < size_; ++n) {
      const std::uint32_t lum = (n + 1U < size_) ? (255U * dist[n] / sd_sum) : (255U - lum_sum);
      lum_sum += lum;
      const color::value_type c = gradient_color_[n].color;
      a += color::get_alpha(c) * lum;
      r += color::get_red(c)   * lum;
      g += color::get_green(c) * lum;
      b += color::get_blue(c)  * lum;
    }
    return color::argb(static_cast<std::uint8_t>(r / 255U), static_cast<std::uint8_t>(g / 255U), static_cast<std::uint8_t>(b / 255U), static_cast<std::uint8_t>(a / 255U));
  }
};



/**
 * Color gradient shader
 * The gradient shader sets the pixel color to an interpolated color for a given vertex
 * Construct a gradient with 4 pixels like:
 * vic::shader::gradient<4> gr = { { 0, 0, vic::color::darkred }, { 100, 20, vic::color::darkorange }, { 250, 50, vic::color::brown }, { 300, 80, vic::color::green } };
 * dc_.shader_register(&gr);
 */
template<std::size_t Max_Pixel>
class gradient final : public gradient_base<Max_Pixel>
{
public:
  /**
   * Initializer list ctor
   * \param il Initializer list of N pixels
   */
  gradient(const std::initializer_list<pixel_type>& il)
    : gradient_base<Max_Pixel>(il)
  { }


  /**
   * Array ctor
   * \param gradient_pixels Array of exact Max_Pixel pixels
   */
  gradient(const pixel_type* gradient_pixels)
    : gradient_base<Max_Pixel>(gradient_pixels)
  { }


  /**
   * Return the mixed color at the given position
   * \param pos The position of the vertex
   * \return The mixed color at the given vertex position
   */
  virtual color::value_type mix(vertex_type pos)
  {
    std::uint32_t dist[Max_Pixel];            // distance calculation

    // calculate the inverse squared distances from pos to all points
    const std::uint32_t factor = 10000000UL;  // use 10M as factor
    for (std::size_t n = 0U; n < this->size_; ++n) {
      dist[n] = factor / (1U + util::distance_squared(pos, this->gradient_color_[n].vertex));
    }
    return this->weighted_color(dist);
  }
};



/**
 * The gradient_horizontal shader sets the pixel color to an interpolated color for a given vertex, y is ignored
 * Construct a gradient with 3 pixels like:
 * vic::shader::gradient_horizontal<3> gr = { { 0, 0, vic::color::darkred }, { 100, 20, vic::color::darkorange }, { 250, 50, vic::color::brown } };
 * dc_.shader_register(&gr);
 */
template<std::size_t Max_Pixel>
class gradient_horizontal final : public gradient_base<Max_Pixel>
{
public:
  /**
   * Initializer list ctor
   * \param il Initializer list of N pixels
   */
  gradient_horizontal(const std::initializer_list<pixel_type>& il)
    : gradient_base<Max_Pixel>(il)
  { }


  /**
   * Array ctor
   * \param gradient_pixels Array of exact Max_Pixel pixels
   */
  gradient_horizontal(const pixel_type* gradient_pixels)
    : gradient_base<Max_Pixel>(gradient_pixels)
  { }


  /**
   * Return the mixed color at the given position
   * \param pos The position of the vertex
   * \return The mixed color at the given vertex position
   */
  virtual color::value_type mix(vertex_type pos)
  {
    std::uint32_t dist[Max_Pixel];            // distance calculation

    const std::uint32_t factor = 10000000UL;  // use 10M as factor
    for (std::size_t n = 0U; n < this->size_; n++) {
      // ignore y
      const std::int32_t dx = this->gradient_color_[n].vertex.x - pos.x;
      dist[n] = factor / (1U + static_cast<std::uint32_t>(dx * dx));
    }
    return this->weighted_color(dist);
  }
};



/**
 * The gradient_vertical shader sets the pixel color to an interpolated color for a given vertex, x is ignored
 * Construct a gradient with 3 pixels like:
 * vic::shader::gradient_vertical<3> gr = { { 0, 0, vic::color::darkred }, { 100, 20, vic::color::darkorange }, { 250, 50, vic::color::brown } };
 * dc_.shader_register(&gr);
 */
template<std::size_t Max_Pixel>
class gradient_vertical final : public gradient_base<Max_Pixel>
{
public:
  /**
   * Initializer list ctor
   * \param il Initializer list of N pixels
   */
  gradient_vertical(const std::initializer_list<pixel_type>& il)
    : gradient_base<Max_Pixel>(il)
  { }


  /**
   * Array ctor
   * \param gradient_pixels Array of exact Max_Pixel pixels
   */
  gradient_vertical(const pixel_type* gradient_pixels)
    : gradient_base<Max_Pixel>(gradient_pixels)
  { }


  /**
   * Return the mixed color at the given position
   * \param pos The position of the vertex
   * \return The mixed color at the given vertex position
   */
  virtual color::value_type mix(vertex_type pos)
  {
    std::uint32_t dist[Max_Pixel];            // distance calculation

    const std::uint32_t factor = 10000000UL;  // use 10M as factor
    for (std::size_t n = 0U; n < this->size_; n++) {
      // ignore x
      const std::int32_t dy = this->gradient_color_[n].vertex.y - pos.y;
      dist[n] = factor / (1U + static_cast<std::uint32_t>(dy * dy));
    }
    return this->weighted_color(dist);
  }
};



/**
 * The gradient_solid shader sets the pixel color to the color of the nearest vertex
 * Construct a gradient with 3 pixels like
 * vic::shader::gradient_solid<3> gr = { { 0, 0, vic::color::darkred }, { 100, 20, vic::color::darkorange }, { 250, 50, vic::color::brown } };
 * dc_.shader_register(&gr);
 */
template<std::size_t Max_Pixel>
class gradient_solid final : public gradient_base<Max_Pixel>
{
public:
  /**
   * Initializer list ctor
   * \param il Initializer list of N pixels
   */
  gradient_solid(const std::initializer_list<pixel_type>& il)
    : gradient_base<Max_Pixel>(il)
  { }


  /**
   * Array ctor
   * \param gradient_pixels Array of exact Max_Pixel pixels
   */
  gradient_solid(const pixel_type* gradient_pixels)
    : gradient_base<Max_Pixel>(gradient_pixels)
  { }


  /**
   * Return the solid base color at the given position
   * \param pos The position of the vertex
   * \return The solid base color at the given position
   */
  virtual color::value_type mix(vertex_type pos)
  {
    // get the nearest point, the first one wins on equal distance
    std::uint32_t d_min = 0xFFFFFFFFUL;
    std::size_t   n_min = 0U;
    for (std::size_t n = 0U; n < this->size_; ++n) {
      const std::uint32_t d = util::distance_squared(pos, this->gradient_color_[n].vertex);
      if (d < d_min) {
        d_min = d;
        n_min = n;
      }
    }
    return this->size_ ? this->gradient_color_[n_min].color : color::none;
  }
};


} // namespace shader
} // namespace vic

#endif  // _VIC_SHADER_GRADIENT_H_
