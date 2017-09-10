///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2001-2017, PALANDesign Hannover, Germany
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
// \brief Color definitions and color/gradient/helper functions
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_COLOR_H_
#define _VIC_COLOR_H_

#include <cstdint>
#include <initializer_list>

#include "util.h"


namespace vic {
namespace color {

  /**
   * Internal vic color format is 32bit ARGB
   */
  typedef std::uint32_t value_type;

  /**
   * Supported color formats
    bits depth  format    usage
    ##########################################################################
     1     1           1   monochrome
     2     2           2    2 bit grayscale
     4     4           4    4 bit grayscale
     8     8           8    8 bit grayscale
     8     8        CLUT    8 bit color lookup table for 256 defined colors
    16    16        CLUT   16 bit color lookup table for 64k defined colors
     8     8       3-3-2    256 RGB display - no palette
    16    12       4-4-4     4k RGB display
    16    15       5-5-5    32k RGB display
    16    16       5-6-5    64k RGB display
    24    18       6-6-6   262k RGB display
    24    24       8-8-8    16M RGB display
    16    12     A-4-4-4     4k RGB display with 4 bit alpha blending
    16    15     A-5-5-5    32k RGB display with 1 bit alpha blending
    24    18     A-6-6-6   262k RGB display
    32    24     A-8-8-8    16M RGB display with 8 bit alpha blending
                   R-G-B   bit meaning
  */
  typedef enum tag_format_type
  {
    format_L1       = 0x10,   //  1 bit luminance (monochrome), XBM format
    format_L2       = 0x11,   //  2 bit grayscale
    format_L4       = 0x12,   //  4 bit grayscale
    format_L8       = 0x13,   //  8 bit, 256 luminance (gray) levels (0 = black, 255 = white)
    format_C8       = 0x14,   //  8 bit CLUT (color lookup table) - reserved for future, unsupported yet
    format_C16      = 0x20,   // 16 bit CLUT (color lookup table) - reserved for future, unsupported yet
    format_RGB332   = 0x15,   //  8 bit RGB
    format_RGB444   = 0x21,   // 12 bit RGB
    format_RGB555   = 0x22,   // 15 bit RGB
    format_RGB565   = 0x23,   // 16 bit RGB
    format_RGB666   = 0x30,   // 18 bit RGB
    format_RGB888   = 0x31,   // 24 bit RGB, no alpha
    format_ARGB4444 = 0x24,   // 16 bit ARGB, 4 bit alpha
    format_ARGB1555 = 0x25,   // 16 bit ARGB, 1 bit alpha (0 = transparent, 1 = opaque, 1)
    format_ARGB6666 = 0x32,   // 18 bit ARGB, 6 bit alpha
    format_ARGB8888 = 0x40    // 24 bit ARGB, 8 bit alpha (the internal default format of vic)
  } format_type;

  /**
   * Color assembly, returns ARGB format out of RGB and alpha components
   * |   A   |   R   |   G   |   B   |
   *  31   24 23   16 15    8 7     0
   * \param red Red color
   * \param green Green color
   * \param blue Blue color
   * \param alpha Alpha level, 0 = transparent, 255 = opaque (default)
   * \return ARGB color
   */
  inline value_type argb(std::uint8_t _red, std::uint8_t _green, std::uint8_t _blue, std::uint8_t _alpha = 255U)
  {
    return ((static_cast<value_type>(_alpha) << 24U) |
            (static_cast<value_type>(_red)   << 16U) |
            (static_cast<value_type>(_green) <<  8U) |
            (static_cast<value_type>(_blue)  <<  0U));
  }

  /**
   * Color assembly, returns ARGB format out of HSV components
   * \param hue Hue in degree
   * \param saturation Saturation from 0-100
   * \param value Value 
   * \param alpha Alpha level, 0 = completely transparent, 255 = opaque (default)
   * \return ARGB color
   */
  inline value_type hsv(const std::uint16_t hue, const std::uint8_t saturation, const std::uint8_t value, const std::uint8_t alpha = 255U)
  {
    const std::uint16_t h   = hue % 360U;
    const std::uint16_t hi  = static_cast<std::uint16_t>(h / 60U);
    const std::uint16_t f   = static_cast<std::uint16_t>(h - static_cast<std::uint16_t>(hi * 60U));
    const std::uint8_t  s_f = static_cast<std::uint8_t>((std::uint16_t)(saturation * f) / 60U);
    const std::uint16_t p   = ((uint16_t)value * (255U - saturation)) / 255U;
    const std::uint16_t q   = ((uint16_t)value * (255U - s_f)) / 255U;
    const std::uint16_t t   = ((uint16_t)value * (255U - saturation + s_f)) / 255U;

    std::uint8_t r, g, b;
    switch (hi) {
      case 0 : r = value;                        g = static_cast<std::uint8_t>(t); b = static_cast<std::uint8_t>(p); break;
      case 1 : r = static_cast<std::uint8_t>(q); g = value;                        b = static_cast<std::uint8_t>(p); break;
      case 2 : r = static_cast<std::uint8_t>(p); g = value;                        b = static_cast<std::uint8_t>(t); break;
      case 3 : r = static_cast<std::uint8_t>(p); g = static_cast<std::uint8_t>(q); b = value;                        break;
      case 4 : r = static_cast<std::uint8_t>(t); g = static_cast<std::uint8_t>(p); b = value;                        break;
      case 5 : r = value;                        g = static_cast<std::uint8_t>(p); b = static_cast<std::uint8_t>(q); break;
      default: r = 0U;                           g = 0U;                           b = 0U;                           break;
    }

    return ((static_cast<value_type>(alpha) << 24U) |
            (static_cast<value_type>(r)     << 16U) |
            (static_cast<value_type>(g)     <<  8U) |
            (static_cast<value_type>(b)     <<  0U));
  }

