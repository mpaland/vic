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
// \brief Driver and driver head interface
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_DRV_H_
#define _VGX_DRV_H_

#include <vgx_cfg.h>      // use < > here, cause vgx_cfg.h may be in some platform folder
#include "vgx_colors.h"
#include "vgx_gpr.h"

#include <cstdint>


namespace vgx {

/**
 * Display hardware interface
 */
typedef enum enum_interface_type
{
  interface_mem = 0,
  interface_spi,
  interface_i2c,
  interface_uart
} interface_type;

/**
 * Display orientation, the driver takes care of screen rotation
 */
typedef enum enum_orientation_type
{
  orientation_0 = 0,
  orientation_90,
  orientation_180,
  orientation_270
} orientation_type;


class drv : public gpr
{
public:
  /////////////////////////////////////////////////////////////////////////////
  // All functions in this section are MANDATORY driver functions!
  // Every driver MUST implement them - even if not supported

  /**
   * Driver init
   */
  virtual void init() = 0;

  /**
   * Driver deinit
   */
  virtual void deinit() = 0;

  /**
   * Set display brightness/backlight
   * \param level 0: dark, backlight off; 255: maximum brightness
   */
  virtual void brightness_set(std::uint8_t level) = 0;

  /**
   * Returns the driver version and name
   * \return Driver version and name
   */
  virtual const char* version() const = 0;

  /**
   * Get the screen width
   * \return Screen width in pixel or chars
   */
  virtual std::uint16_t get_width() const = 0;

  /**
   * Get the screen height
   * \return Screen height in pixel or chars
   */
  virtual std::uint16_t get_height() const = 0;

  /**
   * Clear screen, set all pixels off, delete all characters or fill screen with background color
   */
  virtual void cls() = 0;


  /////////////////////////////////////////////////////////////////////////////
  // The functions in this section are MANDATORY FOR GRAPHIC drivers!
  // Every graphic display driver MUST implement them., even if unsupported
  // In case of alpha numeric display, leave unimplemented in derived class

  /**
   * Set pixel (in actual drawing color)
   * \param x X value
   * \param y Y value
   */
  virtual void pixel_set(std::int16_t x, std::int16_t y)
  { (void)x; (void)y; }

  /**
   * Set pixel in given color, the color doesn't change the actual drawing color
   * \param x X value
   * \param y Y value
   * \param color Color of pixel in ARGB format
   */
  virtual void pixel_set_color(std::int16_t x, std::int16_t y, std::int32_t color)
  { (void)x; (void)y; (void)color; }

  /**
   * Get pixel color
   * \param x X value
   * \param y Y value
   * \return Color of pixel in ARGB format
   */
  virtual std::uint32_t pixel_get(std::int16_t x, std::int16_t y) const
  { (void)x; (void)y; return 0U; }

  /////////////////////////////////////////////////////////////////////////////

  /**
   * Set alpha blending level of framebuffer/plane. If driver has no alpha support,
   * activate the according framebuffer/plane
   * \param index The index of the framebuffer/plane, 0 for 1st
   * \param alpha Alpha level, 0 = disabled and complete transparent, 255 = opaque or activated
   */
  virtual bool framebuffer_set(std::uint8_t index, std::uint8_t alpha)
  { (void)index; (void)alpha; return false; }

  /**
   * Returns the number of available framebuffers/planes. 0 if no framebuffer
   * \return Number of frame buffers
   */
  virtual std::uint8_t framebuffer_get_count()
  { return 0U; }
};


template<typename COLOR_TYPE, std::uint8_t COLOR_DEPTH>
class drv_head : public drv
{
public:
  /**
   * ctor
   * \param xsize Screen width
   * \param ysize Screen height
   * \param xoffset X offset of the screen, relative to top/left corner
   * \param yoffset Y offset of the screen, relative to top/left corner
   */
  drv_head(std::uint16_t xsize, std::uint16_t ysize, std::int16_t xoffset, std::int16_t yoffset)
    : xsize_(xsize)
    , ysize_(ysize)
    , xoffset_(xoffset)
    , yoffset_(yoffset)
  { }

