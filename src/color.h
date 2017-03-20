///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2001-2015, PALANDesign Hannover, Germany
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
// \brief Color definitions and color/gradient/helper functions
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_COLOR_H_
#define _VGX_COLOR_H_

#include <cstdint>
#include <initializer_list>


namespace vgx {

struct color
{
  /**
   * Default color format is 32 bit ARGB
   */
  typedef std::uint32_t   value_type;

  /**
   * Supported color formats
    bits depth  format    usage
    ##########################################################################
     1     1           1   monochrome
     2     2           2   2 bit grayscale
     4     4           4   4 bit grayscale
     8     8           8   8 bit grayscale
     8     8        CLUT   8 bit color lookup table for 256 defined colors
     8     8       3-3-2    256 RGB display - no palette
    15    15       5-5-5    32k RGB display
    16    16       5-6-5    64k RGB display
    16    12     A-4-4-4     4k RGB display with 4 bit alpha blending
    16    15     A-5-5-5    32k RGB display with 1 bit alpha blending
    18    18     0-6-6-6   262k RGB display
    24    24     0-8-8-8    16M RGB display
    32    24     A-8-8-8    16M RGB display with alpha blending
                   R-G-B
  */
  typedef enum enum_format_type
  {
    format_L1 = 0,    //  1 bit luminance (monochrome)
    format_L2,        //  2 bit
    format_L4,        //  4 bit
    format_L8,        //  8 bit, 256 luminance (gray) levels (0 = black, 255 = white)
    format_C8,        //  8 bit CLUT (color lookup table) - reserved for future, unsupported yet
    format_C16,       // 16 bit CLUT (color lookup table) - reserved for future, unsupported yet
    format_RGB332,    //  8 bit RGB
    format_RGB555,    // 15 bit RGB
    format_RGB565,    // 16 bit RGB
    format_RGB666,    // 18 bit RGB
    format_RGB888,    // 24 bit RGB, no alpha
    format_ARGB1555,  // 16 bit ARGB, 1 bit alpha (0 = opaque, 1 = transparent)
    format_ARGB4444,  // 16 bit ARGB, 4 bit alpha
    format_ARGB8888   // 32 bit ARGB format (internal default format of vgx)
  } format_type;

  /**
   * Color assembly, returns ARGB format out of RGB components
   * \param red Red color
   * \param green Green color
   * \param blue Blue color
   * \param alpha Alpha level, 0 = opaque, 255 = completely transparent
   * \return ARGB color
   */
  static inline value_type rgb(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 0U)
  { return static_cast<value_type>((((value_type)alpha) << 24U) | (((value_type)red) << 16U) | (((value_type)green) << 8U) | ((value_type)blue)); }

  /**
   * Color assembly, returns ARGB format out of HSV components
   * \param hue Hue (0-359) in degree
   * \param saturation Saturation from 0-100
   * \param value Value 
   * \param alpha Alpha level, 0 = opaque, 255 = completely transparent
   * \return ARGB color
   */
  static inline value_type hsv(std::uint16_t hue, std::uint8_t saturation, std::uint8_t value, std::uint8_t alpha = 0U)
  {
    hue = hue % 360U;
    const std::uint16_t hi  = static_cast<std::uint16_t>(hue / 60U);
    const std::uint16_t f   = static_cast<std::uint16_t>(hue - static_cast<std::uint16_t>(hi * 60U));
    const std::uint8_t  s_f = static_cast<std::uint8_t>((std::uint16_t)saturation * f / 60U);
    const std::uint16_t p   = ((uint16_t)value * (255U - saturation)) / 255U;
    const std::uint16_t q   = ((uint16_t)value * (255U - s_f)) / 255U;
    const std::uint16_t t   = ((uint16_t)value * (255U - saturation + s_f)) / 255U;

    std::uint8_t red, green, blue;
    switch (hi) {
      case 0 : red = value;                        green = static_cast<std::uint8_t>(t); blue = static_cast<std::uint8_t>(p); break;
      case 1 : red = static_cast<std::uint8_t>(q); green = value;                        blue = static_cast<std::uint8_t>(p); break;
      case 2 : red = static_cast<std::uint8_t>(p); green = value;                        blue = static_cast<std::uint8_t>(t); break;
      case 3 : red = static_cast<std::uint8_t>(p); green = static_cast<std::uint8_t>(q); blue = value;                        break;
      case 4 : red = static_cast<std::uint8_t>(t); green = static_cast<std::uint8_t>(p); blue = value;                        break;
      case 5 : red = value;                        green = static_cast<std::uint8_t>(p); blue = static_cast<std::uint8_t>(q); break;
      default: red = 0U;                           green = 0U;                           blue = 0U;                           break;
    }

    return ((static_cast<value_type>(alpha) << 24U) |
            (static_cast<value_type>(red)   << 16U) |
            (static_cast<value_type>(green) <<  8U) |
            (static_cast<value_type>(blue)  <<  0U));
  }

