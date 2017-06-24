///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2016, PALANDesign Hannover, Germany
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
// \brief Dummy driver, use this as a non existent display replacement for
// testing purpose
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_DRV_DUMMY_H_
#define _VIC_DRV_DUMMY_H_

#include "../drv.h"

// defines the driver name and version
#define VIC_DRV_DUMMY_VERSION   "Dummy driver 1.00"


namespace vic {
namespace head {

/**
 * Dummy head
 * \param Screen_Size_X Screen (buffer) width
 * \param Screen_Size_Y Screen (buffer) height
 * \param Viewport_Size_X Viewport (window) width
 * \param Viewport_Size_Y Viewport (window) height
 */
template<std::uint16_t Screen_Size_X = 0U, std::uint16_t Screen_Size_Y = 0U,
         std::uint16_t Viewport_Size_X = Screen_Size_X, std::uint16_t Viewport_Size_Y = Screen_Size_Y>
class dummy : public drv
{
  bool is_graphic_;

public:
/////////////////////////////////////////////////////////////////////////////
// M A N D A T O R Y   F U N C T I O N S

  /**
   * ctor
   * \param viewport_x Viewport x position (left) on the screen, relative to top/left corner
   * \param viewport_y Viewport y position (top)  on the screen, relative to top/left corner
   */
  dummy(std::uint16_t viewport_x = 0U, std::uint16_t viewport_y = 0U, bool is_graphic = true)
    : is_graphic_(is_graphic)
    , drv(Screen_Size_X, Screen_Size_Y,
          Viewport_Size_X, Viewport_Size_Y,
          viewport_x, viewport_y)
  { }

  /**
   * dtor
   * Shutdown the driver
   */
  ~dummy()
  { shutdown(); }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   C O M M O N   D R I V E R   F U N C T I O N S

  virtual void drv_init()
  { }
  
  virtual void drv_shutdown()
  { }

  virtual const char* drv_version() const
  { return (const char*)VIC_DRV_DUMMY_VERSION; }

  virtual bool drv_is_graphic() const
  { return is_graphic_; }

  virtual void drv_cls()
  { }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S   F O R   G R A P H I C   H E A D S

  virtual void drv_pixel_set_color(vertex_type, color::value_type)
  { }

  virtual color::value_type drv_pixel_get(vertex_type) const
  { }


  /////////////////////////////////////////////////////////////////////////////
  // F U N C T I O N S   F O R   A L P H A   N U M E R I C   D I S P L A Y S
  //
  // The functions in this section are MANDATORY FOR ALPHA NUMERIC drivers!
  // Every alpha numeric display driver MUST implement/override them.

  /**
   * Select the font
   * \param font Reference to font to use
   */
  virtual void drv_text_font_select(const font::font_type& font)
  { }

  /**
   * Set the new text position
   * \param x X value in pixel on graphic displays, char pos on alpha displays
   * \param y Y value in pixel on graphic displays, char pos on alpha displays
   */
  virtual void text_pos(std::int16_t x, std::int16_t y)
  { }

  /**
   * Set the text mode
   * \param mode Set normal or inverse video
   */
  virtual void drv_text_mode(text_mode_type mode)
  { }

  /**
   * Output one character
   * \param ch Output character in ASCII/UNICODE (NOT UTF-8) format
   * \return The x size (distance) of the rendered char
   */
  virtual std::uint16_t text_char(std::uint16_t ch)
  { }

  virtual void drv_present()
  {
    // copy/refresh contents of the screen buffer
  }


  /////////////////////////////////////////////////////////////////////////////
  // O P T I O N A L   O V E R R I D E S

};

} // namespace head
} // namespace vic

#endif  // _VIC_DRV_DUMMY_H_
