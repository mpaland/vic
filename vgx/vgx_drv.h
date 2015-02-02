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
  interface_uart,       // UART interface
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


/**
 * supported color formats
  bits depth  format    usage
  ##########################################################################
   1     1           1   monochrome
   2     2           2   2 bit grayscale
   4     4           4   4 bit grayscale
   8     8           8   8 bit grayscale
   8     8        CLUT   8 bit color lookup table for 256 defined colors
   8     8       3-3-2    256 RGB display - no palette
  15    15       5-5-5    32k RGB display
  16    16       5-6-5    64k RGB display
  16    12     A-4-4-4     4k RGB display with 4 bit alpha blending
  16    15     A-5-5-5    32k RGB display with 1 bit alpha blending
  18    18     0-6-6-6   262k RGB display
  24    24     0-8-8-8    16M RGB display
  32    24     A-8-8-8    16M RGB display with alpha blending
                 R-G-B
*/
typedef enum enum_color_format_type
{
  color_format_L1 = 0,    // 1 bit luminance (monochrome)
  color_format_L2,        // 2 bit
  color_format_L4,        // 4 bit
  color_format_L8,        // 8 bit, 256 luminance (gray) levels
  color_format_C8,        // 8 bit CLUT (color lookup table) - reserved for future, unsupported yet
  color_format_RGB332,
  color_format_RGB555,
  color_format_RGB565,
  color_format_RGB666,
  color_format_RGB888,
  color_format_ARGB1555,
  color_format_ARGB4444,
  color_format_ARGB8888
} color_format_type;


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
  virtual void pixel_set_color(std::int16_t x, std::int16_t y, std::uint32_t color)
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
   * Returns the number of available framebuffers/planes. 1 if no framebuffer support (so just one buffer)
   * \return Number of frame buffers
   */
  virtual std::uint8_t framebuffer_get_count()
  { return 0U; }