  // color channel values out of 32BBP ARGB color
  static inline std::uint8_t get_alpha(value_type color) { return static_cast<std::uint8_t>((color & 0xFF000000UL) >> 24U); }
  static inline std::uint8_t get_red  (value_type color) { return static_cast<std::uint8_t>((color & 0x00FF0000UL) >> 16U); }
  static inline std::uint8_t get_green(value_type color) { return static_cast<std::uint8_t>((color & 0x0000FF00UL) >>  8U); }
  static inline std::uint8_t get_blue (value_type color) { return static_cast<std::uint8_t>((color & 0x000000FFUL));        }

  // generic color dimming, alpha channel is unaffected
  static inline value_type dim_25(value_type color)
  { return ((color & (0x00FCFCFCUL)) >> 2U); }

  static inline value_type dim_50(value_type color)
  { return ((color & (0x00FEFEFEUL)) >> 1U); }

  static inline value_type dim_75(value_type color)
  { return (dim_25(color) + dim_50(color)); }

  /**
   * Dim the given color, alpha channel is unaffected
   * \param color Color in ARGB format
   * \param lum Luminance, 0: dark, 255: bright (original color unchanged)
   * \return dimmed color in ARGB format
   */
  static inline value_type dim(value_type color, std::uint8_t lum)
  { return ((color & 0xFF000000UL) | ((((color & 0x00FF0000UL) * (lum + 1U)) >> 8U) & 0x00FF0000UL) | ((((color & 0x0000FF00UL) * (lum + 1U)) >> 8U) & 0x0000FF00UL) | ((((color & 0x000000FFUL) * (lum + 1U)) >> 8U) & 0x000000FFUL)); }

  // generic color mixing, alpha channel is unaffected
  static inline value_type mix_25(value_type fore, value_type back)
  { return (dim_25(fore) + dim_75(back)); }

  static inline value_type mix_50(value_type fore, value_type back)
  { return (dim_50(fore) + dim_50(back)); }

  static inline value_type mix_75(value_type fore, value_type back)
  { return (dim_75(fore) + dim_25(back)); }

  static inline value_type mix(value_type fore, value_type back, std::uint8_t lum)
  { return (dim(fore, lum) + dim(back, 0xFFU - lum)); }


  // this base class is used as reference to the templated instance
  struct gradient_base {
    // array of the gradient colors and the according start values
    typedef struct struct_gradient_color_type {
      std::uint16_t start;    // start of the color in per mille [1/1000]
      value_type    color;    // color
    } gradient_color_type;

    virtual value_type mix(std::uint16_t pos) const = 0;
    virtual value_type solid(std::uint16_t pos) const = 0;
  };

  /**
   * Color gradient structure
   * A gradient has a range from 0 to 1000, all color start positions have to be defined within this range
   * Construct a gradient with 3 colors like
   * vgx::color::gradient<3U> gr = { {0, vgx::color::green}, {500, vgx::color::yellow}, {750, vgx::color::red} };
   *                                 green                   yellow                     red                    red
   */
  template<std::size_t N>
  struct gradient : public gradient_base
  {
    gradient(const std::initializer_list<gradient_color_type>& il) {
      std::size_t n = 0U;
      for (std::initializer_list<gradient_color_type>::const_iterator it = il.begin(); it != il.end(); ++it) {
        color_[n++] = *it;
      }
    }

    /**
     * Return the mixed color at the given position
     * \param pos The position in per mille [1/1000]
     * \return The mixed color at the given position
     */
    virtual value_type mix(std::uint16_t pos) const {
      std::uint_fast16_t i;
      for (i = 1U; i < N && pos > color_[i].start; ++i);
      if (i == N) {
        return color_[i - 1U].color;
      }
      return color::mix(color_[i - 1U].color, color_[i].color,
                        static_cast<std::uint8_t>((std::uint32_t)(color_[i].start - pos) * 255UL / (color_[i].start - color_[i - 1U].start)));
    }

    /**
     * Returns the solid color at the given position
     * \param pos The position in per mille [1/1000]
     * \return The solid color at the given position
     */
    virtual value_type solid(std::uint16_t pos) const {
      std::uint_fast16_t i;
      for (i = 1U; i < N && pos > color_[i].start; ++i);
      return color_[i - 1U].color;
    }

  private:
    // array of the gradient colors and the according start values
    gradient_color_type color_[N];
  };


  //////////////////////////////////////////////////////////////////////////
  // C O L O R S

