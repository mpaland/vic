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
// \brief Driver and driver head interface
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_DRV_H_
#define _VGX_DRV_H_

#include "vgx_gpr.h"


namespace vgx {


class drv : public gpr
{
public:

  /**
   * Display hardware interface
   */
  typedef enum enum_interface_type
  {
    interface_mem = 0,    // memory mapped interface
    interface_dio,        // digital IO interface
    interface_spi,        // SPI interface
    interface_i2c,        // I²C interface
    interface_can,        // CAN interface
    interface_usb,        // USB interface
    interface_uart,       // UART interface
    intercace_cust        // custom interface (bit banging etc.)
  } interface_type;

  /**
   * Display orientation, the driver takes care of screen rotation
   * Set the orientation that (0,0) is always left/top
   */
  typedef enum enum_orientation_type
  {
    orientation_0 = 0,    //   0° one to one
    orientation_90,       //  90° clockwise
    orientation_180,      // 180° clockwise
    orientation_270,      // 270° clockwise
    orientation_0m,       //   0° vertical screen mirror
    orientation_90m,      //  90° clockwise, vertical screen mirror
    orientation_180m,     // 180° clockwise, vertical screen mirror
    orientation_270m,     // 270° clockwise, vertical screen mirror
  } orientation_type;

  ///////////////////////////////////////////////////////////////////////////////

  /**
   * ctor
   * \param screen_size_x Screen width
   * \param screen_size_y Screen height
   * \param viewport_size_x Viewport width
   * \param viewport_size_y Viewport height
   * \param viewport_x X offset within the screen, relative to top/left corner
   * \param viewport_y Y offset within the screen, relative to top/left corner
   * \param orientation Orientation of the display
   */
  drv(std::uint16_t screen_size_x,   std::uint16_t screen_size_y,
      std::uint16_t viewport_size_x, std::uint16_t viewport_size_y,
      std::uint16_t viewport_x = 0U, std::uint16_t viewport_y = 0U,
      orientation_type orientation = orientation_0)
    : screen_size_x_(screen_size_x)
    , screen_size_y_(screen_size_y)
    , viewport_size_x_(viewport_size_x)
    , viewport_size_y_(viewport_size_y)
    , viewport_x_(viewport_x)
    , viewport_y_(viewport_y)
    , orientation_(orientation)
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
   * Returns the driver version and name
   * \return Driver version and name
   */
  virtual const char* version() const = 0;

  /**
   * Returns the display capability: graphic or alpha numeric
   * \return True if graphic display
   */
  virtual bool is_graphic() const = 0;

  /**
   * Clear screen, set all pixels off, delete all characters or fill screen with background color
   */
  virtual void cls() = 0;


  /////////////////////////////////////////////////////////////////////////////
  // G R A P H I C   D I S P L A Y S
  //
  // These functions are MANDATORY FOR GRAPHIC drivers!
  // Every graphic display driver MUST implement/override them.
  // In case of alpha numeric display, leave unimplemented

  /**
   * Set pixel in given color, the color doesn't change the actual drawing color
   * \param x X value
   * \param y Y value
   * \param color Color of pixel in ARGB format
   */
  //virtual void drv_pixel_set_color(std::int16_t x, std::int16_t y, color::value_type color);

  /**
   * Get pixel color
   * \param x X value
   * \param y Y value
   * \return Color of pixel in ARGB format
   */
  //virtual color::value_type drv_pixel_get(std::int16_t x, std::int16_t y) const;


  /////////////////////////////////////////////////////////////////////////////
  // A L P H A   N U M E R I C   D I S P L A Y S
  //
  // The functions in this section are MANDATORY FOR ALPHA NUMERIC drivers!
  // Every alpha numeric display driver MUST implement/override them.
  // In case of a graphic display, leave unimplemented

  /**
  * Select the font
  * \param Reference to font to use
  * \return true if font set successfully
  */
  //virtual bool drv_text_set_font(const font_type& font);

  /**
  * Set the new text position
  * \param x X value in pixel on graphic displays, char pos on alpha displays
  * \param y Y value in pixel on graphic displays, char pos on alpha displays
  * \return true if position is set successfully
  */
  //virtual bool drv_text_set_pos(std::int16_t x, std::int16_t y);

  /**
  * Set the text mode
  * \param mode Set normal or inverse video
  * \return true if successful
  */
  //virtual bool drv_text_set_mode(text_mode_type mode);

  /**
  * Output one character
  * \param ch Output character in ASCII/UNICODE (NOT UTF-8) format
  */
  //virtual void drv_text_char(std::uint16_t ch);


