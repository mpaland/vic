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
// \brief Driver base class and head interface functions
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_DRV_H_
#define _VGX_DRV_H_

#include "gpr.h"
#include "txr.h"
#include <io.h>     // hardware dependent IO access, use < > here, cause io.h may be in some platform folder


namespace vgx {


typedef struct tag_clipping_type
{
  /**
   * ctor
   * Create a clipping region, default disabled
   */
  tag_clipping_type()
    : active_(false)
  { }

  /**
   * ctor
   * Create an enabled clipping region
   * \param v0 Left top corner of the region
   * \param v1 Bottom right corner of the region
   * \param inside True if the clipping region is INSIDE the given box, so all pixels inside the clipping region are drawn. This is the default.
   */
  tag_clipping_type(vertex_type v0, vertex_type v1, bool inside = true) {
    set(v0, v1, inside);
  }

  /**
   * Set the clipping region
   * \param v0 Left top corner of the region
   * \param v1 Bottom right corner of the region
   * \param inside True if the clipping region is INSIDE the given box, so all pixels inside the clipping region are drawn. This is the default.
   */
  void set(vertex_type v0, vertex_type v1, bool inside = true) {
    v0_ = v0; v1_ = v1; inside_ = inside; active_ = true;
    if (v0_.x > v1_.x) { std::int16_t t = v0_.x; v0_.x = v1_.x; v1_.x = t; }
    if (v0_.y > v1_.y) { std::int16_t t = v0_.y; v0_.y = v1_.y; v1_.y = t; }
  }

  /**
   * Test if clipping is active and if the given vertex is inside the clipping region
   * \param v Vertex to test
   * \return True if given vertex is within the active clipping region and should be drawn
   */
  inline bool is_inside(vertex_type v) const {
    return !active_ || ((v.x >= v0_.x && v.x <= v1_.x && v.y >= v0_.y && v.y <= v1_.y) ? inside_ : !inside_);
  }

  /**
   * Enable the clipping function
   * \param enable True to enable
   */
  inline void enable(bool _enable = true) {
    active_ = _enable;
  }

  /**
   * Return the clipping status
   * \return True if clipping is enabled
   */
  inline bool is_enabled() const {
    return active_;
  }

private:
  vertex_type v0_;
  vertex_type v1_;
  bool        active_;
  bool        inside_;
} clipping_type;


class drv : public gpr, public txr
{
public:

  /**
   * Display orientation, the driver takes care of screen rotation
   * Set the orientation so that (0,0) is always top/left
   */
  typedef enum tag_orientation_type
  {
    orientation_0 = 0,    //   0° one to one
    orientation_90,       //  90° clockwise
    orientation_180,      // 180° clockwise
    orientation_270,      // 270° clockwise
    orientation_0m,       //   0°            vertical screen mirror
    orientation_90m,      //  90° clockwise, vertical screen mirror
    orientation_180m,     // 180° clockwise, vertical screen mirror
    orientation_270m      // 270° clockwise, vertical screen mirror
  } orientation_type;

  ///////////////////////////////////////////////////////////////////////////////

protected:

  /**
   * ctor
   * \param screen_size_x Screen (buffer) width in pixel on graphic displays or chars on text displays
   * \param screen_size_y Screen (buffer) height  in pixel on graphic displays or chars on text displays
   * \param viewport_size_x Viewport (physical) width in pixel on graphic displays or chars on text displays
   * \param viewport_size_y Viewport (physical) height  in pixel on graphic displays or chars on text displays
   * \param viewport_x X offset within the screen, relative to top/left corner
   * \param viewport_y Y offset within the screen, relative to top/left corner
   * \param orientation Orientation of the display
   */
  drv(std::uint16_t screen_size_x,   std::uint16_t screen_size_y,
      std::uint16_t viewport_size_x, std::uint16_t viewport_size_y,
      std::int16_t  viewport_x = 0U, std::int16_t  viewport_y = 0U,
      orientation_type orientation = orientation_0)
    : screen_size_x_(screen_size_x)
    , screen_size_y_(screen_size_y)
    , viewport_size_x_(viewport_size_x)
    , viewport_size_y_(viewport_size_y)
    , viewport_({ viewport_x, viewport_y })
    , orientation_(orientation)
  { }


/////////////////////////////////////////////////////////////////////////////
// D R I V E R   B A S E   F U N C T I O N S
//
public:

  inline void init()
  {
    drv_init();
  }


  inline void shutdown()
  {
    drv_shutdown();
  }


  inline const char* version() const
  {
    return drv_version();
  }


  /**
   * Clear screen, set all pixels off, delete all characters or fill screen with background/blank color
   */
  inline void cls()
  {
    drv_cls();
    present();
  }


  /**
   * Returns the screen (buffer) width
   * \return Screen width in pixel or chars
   */
  inline std::uint16_t screen_width() const
  { return screen_size_x_; }


