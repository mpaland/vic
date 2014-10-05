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

#include "vgx_gpr.h"


namespace vgx {

/**
 * Display hardware interface
 */
typedef enum enum_interface_type
{
  interface_mem = 0,    // memory mapped interface
  interface_spi,        // SPI interface
  interface_i2c,        // I²C interface
  interface_can,        // CAN interface
  interface_usb,        // USB interface
  interface_uart        // UART interface
  intercace_cust        // custom interface (bit banging etc.)
} interface_type;

/**
 * Display orientation, the driver takes care of screen rotation
 * (0,0) is always logical (left/top)
 */
typedef enum enum_orientation_type
{
  orientation_0 = 0,
  orientation_90,
  orientation_180,
  orientation_270
} orientation_type;


///////////////////////////////////////////////////////////////////////////////
// H A R D W A R E   A C C E S S
//
// define the according functions in your project
//
template<typename DATA_TYPE>
void out_mem_set(volatile void* address, DATA_TYPE data);

template<typename DATA_TYPE>
DATA_TYPE out_mem_get(volatile void* address);


bool out_spi(std::uint16_t device_id,
             const std::uint8_t* data_out, std::uint16_t data_out_length,
             std::uint8_t* data_in, std::uint16_t data_in_length);


bool out_i2c(std::uint8_t device_id,
             const std::uint8_t* data_out, std::uint16_t data_out_length,
             std::uint8_t* data_in, std::uint16_t data_in_length);


bool out_uart_tx(std::uint8_t port,
                 const std::uint8_t* data_out, std::uint16_t data_out_length);

bool out_uart_rx(std::uint8_t port,
                 std::uint8_t* data_in, std::uint16_t data_in_length, std::uint16_t& bytes_received);

///////////////////////////////////////////////////////////////////////////////


class drv : public gpr
{
public:
  /**
   * ctor
   * \param xsize Screen width
   * \param ysize Screen height
   * \param xoffset X offset within the screen, relative to top/left corner, usefull if border pixels are behind a bezel
   * \param yoffset Y offset within the screen, relative to top/left corner, usefull if border pixels are behind a bezel
   */
  drv(std::uint16_t xsize, std::uint16_t ysize, std::int16_t xoffset, std::int16_t yoffset)
    : xsize_(xsize)
    , ysize_(ysize)
    , xoffset_(xoffset)
    , yoffset_(yoffset)
  { }

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
   * Set display or backlight brightness
   * \param level 0: dark, backlight off; 255: maximum brightness, backlight full on
   */
  virtual void brightness_set(std::uint8_t level) = 0;

  /**
   * Returns the driver version and name
   * \return Driver version and name
   */
  virtual const char* version() const = 0;

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
   * Returns the screen width
   * \return Screen width in pixel or chars
   */
  virtual std::uint16_t get_width()  const
  { return xsize_; }

  /**
   * Returns the screen height
   * \return Screen height in pixel or chars
   */
  virtual std::uint16_t get_height() const
  { return ysize_; }

  /**
   * Set alpha blending level of framebuffer/plane. If driver has no alpha support,
   * activate the according framebuffer/plane
   * \param index The index of the framebuffer/plane, 0 for 1st
   * \param alpha Alpha level, 0 = opaque/active, 255 = complete transparent/disabled
   */
  virtual bool framebuffer_set(std::uint8_t index, std::uint8_t alpha)
  { (void)index; (void)alpha; return false; }

  /**
   * Returns the number of available framebuffers/planes. 0 if no framebuffer
   * \return Number of frame buffers
   */
  virtual std::uint8_t framebuffer_get_count()
  { return 0U; }

protected:
  /** common color conversion routines

  color depth  format    usage
  ##########################################################################
  1                     1   monochrome
   2                 2   2 bit grayscale
   4                 4   4 bit grayscale
   8                 8   8 bit grayscale
  8                 3-3-2    256 RGB display - no palette
  15                5-5-5    32k RGB display
  16                5-6-5    64k RGB display
  18              0-6-6-6   262k RGB display
  24              0-8-8-8    16M RGB display
  32              A-8-8-8    16M RGB display with alpha blending support
                 R-G-B
  */

  /**
   * Convert internal 32 bpp ARGB color to native head color format
   * \param color Internal 32 bpp ARGB color value
   * \return Native head color value
   */
  inline std::uint8_t color_to_head_g1(std::uint32_t color) const
  { return static_cast<std::uint8_t>((color & 0x00FFFFFFUL) != (std::uint32_t)0U ? 1U : 0U); }

  inline std::uint8_t color_to_head_g2(std::uint32_t color) const
  { return static_cast<std::uint8_t>(((std::uint16_t)((std::uint16_t)color_get_red(color) + (std::uint16_t)color_get_green(color) + (std::uint16_t)color_get_blue(color)) / 3U) >> 6U); }