  // static RGB color assembly
  template <std::uint8_t Red, std::uint8_t Green, std::uint8_t Blue, std::uint8_t Alpha = 0U>
  struct rgb_maker {
    static const value_type value = ((static_cast<value_type>(Alpha) << 24U) |
                                     (static_cast<value_type>(Red)   << 16U) |
                                     (static_cast<value_type>(Green) <<  8U) |
                                     (static_cast<value_type>(Blue)  <<  0U));
  };


  // no color
  static const value_type none          = rgb_maker<  0,   0,   0, 255>::value;
  static const value_type transparent   = rgb_maker<  0,   0,   0, 255>::value;

  // black, white and gray tones
  static const value_type black         = rgb_maker<  0,   0,   0>::value;
  static const value_type white         = rgb_maker<255, 255, 255>::value;
  static const value_type gray01        = rgb_maker< 16,  16,  16>::value;
  static const value_type gray02        = rgb_maker< 32,  32,  32>::value;
  static const value_type gray03        = rgb_maker< 48,  48,  48>::value;
  static const value_type gray04        = rgb_maker< 64,  64,  64>::value;
  static const value_type gray05        = rgb_maker< 80,  80,  80>::value;
  static const value_type gray06        = rgb_maker< 96,  96,  96>::value;
  static const value_type gray07        = rgb_maker<112, 112, 112>::value;
  static const value_type gray08        = rgb_maker<128, 128, 128>::value;
  static const value_type gray09        = rgb_maker<144, 144, 144>::value;
  static const value_type gray10        = rgb_maker<160, 160, 160>::value;
  static const value_type gray11        = rgb_maker<176, 176, 176>::value;
  static const value_type gray12        = rgb_maker<192, 192, 192>::value;
  static const value_type gray13        = rgb_maker<208, 208, 208>::value;
  static const value_type gray14        = rgb_maker<224, 224, 224>::value;
  static const value_type gray15        = rgb_maker<240, 240, 240>::value;

  // bright tones
  static const value_type brightblue    = rgb_maker<  0,   0, 255>::value;
  static const value_type brightgreen   = rgb_maker<  0, 255,   0>::value;
  static const value_type brightred     = rgb_maker<255,   0,   0>::value;
  static const value_type brightcyan    = rgb_maker<  0, 255, 255>::value;
  static const value_type brightmagenta = rgb_maker<255,   0, 255>::value;
  static const value_type brightyellow  = rgb_maker<255, 255,   0>::value;

  // light tones
  static const value_type lightblue     = rgb_maker<128, 128, 255>::value;
  static const value_type lightgreen    = rgb_maker<128, 255, 128>::value;
  static const value_type lightred      = rgb_maker<255, 128, 128>::value;
  static const value_type lightcyan     = rgb_maker<128, 255, 255>::value;
  static const value_type lightmagenta  = rgb_maker<255, 128, 255>::value;
  static const value_type lightyellow   = rgb_maker<255, 255,  64>::value;
  static const value_type lightorange   = rgb_maker<255, 200,   0>::value;
  static const value_type lightgray     = rgb_maker<192, 192, 192>::value;

  // normal tones
  static const value_type blue          = rgb_maker<  0,   0, 192>::value;
  static const value_type green         = rgb_maker<  0, 192,   0>::value;
  static const value_type red           = rgb_maker<192,   0,   0>::value;
  static const value_type cyan          = rgb_maker<  0, 192, 192>::value;
  static const value_type magenta       = rgb_maker<192,   0, 192>::value;
  static const value_type yellow        = rgb_maker<192, 192,  32>::value;
  static const value_type orange        = rgb_maker<255, 187,  76>::value;
  static const value_type gray          = rgb_maker<128, 128, 128>::value;
  static const value_type brown         = rgb_maker<255, 128,   0>::value;

  // dark tones
  static const value_type darkorange    = rgb_maker<255, 140,   0>::value;
  static const value_type darkblue      = rgb_maker<  0,   0,  64>::value;
  static const value_type darkgreen     = rgb_maker<  0,  64,   0>::value;
  static const value_type darkred       = rgb_maker< 64,   0,   0>::value;
  static const value_type darkgray      = rgb_maker< 64,  64,  64>::value;

  // special tones
  static const value_type gold          = rgb_maker<255, 215,   0>::value;
  static const value_type saddlebrown   = rgb_maker<139,  69,  19>::value;
  static const value_type sienna        = rgb_maker<160,  82,  45>::value;
  static const value_type peru          = rgb_maker<205, 133,  63>::value;
  static const value_type burlywood     = rgb_maker<222, 184, 135>::value;
  static const value_type wheat         = rgb_maker<245, 245, 220>::value;
  static const value_type tan           = rgb_maker<210, 180, 140>::value;
};

} // namespace vgx

#endif  // _VGX_COLOR_H_
