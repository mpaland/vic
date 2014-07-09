///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2001-2014, PALANDesign Hannover, Germany
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
// \brief Graphic Primitive Renderer of the vgxlib to render lines, arcs, boxes etc.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_GPR_H_
#define _VGX_GPR_H_

#include <cstdint>

#include "vgx_fonts.h"


namespace vgx {

typedef enum enum_line_style_type {
  vgx_line_style_solid = 0,
  vgx_line_style_dotted,
  vgx_line_style_dashed
} line_style_type;

typedef struct struct_vertex_type {
  std::uint16_t x;
  std::uint16_t y;
} vertex_type;

typedef struct struct_gradient_type {
  bool           horizontal;
  std::uint16_t  color_count;
  std::uint32_t* colors;
} gradient_type;

typedef enum enum_text_mode_type {
  text_mode_normal = 0,
  text_mode_inverse
} text_mode_type;


class gpr
{
public:
  /**
   * Color assembly, returns ARGB format out of color components
   * \param red Red color
   * \param green Green color
   * \param blue Blue color
   * \param alpha Alpha level, 0 = opaque, 255 = completely transparent
   * \return ARGB color
   */
  inline std::uint32_t color_rgb(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 0U) const
  { return static_cast<std::uint32_t>((((std::uint32_t)alpha) << 24U) | (((std::uint32_t)red) << 16U) | (((std::uint32_t)green) << 8U) | ((std::uint32_t)blue)); }

  // color channel values out of 32BBP color
  inline std::uint8_t color_get_red  (std::uint32_t color) const { return static_cast<std::uint8_t>((color & 0x00FF0000UL) >> 16U); }
  inline std::uint8_t color_get_green(std::uint32_t color) const { return static_cast<std::uint8_t>((color & 0x0000FF00UL) >>  8U); }
  inline std::uint8_t color_get_blue (std::uint32_t color) const { return static_cast<std::uint8_t>((color & 0x000000FFUL));        }
  inline std::uint8_t color_get_alpha(std::uint32_t color) const { return static_cast<std::uint8_t>((color & 0xFF000000UL) >> 24U); }

  // generic color dimming, alpha channel is unaffected
  inline std::uint32_t color_dim_25(std::uint32_t color) const
  { return ((color & (0x00FCFCFCUL)) >> 2U); }

  inline std::uint32_t color_dim_50(std::uint32_t color) const
  { return ((color & (0x00FEFEFEUL)) >> 1U); }

  inline std::uint32_t color_dim_75(std::uint32_t color) const
  { return (color_dim_25(color) + color_dim_50(color)); }

  /**
   * Dim the given color, alpha channel is unaffected
   * \param color Color in ARGB format
   * \param lum Luminance, 0: dark, 255: bright (original color unchanged)
   * \return dimmed color in ARGB format
   */
  inline std::uint32_t color_dim(std::uint32_t color, std::uint8_t lum) const
  { return ((color & 0xFF000000UL) | ((((color & 0x00FF0000UL) * (lum + 1U)) >> 8U) & 0x00FF0000UL) | ((((color & 0x0000FF00UL) * (lum + 1U)) >> 8U) & 0x0000FF00UL) | ((((color & 0x000000FFUL) * (lum + 1U)) >> 8U) & 0x000000FFUL)); }

  // generic color mixing, alpha channel is unaffected
  inline std::uint32_t color_mix_25(std::uint32_t fore, std::uint32_t back) const
  { return (color_dim_25(fore) + color_dim_75(back)); }

  inline std::uint32_t color_mix_50(std::uint32_t fore, std::uint32_t back) const
  { return (color_dim_50(fore) + color_dim_50(back)); }

  inline std::uint32_t color_mix_75(std::uint32_t fore, std::uint32_t back) const
  { return (color_dim_75(fore) + color_dim_25(back)); }

  inline std::uint32_t color_mix(std::uint32_t fore, std::uint32_t back, std::uint8_t lum) const
  { return (color_dim(fore, lum) + color_dim(back, 0xFFU - lum)); }


///////////////////////////////////////////////////////////////////////////////
// D R I V E R   F U N C T I O N S

  /**
   * Plot a point, implemented in driver
   * \param x X value
   * \param y Y value
   */
  virtual void pixel_set(std::int16_t x, std::int16_t y) = 0;

