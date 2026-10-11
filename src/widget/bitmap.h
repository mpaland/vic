///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2020, PALANDesign Hannover, Germany
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
// \brief Bitmap widget, renders raw bitmaps (several color formats) and XPM3 images
// The raw data is read byte wise with the least significant byte first, so there
// are no alignment requirements.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_BITMAP_H_
#define _VIC_WIDGET_BITMAP_H_

#include <string.h>   // memcpy/memcmp/memset
#include "widget.h"


// defines the maximum chars per XPM pixel
// value of 4 is default, one CLUT entry takes 8 byte than
#define VIC_WIDGET_BITMAP_XPM_MAX_CHAR_ON_PIXEL  4U

// Entrys in the Color LookUp Table for XPM bitmaps
// 16/32 is a good value, so the CLUT takes 128/256 byte
#define VIC_WIDGET_BITMAP_XPM_CLUT_SIZE         16U


namespace vic {
namespace widget {


class bitmap : public base
{
private:
  // xpm color lookup table
  typedef struct tag_clut_type {
    char              code[VIC_WIDGET_BITMAP_XPM_MAX_CHAR_ON_PIXEL];  // color code
    color::value_type color;                                          // assigned color
  } clut_type;

  // xpm data
  typedef struct tag_xpm_data_type {
    std::uint16_t colors;           // colors of the bitmap
    std::uint16_t char_on_pixel;    // chars per pixel
    const char**  image;            // pointer to the bitmap
  } xpm_data_type;

  // vars
  dc            dc_;                // drawing context
  std::uint16_t bmp_width_;         // bitmap width
  std::uint16_t bmp_height_;        // bitmap height
  vertex_type   bmp_origin_;        // top-left corner
  xpm_data_type xpm_data_;          // XPM data

  // XPM color lookup table
  clut_type     clut_[VIC_WIDGET_BITMAP_XPM_CLUT_SIZE];
  std::size_t   clut_idx_;

public:
  /**
   * ctor
   * \param head Reference to the head driver
   */
  bitmap(drv& head)
    : base(head)
    , dc_(head)
    , bmp_width_(0U)
    , bmp_height_(0U)
    , bmp_origin_({ 0, 0 })
    , clut_idx_(0U)
  {
    xpm_data_.colors        = 0U;
    xpm_data_.char_on_pixel = 0U;
    xpm_data_.image         = nullptr;
    memset(clut_, 0, sizeof(clut_));
  }


  /**
   * Init - unused
   */
  virtual void init() final
  { }


  /**
   * Set the foreground color for monochrome (L1) bitmaps
   * \param color Foreground color, default is gray
   */
  void set_color(color::value_type color)
  {
    dc_.set_color(color);
  }


