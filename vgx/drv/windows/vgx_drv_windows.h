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
// \brief Windows head driver
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_DRV_WINDOWS_H_
#define _VGX_DRV_WINDOWS_H_

#include "vgx_drv.h"

#include <Windows.h>


namespace vgx {
namespace head {

// define 32bit 24bpp (no alpha channel) windows head
class windows : public drv
{
public:
  /**
   * ctor
   * \param xsize Screen width
   * \param ysize Screen height
   * \param xoffset X offset of the screen, relative to top/left corner
   * \param yoffset Y offset of the screen, relative to top/left corner
   * \param xpos X coordinate of the top/left window position
   * \param ypos Y coordinate of the top/left window position
   * \param xzoom X zoom factor
   * \param yzoom Y zoom factor
   */
  windows(std::uint16_t xsize, std::uint16_t ysize, std::int16_t xoffset, std::int16_t yoffset,
          std::int16_t xpos, std::int16_t ypos, std::uint8_t xzoom, std::uint8_t yzoom)
    : drv(xsize, ysize, xoffset, yoffset)
    , xpos_(xpos)
    , ypos_(ypos)
    , xzoom_(xzoom)
    , yzoom_(yzoom)
    , wnd_state_(create)
  { }

  /**
   * dtor
   * Deinit the driver
   */
  ~windows()
  { deinit(); }

  // mandatory driver functions
  virtual void init();                                    // driver init
  virtual void deinit();                                  // driver deinit
  virtual void brightness_set(std::uint8_t level);        // set display or backlight brightness
  virtual const char* version() const;                    // get driver name and version
  virtual void cls();                                     // clear display, all pixels off (black)

/////////////////////////////////////////////////////////////////////////////
// M A N D A T O R Y   F U N C T I O N S   F O R   G R A P H I C   H E A D S

  /**
   * Set pixel (in actual drawing color)
   * \param x X value
   * \param y Y value
   */
  virtual void drv_pixel_set(std::int16_t x, std::int16_t y);

  /**
   * Set pixel in given color, the color doesn't change the actual drawing color
   * \param x X value
   * \param y Y value
   * \param color Color of pixel in ARGB format
   */
  virtual void drv_pixel_set_color(std::int16_t x, std::int16_t y, std::uint32_t color);

  /**
   * Get pixel color
   * \param x X value
   * \param y Y value
   * \return Color of pixel in ARGB format
   */
  virtual std::uint32_t drv_pixel_get(std::int16_t x, std::int16_t y) const;


  /**
   * Rendering is done (copy RAM / frame buffer to screen)
   */
  virtual void drv_primitive_done();


public:
  // public for thread accessibility
  typedef enum enum_window_state_type {
    create = 0,
    ready,
    error,
    end
  } window_state_type;

  static void worker_thread(void* arg);   // worker thread
  ::HANDLE thread_handle_;                // worker thread handle
  volatile window_state_type wnd_state_;  // window state

  const std::int16_t  xpos_;              // x coordinate of output window
  const std::int16_t  ypos_;              // y coordinate of output window
  const std::uint8_t  xzoom_;             // x zoom factor of output window
  const std::uint8_t  yzoom_;             // y zoom factor of output window
  ::HWND              hwnd_;
  ::HDC               hmemdc_;
  ::HBITMAP           hbmp_;
};

} // namespace head
} // namespace vgx

#endif  // _VGX_DRV_WINDOWS_H_