  /**
   * Plot a point in the given color, implemented in driver
   * \param x X value
   * \param y Y value
   * \param color Color to plot
   */
  virtual void pixel_set_color(std::int16_t x, std::int16_t y, std::uint32_t color) = 0;

  /**
   * Get the color of a pixel, implemented in driver
   * \param x X value
   * \param y Y value
   * \return Color of the pixel
   */
  virtual std::uint32_t pixel_get(std::int16_t x, std::int16_t y) const = 0;

  /**
   * Primitive rendering is done. May be overridden by driver to update display,
   * frame buffer or something else (like copy RAM / rendering buffer to screen)
   */
  virtual void primitive_done()
  { }


///////////////////////////////////////////////////////////////////////////////
// C O L O R   F U N C T I O N S

  /**
   * Set the drawing color
   * \param color New drawing color in ARGB format
   */
  virtual void color_set(std::uint32_t color)
  { color_ = color; }

  /**
   * Get the actual drawing color
   * \return Actual drawing color in ARGB format
   */
  inline std::uint32_t color_get() const
  { return color_; }

  /**
   * Set the background color (e.g. for cls)
   * \param color_background New background color in ARGB format
   */
  virtual void color_set_bg(std::uint32_t color_background)
  { color_bg_ = color_background; }


///////////////////////////////////////////////////////////////////////////////
// G R A P H I C   P R I M I T I V E   F U N C T I O N S

  /**
   * Draw a line, width is one pixel
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in line
   * \param y1 Y end value, included in line
   * \return true if successful
   */
  bool line(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1);

  /**
   * Draw a horizontal line, width is one pixel
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in line
   * \return true if successful
   */
  bool line_horz(std::int16_t x0, std::int16_t y0, std::int16_t x1);

  /**
   * Draw a vertical line, width is one pixel
   * \param x0 X start value
   * \param y0 Y start value
   * \param y1 Y end value, included in line
   * \return true if successful
   */
  bool line_vert(std::int16_t x0, std::int16_t y0, std::int16_t y1);

  /**
   * Draw a line with specified width
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in line
   * \param y1 Y end value, included in line
   * \param width Line width in pixel
   * \return true if successful
   */
  bool line_width(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::uint16_t width);

  /**
   * Draw a rectangle (frame)
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in rect
   * \param y1 Y end value, included in rect
   * \return true if successful
   */
  bool rect(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1);

  /**
   * Draw a box (filled rectangle)
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in box
   * \param y1 Y end value, included in box
   * \return true if successful
   */
  bool box(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1);

  /**
   * Draw a box (filled rectangle) with gradient colors
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in line
   * \param y1 Y end value, included in line
   * \param colors Pointer to array of gradient colors
   * \param color_count Number of colors in the array
   * \return true if successful
   */
  bool box_gradient(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, const gradient_type& gradient);

  /**
   * Draw a polygon
   * \param vertexes Pointer to array of polygon vertexes, at least 3
   * \param vertex_count Number of polygon points in the structure, at least 3
   * \return true if successful
   */
  bool polygon(const vertex_type* vertexes, std::uint16_t vertex_count);

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
  bool triangle(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2);

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
  bool triangle_solid(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2);

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
  bool arc(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2);

  /**
   * Draw a circle
   * \param x X center value
   * \param y Y center value
   * \param radius Circle radius
   * \return true if successful
   */
  bool circle(std::int16_t x, std::int16_t y, std::uint16_t radius);

  /**
   * Draw a disc (filled circle)
   * \param x X center value
   * \param y Y center value
   * \param radius Disc radius
   * \return true if successful
   */
  bool disc(std::int16_t x, std::int16_t y, std::uint16_t radius);

  /**
   * Draw a disc section (filled quarter circle)
   * \param x X center value
   * \param y Y center value
   * \param radius Disc section radius
   * \param section Section number: 0: top/right, 1: top/left, 2: bottom/left, 3: bottom/right
   * \return true if successful
   */
  bool disc_section(std::int16_t x, std::int16_t y, std::int16_t radius, std::uint8_t section);

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
  bool sector(std::int16_t x, std::int16_t y, std::uint16_t inner_radius, std::uint16_t outer_radius, std::uint16_t start_angle, std::uint16_t end_angle);

