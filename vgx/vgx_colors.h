///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2001-2014, PALANDesign Hannover, Germany
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
// \brief Color definitions
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_COLORS_H_
#define _VGX_COLORS_H_

#include <cstdint>


namespace vgx {

// color assembly
#define VGX_COLOR_RGB(RED, GREEN, BLUE) \
  static_cast<std::uint32_t>((((std::uint32_t)RED) << 16U) | (((std::uint32_t)GREEN) << 8U) | ((std::uint32_t)BLUE))


#ifndef VGX_COLOR_BLACK
  #define VGX_COLOR_BLACK           VGX_COLOR_RGB(0, 0, 0)
#endif
#ifndef VGX_COLOR_WHITE
  #define VGX_COLOR_WHITE           VGX_COLOR_RGB(255, 255, 255)
#endif

// bright tones
#ifndef VGX_COLOR_BRIGHTBLUE
  #define VGX_COLOR_BRIGHTBLUE      VGX_COLOR_RGB(0, 0, 255)
#endif
#ifndef VGX_COLOR_BRIGHTGREEN
  #define VGX_COLOR_BRIGHTGREEN     VGX_COLOR_RGB(0, 255, 0)
#endif
#ifndef VGX_COLOR_BRIGHTCYAN
  #define VGX_COLOR_BRIGHTCYAN      VGX_COLOR_RGB(0, 255, 255)
#endif
#ifndef VGX_COLOR_BRIGHTRED
  #define VGX_COLOR_BRIGHTRED       VGX_COLOR_RGB(255, 0, 0)
#endif
#ifndef VGX_COLOR_BRIGHTMAGENTA
  #define VGX_COLOR_BRIGHTMAGENTA   VGX_COLOR_RGB(255, 0, 255)
#endif
#ifndef VGX_COLOR_BRIGHTYELLOW
  #define VGX_COLOR_BRIGHTYELLOW    VGX_COLOR_RGB(255, 255, 0)
#endif

// light tones
#ifndef VGX_COLOR_LIGHTYELLOW
  #define VGX_COLOR_LIGHTYELLOW     VGX_COLOR_RGB(255, 255, 64)
#endif
#ifndef VGX_COLOR_LIGHTBLUE
  #define VGX_COLOR_LIGHTBLUE       VGX_COLOR_RGB(128, 128, 255)
#endif
#ifndef VGX_COLOR_LIGHTGREEN
  #define VGX_COLOR_LIGHTGREEN      VGX_COLOR_RGB(128, 255, 128)
#endif
#ifndef VGX_COLOR_LIGHTCYAN
  #define VGX_COLOR_LIGHTCYAN       VGX_COLOR_RGB(128, 255, 255)
#endif
#ifndef VGX_COLOR_LIGHTRED
  #define VGX_COLOR_LIGHTRED        VGX_COLOR_RGB(255, 128, 128)
#endif
#ifndef VGX_COLOR_LIGHTMAGENTA
  #define VGX_COLOR_LIGHTMAGENTA    VGX_COLOR_RGB(255, 128, 255)
#endif
#ifndef VGX_COLOR_LIGHTORANGE
  #define VGX_COLOR_LIGHTORANGE     VGX_COLOR_RGB(255, 200, 0)
#endif
#ifndef VGX_COLOR_LIGHTGRAY
  #define VGX_COLOR_LIGHTGRAY       VGX_COLOR_RGB(192, 192, 192)
#endif

// normal tones
#ifndef VGX_COLOR_YELLOW
  #define VGX_COLOR_YELLOW          VGX_COLOR_RGB(192, 192, 32)
#endif
#ifndef VGX_COLOR_BLUE
  #define VGX_COLOR_BLUE            VGX_COLOR_RGB(0, 0, 192)
#endif
#ifndef VGX_COLOR_GREEN
  #define VGX_COLOR_GREEN           VGX_COLOR_RGB(0, 192, 0)
#endif
#ifndef VGX_COLOR_CYAN
  #define VGX_COLOR_CYAN            VGX_COLOR_RGB(0, 192, 192)
#endif
#ifndef VGX_COLOR_RED
  #define VGX_COLOR_RED             VGX_COLOR_RGB(192, 0, 0)
#endif
#ifndef VGX_COLOR_MAGENTA
  #define VGX_COLOR_MAGENTA         VGX_COLOR_RGB(192, 0, 192)
#endif
#ifndef VGX_COLOR_BROWN
  #define VGX_COLOR_BROWN           VGX_COLOR_RGB(255, 128, 0)
#endif
#ifndef VGX_COLOR_ORANGE
  #define VGX_COLOR_ORANGE          VGX_COLOR_RGB(255, 187, 76)
#endif
#ifndef VGX_COLOR_GRAY
  #define VGX_COLOR_GRAY            VGX_COLOR_RGB(128, 128, 128)
#endif

// dark tones
#ifndef VGX_COLOR_DARKORANGE
  #define VGX_COLOR_DARKORANGE      VGX_COLOR_RGB(255, 140, 0)
#endif
#ifndef VGX_COLOR_DARKBLUE
  #define VGX_COLOR_DARKBLUE        VGX_COLOR_RGB(0, 0, 64)
#endif
#ifndef VGX_COLOR_DARKGREEN
  #define VGX_COLOR_DARKGREEN       VGX_COLOR_RGB(0, 64, 0)
#endif
#ifndef VGX_COLOR_DARKRED
  #define VGX_COLOR_DARKRED         VGX_COLOR_RGB(64, 0, 0)
#endif
#ifndef VGX_COLOR_DARKGRAY
  #define VGX_COLOR_DARKGRAY        VGX_COLOR_RGB(64, 64, 64)
#endif

// special tones
#ifndef VGX_COLOR_GOLD
  #define VGX_COLOR_GOLD            VGX_COLOR_RGB(255, 215, 0)
#endif
#ifndef VGX_COLOR_SADDLEBROWN
  #define VGX_COLOR_SADDLEBROWN     VGX_COLOR_RGB(139, 69, 19)
#endif
#ifndef VGX_COLOR_SIENNA
  #define VGX_COLOR_SIENNA          VGX_COLOR_RGB(160, 82, 45)
#endif
#ifndef VGX_COLOR_PERU
  #define VGX_COLOR_PERU            VGX_COLOR_RGB(205, 133, 63)
#endif
#ifndef VGX_COLOR_BURLYWOOD
  #define VGX_COLOR_BURLYWOOD       VGX_COLOR_RGB(222, 184, 135)
#endif
#ifndef VGX_COLOR_WHEAT
  #define VGX_COLOR_WHEAT           VGX_COLOR_RGB(245, 245, 220)
#endif
#ifndef VGX_COLOR_TAN
  #define VGX_COLOR_TAN             VGX_COLOR_RGB(210, 180, 140)
#endif

// gray tones
#ifndef VGX_COLOR_GRAY01
  #define VGX_COLOR_GRAY01          VGX_COLOR_RGB(16, 16, 16)
#endif
#ifndef VGX_COLOR_GRAY02
  #define VGX_COLOR_GRAY02          VGX_COLOR_RGB(32, 32, 32)
#endif
#ifndef VGX_COLOR_GRAY03
  #define VGX_COLOR_GRAY03          VGX_COLOR_RGB(48, 48, 48)
#endif
#ifndef VGX_COLOR_GRAY04
  #define VGX_COLOR_GRAY04          VGX_COLOR_RGB(64, 64, 64)
#endif
#ifndef VGX_COLOR_GRAY05
  #define VGX_COLOR_GRAY05          VGX_COLOR_RGB(80, 80, 80)
#endif
#ifndef VGX_COLOR_GRAY06
  #define VGX_COLOR_GRAY06          VGX_COLOR_RGB(96, 96, 96)
#endif
#ifndef VGX_COLOR_GRAY07
  #define VGX_COLOR_GRAY07          VGX_COLOR_RGB(112, 112, 112)
#endif
#ifndef VGX_COLOR_GRAY08
  #define VGX_COLOR_GRAY08          VGX_COLOR_RGB(128, 128, 128)
#endif
#ifndef VGX_COLOR_GRAY09
  #define VGX_COLOR_GRAY09          VGX_COLOR_RGB(144, 144, 144)
#endif
#ifndef VGX_COLOR_GRAY10
  #define VGX_COLOR_GRAY10          VGX_COLOR_RGB(160, 160, 160)
#endif
#ifndef VGX_COLOR_GRAY11
  #define VGX_COLOR_GRAY11          VGX_COLOR_RGB(176, 176, 176)
#endif
#ifndef VGX_COLOR_GRAY12
  #define VGX_COLOR_GRAY12          VGX_COLOR_RGB(192, 192, 192)
#endif
#ifndef VGX_COLOR_GRAY13
  #define VGX_COLOR_GRAY13          VGX_COLOR_RGB(208, 208, 208)
#endif
#ifndef VGX_COLOR_GRAY14
  #define VGX_COLOR_GRAY14          VGX_COLOR_RGB(224, 224, 224)
#endif
#ifndef VGX_COLOR_GRAY15            // equivalent to white in 4 bit mode
  #define VGX_COLOR_GRAY15          VGX_COLOR_RGB(240, 240, 240)
#endif

} // namespace vgx

#endif  // _VGX_COLORS_H_
