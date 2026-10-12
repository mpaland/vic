///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2017-2018, PALANDesign Hannover, Germany
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
// \brief Utils and types for vic
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_UTIL_H_
#define _VIC_UTIL_H_

#include <cstdint>
#include <cstddef>


namespace vic {


/**
 * POD structure to store a vertex (x,y coordinates)
 */
typedef struct tag_vertex_type {
  std::int16_t x;
  std::int16_t y;

  // operators
  inline bool operator==(const tag_vertex_type& rhs) const {
    return (x == rhs.x) && (y == rhs.y);
  }
  inline bool operator!=(const tag_vertex_type& rhs) const {
    return (x != rhs.x) || (y != rhs.y);
  }
  inline bool operator<(const tag_vertex_type& rhs) const {
    return (y < rhs.y) || ((y == rhs.y) && (x < rhs.x));
  }
  inline tag_vertex_type operator+(const tag_vertex_type& rhs) const {
    return { static_cast<std::int16_t>(x + rhs.x), static_cast<std::int16_t>(y + rhs.y) };
  }
  inline tag_vertex_type operator-(const tag_vertex_type& rhs) const {
    return { static_cast<std::int16_t>(x - rhs.x), static_cast<std::int16_t>(y - rhs.y) };
  }
  inline tag_vertex_type& operator+=(const tag_vertex_type& rhs) {
    x = static_cast<std::int16_t>(x + rhs.x);
    y = static_cast<std::int16_t>(y + rhs.y);
    return *this;
  }
  inline tag_vertex_type& operator-=(const tag_vertex_type& rhs) {
    x = static_cast<std::int16_t>(x - rhs.x);
    y = static_cast<std::int16_t>(y - rhs.y);
    return *this;
  }

  /**
   * Helper function to swap two vertices (as coordinates)
   * \param v2 Second vertex
   */
  inline void swap(tag_vertex_type& v2)
  {
    const tag_vertex_type v = v2;
    v2    = *this;
    *this = v;
  }

  /**
   * Helper function to swap two vertices so that this vertex contains min x
   * \param max_x Vertex with the bigger x coord on return
   */
  inline void min_x(tag_vertex_type& max_x)
  {
    if (x > max_x.x) {
      swap(max_x);
    }
  }

  /**
   * Helper function to swap two vertices so that this vertex contains min y
   * \param max_y Vertex with the bigger y coord on return
   */
  inline void min_y(tag_vertex_type& max_y)
  {
    if (y > max_y.y) {
      swap(max_y);
    }
  }
} vertex_type;


/**
 * POD structure to store a pixel (vertex with an associated ARGB color)
 */
typedef struct tag_pixel_type {
  vertex_type   vertex;
  std::uint32_t color;    // ARGB 8-8-8-8 color

  // operators
  inline bool operator==(const tag_pixel_type& rhs) const {
    return vertex == rhs.vertex;
  }
  inline bool operator!=(const tag_pixel_type& rhs) const {
    return vertex != rhs.vertex;
  }
  inline tag_pixel_type operator+(const tag_vertex_type& rhs) const {
    return {{ static_cast<std::int16_t>(vertex.x + rhs.x), static_cast<std::int16_t>(vertex.y + rhs.y) }, color };
  }
  inline tag_pixel_type operator-(const tag_vertex_type& rhs) const {
    return {{ static_cast<std::int16_t>(vertex.x - rhs.x), static_cast<std::int16_t>(vertex.y - rhs.y) }, color };
  }
  inline tag_pixel_type& operator+=(const tag_vertex_type& rhs) {
    vertex.x = static_cast<std::int16_t>(vertex.x + rhs.x);
    vertex.y = static_cast<std::int16_t>(vertex.y + rhs.y);
    return *this;
  }
} pixel_type;


/**
 * POD structure to store rectangles
 * The rect must be "normalized", means top <= bottom and left <= right
 * Top and left side is within the rect. Bottom and right side are outside the rect.
 */
typedef struct tag_rect_type {
  std::int16_t left;
  std::int16_t top;
  std::int16_t right;
  std::int16_t bottom;

  // clear rect
  inline void clear()
  { *this = { 0, 0, 0, 0 }; }

  // set rect and normalize input
  // Both vertices will be inside the box
  // Right and bottom are clamped to the int16 maximum, so a vertex at 32767 is NOT inside
  void normalize(const vertex_type& v1, const vertex_type& v2)
  {
    const std::int32_t max_x = (v2.x < v1.x ? v1.x : v2.x) + 1;
    const std::int32_t max_y = (v2.y < v1.y ? v1.y : v2.y) + 1;
    left   = v1.x < v2.x ? v1.x : v2.x;
    top    = v1.y < v2.y ? v1.y : v2.y;
    right  = static_cast<std::int16_t>(max_x > 32767 ? 32767 : max_x);
    bottom = static_cast<std::int16_t>(max_y > 32767 ? 32767 : max_y);
  }

  inline vertex_type& top_left()
  { return *((vertex_type*)this); }

  inline const vertex_type& top_left() const
  { return *((vertex_type*)this); }

  inline vertex_type& bottom_right()
  { return *((vertex_type*)this + 1U); }

  inline const vertex_type& bottom_right() const
  { return *((vertex_type*)this + 1U); }

  inline std::int16_t width() const
  { return right - left; }

  inline std::int16_t height() const
  { return bottom - top; }

  // returns true if width and/or height is 0
  inline bool empty() const
  { return !width() || !height(); }

  /**
   * A vertex is within the rect if it lies on the left or top side or is within all four sides.
   * A vertex on the right or bottom side is outside the rect.
   * \param vertex The vertex to check
   * \return True if the vertex is inside the rect
   */
  inline bool contain(const vertex_type& vertex) const
  {
    return (vertex.x < right) && (vertex.y < bottom) && (vertex.x >= left) && (vertex.y >= top);
  }

  /**
   * Inflate the rect so that the given vertex is inside the rect
   * If the vertex is already inside the rect, nothing is done
   * \param rect The vertex to be inside the rect
   */
  inline void inflate(const vertex_type& vertex)
  {
    if (vertex.x >= right) {
      right = vertex.x + 1;
    }
    else if (vertex.x < left) {
      left = vertex.x;
    }
    if (vertex.y >= bottom) {
      bottom = vertex.y + 1;
    }
    else if (vertex.y < top) {
      top = vertex.y;
    }
  }

  // operators
  inline bool operator==(const tag_rect_type& rhs) const {
    return (right == rhs.right) && (bottom == rhs.bottom) && (left == rhs.left) && (top == rhs.top);
  }
  inline bool operator!=(const tag_rect_type& rhs) const {
    return !(*this == rhs);
  }
} rect_type;



namespace util {


// return the minimum of a and b
template <typename T>
inline const T& min2(const T& a, const T& b)
{
  return a < b ? a : b;
}


// return the maximum of a and b
template <typename T>
inline T& max2(T& a, T& b)
{
  return a > b ? a : b;
}


// return the minimum of a, b and c
template <typename T>
inline const T& min3(const T& a, const T& b, const T& c)
{
  return min2(min2(a, b), c);
}


// return the maximum of a, b and c
template <typename T>
inline const T& max3(const T& a, const T& b, const T& c)
{
  return max2(max2(a, b), c);
}


/**
 * Swap values so that minimum contains the minimum afterwarts
 * \param minimum Minimum value after swap
 * \param maximum Maximum value after swap
 */
template<typename T>
inline void min_max_swap(T& minimum, T& maximum)
{
  if (minimum > maximum) {
    const T tmp = minimum;
    minimum     = maximum;
    maximum     = tmp;
  }
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
 * Decode one UTF-8 sequence (1 to 3 bytes, BMP range) and advance the string pointer
 * The function never reads past a terminating NUL, the continuation bytes are checked one by one.
 * Invalid lead bytes, stray continuation bytes and 4 byte sequences are skipped byte wise.
 * \param string Reference to the string pointer, advanced past the decoded sequence. Stays on the NUL at the end of the string
 * \return The decoded code point in 16 bit UNICODE format, 0 at the end of the string
 */
inline std::uint16_t utf8_decode(const std::uint8_t*& string)
{
  for (;;) {
    const std::uint8_t b0 = *string;
    if (b0 == 0x00U) {
      // end of string, don't advance
      return 0U;
    }
    if (b0 < 0x80U) {
      // 1 byte sequence (ASCII char)
      ++string;
      return b0;
    }
    if ((b0 & 0xE0U) == 0xC0U) {
      // 2 byte sequence
      const std::uint8_t b1 = string[1];
      if ((b1 & 0xC0U) != 0x80U) {
        // truncated or invalid sequence, skip the lead byte only
        ++string;
        continue;
      }
      string += 2U;
      return static_cast<std::uint16_t>((static_cast<std::uint32_t>(b0 & 0x1FU) << 6U) | (b1 & 0x3FU));
    }
    if ((b0 & 0xF0U) == 0xE0U) {
      // 3 byte sequence
      const std::uint8_t b1 = string[1];
      if ((b1 & 0xC0U) != 0x80U) {
        ++string;
        continue;
      }
      const std::uint8_t b2 = string[2];   // only read after b1 is proven to be no NUL
      if ((b2 & 0xC0U) != 0x80U) {
        string += 2U;
        continue;
      }
      string += 3U;
      return static_cast<std::uint16_t>((static_cast<std::uint32_t>(b0 & 0x0FU) << 12U) | (static_cast<std::uint32_t>(b1 & 0x3FU) << 6U) | (b2 & 0x3FU));
    }
    // 4 byte sequence lead byte or stray continuation byte, skip it
    ++string;
  }
}


/**
 * Integer square root
 * \param value Input value
 * \return floor(sqrt(value))
 */
inline std::uint32_t isqrt(std::uint32_t value)
{
  std::uint32_t result = 0U;
  std::uint32_t bit = 1UL << 30U;    // the highest power of four <= value
  while (bit > value) {
    bit >>= 2U;
  }
  while (bit != 0U) {
    if (value >= result + bit) {
      value  -= result + bit;
      result  = (result >> 1U) + bit;
    }
    else {
      result >>= 1U;
    }
    bit >>= 2U;
  }
  return result;
}


/**
 * Helper function for fast byte reverse (e.g. 0x80 to 0x01, 0x40 to 0x02)
 * \param data Input byte
 * \return Reversed byte
 */
inline std::uint8_t byte_reverse(std::uint8_t data)
{
  return static_cast<std::uint8_t>(((data * 0x0802LU & 0x22110LU) | (data * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16U);
}


/**
 * Helper function for fast division by 255
 * Normally a modern compiler should use this / 255 replacement under the hood.
 * \param data Input data
 * \return data / 255U
 */
inline std::uint16_t div255(std::uint16_t data)
{
  return static_cast<std::uint16_t>((++data * 257U) >> 16U);
}


/**
 * Helper function for integer division with round to closest, like 8 / 3 = 3
 * \param data Input data
 * \param divisor Divisor
 * \return data / divisor
 */
inline std::int32_t div_round_closest(std::int32_t data, std::int16_t divisor)
{
  return ((data < 0) ^ (divisor < 0)) ? ((data - divisor / 2) / divisor) : ((data + divisor / 2) / divisor);
}


/**
 * Calculate the squared distance between two vertices (v0 and v1)
 * \param v0 Vertex v0
 * \param v1 Vertex v1
 * \return The squared distance. To get the real distance square root this returned value
 */
inline std::uint32_t distance_squared(const vertex_type& v0, const vertex_type& v1)
{
  return static_cast<std::uint32_t>((v1.x - v0.x) * (v1.x - v0.x) + (v1.y - v0.y) * (v1.y - v0.y));
}


/**
 * Orient 2D function
 * v0 and v1 define a line, result gives the relation of p to this line
 * \param p Vertex to test
 * \param v0 Line start vertex
 * \param v1 Line end vertex
 * \return result > 0 if p is on the "right" side, result = 0 if p is exactly on the line, result < 0 if p is on the "left" side
 */
inline std::int32_t orient_2d(const vertex_type& p, const vertex_type& v0, const vertex_type& v1)
{
  return (p.x - v1.x) * (v0.y - v1.y) - (p.y - v1.y) * (v0.x - v1.x);
}


/**
 * Fast sine function
 * Calculates (lookup) sin(x) normalized to 16384 (to be fast divided by 2^14)
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

  // use 32 bit math, abs(-32768) doesn't fit into int16
  bool invers = angle < 0;
  angle = static_cast<std::int16_t>(abs<std::int32_t>(angle) % 360);
  invers = (angle <= 180) ? invers : !invers;
  angle = static_cast<std::int16_t>(angle % 180);

  if (angle == 90) {
    return invers ? -16384 : 16384;
  }

  const std::int16_t val = sin90[angle < 90 ? angle : 180 - angle];
  return invers ? static_cast<std::int16_t>(-val) : val;
}


/**
 * Fast cosine function
 * Calculates (lookup) cos(x) normalized to 16384
 * \param angle Angle in degree
 * \return cos(x) * 16384
 */
inline std::int16_t cos(std::int16_t angle)
{
  return sin(90 - angle);
}


/**
 * Helper function to rotate a vertex around a given center with precomputed sine and cosine
 * Use this variant if many vertices are rotated by the same angle
 * \param point Vertex to rotate
 * \param center Rotation center
 * \param sin_value sin(angle) normalized to 16384, see util::sin()
 * \param cos_value cos(angle) normalized to 16384, see util::cos()
 * \return Rotated vertex
 */
inline vertex_type vertex_rotate(const vertex_type& point, const vertex_type& center, std::int32_t sin_value, std::int32_t cos_value)
{
  // translate point to center
  const vertex_type p = point - center;

  // rotate point and translate back
  return { static_cast<std::int16_t>(div_round_closest((static_cast<std::int32_t>(p.x) * cos_value - static_cast<std::int32_t>(p.y) * sin_value), 16384) + center.x),
           static_cast<std::int16_t>(div_round_closest((static_cast<std::int32_t>(p.x) * sin_value + static_cast<std::int32_t>(p.y) * cos_value), 16384) + center.y)
         };
}


/**
 * Helper function to rotate a vertex of a given angle in respect to given center
 * \param point Vertex to rotate
 * \param center Rotation center
 * \param angle Rotation angle in degree, direction is math positive (clockwise in screen coords system)
 * \return Rotated vertex
 */
inline vertex_type vertex_rotate(const vertex_type& point, const vertex_type& center, std::int16_t angle)
{
  return vertex_rotate(point, center, sin(angle), cos(angle));
}


} // namespace util
} // namespace vic

#endif  // _VIC_UTIL_H_
