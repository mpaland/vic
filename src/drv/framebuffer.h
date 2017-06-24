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

#ifndef _VGX_DRV_FRAMEBUFFER_H_
#define _VGX_DRV_FRAMEBUFFER_H_

#include "../drv.h"


// defines the driver name and version
#define VGX_DRV_FRAMEBUFFER_VERSION   "Framebuffer driver 1.00"

namespace vgx {
namespace head {


/**
 * Skeleton driver
 * \param Screen_Size_X Screen (buffer) width
 * \param Screen_Size_Y Screen (buffer) height
 * \param PLANE_COUNT Number of planes
 * \param PLANE_FORMAT Reserved (later fused for bytes per pixel)
 */
template<std::uint16_t Screen_Size_X, std::uint16_t Screen_Size_Y,
         std::size_t PLANE_COUNT, color::format_type FORMAT = color::format_ARGB8888>
class framebuffer : public drv
{
public:

  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   D R I V E R   F U N C T I O N S

  /**
   * ctor
   * \param head The bound head
   */
  framebuffer(drv& head)
    : drv(Screen_Size_X, Screen_Size_Y,
          Screen_Size_X, Screen_Size_Y)
    , head_(head)
    , plane_active_(0U)
    , plane_display_(0U)
  { }


  /**
   * dtor
   * Shutdown the driver
   */
  ~framebuffer()
  {
    // normally a head is not deconstructed. But if so, shutdown the driver
    drv_shutdown();
  }


protected:

  virtual void drv_init()
  {
    head_.init();
  }


  virtual void drv_shutdown()
  {
    head_.shutdown();
  }


  virtual inline const char* drv_version() const
  {
    // return the driver version, like
    return (const char*)VGX_DRV_FRAMEBUFFER_VERSION;
  }


  virtual inline bool drv_is_graphic() const
  {
    // return if the display is a graphic display (true)
    return true;
  }


  virtual void drv_cls()
  {
    for (std::uint16_t y = 0U; y < Screen_Size_Y; ++y) {
      for (std::uint16_t x = 0U; x < Screen_Size_X; ++x) {
        buffer_[plane_active_][x][y] = 0U;
      }
    }
    head_.cls();
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
    if (!screen_is_inside(point)) {
      // out of bounds or outside clipping region
      return;
    }

    // store in buffer
    buffer_[plane_active_][point.x][point.y] = color;

    // to head
    if (plane_display_ == plane_active_) {
      head_.pixel_set(point, color);
    }
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
    return buffer_[plane_active_][point.x][point.y];
  }


  /**
   * Rendering is done (copy RAM / frame buffer to screen)
   */
  virtual void drv_present()
  {
    head_.present();
  }


  virtual bool framebuffer_set_display(std::size_t plane, std::uint8_t)
  {
    if (plane_display_ != plane) {
      plane_display_ = plane;

      // present the new active plane
      for (std::int16_t y = 0U; y < Screen_Size_Y; ++y) {
        for (std::int16_t x = 0U; x < Screen_Size_X; ++x) {
          head_.pixel_set({ x, y }, buffer_[plane_display_][x][y]);
        }
      }
      head_.present();
    }
    return true;
  }


  virtual bool framebuffer_set_access(std::size_t plane)
  {
    plane_active_ = plane;
    return true;
  }


  /**
   * Returns the number of available framebuffers/planes. 1 if no framebuffer support (so just one buffer)
   * \return Number of frame buffers
   */
  virtual std::size_t framebuffer_get_count() const
  { return 0U; }


private:

  color::value_type buffer_[PLANE_COUNT][Screen_Size_X][Screen_Size_Y];
  drv&              head_;
  std::size_t       plane_active_;
  std::size_t       plane_display_;
};

} // namespace head
} // namespace vgx

#endif  // _VGX_DRV_FRAMEBUFFER_H_
