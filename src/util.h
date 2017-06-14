///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2017-2017, PALANDesign Hannover, Germany
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
// \brief Utils and types for vgx
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_UTIL_H_
#define _VGX_UTIL_H_

#include <cstdint>
#include <cstddef>


namespace vgx {


/**
 * Structure to store coordinates
 */
typedef struct tag_vertex_type {
  std::int16_t x;
  std::int16_t y;

  bool operator==(tag_vertex_type o) {
    return (x == o.x) && (y == o.y);
  }
  bool operator!=(tag_vertex_type o) {
    return (x != o.x) || (y != o.y);
  }
} vertex_type;


/**
 * Structure to store coordinates and an associated ARGB color
 */
typedef struct tag_pixel_type {
  vertex_type   vertex;   // vertex
  std::uint32_t color;    // ARGB color
} pixel_type;


namespace util {


// return the minimum of a and b
inline std::int16_t min2(std::int16_t a, std::int16_t b)
{
  return b ^ ((a ^ b) & -(a < b));
}


// return the maximum of a and b
inline std::int16_t max2(std::int16_t a, std::int16_t b)
{
  return a ^ ((a ^ b) & -(a < b));
}


// return the minimum of a, b and c
inline std::int16_t min3(std::int16_t a, std::int16_t b, std::int16_t c)
{
  return min2(min2(a, b), c);
}


// return the maximum of a, b and c
inline std::int16_t max3(std::int16_t a, std::int16_t b, std::int16_t c)
{
  return max2(max2(a, b), c);
}


/**
 * Return the absolue value
 * \param val Input value, negative or positive
 * \return Positive value
 */
template<typename T>
inline T abs(T val)
{
  const T mask = val >> (sizeof(T) * 8U - 1U);
  return (val ^ mask) - mask;
}


/**
 * Calculate the squared distance between two vertices
 * \param a Vertex a
 * \param b Vertex b
 * \return The squared distance. To get the real distance square root this returned value
 */
inline std::uint32_t distance_squared(vertex_type a, vertex_type b)
{
  return static_cast<std::uint32_t>((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
}


/**
 * \param a Vertex a
 * \param b Vertex b
 * \param c Vertex c
 * \return Orient 2D value
 */
inline std::int16_t orient_2d(vertex_type a, vertex_type b, vertex_type c)
{
  return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}


/**
 * Helper function to calculate (lookup) sin(x) normalized to 16384 (to be fast divided by 2^15)
 * \param angle Angle in degree
 * \return sin(x) * 16384
 */
inline std::int16_t sin(std::int16_t angle)
{
  static const std::int16_t sin90[90] = {
        0,   286,   572,   857,  1143,  1428,  1713,  1997,  2280,  2563,
     2845,  3126,  3406,  3686,  3964,  4240,  4516,  4790,  5063,  5334,
     5604,  5872,  6138,  6402,  6664,  6924,  7182,  7438,  7692,  7943,
     8192,  8438,  8682,  8923,  9162,  9397,  9630,  9860, 10087, 10311,
    10531, 10749, 10963, 11174, 11381, 11585, 11786, 11982, 12176, 12365,
    12551, 12733, 12911, 13085, 13255, 13421, 13583, 13741, 13894, 14044,
    14189, 14330, 14466, 14598, 14726, 14849, 14968, 15082, 15191, 15296,
    15396, 15491, 15582, 15668, 15749, 15826, 15897, 15964, 16026, 16083,
    16135, 16182, 16225, 16262, 16294, 16322, 16344, 16362, 16374, 16382
  };

  const bool invers = angle < 0;
  angle = abs<std::int16_t>(angle) % 360;

  if ((static_cast<std::uint16_t>(angle) == 90U) || (static_cast<std::uint16_t>(angle) == 270U)) {
    return 16384;
  }

  std::int16_t val = sin90[static_cast<std::uint16_t>(angle) % 90U];
  return (angle <= 180) != invers ? val : -val;
}


/**
 * Helper function to calculate (lookup) cos(x) normalized to 16384
 * \param angle Angle in degree
 * \return cos(x) * 16384
 */
inline std::int16_t cos(std::int16_t angle)
{
  return sin(90 - angle);
}


/**
 * Helper function to rotate a vertex of a given angle in respect to given center
 */
inline vertex_type vertex_rotate(vertex_type point, vertex_type center, std::int16_t angle)
{
  const std::int32_t s = sin(angle);  // normalized to 16384
  const std::int32_t c = cos(angle);  // normalized to 16384

  // translate point to center
  point.x -= center.x;
  point.y -= center.y;

  // rotate point and translate back
  vertex_type rp;
  rp.x = static_cast<std::int16_t>(((std::int32_t)point.x * c - (std::int32_t)point.y * s) / 16384) + center.x;
  rp.y = static_cast<std::int16_t>(((std::int32_t)point.x * s - (std::int32_t)point.y * c) / 16384) + center.y;

  return rp;
}


} // namespace util
} // namespace vgx

#endif  // _VGX_UTIL_H_