  /**
   * Returns the screen (buffer) height
   * \return Screen height in pixel or chars
   */
  inline std::uint16_t screen_height() const
  { return screen_size_y_; }


  /**
   * Returns the true if the given vertex is within the screen area
   * \param x X value in screen coordinates
   * \param y Y value in screen coordinates
   * \return true if the given vertex is within the screen area
   */
  inline bool screen_is_inside(vertex_type v) const
  { return v.x >= 0 && v.x < screen_size_x_ && v.y >= 0 && v.y < screen_size_y_; }


  /**
   * Returns the viewport (display) height
   * \return Viewport height in pixel or chars
   */
  inline std::uint16_t viewport_width() const
  { return viewport_size_x_; }


  /**
   * Returns the viewport (display) height
   * \return Viewport height in pixel or chars
   */
  inline std::uint16_t viewport_height() const
  { return viewport_size_y_; }


  /**
   * Returns true, if the given vertex is within the visible viewport
   * \param x X value in screen coordinates
   * \param y Y value in screen coordinates
   * \return true if the given vertex is within the viewport area
   */
  inline bool viewport_is_inside(vertex_type v) const
  { return v.x >= viewport_.x && v.x < (viewport_.x + viewport_size_x_) &&
           v.y >= viewport_.y && v.y < (viewport_.y + viewport_size_y_); }


  /**
   * Set the new viewport origin
   * \param x Left corner in screen coordinates
   * \param y Top corner in screen coordinates
   */
  inline virtual void viewport_set(vertex_type v)
  {
    viewport_ = v;
    present(); // refresh the screen
  }


  /**
   * Get the actual viewport origin
   * \param x Left corner in screen coordinates
   * \param y Top corner in screen coordinates
   */
  inline virtual vertex_type viewport_get() const
  { return viewport_; }


  /**
   * Set alpha blending level of framebuffer/plane. If driver has no alpha support,
   * activate the according framebuffer/plane
   * \param index The index of the framebuffer/plane, 0 for 1st
   * \param alpha Alpha level, 0 = opaque/active, 255 = complete transparent/disabled
   */
  virtual bool framebuffer_set(std::uint8_t index, std::uint8_t alpha)
  { (void)index; (void)alpha; return false; }


  /**
   * Returns the number of available framebuffers/planes. 1 if no framebuffer support (so just one buffer)
   * \return Number of frame buffers
   */
  virtual std::uint8_t framebuffer_get_count() const
  { return 1U; }


  /**
   * Set display or backlight brightness
   * \param level 0: dark, backlight off; 255: maximum brightness, backlight full on
   */
  virtual void brightness_set(std::uint8_t level)
  { (void)level; }


  ///////////////////////////////////////////////////////////////////////////////
  // C L I P P I N G   F U N C T I O N S 
  //

  /**
   * Set the clipping region
   * All further points within the region are not drawn
   * \param top_left Top left corner of the clipping region
   * \param bottom_right Bottom right corner of the clipping region
   * \param inside True if 
   */
  void inline clipping_set(vertex_type top_left, vertex_type bottom_right, bool inside = true)
  {
    clipping_.set(top_left, bottom_right, inside);
  }


  /**
   * Disable clipping
   */
  void inline clipping_reset()
  {
    clipping_.enable(false);
  }

  ///////////////////////////////////////////////////////////////////////////////
  // C O L O R   C O N V E R S I O N
  //

  /**
   * Convert internal 32 bpp ARGB color to native head color format
   * \param color Internal 32 bpp ARGB color value
   * \return Native head color value
   */
  inline std::uint8_t color_to_head_L1(color::value_type color) const
  { return static_cast<std::uint8_t>((color & 0x00FFFFFFUL) != (std::uint32_t)0U ? 1U : 0U); }

  inline std::uint8_t color_to_head_L2(color::value_type color) const
  { return static_cast<std::uint8_t>(((std::uint16_t)((std::uint16_t)color::get_red(color) + (std::uint16_t)color::get_green(color) + (std::uint16_t)color::get_blue(color)) / 3U) >> 6U); }

  inline std::uint8_t color_to_head_L4(color::value_type color) const
  { return static_cast<std::uint8_t>(((std::uint16_t)((std::uint16_t)color::get_red(color) + (std::uint16_t)color::get_green(color) + (std::uint16_t)color::get_blue(color)) / 3U) >> 4U); }

  inline std::uint8_t color_to_head_L8(color::value_type color) const
  { return static_cast<std::uint8_t>((std::uint16_t)((std::uint16_t)color::get_red(color) + (std::uint16_t)color::get_green(color) + (std::uint16_t)color::get_blue(color)) / 3U); }

  inline std::uint8_t color_to_head_RGB332(color::value_type color) const
  { return static_cast<std::uint8_t>((std::uint8_t)(color::get_red(color) & 0xE0U) | (std::uint8_t)((color::get_green(color) & 0xE0U) >> 3U) | (std::uint8_t)((color::get_blue(color)) >> 6U)); }

