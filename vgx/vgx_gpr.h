///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2001-2015, PALANDesign Hannover, Germany
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
// \brief Graphic Primitive Renderer to render lines, arcs, boxes etc.
// This is the base class of any driver. If a driver can't provide drawing functions
// on its own, this routines are taken.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_GPR_H_
#define _VGX_GPR_H_

#include <cstdint>
#include <initializer_list>

#include <vgx_cfg.h>      // use < > here, cause vgx_cfg.h may be in some platform folder
#include "vgx_fonts.h"
#include "vgx_color.h"


namespace vgx {


typedef struct struct_vertex_type {
  std::int16_t x;
  std::int16_t y;
} vertex_type;


typedef enum enum_line_style_type {
  vgx_line_style_solid = 0,
  vgx_line_style_dotted,
  vgx_line_style_dashed
} line_style_type;


typedef enum enum_text_mode_type {
  text_mode_normal = 0,
  text_mode_inverse
} text_mode_type;


/**
 * Graphic Primitive Renderer
 */
class gpr
{
public:

  /**
   * ctor
   * Init vars
   */
  gpr()
    : color_(color::white)
    , color_bg_(color::black)
    , primitive_lock_(false)
    , text_font_(nullptr)
    , text_x_set_(0)
    , text_x_act_(0)
    , text_y_act_(0)
    , text_mode_(text_mode_normal)
  #if defined(VGX_CFG_ANTIALIASING)
    , anti_aliasing_(false)
  #endif
  { }

///////////////////////////////////////////////////////////////////////////////
// C O L O R   F U N C T I O N S

  /**
   * Set the drawing color
   * \param color New drawing color in ARGB format
   */
  virtual inline void color_set(color::value_type color)
  { color_ = color; }

  /**
   * Get the actual drawing color
   * \return Actual drawing color in ARGB format
   */
  virtual inline color::value_type color_get() const
  { return color_; }

  /**
   * Set the background color (e.g. for cls)
   * \param color_background New background color in ARGB format
   */
  virtual inline void color_set_bg(color::value_type color_background)
  { color_bg_ = color_background; }

  /**
   * Get the actual background color
   * \return Actual background color in ARGB format
   */
  virtual inline color::value_type color_get_bg() const
  { return color_bg_; }


///////////////////////////////////////////////////////////////////////////////
// G R A P H I C   P R I M I T I V E   F U N C T I O N S

  /**
   * Plot a point
   * \param x X value
   * \param y Y value
   * \return true if successful
   */
  bool plot(std::int16_t x, std::int16_t y)
  {
    drv_pixel_set(x, y);
    drv_primitive_done();
    return true;
  }


  /**
   * Plot a point with a given color, drawing color is not affected
   * \param x X value
   * \param y Y value
   * \param color Color of the pixel
   * \return true if successful
   */
  bool plot(std::int16_t x, std::int16_t y, color::value_type color)
  {
    drv_pixel_set_color(x, y, color);
    drv_primitive_done();
    return true;
  }