  /////////////////////////////////////////////////////////////////////////////
  // B A S E  F U N C T I O N S
  //

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
  inline bool is_screen(std::int16_t x, std::int16_t y) const
  { return x >= 0 && x < screen_size_x_ && y >= 0 && y < screen_size_y_; }

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
   * Set the viewport origin
   * \param x Left corner in screen coordinates
   * \param y Top corner in screen coordinates
   */
  inline virtual void viewport_set(std::uint16_t x = 0U, std::uint16_t y = 0U)
  {
    viewport_x_ = x;
    viewport_y_ = y;
  }

  /**
   * Get the actual viewport origin
   * \param x Left corner in screen coordinates
   * \param y Top corner in screen coordinates
   */
  inline virtual void viewport_get(std::uint16_t& x, std::uint16_t& y) const
  {
    x = viewport_x_;
    y = viewport_y_;
  }


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
  inline virtual std::uint8_t framebuffer_get_count() const
  { return 1U; }


  /**
   * Set display or backlight brightness
   * \param level 0: dark, backlight off; 255: maximum brightness, backlight full on
   */
  virtual void brightness_set(std::uint8_t level)
  { (void)level; }


protected:
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

  inline color::value_type color_from_head_RGB555(std::uint16_t head_color) const
  { return color::rgb(static_cast<std::uint8_t>((head_color & 0x7C00U) >> 7U), static_cast<std::uint8_t>((head_color & 0x03E0U) >> 2U), static_cast<std::uint8_t>((head_color & 0x001FU) << 3U)); }

  inline color::value_type color_from_head_RGB565(std::uint16_t head_color) const
  { return color::rgb(static_cast<std::uint8_t>((head_color & 0xF800U) >> 8U), static_cast<std::uint8_t>((head_color & 0x07E0U) >> 3U), static_cast<std::uint8_t>((head_color & 0x001FU) << 3U)); }

  inline color::value_type color_from_head_RGB666(std::uint32_t head_color) const
  { return color::rgb(static_cast<std::uint8_t>((head_color & 0x0003F000UL) >> 10U), static_cast<std::uint8_t>((head_color & 0x00000FC0UL) >> 4U), static_cast<std::uint8_t>((head_color & 0x0000003FUL) << 2U)); }

  inline color::value_type color_from_head_RGB888(std::uint32_t head_color) const
  { return static_cast<color::value_type>(head_color & 0x00FFFFFFUL); }


  ///////////////////////////////////////////////////////////////////////////////
  // H A R D W A R E   A C C E S S
  //
  // Define all IO functions in your project
  //
  struct io {
    // Direct memory access, write
    template<typename VALUE_TYPE>
    static void mem_set(volatile void* address, VALUE_TYPE value);

    // Direct memory access, masked write
    template<typename VALUE_TYPE>
    static void mem_mask(volatile void* address, VALUE_TYPE value, VALUE_TYPE mask);

    // Direct memory access, read
    template<typename VALUE_TYPE>
    static VALUE_TYPE mem_get(volatile void* address);

    /**
     * IO write/read access to device
     * The bytes read are only valid on interfaces, where writing and reading is simultaneous, like SPI
     * \param if_type Interface type
     * \param device_id Logical device ID of the according interface
     * \param data_out Data transmit buffer
     * \param data_out_length Data length to send
     * \param data_in Data receive buffer
     * \param data_in_length Additional input length
     * \return true if successful
     */ 
    static bool dev_set(interface_type if_type, std::uint16_t device_id,
                        const std::uint8_t* data_out, std::uint16_t data_out_length,
                        std::uint8_t* data_in, std::uint16_t data_in_length);

    /**
     * IO read access from device
     * \param if_type Interface type
     * \param device_id Logical device ID of the according interface
     * \param data_in Data receive buffer
     * \param data_in_length Maximum buffer size on input, received chars on output
     * \return true if successful
     */ 
    static bool dev_get(interface_type if_type, std::uint16_t device_id,
                        std::uint8_t* data_in, std::uint16_t* data_in_length);
  } io_;

protected:
  const std::uint16_t screen_size_x_;     // screen (buffer) width  in pixel (graphic) or chars (alpha)
  const std::uint16_t screen_size_y_;     // screen (buffer) height in pixel (graphic) or chars (alpha)
  const std::uint16_t viewport_size_x_;   // viewport (display) width in pixel (graphic) or chars (alpha)
  const std::uint16_t viewport_size_y_;   // viewport (display) height in pixel (graphic) or chars (alpha)
  std::uint16_t       viewport_x_;        // viewport left corner (x offset to screen)
  std::uint16_t       viewport_y_;        // viewport top corner  (y offset to screen)
  orientation_type    orientation_;       // orientation of the display
};

} // namespace vgx

#endif  // _VGX_DRV_H_
