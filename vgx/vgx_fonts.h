///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2014, PALANDesign Hannover, Germany
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
// \brief Font support structures
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_FONT_H_
#define _VGX_FONT_H_

#include <vgx_cfg.h>

#if defined(VGX_CFG_FONT)

#include <cstdint>


namespace vgx {

// standard character info (prop font)
typedef struct struct_font_charinfo_type {
  std::uint8_t xsize;             // x-size to render in pixel 
  std::uint8_t xdist;             // x-distance (cursor movement)
  std::uint8_t bytes_per_line;    // bytes to store the char line
  const std::uint8_t* data;       // char data
} font_charinfo_type;

// extended character info (prop font)
typedef struct struct_font_charinfo_ext_type {
  std::uint8_t xsize;             // x-size to render in pixel
  std::uint8_t ysize;             // y-size to render in pixel
  std::int8_t  xpos;              // x-start position
  std::int8_t  ypos;              // y-start position
  std::uint8_t xdist;             // x-distance (cursor movement)
  const std::uint8_t* data;       // char data
} font_charinfo_ext_type;

// standard mono font (ASCII/ANSI support)
typedef struct struct_font_mono_type {
  std::uint8_t first;             // first char code
  std::uint8_t last;              // last char code
  std::uint8_t xsize;             // x-size of the char in pixel 
  std::uint8_t bytes_per_line;    // bytes to store the char line
  const std::uint8_t* data;       // char data
} font_mono_type;

// standard prop font (ASCII/ANSI support)
typedef struct struct_font_prop_type {
  std::uint8_t first;
  std::uint8_t last;
  const font_charinfo_type*           char_info;
  const struct struct_font_prop_type* next;
} font_prop_type;

// extended prop font (UNICODE support)
typedef struct struct_font_prop_ext_type {
  std::uint16_t first;
  std::uint16_t last;
  const font_charinfo_ext_type*           char_info_ext;
  const struct struct_font_prop_ext_type* next;
} font_prop_ext_type;

// font info
typedef struct struct_font_type {
  std::uint8_t font_attr;               // font attributes
  std::uint8_t ysize;                   // y-size to render in pixel
  std::uint8_t ydist;                   // y-distance (cursor movement for \n)
  std::uint8_t baseline;                // reserved
  union union_font_type_type {
    const font_mono_type*     font_mono;
    const font_prop_type*     font_prop;
    const font_prop_ext_type* font_prop_ext;
    union_font_type_type() : font_prop(nullptr) { }
    union_font_type_type(const font_mono_type* p)     : font_mono(p) { }
    union_font_type_type(const font_prop_type* p)     : font_prop(p) { }
    union_font_type_type(const font_prop_ext_type* p) : font_prop_ext(p) { }
  } font_type_type;
} font_type;


// FONT ATTRIBUTES
#define VGX_FONT_AA_MASK            0x0FU
#define VGX_FONT_AA_NONE            0x01U   // 1bpp font - no antialiasing
#define VGX_FONT_AA_2               0x02U   // 2bpp antialiased font
#define VGX_FONT_AA_4               0x04U   // 4bpp antialiased font
#define VGX_FONT_AA_8               0x08U   // 8bpp antialiased font

#define VGX_FONT_ENCODING_MASK      0x10U
#define VGX_FONT_ENCODING_ASCII     0x00U   // character encoding: ASCII + ISO8859
#define VGX_FONT_ENCODING_UNICODE   0x10U   // Character encoding: Unicode

#define VGX_FONT_TYPE_MASK          0x20U
#define VGX_FONT_TYPE_MONO          0x00U   // monospaced font
#define VGX_FONT_TYPE_PROP          0x20U   // proportional font


////////////////////////////////////////////////////////////////
// AVAILABLE FONTS
//
// declarations of standard fonts, this fonts must be available when using controls
extern const font_type              font_std_LCD_8x8;
extern const font_type              font_std_arial_8x12_aa2;

// extern declarations for ALL fonts, just include your really used cpp font modules
extern const font_type              font_Consolas_16;
extern const font_type              font_Arial_num_16x24_aa4;
extern const font_type              font_Arial_15;
extern const font_type              font_9x15;
extern const font_type              font_LCD_6x8;
extern const font_type              font_LCD_6x10;
extern const font_type              font_LCD_8x8;


////////////////////////////////////////////////////////////////
// CONSTANTS AND DEFINES

// 1 bpp char defines
#define ________  0x00U
#define _______X  0x01U
#define ______X_  0x02U
#define ______XX  0x03U
#define _____X__  0x04U
#define _____X_X  0x05U
#define _____XX_  0x06U
#define _____XXX  0x07U
#define ____X___  0x08U
#define ____X__X  0x09U
#define ____X_X_  0x0AU
#define ____X_XX  0x0BU
#define ____XX__  0x0CU
#define ____XX_X  0x0DU
#define ____XXX_  0x0EU
#define ____XXXX  0x0FU
#define ___X____  0x10U
#define ___X___X  0x11U
#define ___X__X_  0x12U
#define ___X__XX  0x13U
#define ___X_X__  0x14U
#define ___X_X_X  0x15U
#define ___X_XX_  0x16U
#define ___X_XXX  0x17U
#define ___XX___  0x18U
#define ___XX__X  0x19U
#define ___XX_X_  0x1AU
#define ___XX_XX  0x1BU
#define ___XXX__  0x1CU
#define ___XXX_X  0x1DU
#define ___XXXX_  0x1EU
#define ___XXXXX  0x1FU
#define __X_____  0x20U
#define __X____X  0x21U
#define __X___X_  0x22U
#define __X___XX  0x23U
#define __X__X__  0x24U
#define __X__X_X  0x25U
#define __X__XX_  0x26U
#define __X__XXX  0x27U
#define __X_X___  0x28U
#define __X_X__X  0x29U
#define __X_X_X_  0x2AU
#define __X_X_XX  0x2BU
#define __X_XX__  0x2CU
#define __X_XX_X  0x2DU
#define __X_XXX_  0x2EU
#define __X_XXXX  0x2FU
#define __XX____  0x30U
#define __XX___X  0x31U
#define __XX__X_  0x32U
#define __XX__XX  0x33U
#define __XX_X__  0x34U
#define __XX_X_X  0x35U
#define __XX_XX_  0x36U
#define __XX_XXX  0x37U
#define __XXX___  0x38U
#define __XXX__X  0x39U
#define __XXX_X_  0x3AU
#define __XXX_XX  0x3BU
#define __XXXX__  0x3CU
#define __XXXX_X  0x3DU
#define __XXXXX_  0x3EU
#define __XXXXXX  0x3FU
#define _X______  0x40U
#define _X_____X  0x41U
#define _X____X_  0x42U
#define _X____XX  0x43U
#define _X___X__  0x44U
#define _X___X_X  0x45U
#define _X___XX_  0x46U
#define _X___XXX  0x47U
#define _X__X___  0x48U
#define _X__X__X  0x49U
#define _X__X_X_  0x4AU
#define _X__X_XX  0x4BU
#define _X__XX__  0x4CU
#define _X__XX_X  0x4DU
#define _X__XXX_  0x4EU
#define _X__XXXX  0x4FU
#define _X_X____  0x50U
#define _X_X___X  0x51U
#define _X_X__X_  0x52U
#define _X_X__XX  0x53U
#define _X_X_X__  0x54U
#define _X_X_X_X  0x55U
#define _X_X_XX_  0x56U
#define _X_X_XXX  0x57U
#define _X_XX___  0x58U
#define _X_XX__X  0x59U
#define _X_XX_X_  0x5AU
#define _X_XX_XX  0x5BU
#define _X_XXX__  0x5CU
#define _X_XXX_X  0x5DU
#define _X_XXXX_  0x5EU
#define _X_XXXXX  0x5FU
#define _XX_____  0x60U
#define _XX____X  0x61U
#define _XX___X_  0x62U
#define _XX___XX  0x63U
#define _XX__X__  0x64U
#define _XX__X_X  0x65U
#define _XX__XX_  0x66U
#define _XX__XXX  0x67U
#define _XX_X___  0x68U
#define _XX_X__X  0x69U
#define _XX_X_X_  0x6AU
#define _XX_X_XX  0x6BU
#define _XX_XX__  0x6CU
#define _XX_XX_X  0x6DU
#define _XX_XXX_  0x6EU
#define _XX_XXXX  0x6FU
#define _XXX____  0x70U
#define _XXX___X  0x71U
#define _XXX__X_  0x72U
#define _XXX__XX  0x73U
#define _XXX_X__  0x74U
#define _XXX_X_X  0x75U
#define _XXX_XX_  0x76U
#define _XXX_XXX  0x77U
#define _XXXX___  0x78U
#define _XXXX__X  0x79U
#define _XXXX_X_  0x7AU
#define _XXXX_XX  0x7BU
#define _XXXXX__  0x7CU
#define _XXXXX_X  0x7DU
#define _XXXXXX_  0x7EU
#define _XXXXXXX  0x7FU
#define X_______  0x80U
#define X______X  0x81U
#define X_____X_  0x82U
#define X_____XX  0x83U
#define X____X__  0x84U
#define X____X_X  0x85U
#define X____XX_  0x86U
#define X____XXX  0x87U
#define X___X___  0x88U
#define X___X__X  0x89U
#define X___X_X_  0x8AU
#define X___X_XX  0x8BU
#define X___XX__  0x8CU
#define X___XX_X  0x8DU
#define X___XXX_  0x8EU
#define X___XXXX  0x8FU
#define X__X____  0x90U
#define X__X___X  0x91U
#define X__X__X_  0x92U
#define X__X__XX  0x93U
#define X__X_X__  0x94U
#define X__X_X_X  0x95U
#define X__X_XX_  0x96U
#define X__X_XXX  0x97U
#define X__XX___  0x98U
#define X__XX__X  0x99U
#define X__XX_X_  0x9AU
#define X__XX_XX  0x9BU
#define X__XXX__  0x9CU
#define X__XXX_X  0x9DU
#define X__XXXX_  0x9EU
#define X__XXXXX  0x9FU
#define X_X_____  0xA0U
#define X_X____X  0xA1U
#define X_X___X_  0xA2U
#define X_X___XX  0xA3U
#define X_X__X__  0xA4U
#define X_X__X_X  0xA5U
#define X_X__XX_  0xA6U
#define X_X__XXX  0xA7U
#define X_X_X___  0xA8U
#define X_X_X__X  0xA9U
#define X_X_X_X_  0xAAU
#define X_X_X_XX  0xABU
#define X_X_XX__  0xACU
#define X_X_XX_X  0xADU
#define X_X_XXX_  0xAEU
#define X_X_XXXX  0xAFU
#define X_XX____  0xB0U
#define X_XX___X  0xB1U
#define X_XX__X_  0xB2U
#define X_XX__XX  0xB3U
#define X_XX_X__  0xB4U
#define X_XX_X_X  0xB5U
#define X_XX_XX_  0xB6U
#define X_XX_XXX  0xB7U
#define X_XXX___  0xB8U
#define X_XXX__X  0xB9U
#define X_XXX_X_  0xBAU
#define X_XXX_XX  0xBBU
#define X_XXXX__  0xBCU
#define X_XXXX_X  0xBDU
#define X_XXXXX_  0xBEU
#define X_XXXXXX  0xBFU
#define XX______  0xC0U
#define XX_____X  0xC1U
#define XX____X_  0xC2U
#define XX____XX  0xC3U
#define XX___X__  0xC4U
#define XX___X_X  0xC5U
#define XX___XX_  0xC6U
#define XX___XXX  0xC7U
#define XX__X___  0xC8U
#define XX__X__X  0xC9U
#define XX__X_X_  0xCAU
#define XX__X_XX  0xCBU
#define XX__XX__  0xCCU
#define XX__XX_X  0xCDU
#define XX__XXX_  0xCEU
#define XX__XXXX  0xCFU
#define XX_X____  0xD0U
#define XX_X___X  0xD1U
#define XX_X__X_  0xD2U
#define XX_X__XX  0xD3U
#define XX_X_X__  0xD4U
#define XX_X_X_X  0xD5U
#define XX_X_XX_  0xD6U
#define XX_X_XXX  0xD7U
#define XX_XX___  0xD8U
#define XX_XX__X  0xD9U
#define XX_XX_X_  0xDAU
#define XX_XX_XX  0xDBU
#define XX_XXX__  0xDCU
#define XX_XXX_X  0xDDU
#define XX_XXXX_  0xDEU
#define XX_XXXXX  0xDFU
#define XXX_____  0xE0U
#define XXX____X  0xE1U
#define XXX___X_  0xE2U
#define XXX___XX  0xE3U
#define XXX__X__  0xE4U
#define XXX__X_X  0xE5U
#define XXX__XX_  0xE6U
#define XXX__XXX  0xE7U
#define XXX_X___  0xE8U
#define XXX_X__X  0xE9U
#define XXX_X_X_  0xEAU
#define XXX_X_XX  0xEBU
#define XXX_XX__  0xECU
#define XXX_XX_X  0xEDU
#define XXX_XXX_  0xEEU
#define XXX_XXXX  0xEFU
#define XXXX____  0xF0U
#define XXXX___X  0xF1U
#define XXXX__X_  0xF2U
#define XXXX__XX  0xF3U
#define XXXX_X__  0xF4U
#define XXXX_X_X  0xF5U
#define XXXX_XX_  0xF6U
#define XXXX_XXX  0xF7U
#define XXXXX___  0xF8U
#define XXXXX__X  0xF9U
#define XXXXX_X_  0xFAU
#define XXXXX_XX  0xFBU
#define XXXXXX__  0xFCU
#define XXXXXX_X  0xFDU
#define XXXXXXX_  0xFEU
#define XXXXXXXX  0xFFU

} // namespace vgx

#endif  // VGX_CFG_FONT
#endif  // _VGX_FONT_H_