  /**
   * Draw a line, width is one pixel
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in line
   * \param y1 Y end value, included in line
   * \return true if successful
   */
  bool line(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
  {
  #if defined(VGX_CFG_ANTIALIASING)
    if (anti_aliasing_) {
      drv_line_aa(x0, y0, x1, y1);
    }
    else {
      drv_line(x0, y0, x1, y1);
    }
  #else
    drv_line(x0, y0, x1, y1);
  #endif
    drv_primitive_done();
    return true;
  }


  /**
   * Draw a horizontal line, width is one pixel
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in line
   * \return true if successful
   */
  bool line_horz(std::int16_t x0, std::int16_t y0, std::int16_t x1)
  {
    drv_line_horz(x0, y0, x1);
    drv_primitive_done();
    return true;
  }


  /**
   * Draw a vertical line, width is one pixel
   * \param x0 X start value
   * \param y0 Y start value
   * \param y1 Y end value, included in line
   * \return true if successful
   */
  bool line_vert(std::int16_t x0, std::int16_t y0, std::int16_t y1)
  {
    drv_line_vert(x0, y0, y1);
    drv_primitive_done();
    return true;
  }


  /**
   * Draw a line with specified width
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in line
   * \param y1 Y end value, included in line
   * \param width Line width in pixel
   * \return true if successful
   */
  bool line_width(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::uint16_t width)
  {
    drv_line_width(x0, y0, x1, y1, width);
    drv_primitive_done();
    return true;
  }


  /**
   * Draw a rectangle (frame)
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in rect
   * \param y1 Y end value, included in rect
   * \return true if successful
   */
  bool rect(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
  {
    drv_rect(x0, y0, x1, y1);
    drv_primitive_done();
    return true;
  }


  /**
   * Draw a box (filled rectangle)
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in box
   * \param y1 Y end value, included in box
   * \return true if successful
   */
  bool box(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
  {
    drv_box(x0, y0, x1, y1);
    drv_primitive_done();
    return true;
  }


  /**
   * Draw a box (filled rectangle) with gradient colors
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in line
   * \param y1 Y end value, included in line
   * \param horizontal True for horizontal gradient
   * \param gr Gradient
   * \return true if successful
   */
  bool box_gradient(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, bool horizontal, const color::gradient_base& gr)
  {
    if (horizontal) {
      // horizontal gradient
      if (x1 < x0) { std::int16_t t = x0; x0 = x1; x1 = t; }
      std::uint16_t seg = x1 - x0;
      for (std::int16_t x = x0; x <= x1; ++x) {
        color_set(gr.mix((std::uint32_t)(x - x0) * 1000UL / seg));
        drv_line_vert(x, y0, y1);
      }
    }
    else {
      // vertical gradient
      if (y1 < y0) { std::int16_t t = y0; y0 = y1; y1 = t; }
      std::uint16_t seg = y1 - y0;
      for (std::int16_t y = y0; y <= y1; ++y) {
        color_set(gr.mix((std::uint32_t)(y - y0) * 1000UL / seg));
        drv_line_horz(x0, y, x1);
      }
    }
    drv_primitive_done();
    return true;
  }


  /**
   * Draw a polygon
   * \param vertexes Pointer to array of polygon vertexes, at least 3
   * \param vertex_count Number of polygon points in the structure, at least 3
   * \return true if successful
   */
  bool polygon(const vertex_type* vertexes, std::uint16_t vertex_count)
  {
    if (!vertex_count || --vertex_count < 2U) {
      return false;
    }
  #if defined(VGX_CFG_ANTIALIASING)
    if (anti_aliasing_) {
      drv_line_aa(vertexes[0].x, vertexes[0].y, vertexes[vertex_count].x, vertexes[vertex_count].y);
      for (; vertex_count > 0U; --vertex_count) {
        drv_line_aa(vertexes[vertex_count].x, vertexes[vertex_count].y, vertexes[vertex_count - 1U].x, vertexes[vertex_count - 1U].y);
      }
    }
    else {
      drv_line(vertexes[0].x, vertexes[0].y, vertexes[vertex_count].x, vertexes[vertex_count].y);
      for (; vertex_count > 0U; --vertex_count) {
        drv_line(vertexes[vertex_count].x, vertexes[vertex_count].y, vertexes[vertex_count - 1U].x, vertexes[vertex_count - 1U].y);
      }
    }
  #else
    drv_line(vertexes[0].x, vertexes[0].y, vertexes[vertex_count].x, vertexes[vertex_count].y);
    for (; vertex_count > 0U; --vertex_count) {
      drv_line(vertexes[vertex_count].x, vertexes[vertex_count].y, vertexes[vertex_count - 1U].x, vertexes[vertex_count - 1U].y);
    }
  #endif
    drv_primitive_done();
    return true;
  }


  /**
   * Draw a triangle
   * \param x0 X value, included in triangle
   * \param y0 Y value, included in triangle
   * \param x1 X value, included in triangle
   * \param y1 Y value, included in triangle
   * \param x2 X value, included in triangle
   * \param y2 Y value, included in triangle
   * \return true if successful
   */
  bool triangle(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2)
  {
  #if defined(VGX_CFG_ANTIALIASING)
    if (anti_aliasing_) {
      drv_triangle_aa(x0, y0, x1, y1, x2, y2);
    }
    else {
      drv_triangle(x0, y0, x1, y1, x2, y2);
    }
  #else
      drv_triangle(x0, y0, x1, y1, x2, y2);
  #endif
    drv_primitive_done();
    return true;
  }


  /**
   * Draw a solid (filled) triangle
   * \param x0 X value, included in triangle
   * \param y0 Y value, included in triangle
   * \param x1 X value, included in triangle
   * \param y1 Y value, included in triangle
   * \param x2 X value, included in triangle
   * \param y2 Y value, included in triangle
   * \return true if successful
   */
  bool triangle_solid(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2)
  {
  #if defined(VGX_CFG_ANTIALIASING)
    if (anti_aliasing_) {
      drv_triangle_solid_aa(x0, y0, x1, y1, x2, y2);
    }
    else {
      drv_triangle_solid(x0, y0, x1, y1, x2, y2);
    }
  #else
      drv_triangle_solid(x0, y0, x1, y1, x2, y2);
  #endif
    drv_primitive_done();
    return true;
  }


  /**
   * Draw an arc (Bézier curve)
   * \param x0 X start value, included in arc
   * \param y0 Y start value, included in arc
   * \param x1 X mid value, included in arc
   * \param y1 Y mid value, included in arc
   * \param x2 X end value, included in arc
   * \param y2 Y end value, included in arc
   * \return true if successful
   */
  bool arc(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2)
  {
    // sign of gradient must not change
    if ((x0 - x1) * (x2 - x1) > 0 ||
        (y0 - y1) * (y2 - y1) > 0) {
      return false;
    }
  #if defined(VGX_CFG_ANTIALIASING)
    if (anti_aliasing_) {
      drv_arc_aa(x0, y0, x1, y1, x2, y2);
    }
    else {
      drv_arc(x0, y0, x1, y1, x2, y2);
    }
  #else
    drv_arc(x0, y0, x1, y1, x2, y2);
  #endif
    drv_primitive_done();
    return true;
  }


  /**
   * Draw a circle
   * \param x X center value
   * \param y Y center value
   * \param radius Circle radius
   * \return true if successful
   */
  bool circle(std::int16_t x, std::int16_t y, std::uint16_t radius)
  {
  #if defined(VGX_CFG_ANTIALIASING)
    if (anti_aliasing_) {
      drv_circle_aa(x, y, radius);
    }
    else {
      drv_circle(x, y, radius);
    }
  #else
    drv_circle(x, y, radius);
  #endif
    drv_primitive_done();
    return true;
  }


  /**
   * Draw a disc (filled circle)
   * \param x X center value
   * \param y Y center value
   * \param radius Disc radius
   * \return true if successful
   */
  bool disc(std::int16_t x, std::int16_t y, std::uint16_t radius)
  {
  #if defined(VGX_CFG_ANTIALIASING)
    if (anti_aliasing_) {
      drv_disc_aa(x, y, radius);
    }
    else {
      drv_disc(x, y, radius);
    }
  #else
    drv_disc(x, y, radius);
  #endif
    drv_primitive_done();
    return true;
  }


  /**
   * Draw a disc section (filled quarter circle)
   * \param x X center value
   * \param y Y center value
   * \param radius Disc section radius
   * \param section Section number: 0: top/right, 1: top/left, 2: bottom/left, 3: bottom/right
   * \return true if successful
   */
  bool disc_section(std::int16_t x, std::int16_t y, std::uint16_t radius, std::uint8_t section)
  {
    drv_disc_section(x, y, radius, section);
    drv_primitive_done();
    return true;
  }


  /**
   * Draw a sector (filled circle piece)
   * \param x X center value
   * \param y Y center value
   * \param inner_radius Inner sector radius
   * \param outer_radius Outer sector radius
   * \param start_angle Start angle in degree, 0° is horizontal right, counting anticlockwise
   * \param end_angle End angle in degree
   * \return true if successful
   */
  bool sector(std::int16_t x, std::int16_t y, std::uint16_t inner_radius, std::uint16_t outer_radius, std::uint16_t start_angle, std::uint16_t end_angle)
  {
    drv_sector(x, y, inner_radius, outer_radius, start_angle, end_angle);
    drv_primitive_done();
    return true;
  }

  ///////////////////////////////////////////////////////////////////////////////

  /**
   *  Fill region up to the bounding color with the drawing color
   * \param x X start value inside region to fill
   * \param y Y start value inside region to fill
   * \param bounding_color Color of the surrounding bound
   * \return true if successful
   */
  bool fill(std::int16_t x, std::int16_t y, color::value_type bounding_color)
  {
    drv_fill(x, y, bounding_color);
    drv_primitive_done();
    return true;
  }

  ///////////////////////////////////////////////////////////////////////////////

  /**
   * Block bit transfer
   * Transfer image to display
   * \param x X destination value
   * \param y Y destination value
   * \param image_width Width of the image
   * \param image_height Height of the image
   * \param image_format Color format of the image
   * \param image_data Image data
   * \return true if successful
   */
  bool blitter(std::int16_t x, std::int16_t y, std::uint16_t image_width, std::uint16_t image_height, color::format_type image_format, std::uint8_t* image_data)
  {
    // TBD: blittering
    (void)x; (void)y; (void)image_width; (void)image_height; (void)image_format; (void)image_data;
    return false;
  }


  /**
   * Move display area
   * \param x0 X source value
   * \param y0 Y source value
   * \param x1 X destination value
   * \param y1 Y destination value
   * \param width Width of the area
   * \param height Height of the area
   * \return true if successful
   */
  bool move(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::uint16_t width, std::uint16_t height)
  {
    drv_move(x0, y0, x1, y1, width, height);
    drv_primitive_done();
    return true;
  }


  /**
   * Enable/disable anti aliasing support
   * \param enable True to enable anti aliasing
   */
  bool anti_aliasing_set(bool enable)
  {
  #if defined(VGX_CFG_ANTIALIASING)
    anti_aliasing_ = enable;
  #endif
    return true;
  }


#if defined(VGX_CFG_CLIPPING)
  /**
   * Set clipping region
   * All further points within the region are not drawn
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in region
   * \param y1 Y end value, included in region
   */
  bool clipping_set(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
  {
    // TBD
    clippling_x0_ = x0;
    clippling_y0_ = y0;
    clippling_x1_ = x1;
    clippling_y1_ = y1;
    return false;
  }


  /**
   * Disable clipping
   */
  bool clipping_clear()
  {
    // TBD
    return false;
  }

#endif


///////////////////////////////////////////////////////////////////////////////
// T E X T   F U C T I O N S 
//

  /**
   * Select the font
   * \param Reference to font to use
   * \return true if successful
   */
  bool text_set_font(const font_type& font)
  {
    text_font_ = &font;
    return drv_text_set_font(font);
  }


  /**
   * Set the new text position
   * \param x X value in pixel on graphic displays, char pos on alpha displays
   * \param y Y value in pixel on graphic displays, char pos on alpha displays
   * \return true if successful
   */
  bool text_set_pos(std::int16_t x, std::int16_t y)
  {
    text_x_act_ = text_x_set_ = x;
    text_y_act_ = y;
    return drv_text_set_pos(x, y);
  }


  /**
   * Set the text mode
   * \param mode Set normal or inverse video
   * \return true if successful
   */
  bool text_set_mode(text_mode_type mode)
  {
    text_mode_ = mode;
    drv_text_set_mode(mode);
    return true;
  }


  /**
   * Output a single ASCII/UNICODE char at the actual cursor position
   * \param ch Output character in ASCII/UNICODE (NOT UTF-8) format, 20-7F is compatible with ASCII
   */
  void text_char(std::uint16_t ch)
  {
    drv_text_char(ch);
    drv_primitive_done();
  }


  /**
   * Render an ASCII/UTF-8 coded string
   * \param string Output string in ASCII/UTF-8 format, zero terminated
   * \return Number of written characters, not bytes (as an UTF-8 character may consist out of more bytes)
   */
  std::uint16_t text_string(const std::uint8_t* string)
  {
    std::uint16_t cnt = drv_text_string(string);
    drv_primitive_done();
    return cnt;
  }


  /**
   * Output a string at x/y position (combines text_set_pos and text_string)
   * \param x X value in pixel on graphic displays, char pos on alpha displays
   * \param y Y value in pixel on graphic displays, char pos on alpha displays
   * \param string Output string in ASCII/UTF-8 format, zero terminated
   * \return Number of written characters, not bytes (as a character may consist out of two bytes)
   */
  std::uint16_t text_string_pos(std::int16_t x, std::int16_t y, const std::uint8_t* string)
  {
    text_set_pos(x, y);
    std::uint16_t cnt = drv_text_string(string);
    drv_primitive_done();
    return cnt;
  }


  /**
   * Output a rotated string
   * \param x X value in pixel on graphic displays
   * \param y Y value in pixel on graphic displays
   * \param angle Angle in degree, 0° is horizontal right, counting anticlockwise
   * \param string Output string in ASCII/UTF-8 format, zero terminated
   * \return Number of written characters, not bytes (as a character may consist out of two bytes)
   */
  std::uint16_t text_string_rotate(std::int16_t x, std::int16_t y, std::uint16_t angle, const std::uint8_t* string)
  {
    text_set_pos(x, y);
    std::uint16_t cnt = drv_text_string_rotate(angle, string);
    drv_primitive_done();
    return cnt;
  }


  /**
   * Returns the width and height the rendered string would take.
   * The string is not rendered on screen
   * \param width Width the rendered string would take
   * \param height Height the rendered string would take
   * \param string Output string in ASCII/UTF-8 format, zero terminated
   * \return Number of written characters, not bytes (as a character may consist out of two bytes)
   */
  std::uint16_t text_string_get_extend(std::uint16_t& width, std::uint16_t height, const std::uint8_t* string)
  {
    // TBD
    (void)width; (void)height; (void)string;
    return 0U;
  }

///////////////////////////////////////////////////////////////////////////////

  void primitive_lock(bool lock = true)
  {
    primitive_lock_ = lock;
    if (!lock) {
      // primitive is done when lock is released
      drv_primitive_done();
    }
  }

///////////////////////////////////////////////////////////////////////////////
// D R I V E R   F U N C T I O N S
//
protected:

  /**
  * Set pixel (in actual drawing color)
  * \param x X value
  * \param y Y value
  */
  virtual void drv_pixel_set(std::int16_t x, std::int16_t y) = 0;

  /**
  * Set pixel in given color, the color doesn't change the actual drawing color
  * \param x X value
  * \param y Y value
  * \param color Color of pixel in ARGB format
  */
  virtual void drv_pixel_set_color(std::int16_t x, std::int16_t y, color::value_type color) = 0;

  /**
  * Return the color of the pixel
  * \param x X value
  * \param y Y value
  * \return Color of pixel in ARGB format
  */
  virtual color::value_type drv_pixel_get(std::int16_t x, std::int16_t y) const = 0;


///////////////////////////////////////////////////////////////////////////////
// F U N C T I O N S   T H E   D R I V E R   C A N   O V E R R I D E
//
// ... if the driver has native (hardware/firmware accelerated) support for it

  /**
   * Primitive rendering is done. May be overridden by driver to update display,
   * frame buffer or something else (like copy RAM / rendering buffer to screen)
   */
  virtual void drv_primitive_done()
  { }


  /**
   * Line drawing algorithm, width is one pixel
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in line
   * \param y1 Y end value, included in line
   */
  virtual void drv_line(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
  {
    std::int16_t dx, dy;
    std::int16_t sx, sy;
    std::int16_t er, er2;

    // check for straight lines
    if (x0 == x1 && y0 < y1) {
      for (; y0 <= y1; ++y0) {
        drv_pixel_set(x0, y0);
      }
      return;
    }
    if (x0 == x1 && y0 > y1) {
      for (; y0 >= y1; --y0) {
        drv_pixel_set(x0, y0);
      }
      return;
    }
    if (y0 == y1 && x0 < x1) {
      for (; x0 <= x1; ++x0) {
        drv_pixel_set(x0, y0);
      }
      return;
    }
    if (y0 == y1 && x0 > x1) {
      for (; x0 >= x1; --x0) {
        drv_pixel_set(x0, y0);
      }
      return;
    }

    // start Bresenham line algorithm
    dx = x1 > x0 ? x1 - x0 : x0 - x1;
    sx = x1 > x0 ? 1 : -1;
    dy = y1 > y0 ? y1 - y0 : y0 - y1;
    sy = y1 > y0 ? 1 : -1;
    er = dx - dy;

    for(;;) {
      drv_pixel_set(x0, y0);
      if (x0 == x1 && y0 == y1) {
        return;
      }
      er2 = er * 2;
      if (er2 + dy > 0) {
        er -= dy;
        x0 += sx;
      }
      if (er2 < dx) {
        er += dx;
        y0 += sy;
      }
    }
  }


  /**
   * Horizontal line drawing algorithm, width is one pixel
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in line
   */
  virtual void drv_line_horz(std::int16_t x0, std::int16_t y0, std::int16_t x1)
  {
    if (x0 < x1) {
      for (; x0 <= x1; ++x0) {
        drv_pixel_set(x0, y0);
      }
    }
    else {
      for (; x1 <= x0; ++x1) {
        drv_pixel_set(x1, y0);
      }
    }
  }


  /**
   * Vertical line drawing algorithm, width is one pixel
   * \param x0 X start value
   * \param y0 Y start value
   * \param y1 Y end value, included in line
   */
  virtual void drv_line_vert(std::int16_t x0, std::int16_t y0, std::int16_t y1)
  {
    if (y0 < y1) {
      for (; y0 <= y1; ++y0) {
        drv_pixel_set(x0, y0);
      }
    }
    else {
      for (; y1 <= y0; ++y1) {
        drv_pixel_set(x0, y1);
      }
    }
  }



  /**
   * Line drawing algorithm with specified line width
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in line
   * \param y1 Y end value, included in line
   * \param width Line width in pixel
   */
  virtual void drv_line_width(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::uint16_t width)
  {
    std::int16_t dx, dy;
    std::int16_t sx, sy;
    std::int16_t er, er2;

  // TBD: calculate wd without using float
    std::int16_t wd, w;
    wd = (int16_t)width;

    // start Bresenham line algorithm
    dx = x1 > x0 ? x1 - x0 : x0 - x1;
    dy = y1 > y0 ? y1 - y0 : y0 - y1;
    sx = x1 > x0 ? 1 : -1;
    sy = y1 > y0 ? 1 : -1;
    er = dx - dy;

    for(;;) {
      if (dx > dy) {
        for (w = y0 - (wd >> 1U); w < y0 + wd - (wd >> 1U); w++)
          drv_pixel_set(x0, w);
      }
      else {
        for (w = x0 - (wd >> 1U); w < x0 + wd - (wd >> 1U); w++)
          drv_pixel_set(w, y0);
      }
      if (x0 == x1 && y0 == y1) {
        return;
      }
      er2 = er * 2;
      if (er2 + dy > 0) {
        er -= dy;
        x0 += sx;
      }
      if (er2 < dx) {
        er += dx;
        y0 += sy;
      }
    }
  }


  /**
   * Rectangle (frame) drawing algorithm
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in rect
   * \param y1 Y end value, included in rect
   */
  virtual void drv_rect(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
  {
    drv_line_horz(x0, y0, x1);
    drv_line_horz(x0, y1, x1);
    drv_line_vert(x0, y0, y1);
    drv_line_vert(x1, y0, y1);
  }


  /**
   * Box (filled rectangle) drawing algorithm
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in box
   * \param y1 Y end value, included in box
   */
  virtual void drv_box(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
  {
    std::int16_t tmp;
    if (x0 > x1) { tmp = x0; x0 = x1; x1 = tmp; }
    if (y0 > y1) { tmp = y0; y0 = y1; y1 = tmp; }

    for (tmp = x0; y0 <= y1; ++y0) {
      for (x0 = tmp; x0 <= x1; ++x0) {
        drv_pixel_set(x0, y0);
      }
    }
  }


  /**
   * Triangle drawing algorithm
   * \param x0 X value, included in triangle
   * \param y0 Y value, included in triangle
   * \param x1 X value, included in triangle
   * \param y1 Y value, included in triangle
   * \param x2 X value, included in triangle
   * \param y2 Y value, included in triangle
   */
  virtual void drv_triangle(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2)
  {
    drv_line(x0, y0, x1, y1);
    drv_line(x0, y0, x2, y2);
    drv_line(x1, y1, x2, y2);
  }


  /**
   * Solid (filled) triangle drawing algorithm
   * \param x0 X value, included in triangle
   * \param y0 Y value, included in triangle
   * \param x1 X value, included in triangle
   * \param y1 Y value, included in triangle
   * \param x2 X value, included in triangle
   * \param y2 Y value, included in triangle
   */
  virtual void drv_triangle_solid(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2)
  {
    std::int16_t t, sy;
    std::int32_t dx1, dx2, dx3, sx, ex;

    // check if triangle is a horizontal line
    if ((y0 == y1) && (y1 == y2)) {
      // sort x values
      if (x2 < x1) {
        t = x1; x1 = x2; x2 = t;
      }
      if (x1 < x0) {
        t = x0; x0 = x1; x1 = t;
        // now we have to make sure that x1 is smaller as x2
        if (x2 < x1) {
          t = x1; x1 = x2; x2 = t;
        }
      }
      drv_line_horz(x0, y0, x2);
      return;
    }

    // sort vertexes that y0 < y1 < y2
    if (y2 < y1) {
      t = x1; x1 = x2; x2 = t; t = y1; y1 = y2; y2 = t;
    }
    if (y1 < y0) {
      t = x0; x0 = x1; x1 = t; t = y0; y0 = y1; y1 = t;
      if (y2 < y1) {
        t = x1; x1 = x2; x2 = t; t = y1; y1 = y2; y2 = t;
      }
    }

    dx1 = (y1 != y0) ? ((std::int32_t)(x1 - x0) << 16U) / (y1 - y0) : 0;
    dx2 = (y2 != y0) ? ((std::int32_t)(x2 - x0) << 16U) / (y2 - y0) : 0;
    dx3 = (y2 != y1) ? ((std::int32_t)(x2 - x1) << 16U) / (y2 - y1) : 0;

    sx = ex = (std::int32_t)x0 << 16U;
    sy = y0;

    if (dx1 > dx2) {
      for (; sy < y1; sy++, sx += dx2, ex += dx1) {
        drv_line_horz((std::int16_t)(sx >> 16U), sy, (std::int16_t)(ex >> 16U) + ((std::int16_t)(ex >> 15U) & 1));
      }
      ex = (std::int32_t)x1 << 16U;
      for (; sy <= y2; sy++, sx += dx2, ex += dx3) {
        drv_line_horz((std::int16_t)(sx >> 16U), sy, (std::int16_t)(ex >> 16U) + ((std::int16_t)(ex >> 15U) & 1));
      }
    }
    else {
      for (; sy < y1; sy++, sx += dx1, ex += dx2) {
        drv_line_horz((std::int16_t)(sx >> 16U), sy, (std::int16_t)(ex >> 16U)+ ((std::int16_t)(ex >> 15U) & 1));
      } 
      sx = (std::int32_t)x1 << 16U;
      sy = y1;
      for (; sy <= y2; sy++, sx += dx3, ex += dx2) {
        drv_line_horz((std::int16_t)(sx >> 16U), sy, (std::int16_t)(ex >> 16U)+ ((std::int16_t)(ex >> 15U) & 1));
      }
    }
  }


  /**
   * Arc (Bézier curve) drawing algorithm
   * \param x0 X start value, included in arc
   * \param y0 Y start value, included in arc
   * \param x1 X mid value, included in arc
   * \param y1 Y mid value, included in arc
   * \param x2 X end value, included in arc
   * \param y2 Y end value, included in arc
   */
  virtual void drv_arc(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2)
  {
    std::int16_t sx = x2-x1, sy = y2-y1;
    std::int32_t xx = x0-x1, yy = y0-y1, xy;            // relative values for checks
    std::int32_t dx, dy, err, cur = xx * sy - yy * sx;  // curvature

    // sign of gradient must not change
    if (xx * sx > 0 || yy * sy > 0) {
      return;
    }

    if (sx * (std::int32_t)sx + sy * (std::int32_t)sy > xx * xx + yy * yy) {  // begin with longer part
      x2 = x0; x0 = sx + x1; y2 = y0; y0 = sy + y1; cur = -cur;     // swap
    }  
    if (cur != 0) {                                   // no straight line
      xx += sx; xx *= sx = x0 < x2 ? 1 : -1;          // x step direction
      yy += sy; yy *= sy = y0 < y2 ? 1 : -1;          // y step direction
      xy  = 2 * xx * yy;
      xx *= xx;
      yy *= yy;                                       // differences 2nd degree
      if (cur * sx * sy < 0) {                        // negated curvature?
        xx = -xx; yy = -yy; xy = -xy; cur = -cur;
      }
      dx = 4 * sy * cur * (x1 - x0) + xx - xy;        // differences 1st degree
      dy = 4 * sx * cur * (y0 - y1) + yy - xy;
      xx += xx;
      yy += yy;
      err = dx + dy + xy;                             // error 1st step
      do {
        drv_pixel_set(x0, y0);
        if (x0 == x2 && y0 == y2) {
          return;                                     // curve finished
        }
        y1 = 2 * err < dx;                            // save value for test of y step
        if (2 * err > dy) {
          x0 += sx; dx -= xy; err += dy += yy;        // x step
        }
        if (y1) {
          y0 += sy; dy -= xy; err += dx += xx;        // y step
        }
      } while (dy < dx );                             // gradient negates -> algorithm fails
    }
    drv_line(x0, y0, x2, y2);                         // plot remaining part to end
  }


  /**
   * Circle drawing algorithm
   * \param x X center value
   * \param y Y center value
   * \param radius Circle radius
   */
  virtual void drv_circle(std::int16_t x, std::int16_t y, std::uint16_t r)
  {
    std::int16_t xo = (std::int16_t)r, yo = 0, err = 1 - xo;

    while (xo >= yo) {
      drv_pixel_set(x + xo, y + yo);  // q4
      drv_pixel_set(x + xo, y - yo);  // q1
      drv_pixel_set(x + yo, y + xo);  // q4
      drv_pixel_set(x + yo, y - xo);  // q1
      drv_pixel_set(x - xo, y + yo);  // q3
      drv_pixel_set(x - xo, y - yo);  // q2
      drv_pixel_set(x - yo, y + xo);  // q3
      drv_pixel_set(x - yo, y - xo);  // q2
      yo++;
      if (err < 0) {
        err += 2 * yo + 1;
      }
      else {
        xo--;
        err += 2 * (yo - xo + 1);
      }
    }
  }


  /**
   * Disc (filled circle) drawing algorithm
   * \param x X center value
   * \param y Y center value
   * \param radius Disc radius
   */
  virtual void drv_disc(std::int16_t x, std::int16_t y, std::uint16_t r)
  {
    std::int16_t xo = (int16_t)r, yo = 0, err = 1 - xo;

    while (xo >= yo) {
      drv_line_horz(x + xo, y + yo, x - xo);
      drv_line_horz(x + yo, y + xo, x - yo);
      drv_line_horz(x - xo, y - yo, x + xo);
      drv_line_horz(x - yo, y - xo, x + yo);
      yo++;
      if (err < 0) {
        err += 2 * yo + 1;
      } else {
        xo--;
        err += 2 * (yo - xo + 1);
      }
    }
  }


  /**
   * Disc section (filled quarter circle) drawing algorithm
   * \param x X center value
   * \param y Y center value
   * \param radius Disc section radius
   * \param section Section number: 0: top/right, 1: top/left, 2: bottom/left, 3: bottom/right
   */
  virtual void drv_disc_section(std::int16_t x, std::int16_t y, std::uint16_t r, std::uint8_t section)
  {
    std::int16_t xo = (std::int16_t)r, yo = 0, err = 1 - xo;

    while (xo >= yo) {
      switch (section) {
        case 1U :
          drv_line_horz(x, y - yo, x + xo);     // q1
          drv_line_horz(x, y - xo, x + yo);     // q1
          break;
        case 2U :
          drv_line_horz(x, y - yo, x - xo);     // q2
          drv_line_horz(x, y - xo, x - yo);     // q2
          break;
        case 3U :
          drv_line_horz(x, y + yo, x - xo);     // q3
          drv_line_horz(x, y + xo, x - yo);     // q3
          break;
        case 4U :
          drv_line_horz(x, y + yo, x + xo);     // q4
          drv_line_horz(x, y + xo, x + yo);     // q4
          break;
        default :
          break;
      }
      yo++;
      if (err < 0) {
        err += 2 * yo + 1;
      } else {
        xo--;
        err += 2 * (yo - xo + 1);
      }
    }
  }


  /**
   * Sector (filled circle piece) drawing algorithm
   * \param x X center value
   * \param y Y center value
   * \param inner_radius Inner sector radius
   * \param outer_radius Outer sector radius
   * \param start_angle Start angle in degree, 0° is horizontal right, counting anticlockwise
   * \param end_angle end angle in degree
   */
  virtual void drv_sector(std::int16_t x, std::int16_t y, std::uint16_t inner_radius, std::uint16_t outer_radius, std::uint16_t start_angle, std::uint16_t end_angle)
  {
    // angle:
    // 0° = 3 o'clock
    //   0° -  89°: Q1 (top/right)
    //  90° - 179°: Q2 (top/left)
    // 180° - 269°: Q3 (bottom/left)
    // 270° - 359°: Q4 (bottom/right)

    bool second_half;
    std::uint16_t end_angle2 = end_angle;
    if ((end_angle > start_angle && end_angle > start_angle + 180U) || 
        (start_angle > end_angle && end_angle + 180U > start_angle)) {
      // more than 180°
      end_angle  = (start_angle + 180U) % 360U;
    }

    do {
      bool q14s = (start_angle < 90U) || (start_angle >= 270U);
      bool q14e = (end_angle   < 90U) || (end_angle   >= 270U);
      bool q24s = (start_angle >= 90U && start_angle < 180U) || (start_angle >= 270U);
      bool q24e = (end_angle   >= 90U && end_angle   < 180U) || (end_angle   >= 270U);

      std::int16_t xss = (std::uint8_t)(sin((q24s ? start_angle - 90U : start_angle) % 90U) >> (q24s ? 8U : 0U)) * (std::int16_t)(q14s ? 1 : -1);
      std::int16_t yss = (std::uint8_t)(sin((q24s ? start_angle - 90U : start_angle) % 90U) >> (q24s ? 0U : 8U)) * (std::int16_t)((start_angle < 180U) ? 1 : -1);
      std::int16_t xse = (std::uint8_t)(sin((q24e ? end_angle   - 90U : end_angle)   % 90U) >> (q24e ? 8U : 0U)) * (std::int16_t)(q14e ? 1 : -1);
      std::int16_t yse = (std::uint8_t)(sin((q24e ? end_angle   - 90U : end_angle)   % 90U) >> (q24e ? 0U : 8U)) * (std::int16_t)((end_angle   < 180U) ? 1 : -1);

      for (std::int16_t yp = y - outer_radius; yp <= y + outer_radius; yp++) {
        for (std::int16_t xp = x - outer_radius; xp <= x + outer_radius; xp++) {
          // check if xp/yp is within the sector
          std::int16_t xr = xp - x;
          std::int16_t yr = y - yp;   // * -1 for coords to screen conversion
          if ( ((xr * xr + yr * yr) >= inner_radius * inner_radius) &&
               ((xr * xr + yr * yr) <  outer_radius * outer_radius) &&
              !((yss * xr) >  (xss * yr)) &&
               ((yse * xr) >= (xse * yr))
             ) {
            drv_pixel_set(xp, yp);
          }
        }
      }
      // second half necessary?
      second_half = false;
      if (end_angle != end_angle2) {
        start_angle = end_angle;
        end_angle   = end_angle2;
        second_half = true;
      }
    } while (second_half);
  }


///////////////////////////////////////////////////////////////////////////////
// A N T I   A L I A S I N G   S U P P O R T
///////////////////////////////////////////////////////////////////////////////
#if defined(VGX_CFG_ANTIALIASING)
  /**
   * Anti aliased line drawing algorithm, width is one pixel
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in line
   * \param y1 Y end value, included in line
   */
  virtual void drv_line_aa(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
  {
    // using Xiaolin Wu's anti aliasing line algorithm
    // adaption of code from Michael Abrash in Dr. Dobbs journal, 1992 June 01

    std::uint16_t ErrorAdj, ErrorAcc, ErrorAccTemp;
    std::int16_t  DeltaX, DeltaY, XDir;

    // make sure the line runs top to bottom
    if (y0 > y1) {
      std::int16_t temp;
      temp = y0; y0 = y1; y1 = temp;
      temp = x0; x0 = x1; x1 = temp;
    }

    // draw the initial pixel, which is always exactly intersected by the line and so needs no weighting
    drv_pixel_set(x0, y0);

    if ((DeltaX = x1 - x0) >= 0) {
      XDir = 1;
    } else {
      XDir = -1;
      DeltaX = -DeltaX; // make DeltaX positive
    }
    // special-case horizontal, vertical, and diagonal lines, which require no weighting
    // because they go right through the center of every pixel
    if ((DeltaY = y1 - y0) == 0) {
      // horizontal line
      drv_line_horz(x0, y0, x1);
      return;
    }
    if (DeltaX == 0) {
      // vertical line
      drv_line_vert(x0, y0, y1);
      return;
    }
    if (DeltaX == DeltaY) {
      // diagonal line
      do {
        x0 += XDir;
        y0++;
        drv_pixel_set(x0, y0);
      } while (--DeltaY != 0);
      return;
    }

    // line is not horizontal, diagonal, or vertical
    ErrorAcc = 0U;    // initialize the line error accumulator to 0

    // Is this an X-major or Y-major line?
    if (DeltaY > DeltaX) {
      // Y-major line; calculate 16-bit fixed-point fractional part of a
      // pixel that X advances each time Y advances 1 pixel, truncating the
      // result so that we won't overrun the endpoint along the X axis
      ErrorAdj = ((std::int32_t)DeltaX << 16U) / (std::int32_t)DeltaY;
      // draw all pixels other than the first and last
      while (--DeltaY) {
        ErrorAccTemp = ErrorAcc;    // remember current accumulated error
        ErrorAcc += ErrorAdj;       // calculate error for next pixel
        if (ErrorAcc <= ErrorAccTemp) {
          // The error accumulator turned over, so advance the X coord
          x0 += XDir;
        }
        y0++; // Y-major, so always advance Y
        // the IntensityBits most significant bits of ErrorAcc give us the intensity weighting
        // for this pixel, and the complement of the weighting for the paired pixel
        // # of bits by which to shift ErrorAcc to get intensity level
        #define INTENSITY_SHIFT (16U - 8U)
        std::uint8_t lum = (std::uint8_t)(ErrorAcc >> INTENSITY_SHIFT);
        drv_pixel_set_color(x0, y0,        color::mix(color_, drv_pixel_get(x0, y0), lum ^ 0xFFU));
        drv_pixel_set_color(x0 + XDir, y0, color::mix(color_, drv_pixel_get(x0 + XDir, y0), lum));
      }
      // draw the final pixel, which is always exactly intersected by the line and so needs no weighting
      drv_pixel_set(x1, y1);
      return;
    }

    // It's an X-major line; calculate 16-bit fixed-point fractional part of a
    // pixel that Y advances each time X advances 1 pixel, truncating the
    // result to avoid overrunning the endpoint along the X axis
    ErrorAdj = ((std::uint32_t)DeltaY << 16U) / (std::uint32_t)DeltaX;
    // draw all pixels other than the first and last
    while (--DeltaX) {
      ErrorAccTemp = ErrorAcc;    // remember current accumulated error
      ErrorAcc += ErrorAdj;       // calculate error for next pixel
      if (ErrorAcc <= ErrorAccTemp) {
        // the error accumulator turned over, so advance the Y coord
        y0++;
      }
      x0 += XDir;   // X-major, so always advance X
      // the IntensityBits most significant bits of ErrorAcc give us the intensity weighting
      // for this pixel, and the complement of the weighting for the paired pixel
      std::uint8_t lum = (std::uint8_t)(ErrorAcc >> INTENSITY_SHIFT);
      drv_pixel_set_color(x0, y0,     color::mix(color_, drv_pixel_get(x0, y0), lum ^ 0xFFU));
      drv_pixel_set_color(x0, y0 + 1, color::mix(color_, drv_pixel_get(x0, y0 + 1), lum));
    }
    // draw the final pixel, which is always exactly intersected by the line and so needs no weighting
    drv_pixel_set(x1, y1);
  }


  /**
   * Anti aliased arc (Bézier curve) drawing algorithm
   * \param x0 X start value, included in arc
   * \param y0 Y start value, included in arc
   * \param x1 X mid value, included in arc
   * \param y1 Y mid value, included in arc
   * \param x2 X end value, included in arc
   * \param y2 Y end value, included in arc
   */
  virtual void drv_arc_aa(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2)
  {
    // TBD
    (void)x0; (void)y0; (void)x1; (void)y1; (void)x2; (void)y2;
    // Linear-constrain circle anti-aliased algorithm, copyright by Konstantin Kirillov, license: MIT
  #if 0
    int I=255;  // intensity range. Determines algorithms' accuracy.

      //No precalculation is required.
      //Calculation of the 1/8 of circle will take
      // R/sq(2) steps = R/sq(2)*(3mult+1div+8add) ~ 1.4R(3mult+1div)
      // All operations are of integer type.
        
        
      //Input: uses precalculated variables D and I.
      public void drawArch( int R ) {
         int R2=R*R;
         int y=0;
         int x=R;

         int B=x*x;
         int xTop=x+1;
         int T=xTop*xTop;

         while( y<x ) {
             int E=R2-y*y;
             int L=E-B;
             int U=T-E;
             if(L<0){ //We had Wu's lemma before: if( dnew < d ) x--;
               xTop=x;
               x--;
               T=B;
               U=-L;
               B=x*x;
               L=E-B;
             }
             int u=I*U/(U+L);
           
             //good for debug:
             con("x="+x+" y="+y+" E="+E+" B="+B+" T="+T+" L="+L+" U="+U+" u="+u);
           
             //These two statements are not a part of the algorithm:
             //Each language, OS, or framework has own ways to put a "pixel".
             putpixel(x,      y,     u,  doDemo);
             putpixel(xTop,   y, (I-u), !doDemo);
           
             y++;
         }    
      }
      //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
      // Calculation of an arc
      //----------------------------------------------------------------

      //----------------------------------------------------------------
      //End of Algorithm.
      //==========================================================================
  #endif
  }


  /**
   * Anti aliased triangle drawing algorithm
   * \param x0 X value, included in triangle
   * \param y0 Y value, included in triangle
   * \param x1 X value, included in triangle
   * \param y1 Y value, included in triangle
   * \param x2 X value, included in triangle
   * \param y2 Y value, included in triangle
   */
  virtual void drv_triangle_aa(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2)
  {
    drv_line_aa(x0, y0, x1, y1);
    drv_line_aa(x0, y0, x2, y2);
    drv_line_aa(x1, y1, x2, y2);
  }


  /**
   * Solid (filled) anti aliased triangle drawing algorithm
   * \param x0 X value, included in triangle
   * \param y0 Y value, included in triangle
   * \param x1 X value, included in triangle
   * \param y1 Y value, included in triangle
   * \param x2 X value, included in triangle
   * \param y2 Y value, included in triangle
   */
  virtual void drv_triangle_solid_aa(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2)
  {
    drv_triangle_solid(x0, y0, x1, y1, x2, y2);
    drv_line_aa(x0, y0, x1, y1);
    drv_line_aa(x0, y0, x2, y2);
    drv_line_aa(x1, y1, x2, y2);
  }


  /**
   * Anti aliased circle drawing algorithm
   * \param x X center value
   * \param y Y center value
   * \param radius Circle radius
   */
  virtual void drv_circle_aa(std::int16_t x, std::int16_t y, std::uint16_t r)
  {
    // TBD: render an AA circle
    (void)x; (void)y; (void)r;
  }


  /**
   * Anti aliased disc (filled circle) drawing algorithm
   * \param x X center value
   * \param y Y center value
   * \param radius Disc radius
   */
  virtual void drv_disc_aa(std::int16_t x, std::int16_t y, std::uint16_t r)
  {
    // TBD: render an AA disc
    (void)x; (void)y; (void)r;
  }


  /**
   * Anti aliased disc section (filled quarter circle) drawing algorithm
   * \param x X center value
   * \param y Y center value
   * \param radius Disc section radius
   * \param section Section number: 0: top/right, 1: top/left, 2: bottom/left, 3: bottom/right
   */
  virtual void drv_disc_section_aa(std::int16_t x, std::int16_t y, std::uint16_t r, std::uint8_t section)
  {
    // TBD: render an AA section
    (void)x; (void)y; (void)r; (void)section;
  }

#endif  // VGX_CFG_ANTIALIASING


  /**
   *  Fill region up to the bounding border_color with the drawing color algorithm
   * \param x X start value inside region to fill
   * \param y Y start value inside region to fill
   * \param bounding_color Color of the bound
   */
  virtual void drv_fill(std::int16_t x, std::int16_t y, color::value_type bounding_color)
  {
    // TBD: fill region up to the bounding border_color with the drawing color
    (void)x; (void)y; (void)bounding_color;
  }


  /**
   * Move display area algorithm
   * \param x0 X source value
   * \param y0 Y source value
   * \param x1 X destination value
   * \param y1 Y destination value
   * \param width Width of the area
   * \param height Height of the area
   */
  virtual void drv_move(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::uint16_t width, std::uint16_t height)
  {
    std::uint16_t w, h;

    if (x0 < x1) {
      if (y0 < y1) {
        for (h = height; h != 0U; --h) {
          for (w = width; w != 0U; --w) {
            drv_pixel_set_color(x1 + w, y1 + h, drv_pixel_get(x0 + w, y0 + h));
          }
        }
      }
      else {
        for (h = 0U; h < height; ++h) {
          for (w = width; w != 0U; --w) {
            drv_pixel_set_color(x1 + w, y1 + h, drv_pixel_get(x0 + w, y0 + h));
          }
        }
      }
    }
    else {
      if (y0 < y1) {
        for (h = height; h != 0U; --h) {
          for (w = 0U; w < width; ++w) {
            drv_pixel_set_color(x1 + w, y1 + h, drv_pixel_get(x0 + w, y0 + h));
          }
        }
      }
      else {
        for (h = 0U; h < height; ++h) {
          for (w = 0U; w < width; ++w) {
            drv_pixel_set_color(x1 + w, y1 + h, drv_pixel_get(x0 + w, y0 + h));
          }
        }
      }
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  // D R I V E R   T E X T   F U N C T I O N S
  /////////////////////////////////////////////////////////////////////////////

  /**
   * Select the font
   * \param Reference to font to use
   * \return true if font set successfully
   */
  virtual bool drv_text_set_font(const font_type& font)
  {
    (void)font;
    return true;
  }


  /**
   * Set the new text position
   * \param x X value in pixel on graphic displays, char pos on alpha displays
   * \param y Y value in pixel on graphic displays, char pos on alpha displays
   * \return true if position is set successfully
   */
  virtual bool drv_text_set_pos(std::int16_t x, std::int16_t y)
  {
    (void)x; (void)y;
    return true;
  }


  /**
   * Set the text mode
   * \param mode Set normal or inverse video
   * \return true if successful
   */
  virtual bool drv_text_set_mode(text_mode_type mode)
  {
    (void)mode;
    return true;
  }


  /**
   * Output one character
   * \param ch Output character in ASCII/UNICODE (NOT UTF-8) format
   */
  virtual void drv_text_char(std::uint16_t ch)
  {
  #if defined(VGX_CFG_FONT)
    std::uint8_t color_depth = (text_font_->font_attr & VGX_FONT_AA_MASK);
    std::uint8_t color_mask  = (1U << color_depth) - 1U;

    // handling of special chars
    if ((char)ch == '\n') {
      // LF: X = 0, Y = next line
      text_x_act_ = text_x_set_;
      text_y_act_ = text_y_act_ + text_font_->ysize;
      return;
    }
    else if ((char)ch == '\r') {
      // CR: X = 0
      text_x_act_ = text_x_set_;
      return;
    }
    else if (ch < 0x0020U) {
      // ignore all other codes
      return;
    }

    if ((text_font_->font_attr & VGX_FONT_ENCODING_MASK) == VGX_FONT_ENCODING_UNICODE) {
      // extended (UNICODE) font
      const font_prop_ext_type* font_prop_ext = text_font_->font_type_type.font_prop_ext;
      do {
        if (ch >= font_prop_ext->first && ch <= font_prop_ext->last) {
          // found char
          const font_charinfo_ext_type* info = &font_prop_ext->char_info_ext[ch - font_prop_ext->first];
          std::uint16_t x, y, d = 0;
          for (y = 0U; y < info->ysize; ++y) {
            d = (1U + ((info->xsize - 1U) * color_depth / 8U)) * y;
            for (x = 0U; x < info->xsize ; ++x) {
              std::uint8_t intensity = (info->data[d + ((x * color_depth) >> 3U)] >> ((8U - (x + 1U) * color_depth) % 8U)) & color_mask;
              if (intensity) {
                if (color_depth == VGX_FONT_AA_NONE) {
                  drv_pixel_set(text_x_act_ + info->xpos + x, text_y_act_ + info->ypos + y);
                }
                else {
                  color::value_type fg = color_;
                  color::value_type bg = drv_pixel_get(text_x_act_+ info->xpos + x, text_y_act_ + info->ypos + y);
                  drv_pixel_set_color(text_x_act_ + info->xpos + x, text_y_act_ + info->ypos + y, color::mix(fg, bg, color_depth == 2U ? intensity << 6U : intensity << 4U));
                }
              }
            }
          }
          text_x_act_ += info->xdist;
          return;
        }
        font_prop_ext = font_prop_ext->next;
      } while (font_prop_ext);
      // char not found
      return;
    }
    else {
      // normal (ASCII) font
      if ((text_font_->font_attr & VGX_FONT_TYPE_MASK) == VGX_FONT_TYPE_PROP) {
        // prop font
        const font_prop_type* font_prop = text_font_->font_type_type.font_prop;
        do {
          if (ch >= font_prop->first && ch <= font_prop->last) {
            // found char
            const font_charinfo_type* info = &font_prop->char_info[ch - font_prop->first];
            std::uint16_t x, y, d;
            for (y = 0U; y < text_font_->ysize; ++y) {
              d = (1U + ((info->xsize - 1U) * color_depth / 8U)) * y;
              for (x = 0U; x < info->xsize ; ++x) {
                uint8_t intensity = (info->data[d + ((x * color_depth) >> 3U)] >> ((8U - (x + 1U) * color_depth) % 8U)) & color_mask;
                if (intensity) {
                  if (color_depth == VGX_FONT_AA_NONE) {
                    drv_pixel_set(text_x_act_ + x, text_y_act_ + y);
                  }
                  else {
                    color::value_type fg = color_;
                    color::value_type bg = drv_pixel_get(text_x_act_ + x, text_y_act_ + y);
                    drv_pixel_set_color(text_x_act_ + x, text_y_act_ + y, color::mix(fg, bg, color_depth == 2U ? intensity << 6U : intensity << 4U));
                  }
                }
              }
            }
            text_x_act_ += info->xdist;
            return;
          }
          font_prop = font_prop->next;
        } while (font_prop);
        // char not found
        return;
      }
      else {
        // mono font
        const font_mono_type* font_mono = text_font_->font_type_type.font_mono;
        if (ch >= font_mono->first && ch <= font_mono->last) {
          std::uint16_t x, y, d;
          for (y = 0U; y < text_font_->ysize; ++y) {
            d = (ch - font_mono->first) * text_font_->ysize * font_mono->bytes_per_line + y * font_mono->bytes_per_line;
            for (x = 0U; x < font_mono->xsize; ++x) {
              uint8_t intensity = (font_mono->data[d + ((x * color_depth) >> 3U)] >> ((8U - (x + 1U) * color_depth) % 8U)) & color_mask;
              if (intensity) {
                if (color_depth == VGX_FONT_AA_NONE) {
                  drv_pixel_set(text_x_act_ + x, text_y_act_ + y);
                }
                else {
                }
              }
            }
          }
          text_x_act_ += font_mono->xsize;   // x-distance is xsize
          return;
        }
      }
    }
  #endif  // VGX_CFG_FONT
  }


  /**
   * Render an ASCII/UTF-8 coded string
   * \param string Output string in ASCII/UTF-8 format, zero terminated
   * \return Number of written characters, not bytes (as an UTF-8 character may consist out of more bytes)
   */
  virtual std::uint16_t drv_text_string(const std::uint8_t* string)
  {
    std::uint16_t ch, cnt = 0U;
    while (*string) {
      if ((*string & 0x80U) == 0x00U) {
        // 1 byte sequence (ASCII char)
        ch = (std::uint16_t)(*string++ & 0x7FU);
      }
      else if ((*string & 0xE0U) == 0xC0U) {
        // 2 byte sequence
        ch = (((std::uint16_t)*string & 0x001FU) << 6U) | ((std::uint16_t)*(string + 1U) & 0x003FU);
        string += 2U;
      } else if ((*string & 0xF0U) == 0xE0U) {
        // 3 byte UTF-8 sequence
        ch = (((std::uint16_t)*string & 0x000FU) << 12U) | (((std::uint16_t)*(string + 1U) & 0x003FU) << 6U) | ((std::uint16_t)*(string + 2U) & 0x003FU);
        string += 3U;
      } else {
        // unknown sequence
        string++;
        continue;
      }
      drv_text_char(ch);
      cnt++;
    }
    return cnt;
  }


  /**
   * Render an ASCII/UTF-8 coded string
   * \param string Output string in ASCII/UTF-8 format, zero terminated
   * \return Number of written characters, not bytes (as an UTF-8 character may consist out of more bytes)
   */
  virtual std::uint16_t drv_text_string_rotate(std::uint16_t angle, const std::uint8_t* string)
  {
    // TBD
    (void)angle; (void)string;
    return 0;
  }

///////////////////////////////////////////////////////////////////////////////

protected:
  color::value_type color_;           // drawing color
  color::value_type color_bg_;        // background color
  bool              primitive_lock_;  // lock for rendering multiple primitives without refresh
  const font_type*  text_font_;       // actual selected font
  std::int16_t      text_x_set_;      // x cursor position for new line
  std::int16_t      text_x_act_;      // actual x cursor position
  std::int16_t      text_y_act_;      // actual y cursor position
  text_mode_type    text_mode_;       // text mode
#if defined(VGX_CFG_ANTIALIASING)
  bool              anti_aliasing_;   // true if AA is enabled
#endif
#if defined(VGX_CFG_CLIPPING)
  std::int16_t  clipping_x0_;         // clipping region top/left
  std::int16_t  clipping_y0_;         // clipping region top/left
  std::int16_t  clipping_x1_;         // clipping region bottom/right
  std::int16_t  clipping_y1_;         // clipping region bottom/right
#endif

private:
  /**
   * Helper function to calculate (lookup) sin(x) and cos(x), normalized to 100
   * \param angle Angle in degree, valid range from 0° to 90°
   * \return sin(x) * 100 in upper byte, cos(x) * 100 in lower byte
   */
  inline std::uint16_t sin(std::uint8_t angle) const   // sin(x) helper function
  {
    const std::uint16_t angle_to_xy[91U] = {
      0x0064U, 0x0264U, 0x0364U, 0x0564U, 0x0764U, 0x0964U, 0x0A63U, 0x0C63U, 0x0E63U, 0x1063U, 0x1162U, 0x1362U, 0x1562U, 0x1661U, 0x1861U, 0x1A61U,
      0x1C60U, 0x1D60U, 0x1F5FU, 0x215FU, 0x225EU, 0x245DU, 0x255DU, 0x275CU, 0x295BU, 0x2A5BU, 0x2C5AU, 0x2D59U, 0x2F58U, 0x3057U, 0x3257U, 0x3456U,
      0x3555U, 0x3654U, 0x3853U, 0x3952U, 0x3B51U, 0x3C50U, 0x3E4FU, 0x3F4EU, 0x404DU, 0x424BU, 0x434AU, 0x4449U, 0x4548U, 0x4747U, 0x4845U, 0x4944U,
      0x4A43U, 0x4B42U, 0x4D40U, 0x4E3FU, 0x4F3EU, 0x503CU, 0x513BU, 0x5239U, 0x5338U, 0x5436U, 0x5535U, 0x5634U, 0x5732U, 0x5730U, 0x582FU, 0x592DU,
      0x5A2CU, 0x5B2AU, 0x5B29U, 0x5C27U, 0x5D25U, 0x5D24U, 0x5E22U, 0x5F21U, 0x5F1FU, 0x601DU, 0x601CU, 0x611AU, 0x6118U, 0x6116U, 0x6215U, 0x6213U,
      0x6211U, 0x6310U, 0x630EU, 0x630CU, 0x630AU, 0x6409U, 0x6407U, 0x6405U, 0x6403U, 0x6402U, 0x6400U
    };
    return angle <= 90 ? angle_to_xy[angle] : 0U;   // out of bounds returns 0
  }

  // non copyable
  const gpr& operator=(const gpr& rhs) { return rhs; }
};

} // namespace vgx

#endif  // _VGX_GPR_H_