  /**
   * Render the bitmap from the given raw color format
   * \param origin Origin (left/top) where the bitmap is rendered
   * \param width Width of the bitmap
   * \param height Height of the bitmap
   * \param format Data format, L1 (XBM like, LSB first, rows are byte aligned), RGB332, RGB555, RGB565, RGB888 (B G R) and ARGB8888 (B G R A) are supported
   * \param bitmap Bitmap data in the given raw format
   */
  void render_raw(vertex_type origin, std::uint16_t width, std::uint16_t height, color::format_type format, const std::uint8_t* bitmap)
  {
    bmp_origin_ = origin;
    bmp_width_  = width;
    bmp_height_ = height;

    dc_.present_lock();
    if (format == color::format_ARGB8888) {
      // block transfer in chunks of one row (or part of it), so a GRAM head gets whole runs
      color::value_type chunk[32];
      for (std::int16_t y = origin.y, ye = static_cast<std::int16_t>(origin.y + height); y < ye; ++y) {
        for (std::uint16_t x = 0U; x < width; x = static_cast<std::uint16_t>(x + 32U)) {
          const std::uint16_t n = static_cast<std::uint16_t>((width - x) < 32U ? (width - x) : 32U);
          for (std::uint16_t i = 0U; i < n; ++i, bitmap += 4U) {
            chunk[i] = color::argb(bitmap[2], bitmap[1], bitmap[0], bitmap[3]);
          }
          dc_.blit({ static_cast<std::int16_t>(origin.x + x), y, static_cast<std::int16_t>(origin.x + x + n), static_cast<std::int16_t>(y + 1) }, chunk, n);
        }
      }
      dc_.present_lock(false);
      return;
    }
    for (std::int16_t y = origin.y, ye = static_cast<std::int16_t>(origin.y + height); y < ye; ++y) {
      std::uint8_t data = 0U;
      for (std::int16_t x = origin.x, xe = static_cast<std::int16_t>(origin.x + width); x < xe; ++x) {
        switch (format)
        {
          case color::format_L1 :
            // monochrome, LSB first, every row starts with a new byte
            if (((x - origin.x) & 0x07) == 0) {
              data = *bitmap++;
            }
            else {
              data = static_cast<std::uint8_t>(data >> 1U);
            }
            if (data & 0x01U) {
              dc_.pixel_set({ x, y });
            }
            break;
          case color::format_RGB332 :
            dc_.pixel_set({ x, y }, color::RGB332_to_color(*bitmap));
            bitmap += 1U;
            break;
          case color::format_RGB555 :
            dc_.pixel_set({ x, y }, color::RGB555_to_color(static_cast<std::uint16_t>(bitmap[0] | (static_cast<std::uint16_t>(bitmap[1]) << 8U))));
            bitmap += 2U;
            break;
          case color::format_RGB565 :
            dc_.pixel_set({ x, y }, color::RGB565_to_color(static_cast<std::uint16_t>(bitmap[0] | (static_cast<std::uint16_t>(bitmap[1]) << 8U))));
            bitmap += 2U;
            break;
          case color::format_RGB888 :
            dc_.pixel_set({ x, y }, color::argb(bitmap[2], bitmap[1], bitmap[0]));
            bitmap += 3U;
            break;
          case color::format_ARGB8888 :
            dc_.pixel_set({ x, y }, color::argb(bitmap[2], bitmap[1], bitmap[0], bitmap[3]));
            bitmap += 4U;
            break;
          default :
            // unsupported format
            dc_.present_lock(false);
            return;
        }
      }
    }
    dc_.present_lock(false);
  }


  /**
   * Render the bitmap from the XPM3 format
   * \param origin Origin (left/top) where the bitmap is rendered
   * \param xpm3_image Image data in XPM3 format, like { "4 2 2 1", "a c #FF0000", "  c None", "aa  ", " aa " }
   */
  void render_xpm(vertex_type origin, const char* xpm3_image[])
  {
    const char* xpm = xpm3_image[0];
    bmp_origin_             = origin;
    bmp_width_              = static_cast<std::uint16_t>(strtol(xpm, &xpm, 10U));
    bmp_height_             = static_cast<std::uint16_t>(strtol(xpm, &xpm, 10U));
    xpm_data_.colors        = static_cast<std::uint16_t>(strtol(xpm, &xpm, 10U));
    xpm_data_.char_on_pixel = static_cast<std::uint16_t>(strtol(xpm, &xpm, 10U));
    xpm_data_.image         = xpm3_image;
    if (xpm_data_.char_on_pixel == 0U) {
      return;
    }

    // init clut
    clut_idx_ = 0U;
    memset(clut_, 0, sizeof(clut_));

    // render the bitmap
    dc_.present_lock();
    std::uint16_t iy = static_cast<std::uint16_t>(xpm_data_.colors + 1U);
    for (std::int16_t y = origin.y, ye = static_cast<std::int16_t>(origin.y + bmp_height_); y < ye; ++y, ++iy) {
      std::uint16_t ix = 0U;
      for (std::int16_t x = origin.x, xe = static_cast<std::int16_t>(origin.x + bmp_width_); x < xe; ++x, ix = static_cast<std::uint16_t>(ix + xpm_data_.char_on_pixel)) {
        const color::value_type _color = xpm_get_clut_color(&xpm3_image[iy][ix]);
        if (_color != color::none) {
          dc_.pixel_set({ x, y }, _color);
        }
      }
    }
    dc_.present_lock(false);
  }


protected:

