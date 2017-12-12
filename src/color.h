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
    format_L1       = 0x10,   //  1 bit luminance (monochrome), like XBM format
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
    format_ARGB6666 = 0x32,   // 18 bit ARGB, 6 bit alpha
    format_ARGB8888 = 0x40,   // 24 bit ARGB, 8 bit alpha (the internal default format of vic)
    format_RGBA8888 = 0x41    // 24 bit ARGB, 8 bit alpha, A on least significant byte
  } format_type;


  /////////////////////////////////////////////////////////////////////////////
  // C O L O R   A S S E M B L Y

  /**
   * RGB color assembly, returns ARGB format out of RGB and alpha components
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
   * Static RGB color assembly, assigns ARGB format out of RGB and alpha components
   * |   A   |   R   |   G   |   B   |
   *  31   24 23   16 15    8 7     0
   * \param Red Red color
   * \param Green Green color
   * \param Blue Blue color
   * \param Alpha Alpha level, 0 = transparent, 255 = opaque (default)
   */
  template <std::uint8_t Red, std::uint8_t Green, std::uint8_t Blue, std::uint8_t Alpha = 255U>
  struct rgb_static {
    static const value_type value = ((static_cast<value_type>(Alpha) << 24U) |
                                     (static_cast<value_type>(Red)   << 16U) |
                                     (static_cast<value_type>(Green) <<  8U) |
                                     (static_cast<value_type>(Blue)  <<  0U));
  };

  /**
   * HSV color assembly, returns ARGB format out of HSV components
   * \param hue Hue in degree
   * \param saturation Saturation from 0-100
   * \param value Value 
   * \param alpha Alpha level, 0 = completely transparent, 255 = opaque (default)
   * \return ARGB color
   */
  inline value_type hsv(std::uint16_t hue, std::uint8_t saturation, std::uint8_t value, std::uint8_t alpha = 255U)
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


  //////////////////////////////////////////////////////////////////////////
  // C O L O R   A C C E S S  /  C O N V E R S I O N

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
  { return ((color & 0xFF000000UL) | (color & (0x00FCFCFCUL)) >> 2U); }

  // color is dimmed to 50%
  inline value_type dim_50(value_type color)
  { return ((color & 0xFF000000UL) | (color & (0x00FEFEFEUL)) >> 1U); }

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
  { return ((color & 0xFF000000UL) | ((((color & 0x00FF0000UL) * (static_cast<std::uint16_t>(lum) + 1U)) >> 8U) & 0x00FF0000UL) |
                                     ((((color & 0x0000FF00UL) * (static_cast<std::uint16_t>(lum) + 1U)) >> 8U) & 0x0000FF00UL) |
                                     ((((color & 0x000000FFUL) * (static_cast<std::uint16_t>(lum) + 1U)) >> 8U) & 0x000000FFUL)); }

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
      // front color is opaque
      return front_color;
    }

    const std::uint16_t aB = get_alpha(back_color);
    if (aB == 0U) {
      // back color is transparent
      return front_color;
    }

    const std::uint16_t aF = get_alpha(front_color);
    if (aF == 0U) {
      // front color is transparent
      return back_color;
    }

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
      // front color is opaque
      return front_color;
    }

    const std::uint8_t aF = get_alpha(front_color);

    return argb(static_cast<std::uint8_t>((static_cast<std::uint16_t>(aF * get_red(front_color))   + (255U - aF) * get_red(opaque_background))   / 255U),
                static_cast<std::uint8_t>((static_cast<std::uint16_t>(aF * get_green(front_color)) + (255U - aF) * get_green(opaque_background)) / 255U),
                static_cast<std::uint8_t>((static_cast<std::uint16_t>(aF * get_blue(front_color))  + (255U - aF) * get_blue(opaque_background))  / 255U),
                static_cast<std::uint8_t>(255U)
               );
  }


  //////////////////////////////////////////////////////////////////////////
  // C O L O R   F O R M A T   C O N V E R S I O N

  /**
   * Helper functions to convert internal ARGB color to native (head/bitmap) color format
   * \param color Internal ARGB color value
   * \return Native head color value
   */
  inline std::uint8_t color_to_L1(value_type color)
  { return static_cast<std::uint8_t>((color & 0x00FFFFFFUL) != (std::uint32_t)0U ? 1U : 0U); }

  inline std::uint8_t color_to_L2(value_type color)
  { return static_cast<std::uint8_t>(((std::uint16_t)((std::uint16_t)get_red(color) + (std::uint16_t)get_green(color) + (std::uint16_t)get_blue(color)) / 3U) >> 6U); }

  inline std::uint8_t color_to_L4(value_type color)
  { return static_cast<std::uint8_t>(((std::uint16_t)((std::uint16_t)get_red(color) + (std::uint16_t)get_green(color) + (std::uint16_t)get_blue(color)) / 3U) >> 4U); }

  inline std::uint8_t color_to_L8(value_type color)
  { return static_cast<std::uint8_t>((std::uint16_t)((std::uint16_t)get_red(color) + (std::uint16_t)get_green(color) + (std::uint16_t)get_blue(color)) / 3U); }

  inline std::uint8_t color_to_RGB332(value_type color)
  { return static_cast<std::uint8_t>((std::uint8_t)(get_red(color) & 0xE0U) | (std::uint8_t)((get_green(color) & 0xE0U) >> 3U) | (std::uint8_t)((get_blue(color)) >> 6U)); }

  inline std::uint16_t color_to_RGB444(value_type color)
  { return static_cast<std::uint16_t>(((std::uint16_t)(get_red(color) & 0xF0U) <<  8U) | ((std::uint16_t)(get_green(color) & 0xF0U) << 4U) | (std::uint16_t)(get_blue(color) >> 4U)); }

  inline std::uint16_t color_to_RGB555(value_type color)
  { return static_cast<std::uint16_t>(((std::uint16_t)(get_red(color) & 0xF8U) << 10U) | ((std::uint16_t)(get_green(color) & 0xF8U) << 5U) | (std::uint16_t)(get_blue(color) >> 3U)); }

  inline std::uint16_t color_to_RGB565(value_type color)
  { return static_cast<std::uint16_t>(((std::uint16_t)(get_red(color) & 0xF8U) << 11U) | ((std::uint16_t)(get_green(color) & 0xFCU) << 5U) | (std::uint16_t)(get_blue(color) >> 3U)); }

  inline std::uint32_t color_to_RGB666(value_type color)
  { return static_cast<std::uint32_t>(((std::uint32_t)(get_red(color) & 0xFCU) << 12U) | ((std::uint32_t)(get_green(color) & 0xFCU) << 6U) | (std::uint32_t)(get_blue(color) >> 2U)); }

  inline std::uint32_t color_to_RGB888(value_type color)
  { return static_cast<std::uint32_t>(color & 0x00FFFFFFUL); }


  /**
   * Helper functions to convert native (head/bitmap) color format to internal ARGB color
   * \param Native head/bitmap color value
   * \return Internal ARGB color value
   */
  inline value_type L1_to_color(std::uint8_t head_color)
  { return head_color ? 0xFFFFFFFFUL : 0xFF000000UL; }

  inline value_type L2_to_color(std::uint8_t head_color)
  { return dim(0xFFFFFFFFUL, 255U / ( 3U * (head_color & 0x03U))); }

  inline value_type L4_to_color(std::uint8_t head_color)
  { return dim(0xFFFFFFFFUL, 255U / (15U * (head_color & 0x0FU))); }

  inline value_type L8_to_color(std::uint8_t head_color)
  { return dim(0xFFFFFFFFUL, head_color); }

  inline value_type RGB332_to_color(std::uint8_t head_color)
  { return argb(static_cast<std::uint8_t>(head_color & 0xE0U), static_cast<std::uint8_t>((head_color & 0x1CU) << 3U), static_cast<std::uint8_t>((head_color & 0x03U) << 6U)); }

  inline value_type RGB444_to_color(std::uint16_t head_color)
  { return argb(static_cast<std::uint8_t>((head_color & 0x0F00U) >> 4U), static_cast<std::uint8_t>((head_color & 0x00F0U)      ), static_cast<std::uint8_t>((head_color & 0x000FU) << 4U)); }

  inline value_type RGB555_to_color(std::uint16_t head_color)
  { return argb(static_cast<std::uint8_t>((head_color & 0x7C00U) >> 7U), static_cast<std::uint8_t>((head_color & 0x03E0U) >> 2U), static_cast<std::uint8_t>((head_color & 0x001FU) << 3U)); }

  inline value_type RGB565_to_color(std::uint16_t head_color)
  { return argb(static_cast<std::uint8_t>((head_color & 0xF800U) >> 8U), static_cast<std::uint8_t>((head_color & 0x07E0U) >> 3U), static_cast<std::uint8_t>((head_color & 0x001FU) << 3U)); }

  inline value_type RGB666_to_color(std::uint32_t head_color)
  { return argb(static_cast<std::uint8_t>((head_color & 0x0003F000UL) >> 10U), static_cast<std::uint8_t>((head_color & 0x00000FC0UL) >> 4U), static_cast<std::uint8_t>((head_color & 0x0000003FUL) << 2U)); }

  inline value_type RGB888_to_color(std::uint32_t head_color)
  { return static_cast<value_type>(head_color | 0xFF000000UL); }



  //////////////////////////////////////////////////////////////////////////
  // P R E D E F I N E D   S T O C K   C O L O R S

  enum : value_type {
    // no/transparent color
    none          = 0x00000000,
    transparent   = none,

    // black, white and gray tones
    black         = 0xFF000000,
    gray          = 0xFF808080,
    gray01        = 0xFF101010,
    gray02        = 0xFF202020,
    gray03        = 0xFF303030,
    gray04        = 0xFF404040,
    gray05        = 0xFF505050,
    gray06        = 0xFF606060,
    gray07        = 0xFF707070,
    gray08        = 0xFF808080,
    gray09        = 0xFF909090,
    gray10        = 0xFFA0A0A0,
    gray11        = 0xFFB0B0B0,
    gray12        = 0xFFC0C0C0,
    gray13        = 0xFFD0D0D0,
    gray14        = 0xFFE0E0E0,
    gray15        = 0xFFF0F0F0,
    white         = 0xFFFFFFFF,

    // normal tones
    red           = 0xFFFF0000,
    green         = 0xFF00FF00,
    blue          = 0xFF0000FF,
    cyan          = 0xFF00FFFF,
    magenta       = 0xFFFF00FF,
    yellow        = 0xFFFFFF00,
    orange        = 0xFFFFA500,
    brown         = 0xFFA52A2A,
    pink          = 0xFFFFC0CB,
    salmon        = 0xFFFA8072,

    // dark tones
    darkred       = 0XFF800000,
    darkgreen     = 0XFF008000,
    darkblue      = 0XFF000080,
    navy          = darkblue,
    darkcyan      = 0XFF008080,
    darkmagenta   = 0XFF800080,
    darkyellow    = 0XFF808000,
    darkorange    = 0xFFD2691E,
    darkpink      = 0xFFFF1493,
    darksalmon    = 0XFFE9967A,
    darkviolet    = 0XFF9400D3,
    darkgray      = 0XFFA9A9A9
  };

} // namespace color
} // namespace vic

#endif  // _VIC_COLOR_H_