  inline std::uint8_t color_to_head_g4(std::uint32_t color) const
  { return static_cast<std::uint8_t>(((std::uint16_t)((std::uint16_t)color_get_red(color) + (std::uint16_t)color_get_green(color) + (std::uint16_t)color_get_blue(color)) / 3U) >> 4U); }

  inline std::uint8_t color_to_head_g8(std::uint32_t color) const
  { return static_cast<std::uint8_t>((std::uint16_t)((std::uint16_t)color_get_red(color) + (std::uint16_t)color_get_green(color) + (std::uint16_t)color_get_blue(color)) / 3U); }

  inline std::uint8_t color_to_head_8(std::uint32_t color) const
  { return static_cast<std::uint8_t>((std::uint8_t)(color_get_red(color) & 0xE0U) | (std::uint8_t)((color_get_green(color) & 0xE0U) >> 3U) | (std::uint8_t)((color_get_blue(color)) >> 6U)); }

  inline std::uint16_t color_to_head_15(std::uint32_t color) const
  { return static_cast<std::uint16_t>(((std::uint16_t)(color_get_red(color) & 0xF8U) << 10U) | ((std::uint16_t)(color_get_green(color) & 0xF8U) << 5U) | (std::uint16_t)(color_get_blue(color) >> 3U)); }

  inline std::uint16_t color_to_head_16(std::uint32_t color) const
  { return static_cast<std::uint16_t>(((std::uint16_t)(color_get_red(color) & 0xF8U) << 11U) | ((std::uint16_t)(color_get_green(color) & 0xFCU) << 5U) | (std::uint16_t)(color_get_blue(color) >> 3U)); }

  inline std::uint32_t color_to_head_18(std::uint32_t color) const
  { return static_cast<std::uint32_t>(((std::uint32_t)(color_get_red(color) & 0xFCU) << 12U) | ((std::uint32_t)(color_get_green(color) & 0xFCU) << 6U) | (std::uint32_t)(color_get_blue(color) >> 2U)); }

  inline std::uint32_t color_to_head_24(std::uint32_t color) const
  { return static_cast<std::uint32_t>(color & 0x00FFFFFFUL); }

  inline std::uint32_t color_from_head_g1(std::uint8_t head_color) const
  { return head_color ? VGX_COLOR_WHITE : VGX_COLOR_BLACK; }

  inline std::uint32_t color_from_head_g2(std::uint8_t head_color) const
  { return color_dim(VGX_COLOR_WHITE, (255U / 3U * (head_color & 0x03U))); }

  inline std::uint32_t color_from_head_g4(std::uint8_t head_color) const
  { return color_dim(VGX_COLOR_WHITE, (255U / 15U * (head_color & 0x0FU))); }

  inline std::uint32_t color_from_head_g8(std::uint8_t head_color) const
  { return color_dim(VGX_COLOR_WHITE, head_color); }

  inline std::uint32_t color_from_head_8(std::uint8_t head_color) const
  { return color_rgb(static_cast<std::uint8_t>(head_color & 0xE0U), static_cast<std::uint8_t>((head_color & 0x1CU) << 3U), static_cast<std::uint8_t>((head_color & 0x03U) << 6U)); }

  inline std::uint32_t color_from_head_15(std::uint16_t head_color) const
  { return color_rgb(static_cast<std::uint8_t>((head_color & 0x7C00U) >> 7U), static_cast<std::uint8_t>((head_color & 0x03E0U) >> 2U), static_cast<std::uint8_t>((head_color & 0x001FU) << 3U)); }

  inline std::uint32_t color_from_head_16(std::uint16_t head_color) const
  { return color_rgb(static_cast<std::uint8_t>((head_color & 0xF800U) >> 8U), static_cast<std::uint8_t>((head_color & 0x07E0U) >> 3U), static_cast<std::uint8_t>((head_color & 0x001FU) << 3U)); }

  inline std::uint32_t color_from_head_18(std::uint32_t head_color) const
  { return color_rgb(static_cast<std::uint8_t>((head_color & 0x0003F000UL) >> 10U), static_cast<std::uint8_t>((head_color & 0x00000FC0UL) >> 4U), static_cast<std::uint8_t>((head_color & 0x0000003FUL) << 2U)); }

  inline std::uint32_t color_from_head_24(std::uint32_t head_color) const
  { return static_cast<std::uint32_t>(head_color & 0x00FFFFFFUL); }

protected:
  const std::uint16_t xsize_;     // x screen size in pixel (graphic) or chars (alpha)
  const std::uint16_t ysize_;     // y screen size in pixel (graphic) or chars (alpha)
  const std::int16_t  xoffset_;   // x offset (left)
  const std::int16_t  yoffset_;   // y offset (top)
};

} // namespace vgx

#endif  // _VGX_DRV_H_
