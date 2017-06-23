///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2017, PALANDesign Hannover, Germany
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
// \brief Skeleton driver, use this as an easy start for own drivers
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_DRV_SKELETON_H_
#define _VGX_DRV_SKELETON_H_

#include "../drv.h"


// defines the driver name and version
#define VGX_DRV_SKELETON_VERSION   "Skeleton driver 1.00"

namespace vgx {
namespace head {


/**
 * Skeleton driver
 * \param Screen_Size_X Screen (buffer) width
 * \param Screen_Size_Y Screen (buffer) height
 * \param Viewport_Size_X Viewport (window / physical display) width
 * \param Viewport_Size_Y Viewport (window / physical display) height
 */
template<std::uint16_t Screen_Size_X,   std::uint16_t Screen_Size_Y,
         std::uint16_t Viewport_Size_X, std::uint16_t Viewport_Size_Y>
class skeleton : public drv
{
public:

  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   D R I V E R   F U N C T I O N S

  /**
   * ctor
   * \param xsize Screen width
   * \param ysize Screen height
   * \param xoffset X offset of the screen, relative to top/left corner
   * \param yoffset Y offset of the screen, relative to top/left corner
   */
  skeleton(orientation_type orientation, device_handle_type device_handle,
           std::uint16_t viewport_x = 0U, std::uint16_t viewport_y = 0U)
    : drv(Screen_Size_X,   Screen_Size_Y,
          Viewport_Size_Y, Viewport_Size_Y,
          viewport_x,      viewport_y,
          orientation)
  {
    // init of memory structures, normally empty
  }


  /**
   * dtor
   * Shutdown the driver
   */
  ~skeleton()
  {
    // normally a head is not deconstructed. But if so, shutdown the driver
    drv_shutdown();
  }


protected:

  virtual void drv_init()
  {
    // init the driver and hardware here
    // this is called from public init()
  }


  virtual void drv_shutdown()
  {
    // shutdown the driver and hardware here
    // or enter speep / powersave mode
    // this is called from public shutdown()
  }


  virtual inline const char* drv_version() const
  {
    // return the driver version, like
    return (const char*)VGX_DRV_SKELETON_VERSION;
  }


  virtual inline bool drv_is_graphic() const
  {
    // return if the display is a graphic display (true) or alpha numeric (false)
    return true;
  }


  virtual void drv_cls()
  {
    // clear the entire screen / buffer
  }


  /**
   * Set pixel in given color, the color doesn't change the actual drawing color
   * \param x X value
   * \param y Y value
   * \param color Color of pixel in ARGB format
   */
  virtual inline void drv_pixel_set_color(vertex_type point, color::value_type color)
  {
    // check limits and clipping
    if (!screen_is_inside(point) || !clipping_.is_inside(point)) {
      // out of bounds or outside clipping region
      return;
    }

    // set the pixel in the display buffer
  }


  /**
   * Get pixel color
   * \param x X value
   * \param y Y value
   * \return Color of pixel in ARGB format
   */
  virtual inline color::value_type drv_pixel_get(vertex_type point) const
  {
    // check limits and clipping
    if (!screen_is_inside(point)) {
      // out of bounds or outside clipping region
      return vgx::color::black;
    }

    // return the pixel color at the given position
    return vgx::color::black;
  }


  /**
   * Rendering is done (copy RAM / frame buffer to screen)
   */
  virtual void drv_present()
  {
    // rendering is done
    // copy memory / buffer to real display
    // leave this function empty if the display has just one buffer
  }


  /////////////////////////////////////////////////////////////////////////////
  // O P T I O N A L   D R I V E R   F U N C T I O N S


  virtual void brightness_set(std::uint8_t level)
  {
    // set the brightness level if your display has support
  }

};

} // namespace head
} // namespace vgx

#endif  // _VGX_DRV_SKELETON_H_