  // get single color channel out of 32 bit ARGB color
  inline std::uint8_t get_alpha(value_type color) { return static_cast<std::uint8_t>((color & 0xFF000000UL) >> 24U); }
  inline std::uint8_t get_red  (value_type color) { return static_cast<std::uint8_t>((color & 0x00FF0000UL) >> 16U); }
  inline std::uint8_t get_green(value_type color) { return static_cast<std::uint8_t>((color & 0x0000FF00UL) >>  8U); }
  inline std::uint8_t get_blue (value_type color) { return static_cast<std::uint8_t>((color & 0x000000FFUL));        }

  // modify/set single color channel in 32 bit ARGB color
  inline value_type set_alpha(value_type color, std::uint8_t _alpha) { return (color & 0x00FFFFFFUL) | (static_cast<std::uint32_t>(_alpha) << 24U); }
  inline value_type set_red  (value_type color, std::uint8_t _red)   { return (color & 0xFF00FFFFUL) | (static_cast<std::uint32_t>(_red)   << 16U); }
  inline value_type set_green(value_type color, std::uint8_t _green) { return (color & 0xFFFF00FFUL) | (static_cast<std::uint32_t>(_green) <<  8U); }
  inline value_type set_blue (value_type color, std::uint8_t _blue)  { return (color & 0xFFFFFF00UL) | (static_cast<std::uint32_t>(_blue));         }

  // generic color dimming, alpha channel is unaffected
  // color is dimmed to 25%
  inline value_type dim_25(value_type color)
  { return ((color & (0x00FCFCFCUL)) >> 2U); }

  // color is dimmed to 50%
  inline value_type dim_50(value_type color)
  { return ((color & (0x00FEFEFEUL)) >> 1U); }

  // color is dimmed to 75%
  inline value_type dim_75(value_type color)
  { return (dim_25(color) + dim_50(color)); }

  /**
   * Dim the given color, alpha channel is unaffected
   * \param color Color in ARGB format
   * \param lum Luminance, 0: dark, 255: bright (original color unchanged)
   * \return dimmed color in ARGB format
   */
  inline value_type dim(value_type color, std::uint8_t lum)
  { return ((color & 0xFF000000UL) | ((((color & 0x00FF0000UL) * (lum + 1U)) >> 8U) & 0x00FF0000UL) | ((((color & 0x0000FF00UL) * (lum + 1U)) >> 8U) & 0x0000FF00UL) | ((((color & 0x000000FFUL) * (lum + 1U)) >> 8U) & 0x000000FFUL)); }

  // generic color mixing, alpha channel is unaffected
  // color mix: 25% foreground, 75% background
  inline value_type mix_25(value_type foregound, value_type backgound)
  { return (dim_25(foregound) + dim_75(backgound)); }

  // color mix: 50% foreground, 50% background
  inline value_type mix_50(value_type foregound, value_type backgound)
  { return (dim_50(foregound) + dim_50(backgound)); }

  // color mix: 75% foreground, 25% background
  inline value_type mix_75(value_type foregound, value_type backgound)
  { return (dim_75(foregound) + dim_25(backgound)); }

  // color mix: lum = 255: 100% foreground color, lum = 0: 0% background
  inline value_type mix(value_type foregound, value_type backgound, std::uint8_t lum)
  { return (dim(foregound, lum) + dim(backgound, 0xFFU - lum)); }

  /**
   * Return true if color is opaque
   * \param color Color to test
   * \return True if color is opaque
   */
  inline bool is_opaque(value_type color)
  { return (color & 0xFF000000UL) == 0xFF000000UL; }

