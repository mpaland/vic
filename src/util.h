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
} vertex_type;


/**
 * Structure to store coordinates and an associated ARGB color
 */
typedef struct tag_pixel_type {
  vertex_type   vertex;
  std::uint32_t color;
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
 * \return The squared distance. To get the real distance square root this value
 */
inline std::uint32_t distance_squared(vertex_type a, vertex_type b)
{
  return static_cast<std::uint32_t>((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
}


} // namespace util
} // namespace vgx

#endif  // _VGX_UTIL_H_