  ///////////////////////////////////////////////////////////////////////////////

  /**
   *  Fill region up to the bounding color with the drawing color
   * \param x X start value inside region to fill
   * \param y Y start value inside region to fill
   * \param bounding_color Color of the surrounding bound
   * \return true if successful
   */
  bool fill(std::int16_t x, std::int16_t y, std::uint32_t bounding_color);

  ///////////////////////////////////////////////////////////////////////////////

  /**
   * Block bit transfer
   * Transfer image to display
   * \param x X destination value
   * \param y Y destination value
   * \param width Width of the image
   * \param height Height of the image
   * \param color_depth Color depth of the image (BBP)
   * \param data Image data
   * \return true if successful
   */
  bool blitter(std::int16_t x, std::int16_t y, std::uint16_t width, std::uint16_t height, std::uint8_t color_depth, std::uint8_t* data);

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
  bool move(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::uint16_t width, std::uint16_t height);

  /**
   * Enable/disable anti aliasing support
   * \param enable True to enable anti aliasing
   */
  bool anti_aliasing_set(bool enable);

#if defined(VGX_CFG_CLIPPING)
  /**
   * Set clipping region
   * All further points within the region are not drawn
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in region
   * \param y1 Y end value, included in region
   */
  bool clipping_set(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1);

  /**
   * Disable clipping
   */
  bool clipping_clr();
#endif


///////////////////////////////////////////////////////////////////////////////
// T E X T   F U C T I O N S 
//

  /**
   * Select the font
   * \param Reference to font to use
   * \return true if successful
   */
  bool text_set_font(const font_type& font);

  /**
   * Set the new text position
   * \param x X value in pixel on graphic displays, char pos on alpha displays
   * \param y Y value in pixel on graphic displays, char pos on alpha displays
   * \return true if successful
   */
  bool text_set_pos(std::int16_t x, std::int16_t y);

  /**
   * Set the text mode
   * \param mode Set normal or inverse video
   * \return true if successful
   */
  bool text_set_mode(text_mode_type mode);

  /**
   * Output one character
   * \param ch Output character in ASCII/UNICODE (NOT UTF-8) format
   */
  void text_char(std::uint16_t ch);

  /**
   * Output a string
   * \param string Output string in ASCII/UTF-8 format, zero terminated
   * \return Number of written characters, not bytes (as an UTF-8 character may consist out of more bytes)
   */
  std::uint16_t text_string(const std::uint8_t* string);

  /**
   * Output a string at x/y position (combines text_set_pos and text_string)
   * \param x X value in pixel on graphic displays, char pos on alpha displays
   * \param y Y value in pixel on graphic displays, char pos on alpha displays
   * \param string Output string in ASCII/UTF-8 format, zero terminated
   * \return Number of written characters, not bytes (as a character may consist out of two bytes)
   */
  std::uint16_t text_string_pos(std::int16_t x, std::int16_t y, const std::uint8_t* string);

  /**
   * Output a rotated string
   * \param x X value in pixel on graphic displays
   * \param y Y value in pixel on graphic displays
   * \param angle Angle in degree, 0° is horizontal right, counting anticlockwise
   * \param string Output string in ASCII/UTF-8 format, zero terminated
   * \return Number of written characters, not bytes (as a character may consist out of two bytes)
   */
  std::uint16_t text_string_rotate(std::int16_t x, std::int16_t y, std::uint16_t angle, const std::uint8_t* string);

  /**
   * Returns the width and height the rendered string would take.
   * The string is not rendered on screen
   * \param width Width the rendered string would take
   * \param height Height the rendered string would take
   * \param string Output string in ASCII/UTF-8 format, zero terminated
   * \return Number of written characters, not bytes (as a character may consist out of two bytes)
   */
  std::uint16_t text_string_get_extend(std::uint16_t& width, std::uint16_t height, const std::uint8_t* string);


///////////////////////////////////////////////////////////////////////////////
// F U N C T I O N S   T H E   D R I V E R   C A N   O V E R R I D E
//
// ... if the driver has native (hardware/firmware accelerated) support for it

protected:
  // drawing functions

  /**
   * Line drawing algorithm, width is one pixel
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in line
   * \param y1 Y end value, included in line
   */
  virtual void drv_line(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1);

