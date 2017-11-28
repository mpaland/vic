///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2017, PALANDesign Hannover, Germany
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
// \brief Driver base class and head interface functions
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_DRV_H_
#define _VIC_DRV_H_

#include "util.h"
#include "color.h"
#include "io.h"     // hardware dependent IO access


namespace vic {

namespace sprite { class base; }
namespace shader { class output; }


class drv
{
  // friends which can access protected functions
  friend class dc;
  friend class tc;
  friend class shader::output;
  friend class sprite::base;

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
    , orientation_(orientation)
    , viewport_({ viewport_x, viewport_y })
  { }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   D R I V E R   F U N C T I O N S
  //
  // All pure virtual functions in this section are MANDATORY driver functions!
  // Every driver MUST implement them - even if unsupported
public:

  /**
   * Driver init
   */
  virtual void init(void) = 0;


  /**
   * Driver shutdown
   */
  virtual void shutdown(void) = 0;


  /**
   * Returns the driver version and name
   * \return Driver version and name
   */
  virtual const char* version(void) const = 0;


  /**
   * Returns the display capability: graphic or alpha numeric
   * \return True if graphic display type
   */
  virtual bool is_graphic(void) const = 0;


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

protected:

  /**
   * Clear screen, set all pixels off, delete all characters or fill screen with background/blank color
   * \param bg_color Backgound/erase color, defines normally the default color of the display
   */
  virtual void cls(color::value_type bg_color = color::none)
  {
    (void)bg_color;
  }


  /**
   * Primitive rendering is done. May be overridden by driver to update display,
   * frame buffer or something else (like copy RAM / rendering buffer to screen)
   */
  virtual void present()
  { }


  ///////////////////////////////////////////////////////////////////////////////
  // G R A P H I C   F U N C T I O N S
  //

  /**
   * Set pixel in given color
   * \param vertex Pixel coordinates
   * \param color Color of pixel in 0RGB format, alpha channel is not evaluated
   */
  virtual void pixel_set(vertex_type vertex, std::uint32_t color)
  {
    (void)vertex; (void)color;
  }


  /**
   * Return the color of the pixel
   * \param point Vertex of the pixel
   * \return Color of pixel in 0RGB format, alpha channel not set/undefined
   */
  virtual color::value_type pixel_get(vertex_type vertex)
  {
    (void)vertex;
    return color::none;
  }


  /**
   * Draw a horizontal line in the given color, width is one pixel
   * This is a slow fallback implementation which should be overridden by a high speed driver implementation
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, y component is ignored
   * \param color Line color - only RGB, no alpha channel
   */
  virtual void line_horz(vertex_type v0, vertex_type v1, std::uint32_t color)
  {
    util::vertex_min_x(v0, v1);   // set v0 to min x
    for (; v0.x <= v1.x; ++v0.x) {
      pixel_set(v0, color);
    }
  }


  /**
   * Draw a vertical line in the given color, width is one pixel
   * This is a slow fallback implementation which should be overridden by a high speed driver implementation
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, x component is ignored
   * \param color Line color - only RGB, no alpha channel
   */
  virtual void line_vert(vertex_type v0, vertex_type v1, std::uint32_t color)
  {
    util::vertex_min_y(v0, v1);   // set v0 to min y
    for (; v0.y <= v1.y; ++v0.y) {
      pixel_set(v0, color);
    }
  }


  /**
   * Draw a box (filled rectangle) in given color
   * This is a slow fallback implementation which should be overridden by a high speed driver implementation
   * \param rect Box bounding, included in box
   * \param color Box color - only RGB, no alpha channel
   */
  virtual void box(rect_type rect, std::uint32_t color)
  {
    for (std::int16_t y = rect.top; y <= rect.bottom; ++y) {
      line_horz({ rect.left, y }, { rect.right, y }, color);
    }
  }