  /**
   * Combine two colors in relation to the alpha channel
   * Porter-Duff composition is used: C = (aF * F + (255 - aF) * aB * B) / (aF + (255 - aF) * aB)
   * \param front_color Color over back_color
   * \parem back_color Color below front_color
   * \return Resulting color
   */
  inline value_type alpha_blend(value_type front_color, value_type back_color)
  {
    if (is_opaque(front_color)) {
      return front_color;
    }

    const std::uint16_t aF  = get_alpha(front_color);
    const std::uint16_t aB  = get_alpha(back_color);
    const std::uint16_t aFB = aB * (255U - aF) / 255U;
    const std::uint16_t aS  = aF + aFB;

    return argb(static_cast<std::uint8_t>((aF * get_red(front_color)   + aFB * get_red(back_color))   / aS),
                static_cast<std::uint8_t>((aF * get_green(front_color) + aFB * get_green(back_color)) / aS),
                static_cast<std::uint8_t>((aF * get_blue(front_color)  + aFB * get_blue(back_color))  / aS),
                static_cast<std::uint8_t>(aS)
               );
  }

  /**
   * Convert an ARGB color to an RGB opaque color in relation to an opaque background color
   * This function is primary used to convert the internal 32bit ARGB color to a discrete display color.
   * \param front_color Color over back_color
   * \parem opaque_background Opaque background color (alpha channel doesn't care)
   * \return Resulting color, alpha is set to 255 (opaque)
   */
  inline value_type argb_to_rgb(value_type front_color, value_type opaque_background)
  {
    if (is_opaque(front_color)) {
      return front_color;
    }

    const std::uint16_t aF = get_alpha(front_color);

    return argb(static_cast<std::uint8_t>((aF * get_red(front_color)   + (255U - aF) * get_red(opaque_background))   / 255U),
                static_cast<std::uint8_t>((aF * get_green(front_color) + (255U - aF) * get_green(opaque_background)) / 255U),
                static_cast<std::uint8_t>((aF * get_blue(front_color)  + (255U - aF) * get_blue(opaque_background))  / 255U),
                255U
               );
  }


  //////////////////////////////////////////////////////////////////////////
  // G R A D I E N T

  /**
   * Color gradient structure
   * A gradient is a collection of pixels which returns an interpolated color for a given vertex
   * Construct a gradient with 3 pixels like
   * vic::color::gradient<3U> gr = { {{0, 0} vic::color::green}, {{500, 0}, vic::color::yellow}, {{750,100}, vic::color::red} };
   * color = gr.mix({300,0});   // get the color at position 300,0
   */
  template<std::size_t Max_Pixel>
  class gradient
  {
    // array of the gradient colors and the according color values
    pixel_type  gradient_color_[Max_Pixel];
    std::size_t size_;

  public:
    /**
     * Initializer list ctor
     * \param il Initializer list of N pixels
     */
    gradient(const std::initializer_list<pixel_type>& il)
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
    gradient(const pixel_type* gradient_pixels)
    {
      set(gradient_pixels, Max_Pixel);
    }

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
    value_type mix(vertex_type pos) const
    {
      // calculate the sum of all inverse squared distances from pos to all points
      const std::uint32_t factor = 10000000UL;  // use 10M as factor
      std::uint32_t sd_sum = 0U;
      for (std::size_t n = 0U; n < size_; ++n) {
        sd_sum += factor / (1U + util::distance_squared(pos, gradient_color_[n].vertex));
      }

      // assemble the mixed color at given pos
      value_type c = 0U;
      for (std::size_t n = 0U; n < size_; ++n) {
        c += dim(gradient_color_[n].color, static_cast<std::uint8_t>(0x100UL * (factor / (1U + util::distance_squared(pos, gradient_color_[n].vertex))) / sd_sum));
      }
      return c;
    }

    /**
     * Return the solid base color at the given position
     * \param pos The position of the vertex
     * \return The solid base color at the given position
     */
    value_type solid(vertex_type pos) const
    {
      // calculate the sum of all inverse squared distances from pos to all points
      const std::uint32_t factor = 10000000UL;  // use 10M as factor
      std::uint32_t sd_sum = 0U;
      for (std::size_t n = 0U; n < size_; ++n) {
        sd_sum += factor / (1U + util::distance_squared(pos, gradient_color_[n].vertex));   // use 10M as factor
      }

      // get the solid color at given pos
      std::uint32_t lum_max = 0U;
      std::size_t   n_max = 0U;
      for (std::size_t n = 0U; n < size_; ++n) {
        const std::uint32_t lum = 0x100UL * (factor / (1U + util::distance_squared(pos, gradient_color_[n].vertex))) / sd_sum;
        if (lum > lum_max) {
          lum_max = lum;
          n_max   = n;
        }
      }
      return gradient_color_[n_max].color;
    }
  };