  /**
   * Check if the given point is inside the control
   * \param vertex Vertex to check
   * \return true if point is inside the control
   */
  virtual bool is_inside(vertex_type vertex) const final
  { return (vertex.x >= bmp_origin_.x && vertex.x < bmp_origin_.x + bmp_width_) &&
           (vertex.y >= bmp_origin_.y && vertex.y < bmp_origin_.y + bmp_height_);
  }


private:

  // get the xpm color out of the image data
  color::value_type xpm_get_image_color(const char* pixel_code) const
  {
    // read color from graphic
    for (std::uint_fast16_t c = 1U; c < xpm_data_.colors + 1U; c++) {
      const char* p = xpm_data_.image[c];
      if (memcmp(p, pixel_code, xpm_data_.char_on_pixel) == 0) {
        // found - get color
        p += xpm_data_.char_on_pixel;
        while (*p == ' ' || *p == 0x09) { p++; }  // skip any space or tab
        while (*p != ' ' && *p != 0x09 && *p != 0) { p++; }  // skip color type (ignored)
        while (*p == ' ' || *p == 0x09) { p++; }  // skip any space or tab

        if (*p == '#') {
          // read RGB color
          return static_cast<color::value_type>(strtol(++p, nullptr, 16U) | 0xFF000000UL);
        }
        else if (memcmp(p, "None", 4U) == 0) {
          return color::none;
        }
        // named colors are not supported
        return color::none;
      }
    }
    return color::none;   // color code not found, should not happen, return transparent
  }


  // get the xpm color via the color lookup table
  color::value_type xpm_get_clut_color(const char* pixel_code)
  {
    if (xpm_data_.char_on_pixel > VIC_WIDGET_BITMAP_XPM_MAX_CHAR_ON_PIXEL) {
      // the code doesn't fit into the CLUT, look it up directly
      return xpm_get_image_color(pixel_code);
    }

    // try to find the color in clut
    for (std::uint_fast16_t n = 0U; n < VIC_WIDGET_BITMAP_XPM_CLUT_SIZE; n++) {
      if (memcmp(clut_[n].code, pixel_code, xpm_data_.char_on_pixel) == 0) {
        // found
        return clut_[n].color;
      }
    }

    // store new color in clut
    const color::value_type _color = xpm_get_image_color(pixel_code);
    memcpy(clut_[clut_idx_].code, pixel_code, xpm_data_.char_on_pixel);
    clut_[clut_idx_].color = _color;
    clut_idx_ = (clut_idx_ + 1U) % VIC_WIDGET_BITMAP_XPM_CLUT_SIZE;
    return _color;
  }


  /**
   * Special fast string to long conversion, only pos values needed
   * \param str String to parse
   * \param endptr Pointer to one char after the number
   * \param base Base, 10 or 16
   * \return Parsed value
   */
  static std::uint32_t strtol(const char* str, const char** endptr, unsigned int base)
  {
    std::size_t i = 0U;

    // skip whitespace(s)
    while (str[i] == ' ' || str[i] == 0x09) {
      i++;
    }

    // parse value
    std::uint32_t res = 0U;
    for (;;) {
      const char ch = str[i];
      std::uint32_t digit;
      if (ch >= '0' && ch <= '9') {
        digit = static_cast<std::uint32_t>(ch - '0');
      }
      else if ((base > 10U) && (ch >= 'A') && (ch <= 'F')) {
        digit = static_cast<std::uint32_t>(ch - 'A' + 10);
      }
      else if ((base > 10U) && (ch >= 'a') && (ch <= 'f')) {
        digit = static_cast<std::uint32_t>(ch - 'a' + 10);
      }
      else {
        if (endptr) {
          *endptr = &str[i];
        }
        return res;
      }
      res = res * base + digit;
      i++;
    }
  }
};

} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_BITMAP_H_