  /**
   * Move display area
   * This is a slow fallback implementation which should be overridden by a high speed driver implementation
   * \param source Source top/left vertex
   * \param destination Destination top/left vertex
   * \param width Width of the area
   * \param height Height of the area
   */
  virtual void move(vertex_type source, vertex_type destination, std::uint16_t width, std::uint16_t height)
  {
    if (source.x < destination.x) {
      if (source.y < destination.y) {
        for (std::int16_t dy = destination.y + height - 1, sy = source.y + height - 1; dy >= destination.y; --dy, --sy) {
          for (std::int16_t dx = destination.x + width - 1, sx = source.x + width - 1; dx >= destination.x; --dx, --sx) {
            pixel_set({ dx, dy }, pixel_get({ sx, sy }));
          }
        }
      }
      else {
        for (std::int16_t dy = destination.y, sy = source.y; dy < destination.y + height; ++dy, ++sy) {
          for (std::int16_t dx = destination.x + width - 1, sx = source.x + width - 1; dx >= destination.x; --dx, --sx) {
            pixel_set({ dx, dy }, pixel_get({ sx, sy }));
          }
        }
      }
    }
    else {
      if (source.y < destination.y) {
        for (std::int16_t dy = destination.y + height - 1, sy = source.y + height - 1; dy >= destination.y; --dy, --sy) {
          for (std::int16_t dx = destination.x, sx = source.x; dx < destination.x + width; ++dx, ++sx) {
            pixel_set({ dx, dy }, pixel_get({ sx, sy }));
          }
        }
      }
      else {
        for (std::int16_t dy = destination.y, sy = source.y; dy < destination.y + height; ++dy, ++sy) {
          for (std::int16_t dx = destination.x, sx = source.x; dx < destination.x + width; ++dx, ++sx) {
            pixel_set({ dx, dy }, pixel_get({ sx, sy }));
          }
        }
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  // A L P H A   T E X T   F U N C T I O N S
  //

  /**
   * Set the new text position
   * \param pos Position in chars on text displays (0/0 is left/top)
   */
  virtual void text_set_pos(vertex_type pos)
  {
    (void)pos;
  }


  /**
   * Set inverse text mode
   * \param inverse Set normal or inverse video
   */
  virtual void text_set_inverse(bool inverse)
  {
    (void)inverse;
  }


  /**
   * Clear actual line from cursor pos (included) to end of line
   */
  virtual void text_clear_eol()
  { }


  /**
   * Clear actual line from start to cursor pos
   */
  virtual void text_clear_sol()
  { }


  /**
   * Clear the actual line
   */
  virtual void text_clear_line()
  { }


  /**
   * Output a single ASCII/UNICODE char at the actual cursor position
   * The cursor position is moved by the char width (distance)
   * \param ch Output character in 16 bit ASCII/UNICODE (NOT UTF-8) format, 00-7F is compatible with ASCII
   */
  virtual void text_out(std::uint16_t ch)
  {
    (void)ch;
  }


  /**
   * Render an UTF-8 / ASCII coded string at the actual cursor position
   * \param string Output string in UTF-8/ASCII format, zero terminated
   * \return Number of written characters, not bytes (as an UTF-8 character may consist out of more bytes)
   */
  virtual std::uint16_t text_out(const std::uint8_t* string)
  {
    std::uint16_t ch, cnt = 0U;
    while (*string) {
      if ((*string & 0x80U) == 0x00U) {
        // 1 byte sequence (ASCII char)
        ch = (std::uint16_t)(*string++ & 0x7FU);
      }
      else if ((*string & 0xE0U) == 0xC0U) {
        // 2 byte UTF-8 sequence
        ch = (((std::uint16_t)*string & 0x001FU) << 6U) | ((std::uint16_t)*(string + 1U) & 0x003FU);
        string += 2U;
      }
      else if ((*string & 0xF0U) == 0xE0U) {
        // 3 byte UTF-8 sequence
        ch = (((std::uint16_t)*string & 0x000FU) << 12U) | (((std::uint16_t)*(string + 1U) & 0x003FU) << 6U) | ((std::uint16_t)*(string + 2U) & 0x003FU);
        string += 3U;
      }
      else {
        // unknown sequence
        string++;
        continue;
      }

      // handling of special chars
      if ((char)ch == '\n') {
        // LF: X = 0, Y = next line
      }
      else if ((char)ch == '\r') {
        // CR: X = 0
      }
      else {
        text_out(ch);
      }
      cnt++;
    }
    present();
    return cnt;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // S C R E E N   /   V I E W P O R T   F U N C T I O N S
  //

public:

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
   * Returns true if the given vertex is within the screen area
   * \param v Vertex in screen coordinates
   * \return true if the given vertex is within the screen area
   */
  inline bool screen_is_inside(const vertex_type& v) const
  { return v.x < screen_size_x_ && v.y < screen_size_y_ && v.x >= 0 && v.y >= 0; }


  /**
   * Returns the viewport (display) width
   * \return Viewport width in pixel or chars
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
   * \param v Vertex in screen coordinates
   * \return true if the given vertex is within the viewport area
   */
  inline bool viewport_is_inside(vertex_type v) const
  { return v.x >= viewport_.x && v.x < (viewport_.x + viewport_size_x_) &&
           v.y >= viewport_.y && v.y < (viewport_.y + viewport_size_y_); }


  /**
   * Set the new viewport origin
   * \param v New viewport origin in screen coordinates
   */
  inline virtual void viewport_set(vertex_type v)
  {
    viewport_ = v;
    present(); // refresh the screen
  }


  /**
   * Get the actual viewport origin
   * \return Viewport origin in screen coordinates
   */
  inline virtual vertex_type viewport_get() const
  { return viewport_; }


  ///////////////////////////////////////////////////////////////////////////////
  // F R A M E B U F F E R
  //

  /**
   * Set the given framebuffer plane index as active display
   * \param plane The index of the framebuffer/plane to display, 0 for 1st
   * \param alpha Alpha level, 0 = opaque/active, 255 = complete transparent/disabled
   */
  virtual bool framebuffer_set_display(std::size_t plane, std::uint8_t alpha = 0U)
  { (void)plane; (void)alpha; return false; }


 /**
   * Use the given framebuffer plane as read/write buffer
   * \param plane The index of the framebuffer/plane to access, 0 for 1st
   */
  virtual bool framebuffer_set_access(std::size_t plane)
  { (void)plane; return false; }


  /**
   * Returns the number of available framebuffers/planes. 1 if no framebuffer support (so just one buffer)
   * \return Number of frame buffers
   */
  virtual std::size_t framebuffer_get_count() const
  { return 1U; }


  ///////////////////////////////////////////////////////////////////////////////
  // D I S P L A Y   C O N T R O L
  //

  /**
   * Enable/disable the display
   * \param enable True to switch the display on, false to switch it off (standby, powersave)
   */
  virtual void display_enable(bool enable = true)
  { (void)enable; }


  /**
   * Set display or backlight brightness
   * \param level 0: dark, backlight off; 255: maximum brightness, backlight full on
   */
  virtual void display_brightness(std::uint8_t level)
  { (void)level; }


  /**
   * Set display contrast brightness
   * \param level 0: minimum; 255: maximum
   */
  virtual void display_contrast(std::uint8_t level)
  { (void)level; }


protected:
  const std::uint16_t     screen_size_x_;     // screen (buffer) width  in pixel (graphic) or chars (alpha)
  const std::uint16_t     screen_size_y_;     // screen (buffer) height in pixel (graphic) or chars (alpha)
  const std::uint16_t     viewport_size_x_;   // viewport (display) width in pixel (graphic) or chars (alpha)
  const std::uint16_t     viewport_size_y_;   // viewport (display) height in pixel (graphic) or chars (alpha)
  const orientation_type  orientation_;       // hardware orientation/rotation  of the display
  vertex_type             viewport_;          // viewport top/left corner (x offset to screen)
};

} // namespace vic

#endif  // _VIC_DRV_H_
