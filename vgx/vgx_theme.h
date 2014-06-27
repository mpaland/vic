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
// \brief Theme support
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_THEME_H_
#define _VGX_THEME_H_

#include <vgx_cfg.h>      // use < > here, cause vgx_cfg.h may be in some platform folder
#include "vgx_colors.h"

#include <cstdint>


namespace vgx {

  // theme definitions
  typedef struct struct_theme_type
  {
    //basesize
    //font
  } theme_type;


  // dark theme
  const std::uint32_t theme_dark[] = {
    VGX_COLOR_BLACK,
    VGX_COLOR_BLUE
  };

  // dark/green theme
  const std::uint32_t theme_aircraft[] = {
    VGX_COLOR_BLACK,
    VGX_COLOR_GREEN
  };

  // white desktopp theme
  const std::uint32_t theme_white[] = {
    VGX_COLOR_WHITE,
    VGX_COLOR_DARKBLUE
  };

  const std::uint32_t theme_dynamic[1] = { 0U };

class theme
{
  typedef enum enum_color_index_type {
    backgound = 0,      // screen background
    window_frame,
    window_text,
    ctrl_face,
    ctrl_focus,
    ctrl_selected,
    button_frame,
    button_face,
    button_text,
    color_count
  } color_index_type;

  // load predefined theme
  void load(const std::uint32_t* theme)
  {
    theme_ = theme;
  }

  /**
   * Returns the theme color of the given index
   */
  std::uint32_t get(color_index_type color_index)
  {
    return theme_ == theme_dynamic ? colors_[color_index] : theme_[color_index];
  }

  /**
   * set dynamic color of the given index
   */
  void set(color_index_type color_index, std::uint32_t color)
  {
    colors_[color_index] = color;
  }

private:
  const std::uint32_t* theme_;
  std::uint32_t colors_[color_count];
};

} // namespace vgx

#endif  // _VGX_THEME_H_