  /**
   * Horizontal line drawing algorithm, width is one pixel
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in line
   */
  virtual void drv_line_horz(std::int16_t x0, std::int16_t y0, std::int16_t x1);

  /**
   * Vertical line drawing algorithm, width is one pixel
   * \param x0 X start value
   * \param y0 Y start value
   * \param y1 Y end value, included in line
   */
  virtual void drv_line_vert(std::int16_t x0, std::int16_t y0, std::int16_t y1);

  /**
   * Line drawing algorithm with specified line width
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in line
   * \param y1 Y end value, included in line
   * \param width Line width in pixel
   */
  virtual void drv_line_width(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::uint16_t width);

  /**
   * Rectangle (frame) drawing algorithm
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in rect
   * \param y1 Y end value, included in rect
   */
  virtual void drv_rect(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1);

  /**
   * Box (filled rectangle) drawing algorithm
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in box
   * \param y1 Y end value, included in box
   */
  virtual void drv_box(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1);

  /**
   * Triangle drawing algorithm
   * \param x0 X value, included in triangle
   * \param y0 Y value, included in triangle
   * \param x1 X value, included in triangle
   * \param y1 Y value, included in triangle
   * \param x2 X value, included in triangle
   * \param y2 Y value, included in triangle
   */
  virtual void drv_triangle(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2);

  /**
   * Solid (filled) triangle drawing algorithm
   * \param x0 X value, included in triangle
   * \param y0 Y value, included in triangle
   * \param x1 X value, included in triangle
   * \param y1 Y value, included in triangle
   * \param x2 X value, included in triangle
   * \param y2 Y value, included in triangle
   */
  virtual void drv_triangle_solid(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2);

  /**
   * Arc (Bézier curve) drawing algorithm
   * \param x0 X start value, included in arc
   * \param y0 Y start value, included in arc
   * \param x1 X mid value, included in arc
   * \param y1 Y mid value, included in arc
   * \param x2 X end value, included in arc
   * \param y2 Y end value, included in arc
   */
  virtual void drv_arc(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2);

  /**
   * Circle drawing algorithm
   * \param x X center value
   * \param y Y center value
   * \param radius Circle radius
   */
  virtual void drv_circle(std::int16_t x, std::int16_t y, std::uint16_t r);

  /**
   * Disc (filled circle) drawing algorithm
   * \param x X center value
   * \param y Y center value
   * \param radius Disc radius
   */
  virtual void drv_disc(std::int16_t x, std::int16_t y, std::uint16_t r);

  /**
   * Disc section (filled quarter circle) drawing algorithm
   * \param x X center value
   * \param y Y center value
   * \param radius Disc section radius
   * \param section Section number: 0: top/right, 1: top/left, 2: bottom/left, 3: bottom/right
   */
  virtual void drv_disc_section(std::int16_t x, std::int16_t y, std::uint16_t r, std::uint8_t section);

  /**
   * Sector (filled circle piece) drawing algorithm
   * \param x X center value
   * \param y Y center value
   * \param inner_radius Inner sector radius
   * \param outer_radius Outer sector radius
   * \param start_angle Start angle in degree, 0° is horizontal right, counting anticlockwise
   * \param end_angle end angle in degree
   */
  virtual void drv_sector(std::int16_t x, std::int16_t y, std::uint16_t inner_radius, std::uint16_t outer_radius, std::uint16_t start_angle, std::uint16_t end_angle);

  /**
   *  Fill region up to the bounding border_color with the drawing color algorithm
   * \param x X start value inside region to fill
   * \param y Y start value inside region to fill
   * \param bounding_color Color of the bound
   */
  virtual void drv_fill(std::int16_t x, std::int16_t y, std::uint32_t bounding_color);

#if defined(VGX_CFG_ANTIALIASING)
  /**
   * Anti aliased line drawing algorithm, width is one pixel
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in line
   * \param y1 Y end value, included in line
   */
  virtual void drv_line_aa(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1);

  /**
   * Anti aliased triangle drawing algorithm
   * \param x0 X value, included in triangle
   * \param y0 Y value, included in triangle
   * \param x1 X value, included in triangle
   * \param y1 Y value, included in triangle
   * \param x2 X value, included in triangle
   * \param y2 Y value, included in triangle
   */
  virtual void drv_triangle_aa(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2);