  inline std::uint16_t color_to_head_RGB444(color::value_type color) const
  { return static_cast<std::uint16_t>(((std::uint16_t)(color::get_red(color) & 0xF0U) <<  8U) | ((std::uint16_t)(color::get_green(color) & 0xF0U) << 4U) | (std::uint16_t)(color::get_blue(color) >> 4U)); }

  inline std::uint16_t color_to_head_RGB555(color::value_type color) const
  { return static_cast<std::uint16_t>(((std::uint16_t)(color::get_red(color) & 0xF8U) << 10U) | ((std::uint16_t)(color::get_green(color) & 0xF8U) << 5U) | (std::uint16_t)(color::get_blue(color) >> 3U)); }

  inline std::uint16_t color_to_head_RGB565(color::value_type color) const
  { return static_cast<std::uint16_t>(((std::uint16_t)(color::get_red(color) & 0xF8U) << 11U) | ((std::uint16_t)(color::get_green(color) & 0xFCU) << 5U) | (std::uint16_t)(color::get_blue(color) >> 3U)); }

  inline std::uint32_t color_to_head_RGB666(color::value_type color) const
  { return static_cast<std::uint32_t>(((std::uint32_t)(color::get_red(color) & 0xFCU) << 12U) | ((std::uint32_t)(color::get_green(color) & 0xFCU) << 6U) | (std::uint32_t)(color::get_blue(color) >> 2U)); }

  inline std::uint32_t color_to_head_RGB888(color::value_type color) const
  { return static_cast<std::uint32_t>(color & 0x00FFFFFFUL); }

  inline color::value_type color_from_head_L1(std::uint8_t head_color) const
  { return head_color ? color::white : color::black; }

  inline color::value_type color_from_head_L2(std::uint8_t head_color) const
  { return color::dim(color::white, (255U / 3U * (head_color & 0x03U))); }

  inline color::value_type color_from_head_L4(std::uint8_t head_color) const
  { return color::dim(color::white, (255U / 15U * (head_color & 0x0FU))); }

  inline color::value_type color_from_head_L8(std::uint8_t head_color) const
  { return color::dim(color::white, head_color); }

  inline color::value_type color_from_head_RGB332(std::uint8_t head_color) const
  { return color::rgb(static_cast<std::uint8_t>(head_color & 0xE0U), static_cast<std::uint8_t>((head_color & 0x1CU) << 3U), static_cast<std::uint8_t>((head_color & 0x03U) << 6U)); }

  inline color::value_type color_from_head_RGB444(std::uint16_t head_color) const
  { return color::rgb(static_cast<std::uint8_t>((head_color & 0x0F00U) >> 4U), static_cast<std::uint8_t>((head_color & 0x00F0U)      ), static_cast<std::uint8_t>((head_color & 0x000FU) << 4U)); }

  inline color::value_type color_from_head_RGB555(std::uint16_t head_color) const
  { return color::rgb(static_cast<std::uint8_t>((head_color & 0x7C00U) >> 7U), static_cast<std::uint8_t>((head_color & 0x03E0U) >> 2U), static_cast<std::uint8_t>((head_color & 0x001FU) << 3U)); }

  inline color::value_type color_from_head_RGB565(std::uint16_t head_color) const
  { return color::rgb(static_cast<std::uint8_t>((head_color & 0xF800U) >> 8U), static_cast<std::uint8_t>((head_color & 0x07E0U) >> 3U), static_cast<std::uint8_t>((head_color & 0x001FU) << 3U)); }

  inline color::value_type color_from_head_RGB666(std::uint32_t head_color) const
  { return color::rgb(static_cast<std::uint8_t>((head_color & 0x0003F000UL) >> 10U), static_cast<std::uint8_t>((head_color & 0x00000FC0UL) >> 4U), static_cast<std::uint8_t>((head_color & 0x0000003FUL) << 2U)); }

  inline color::value_type color_from_head_RGB888(std::uint32_t head_color) const
  { return static_cast<color::value_type>(head_color & 0x00FFFFFFUL); }


protected:
  const std::uint16_t screen_size_x_;     // screen (buffer) width  in pixel (graphic) or chars (alpha)
  const std::uint16_t screen_size_y_;     // screen (buffer) height in pixel (graphic) or chars (alpha)
  const std::uint16_t viewport_size_x_;   // viewport (display) width in pixel (graphic) or chars (alpha)
  const std::uint16_t viewport_size_y_;   // viewport (display) height in pixel (graphic) or chars (alpha)
  vertex_type         viewport_;          // viewport top/left corner (x offset to screen)
  orientation_type    orientation_;       // orientation of the display
  clipping_type       clipping_;          // clipping region
  drv_io              io;                 // driver IO access
};

} // namespace vgx

#endif  // _VGX_DRV_H_
