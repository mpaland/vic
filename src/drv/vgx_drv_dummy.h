///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2015, PALANDesign Hannover, Germany
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
// \brief Dummy driver, use this as a non existent display replacement for
// testing purpose
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_DRV_DUMMY_H_
#define _VGX_DRV_DUMMY_H_

#include "vgx_drv.h"

// defines the driver name and version
#define VGX_DRV_DUMMY_VERSION   "Dummy driver 1.00"


namespace vgx {
namespace head {

/**
 * Dummy head
 * \param Screen_Size_X Screen (buffer) width
 * \param Screen_Size_Y Screen (buffer) height
 * \param Viewport_Size_X Viewport (window) width
 * \param Viewport_Size_Y Viewport (window) height
 */
template<std::uint16_t Screen_Size_X = 0U,   std::uint16_t Screen_Size_Y = 0U,
         std::uint16_t Viewport_Size_X = 0U, std::uint16_t Viewport_Size_Y = 0U>
class dummy : public drv
{
public:
/////////////////////////////////////////////////////////////////////////////
// M A N D A T O R Y   F U N C T I O N S

  /**
   * ctor
   * \param viewport_x Viewport x position (left) on the screen, relative to top/left corner
   * \param viewport_y Viewport y position (top)  on the screen, relative to top/left corner
   */
  dummy(std::uint16_t viewport_x = 0U, std::uint16_t viewport_y = 0U)
    : drv(Screen_Size_X, Screen_Size_Y,
          Viewport_Size_X, Viewport_Size_Y,
          viewport_x, viewport_y)
  { }

  /**
   * dtor
   * Deinit the driver
   */
  ~dummy()
  { deinit(); }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   D R I V E R   F U N C T I O N S

  virtual void init()
  { }
  
  virtual void deinit()
  { }

  virtual const char* version() const
  { return (const char*)VGX_DRV_DUMMY_VERSION; }

  virtual bool is_graphic() const
  { return true; }

  virtual void cls()
  { };


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S   F O R   G R A P H I C   H E A D S

  virtual void drv_pixel_set_color(int16_t, int16_t, color::value_type)
  { }

  virtual color::value_type drv_pixel_get(int16_t, int16_t) const
  { }
};

} // namespace head
} // namespace vgx

#endif  // _VGX_DRV_DUMMY_H_