  /**
   * Returns the screen width
   * \return Width in pixel
   */
  virtual std::uint16_t get_width()  const { return xsize_; }

  /**
   * Returns the screen height
   * \return Height in pixel
   */
  virtual std::uint16_t get_height() const { return ysize_; }

protected:
  /*
  RGB conversion macros to head color depth
  Color depths of 1, 2 or 4 are for monochrome and 2/4bit grayscale format

  COLOR_DEPTH     Format    Usage
  ##########################################################################
  1                     1   monochrome
  2                     2   grayscale
  4                     4   grayscale
  8                 3-3-2    256 RGB display - no palette
  15                5-5-5    32k RGB display
  16                5-6-5    64k RGB display
  18              0-6-6-6   262k RGB display
  24              0-8-8-8    16M RGB display
  32              A-8-8-8    16M RGB display with alpha blending support
  */
  template<std::uint8_t COLOR_DEPTH>
  inline COLOR_TYPE head_color(std::uint32_t color) const
  { return 0UL; }   // unsupported color depth

  template<>
  inline COLOR_TYPE head_color<1U>(std::uint32_t color) const
  { return static_cast<COLOR_TYPE>((color & 0x00FFFFFFUL) != std::uint32_t(0U) ? 1U : 0U); }

  template<>
  inline COLOR_TYPE head_color<2U>(std::uint32_t color) const
  { return static_cast<COLOR_TYPE>((((std::uint16_t)color_get_red(color) + (std::uint16_t)color_get_green(color) + (std::uint16_t)color_get_blue(color)) / 3U) >> 6U); }

  template<>
  inline COLOR_TYPE head_color<4U>(std::uint32_t color) const
  { return static_cast<COLOR_TYPE>((((std::uint16_t)color_get_red(color) + (std::uint16_t)color_get_green(color) + (std::uint16_t)color_get_blue(color)) / 3U) >> 4U); }

  template<>
  inline COLOR_TYPE head_color<8U>(std::uint32_t color) const
  { return static_cast<COLOR_TYPE>((color_get_red(color) & 0xE0U) | ((color_get_green(color) & 0xE0U) >> 3U) | ((color_get_blue(color)) >> 6U)); }

  template<>
  inline COLOR_TYPE head_color<15U>(std::uint32_t color) const
  { return static_cast<COLOR_TYPE>(((COLOR_TYPE)(color_get_red(color) & 0xF8U) << 10U) | ((COLOR_TYPE)(color_get_green(color) & 0xF8U) << 5U) | (COLOR_TYPE)(color_get_blue(color) >> 3U)); }

  template<>
  inline COLOR_TYPE head_color<16U>(std::uint32_t color) const
  { return static_cast<COLOR_TYPE>(((COLOR_TYPE)(color_get_red(color) & 0xF8U) << 11U) | ((COLOR_TYPE)(color_get_green(color) & 0xFCU) << 5U) | (COLOR_TYPE)(color_get_blue(color) >> 3U)); }

  template<>
  inline COLOR_TYPE head_color<18U>(std::uint32_t color) const
  { return static_cast<COLOR_TYPE>(((COLOR_TYPE)(color_get_red(color) & 0xFCU) << 12U) | ((COLOR_TYPE)(color_get_green(color) & 0xFCU) << 6U) | (COLOR_TYPE)(color_get_blue(color) >> 2U)); }

  template<>
  inline COLOR_TYPE head_color<24U>(std::uint32_t color) const
  { return static_cast<COLOR_TYPE>(color & 0x00FFFFFFUL); }

  template<>
  inline COLOR_TYPE head_color<32U>(std::uint32_t color) const
  { return static_cast<COLOR_TYPE>(color); }

protected:
  const std::uint16_t xsize_;     // x screen size in pixel (graphic) or chars (alpha)
  const std::uint16_t ysize_;     // y screen size in pixel (graphic) or chars (alpha)
  const std::int16_t  xoffset_;   // x offset (left)
  const std::int16_t  yoffset_;   // y offset (top)
};

} // namespace vgx

#endif  // _VGX_DRV_H_