protected:
  /**
   * Convert internal 32 bpp ARGB color to native head color format
   * \param color Internal 32 bpp ARGB color value
   * \return Native head color value
   */
  inline std::uint8_t color_to_head_L1(std::uint32_t color) const
  { return static_cast<std::uint8_t>((color & 0x00FFFFFFUL) != (std::uint32_t)0U ? 1U : 0U); }

  inline std::uint8_t color_to_head_L2(std::uint32_t color) const
  { return static_cast<std::uint8_t>(((std::uint16_t)((std::uint16_t)color_get_red(color) + (std::uint16_t)color_get_green(color) + (std::uint16_t)color_get_blue(color)) / 3U) >> 6U); }

  inline std::uint8_t color_to_head_L4(std::uint32_t color) const
  { return static_cast<std::uint8_t>(((std::uint16_t)((std::uint16_t)color_get_red(color) + (std::uint16_t)color_get_green(color) + (std::uint16_t)color_get_blue(color)) / 3U) >> 4U); }

  inline std::uint8_t color_to_head_L8(std::uint32_t color) const
  { return static_cast<std::uint8_t>((std::uint16_t)((std::uint16_t)color_get_red(color) + (std::uint16_t)color_get_green(color) + (std::uint16_t)color_get_blue(color)) / 3U); }

  inline std::uint8_t color_to_head_RGB332(std::uint32_t color) const
  { return static_cast<std::uint8_t>((std::uint8_t)(color_get_red(color) & 0xE0U) | (std::uint8_t)((color_get_green(color) & 0xE0U) >> 3U) | (std::uint8_t)((color_get_blue(color)) >> 6U)); }

  inline std::uint16_t color_to_head_RGB555(std::uint32_t color) const
  { return static_cast<std::uint16_t>(((std::uint16_t)(color_get_red(color) & 0xF8U) << 10U) | ((std::uint16_t)(color_get_green(color) & 0xF8U) << 5U) | (std::uint16_t)(color_get_blue(color) >> 3U)); }

  inline std::uint16_t color_to_head_RGB565(std::uint32_t color) const
  { return static_cast<std::uint16_t>(((std::uint16_t)(color_get_red(color) & 0xF8U) << 11U) | ((std::uint16_t)(color_get_green(color) & 0xFCU) << 5U) | (std::uint16_t)(color_get_blue(color) >> 3U)); }

  inline std::uint32_t color_to_head_RGB666(std::uint32_t color) const
  { return static_cast<std::uint32_t>(((std::uint32_t)(color_get_red(color) & 0xFCU) << 12U) | ((std::uint32_t)(color_get_green(color) & 0xFCU) << 6U) | (std::uint32_t)(color_get_blue(color) >> 2U)); }

  inline std::uint32_t color_to_head_RGB888(std::uint32_t color) const
  { return static_cast<std::uint32_t>(color & 0x00FFFFFFUL); }

  inline std::uint32_t color_from_head_L1(std::uint8_t head_color) const
  { return head_color ? VGX_COLOR_WHITE : VGX_COLOR_BLACK; }

  inline std::uint32_t color_from_head_L2(std::uint8_t head_color) const
  { return color_dim(VGX_COLOR_WHITE, (255U / 3U * (head_color & 0x03U))); }

  inline std::uint32_t color_from_head_L4(std::uint8_t head_color) const
  { return color_dim(VGX_COLOR_WHITE, (255U / 15U * (head_color & 0x0FU))); }

  inline std::uint32_t color_from_head_L8(std::uint8_t head_color) const
  { return color_dim(VGX_COLOR_WHITE, head_color); }

  inline std::uint32_t color_from_head_RGB332(std::uint8_t head_color) const
  { return color_rgb(static_cast<std::uint8_t>(head_color & 0xE0U), static_cast<std::uint8_t>((head_color & 0x1CU) << 3U), static_cast<std::uint8_t>((head_color & 0x03U) << 6U)); }

  inline std::uint32_t color_from_head_RGB555(std::uint16_t head_color) const
  { return color_rgb(static_cast<std::uint8_t>((head_color & 0x7C00U) >> 7U), static_cast<std::uint8_t>((head_color & 0x03E0U) >> 2U), static_cast<std::uint8_t>((head_color & 0x001FU) << 3U)); }

  inline std::uint32_t color_from_head_RGB565(std::uint16_t head_color) const
  { return color_rgb(static_cast<std::uint8_t>((head_color & 0xF800U) >> 8U), static_cast<std::uint8_t>((head_color & 0x07E0U) >> 3U), static_cast<std::uint8_t>((head_color & 0x001FU) << 3U)); }

  inline std::uint32_t color_from_head_RGB666(std::uint32_t head_color) const
  { return color_rgb(static_cast<std::uint8_t>((head_color & 0x0003F000UL) >> 10U), static_cast<std::uint8_t>((head_color & 0x00000FC0UL) >> 4U), static_cast<std::uint8_t>((head_color & 0x0000003FUL) << 2U)); }

  inline std::uint32_t color_from_head_RGB888(std::uint32_t head_color) const
  { return static_cast<std::uint32_t>(head_color & 0x00FFFFFFUL); }


  ///////////////////////////////////////////////////////////////////////////////
  // H A R D W A R E   A C C E S S
  //
  // Define the according functions in your project
  //
  struct out {
    // Direct memory access, write
    template<typename VALUE_TYPE>
    static void mem_set(volatile void* address, VALUE_TYPE value);

    // Direct memory access, masked write
    template<typename VALUE_TYPE>
    static void mem_mask(volatile void* address, VALUE_TYPE value, VALUE_TYPE mask);

    // Direct memory access, read
    template<typename VALUE_TYPE>
    static VALUE_TYPE mem_get(volatile void* address);

    // SPI access
    static bool spi(std::uint16_t device_id,
                    const std::uint8_t* data_out, std::uint16_t data_out_length,
                    std::uint8_t* data_in, std::uint16_t data_in_length);

    // I²C access
    static bool i2c(std::uint8_t device_id,
                    const std::uint8_t* data_out, std::uint16_t data_out_length,
                    std::uint8_t* data_in, std::uint16_t data_in_length);

    // UART access, send
    static bool uart_tx(std::uint8_t port,
                        const std::uint8_t* data_out, std::uint16_t data_out_length);

    // UART access, receive
    static bool uart_rx(std::uint8_t port,
                        std::uint8_t* data_in, std::uint16_t data_in_length, std::uint16_t& bytes_received);

    // Digital I/O, pin set (used for bit banging, etc)
    static bool dio_set(std::uint8_t id, bool value);

    // Digital I/O, pin get
    static bool dio_get(std::uint8_t id);
  };

protected:
  const std::uint16_t xsize_;     // x screen size in pixel (graphic) or chars (alpha)
  const std::uint16_t ysize_;     // y screen size in pixel (graphic) or chars (alpha)
  const std::int16_t  xoffset_;   // x offset (left)
  const std::int16_t  yoffset_;   // y offset (top)
};

} // namespace vgx

#endif  // _VGX_DRV_H_
