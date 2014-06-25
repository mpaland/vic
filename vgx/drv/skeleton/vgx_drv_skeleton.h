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
// \brief Skeleton driver, use this as a start for own drivers
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_DRV_SKELETON_
#define _VGX_DRV_SKELETON_

#include "vgx/vgx_drv.h"


namespace vgx {

class drv_skeleton : public drv_head<std::uint32_t, 24U>
{
public:
/////////////////////////////////////////////////////////////////////////////
// M A N D A T O R Y   F U N C T I O N S

  drv_skeleton(std::uint16_t xsize, std::uint16_t ysize, std::int16_t xoffset, std::int16_t yoffset)
    : drv_head(xsize, ysize, xoffset, yoffset)
  { }
  virtual void init();                                    // driver init
  virtual void deinit();                                  // driver deinit
  virtual void brightness_set(std::uint8_t level);        // set display brightness/backlight
  virtual const char* version() const;                    // get driver name and version
  virtual void primitive_done();                          // rendering done (copy RAM / frame buffer to screen)
  virtual void cls();                                     // clear display, all pixels off (black)

protected:
/////////////////////////////////////////////////////////////////////////////
// M A N D A T O R Y   F U N C T I O N S   F O R   G R A P H I C   H E A D S

  /**
   * Set pixel (in actual drawing color)
   * \param x X value
   * \param y Y value
   */
  virtual bool pixel_set(std::int16_t x, std::int16_t y);

  /**
   * Set pixel in given color, the color doesn't change the actual drawing color
   * \param x X value
   * \param y Y value
   * \param color Color of pixel in ARGB format
   */
  virtual bool pixel_set_color(std::int16_t x, std::int16_t y, std::uint32_t color);

  /**
   * Get pixel color
   * \param x X value
   * \param y Y value
   * \return Color of pixel in ARGB format
   */
  virtual std::uint32_t pixel_get(std::int16_t x, std::int16_t y) const;

  /////////////////////////////////////////////////////////////////////////////
  // O P T I O N A L   D R A W I N G   F U N C T I O N S
  //
  // Implement these functions only if the display (controller) has NATIVE support for it
  // If unsupported, remove the according function, the gpr will draw the primitive instead
  // See function description is in gpr

  virtual void drv_line(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1);
  virtual void drv_line_horz(std::int16_t x0, std::int16_t y0, std::int16_t x1);
  virtual void drv_line_vert(std::int16_t x0, std::int16_t y0, std::int16_t y1);
  virtual void drv_line_width(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::uint16_t width);
  virtual void drv_rect(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1);
  virtual void drv_box(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1);
  virtual void drv_triangle(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2);
  virtual void drv_triangle_solid(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2);
  virtual void drv_arc(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2);
  virtual void drv_circle(std::int16_t x, std::int16_t y, std::uint16_t r);
  virtual void drv_disc(std::int16_t x, std::int16_t y, std::uint16_t r);
  virtual void drv_disc_section(std::int16_t x, std::int16_t y, std::uint16_t r, std::uint8_t section);
  virtual void drv_sector(std::int16_t x, std::int16_t y, std::uint16_t inner_radius, std::uint16_t outer_radius, std::uint16_t start_angle, std::uint16_t end_angle);
  virtual void drv_fill(std::int16_t x, std::int16_t y, std::uint32_t bounding_color);

#if defined(VGX_CFG_ANTIALIASING)
  virtual void drv_line_aa(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1);
  virtual void drv_triangle_aa(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2);
  virtual void drv_triangle_solid_aa(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2);
  virtual void drv_arc_aa(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2);
  virtual void drv_circle_aa(std::int16_t x, std::int16_t y, std::uint16_t r);
  virtual void drv_disc_aa(std::int16_t x, std::int16_t y, std::uint16_t r);
  virtual void drv_disc_section_aa(std::int16_t x, std::int16_t y, std::uint16_t r, std::uint8_t section);
#endif  // VGX_CFG_ANTIALIASING

  virtual void drv_move(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::uint16_t width, std::uint16_t height);

  virtual bool drv_text_set_font(const font_type& font);
  virtual bool drv_text_set_pos(std::int16_t x, std::int16_t y);
  virtual void drv_text_set_mode(text_mode_type mode);
  virtual void drv_text_char(std::uint16_t ch);
  virtual std::uint16_t drv_text_string(const std::uint8_t* string);
};

} // namespace vgx

#endif  // _VGX_DRV_SKELETON_H_