  /**
   * Solid (filled) anti aliased triangle drawing algorithm
   * \param x0 X value, included in triangle
   * \param y0 Y value, included in triangle
   * \param x1 X value, included in triangle
   * \param y1 Y value, included in triangle
   * \param x2 X value, included in triangle
   * \param y2 Y value, included in triangle
   */
  virtual void drv_triangle_solid_aa(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2);

  /**
   * Anti aliased arc (Bézier curve) drawing algorithm
   * \param x0 X start value, included in arc
   * \param y0 Y start value, included in arc
   * \param x1 X mid value, included in arc
   * \param y1 Y mid value, included in arc
   * \param x2 X end value, included in arc
   * \param y2 Y end value, included in arc
   */
  virtual void drv_arc_aa(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2);

  /**
   * Anti aliased circle drawing algorithm
   * \param x X center value
   * \param y Y center value
   * \param radius Circle radius
   */
  virtual void drv_circle_aa(std::int16_t x, std::int16_t y, std::uint16_t r);

  /**
   * Anti aliased disc (filled circle) drawing algorithm
   * \param x X center value
   * \param y Y center value
   * \param radius Disc radius
   */
  virtual void drv_disc_aa(std::int16_t x, std::int16_t y, std::uint16_t r);

  /**
   * Anti aliased disc section (filled quarter circle) drawing algorithm
   * \param x X center value
   * \param y Y center value
   * \param radius Disc section radius
   * \param section Section number: 0: top/right, 1: top/left, 2: bottom/left, 3: bottom/right
   */
  virtual void drv_disc_section_aa(std::int16_t x, std::int16_t y, std::uint16_t r, std::uint8_t section);
#endif  // VGX_CFG_ANTIALIASING

  /**
   * Move display area algorithm
   * \param x0 X source value
   * \param y0 Y source value
   * \param x1 X destination value
   * \param y1 Y destination value
   * \param width Width of the area
   * \param height Height of the area
   */
  virtual void drv_move(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::uint16_t width, std::uint16_t height);

  // text functions

  /**
   * Select the font
   * \param Reference to font to use
   * \return true if font set successful
   */
  virtual bool drv_text_set_font(const font_type& font);

  /**
   * Set the new text position
   * \param x X value in pixel on graphic displays, char pos on alpha displays
   * \param y Y value in pixel on graphic displays, char pos on alpha displays
   * \return true if font set successful
   */
  virtual bool drv_text_set_pos(std::int16_t x, std::int16_t y);

  /**
   * Set the text mode
   * \param mode Set normal or inverse video
   * \return true if successful
   */
  virtual void drv_text_set_mode(text_mode_type mode);

  /**
   * Output one character
   * \param ch Output character in ASCII/UNICODE (NOT UTF-8) format
   */
  virtual void drv_text_char(std::uint16_t ch);

  /**
   * Output a string
   * \param string Output string in ASCII/UTF-8 format, zero terminated
   * \return Number of written characters, not bytes (as an UTF-8 character may consist out of more bytes)
   */
  virtual std::uint16_t drv_text_string(const std::uint8_t* string);

///////////////////////////////////////////////////////////////////////////////

  /**
   * ctor
   * Init vars
   */
  gpr()
    : color_(0U)
    , color_bg_(0U)
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

protected:
  std::uint32_t     color_;           // drawing color
  std::uint32_t     color_bg_;        // background color

  const font_type*  text_font_;       // actual selected font
  std::int16_t      text_x_set_;      // x cursor position for new line
  std::int16_t      text_x_act_;      // actual x cursor position
  std::int16_t      text_y_act_;      // actual y cursor position
  text_mode_type    text_mode_;       // text mode
#if defined(VGX_CFG_ANTIALIASING)
  bool              anti_aliasing_;   // true if AA is enabled
#endif
#if defined(VGX_CFG_CLIPPING)
  std::int16_t  clippling_x0_;
  std::int16_t  clippling_y0_;
  std::int16_t  clippling_x1_;
  std::int16_t  clippling_y1_;
#endif

private:
  inline std::uint16_t sin(std::uint8_t angle) const;   // sin(x) helper function

  const gpr& operator=(const gpr& rhs) { return rhs; }  // non copyable
};

} // namespace vgx

#endif  // _VGX_GPR_H_