  //////////////////////////////////////////////////////////////////////////
  // P R E D E F I N E D   S T O C K   C O L O R S

  // static RGB color assembly
  template <std::uint8_t Red, std::uint8_t Green, std::uint8_t Blue, std::uint8_t Alpha = 255U>
  struct rgb_static {
    static const value_type value = ((static_cast<value_type>(Alpha) << 24U) |
                                     (static_cast<value_type>(Red)   << 16U) |
                                     (static_cast<value_type>(Green) <<  8U) |
                                     (static_cast<value_type>(Blue)  <<  0U));
  };


  // no/transparent color
  const value_type none          = rgb_static<  0,   0,   0,   0>::value;
  const value_type transparent   = none;

  // black, white and gray tones
  const value_type black         = rgb_static<  0,   0,   0>::value;
  const value_type gray01        = rgb_static< 16,  16,  16>::value;
  const value_type gray02        = rgb_static< 32,  32,  32>::value;
  const value_type gray03        = rgb_static< 48,  48,  48>::value;
  const value_type gray04        = rgb_static< 64,  64,  64>::value;
  const value_type gray05        = rgb_static< 80,  80,  80>::value;
  const value_type gray06        = rgb_static< 96,  96,  96>::value;
  const value_type gray07        = rgb_static<112, 112, 112>::value;
  const value_type gray08        = rgb_static<128, 128, 128>::value;
  const value_type gray09        = rgb_static<144, 144, 144>::value;
  const value_type gray10        = rgb_static<160, 160, 160>::value;
  const value_type gray11        = rgb_static<176, 176, 176>::value;
  const value_type gray12        = rgb_static<192, 192, 192>::value;
  const value_type gray13        = rgb_static<208, 208, 208>::value;
  const value_type gray14        = rgb_static<224, 224, 224>::value;
  const value_type gray15        = rgb_static<240, 240, 240>::value;
  const value_type white         = rgb_static<255, 255, 255>::value;

  // bright tones
  const value_type brightblue    = rgb_static<  0,   0, 255>::value;
  const value_type brightgreen   = rgb_static<  0, 255,   0>::value;
  const value_type brightred     = rgb_static<255,   0,   0>::value;
  const value_type brightcyan    = rgb_static<  0, 255, 255>::value;
  const value_type brightmagenta = rgb_static<255,   0, 255>::value;
  const value_type brightyellow  = rgb_static<255, 255,   0>::value;
  const value_type brightgray    = rgb_static<224, 224, 224>::value;

  // light tones
  const value_type lightblue     = rgb_static<128, 128, 255>::value;
  const value_type lightgreen    = rgb_static<128, 255, 128>::value;
  const value_type lightred      = rgb_static<255, 128, 128>::value;
  const value_type lightcyan     = rgb_static<128, 255, 255>::value;
  const value_type lightmagenta  = rgb_static<255, 128, 255>::value;
  const value_type lightyellow   = rgb_static<255, 255,  64>::value;
  const value_type lightorange   = rgb_static<255, 200,   0>::value;
  const value_type lightgray     = rgb_static<192, 192, 192>::value;

  // normal tones
  const value_type blue          = rgb_static<  0,   0, 192>::value;
  const value_type green         = rgb_static<  0, 192,   0>::value;
  const value_type red           = rgb_static<192,   0,   0>::value;
  const value_type cyan          = rgb_static<  0, 192, 192>::value;
  const value_type magenta       = rgb_static<192,   0, 192>::value;
  const value_type yellow        = rgb_static<192, 192,  32>::value;
  const value_type orange        = rgb_static<255, 187,  76>::value;
  const value_type gray          = rgb_static<128, 128, 128>::value;
  const value_type brown         = rgb_static<255, 128,   0>::value;

  // dark tones
  const value_type darkred       = rgb_static< 64,   0,   0>::value;
  const value_type darkgreen     = rgb_static<  0,  64,   0>::value;
  const value_type darkblue      = rgb_static<  0,   0,  64>::value;
  const value_type darkyellow    = rgb_static< 64,  64,   0>::value;
  const value_type darkorange    = rgb_static<255, 140,   0>::value;
  const value_type darkgray      = rgb_static< 64,  64,  64>::value;

  // special tones
  const value_type gold          = rgb_static<255, 215,   0>::value;
  const value_type saddlebrown   = rgb_static<139,  69,  19>::value;
  const value_type sienna        = rgb_static<160,  82,  45>::value;
  const value_type peru          = rgb_static<205, 133,  63>::value;
  const value_type burlywood     = rgb_static<222, 184, 135>::value;
  const value_type wheat         = rgb_static<245, 245, 220>::value;
  const value_type tan           = rgb_static<210, 180, 140>::value;

} // namespace color
} // namespace vic

#endif  // _VIC_COLOR_H_
