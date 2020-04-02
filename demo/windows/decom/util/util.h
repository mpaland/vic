///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2012-2017, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// This file is part of the decom library.
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
// \brief helper functions
//
// This namespace provides common utility support and conversion functions
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_UTIL_H_
#define _DECOM_UTIL_H_

#include <cstdint>
#include <limits>


namespace decom {
namespace util {

/**
 * Returns a (Long) concatenated arg out of two (Short) arguments
 * \param low Low part
 * \param high High part
 * \return Long concatenated value
 */
template<typename Short, typename Long>
inline Long make_large(Short low, Short high)
{
  Long val = static_cast<Long>(high) << std::numeric_limits<Short>::digits;
  return static_cast<Long>(val | low);
}


/**
 * Returns the low (Short) part out of Long arg
 * \param arg Long value
 * \return Short value low part
 */
template<typename Long, typename Short>
inline Short lo_part(const Long& arg)
{
  return static_cast<Short>(arg);
}


/**
 * Returns the high (Short) part out of Long arg
 * \param arg Long value
 * \return Short value high part
 */
template<typename Long, typename Short>
inline Short hi_part(const Long& high)
{
  return static_cast<Short>(high >> std::numeric_limits<Short>::digits);
}


/**
 * hex to int conversion
 * strtol can be an alternative but it's not available in all uC libs
 * \param hex Zero terminated HEX string to convert
 * \return Converted integer value of given hex number
 */
template<typename Int>
Int hex2int(const char* hex)
{
  Int i = 0;
  while (*hex) {
    i = i << 4U;
    i += (*hex <= 0x39U) ? (*hex & 0x0FU) : 9U + (*hex & 0x0FU);
    ++hex;
  }
  return i;
}


/**
 * Generic alignof implementation
 * usage: alignment = decom::util::alignof(type)
 * \return The alignment of the given type
 */
template <typename T> struct struct_alignof;
template <typename T, int size_diff>
struct struct_alignof_helper
{
  enum { value = size_diff };
};
template <typename T>
struct struct_alignof_helper<T, 0>
{
  enum { value = struct_alignof<T>::value };
};
template <typename T>
struct struct_alignof
{
  struct big { T x; char c; };
  enum { diff = sizeof(big) - sizeof(T),
         value = struct_alignof_helper<big, diff>::value
       };
};
#define alignof(type) struct_alignof<type>::value

///////////////////////////////////////////////////////////////////////////////

namespace net {

/**
 * ntoh implementation for network headers
 * \return The type in network byte order (Big-Endian format)
 */
template<typename T> inline T ntoh(T x) { return x; };
template<> inline std::uint16_t ntoh<std::uint16_t>(std::uint16_t x) {
#if DECOM_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
  return ((x & 0x00FFU) << 8U) |
         ((x & 0xFF00U) >> 8U);
#else
  return x;
#endif
}
template<> inline std::uint32_t ntoh<std::uint32_t>(std::uint32_t x) {
#if DECOM_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
  return ((x & 0x000000FFUL) << 24U) |
         ((x & 0x0000FF00UL) <<  8U) |
         ((x & 0x00FF0000UL) >>  8U) |
         ((x & 0xFF000000UL) >> 24U);
#else
  return x;
#endif
}
template<> inline std::uint64_t ntoh<std::uint64_t>(std::uint64_t x) {
#if DECOM_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
  return ((x & 0x00000000000000FFULL) << 56U) |
         ((x & 0x000000000000FF00ULL) << 40U) |
         ((x & 0x0000000000FF0000ULL) << 24U) |
         ((x & 0x00000000FF000000ULL) <<  8U) |
         ((x & 0x000000FF00000000ULL) >>  8U) |
         ((x & 0x0000FF0000000000ULL) >> 24U) |
         ((x & 0x00FF000000000000ULL) >> 40U) |
         ((x & 0xFF00000000000000ULL) >> 56U);
#else
  return x;
#endif
}

// hton implementation
template<typename T> inline T hton(T x) {
#if DECOM_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
  return ntoh(x);
#else
  return x;
#endif
}

} // namespace net

} // namespace util
} // namespace decom

#endif // _DECOM_UTIL_H_
