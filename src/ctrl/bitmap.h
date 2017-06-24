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
// \brief Bitmap control
// This control renders a bitmap in XPM format or other various XBM formats
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_CTRL_BITMAP_H_
#define _VIC_CTRL_BITMAP_H_

#include <cstdint>
#include <cstddef>
#include <string.h>   // memcpy/memcmp

#include "ctrl.h"


// defines the maximum chars per XPM pixel
// value of 4 is default, one CLUT entry takes 8 byte than
#define VIC_CTRL_BITMAP_XPM_MAX_CHAR_ON_PIXEL  4U

// Entrys in the Color LookUp Table for XPM bitmaps
// 16/32 is a good value, so CLUT takes 128/256 byte
#define VIC_CTRL_BITMAP_XPM_CLUT_SIZE 16U


namespace vic {

class bitmap : public ctrl
{
public:

  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  bitmap(drv& head)
    : ctrl(head)
    , bmp_width_(0)
    , bmp_height_(0)
    , bmp_origin_({ 0, 0 })
    , clut_idx_(0U)
  { }


  /**
   * Check if the given point is inside the control
   * \param point Point to check
   * \return true if point is inside the control
   */
  virtual bool is_inside(vertex_type point)
  { return (point.x >= bmp_origin_.x && point.x < bmp_origin_.x + bmp_width_) &&
           (point.y >= bmp_origin_.y && point.y < bmp_origin_.y + bmp_height_);
  }


  /**
   * Render the bitmap of the given format
   */
  void render_bmp(vertex_type origin, std::uint16_t width, std::uint16_t height, color::format_type format, const std::uint8_t* bitmap)
  {
    bmp_origin_ = origin;
    bmp_width_  = width;
    bmp_height_ = height;

    // lock output
    head_.present_lock();

    switch (format)
    {
      case color::format_L1 :
        for (std::int16_t y = origin.y, ye = origin.y + height; y < ye; ++y) {
          std::uint16_t bit = 0U;
          std::uint8_t  data = *bitmap;
          for (std::int16_t x = origin.x, xe = origin.x + width; x < xe; ++x) {
            if ((bit++ & 0x07U) == 0x00U) {
              data = *bitmap++;
            }
            else {
              data >>= 1U;
            }
            if (data & 0x01U) {
              head_.plot({ x, y });
            }
          }
        }
        break;

      case color::format_RGB332 :
        for (std::int16_t y = origin.y, ye = origin.y + height; y < ye; ++y) {
          for (std::int16_t x = origin.x, xe = origin.x + width; x < xe; ++x, bitmap++) {
            head_.plot({ x, y }, head_.color_from_head_RGB332(*bitmap));
          }
        }
        break;

      case color::format_RGB555 :
        for (std::int16_t y = origin.y, ye = origin.y + height; y < ye; ++y) {
          for (std::int16_t x = origin.x, xe = origin.x + width; x < xe; ++x, bitmap += 2U) {
            head_.plot({ x, y }, head_.color_from_head_RGB555(*bitmap));
          }
        }
        break;

      case color::format_RGB565 :
        for (std::int16_t y = origin.y, ye = origin.y + height; y < ye; ++y) {
          for (std::int16_t x = origin.x, xe = origin.x + width; x < xe; ++x, bitmap += 2U) {
            head_.plot({ x, y }, head_.color_from_head_RGB565(*bitmap));
          }
        }
        break;

      case color::format_RGB888 :
        for (std::int16_t y = origin.y, ye = origin.y + height; y < ye; ++y) {
          for (std::int16_t x = origin.x, xe = origin.x + width; x < xe; ++x, bitmap += 3U) {
            head_.plot({ x, y }, head_.color_from_head_RGB888(*bitmap));
          }
        }
        break;

      case color::format_ARGB8888 :
        for (std::int16_t y = origin.y, ye = origin.y + height; y < ye; ++y) {
          for (std::int16_t x = origin.x, xe = origin.x + width; x < xe; ++x, bitmap += 4U) {
            head_.plot({ x, y }, head_.color_from_head_RGB555(*bitmap));
          }
        }
        break;

      default :
        // unsupported format
        break;
    }
    head_.present_lock(false);
  }


  /**
   * Render a bitmap in XPM3 format
   * \param destination Destination vertex (left/top)
   * \param max_width Maximum width of the bitmap
   * \param max_height Maximum height of the bitmap
   * \param xpm3_image Image data in XPM3 format
   */
  void render_xpm(vertex_type origin, const char* xpm3_image[])
  {
    char* xpm = (char*)xpm3_image[0];
    bmp_origin_             = origin;
    bmp_width_              = static_cast<std::uint16_t>(strtol(xpm, &xpm, 10U));
    bmp_height_             = static_cast<std::uint16_t>(strtol(xpm, &xpm, 10U));
    xpm_data_.colors        = static_cast<std::uint16_t>(strtol(xpm, &xpm, 10U));
    xpm_data_.char_on_pixel = static_cast<std::uint16_t>(strtol(xpm, &xpm, 10U));
    xpm_data_.image         = xpm3_image;

    // init clut
    clut_idx_ = 0U;
    memset(clut_, 0, sizeof(clut_));

    // render the bitmap
    head_.present_lock();
    std::uint16_t ix = 0U, iy = xpm_data_.colors + 1U;
    for (std::int16_t y = origin.y, ye = origin.y + bmp_height_; y < ye; ++y, ++iy, ix = 0U) {
      for (std::int16_t x = origin.x, xe = origin.x + bmp_width_; x < xe; ++x, ix += xpm_data_.char_on_pixel) {
        const color::value_type _color = xpm_get_clut_color(&xpm3_image[iy][ix]);
        if (_color != color::none) {
          head_.plot({ x, y }, _color);
        }
      }
    }
    head_.present_lock(false);
  }


private:

  // get the xpm color out of the image data
  inline color::value_type xpm_get_image_color(const char* pixel_code)
  {
    // read color from graphic
    for (std::uint_fast16_t c = 1U; c < xpm_data_.colors + 1U; c++) {
      char* p = (char*)xpm_data_.image[c];
      if (memcmp(p, pixel_code, xpm_data_.char_on_pixel) == 0) {
        // found - get color
        p += xpm_data_.char_on_pixel;
        while (*p == ' ' || *p == 0x09) { p++; }  // skip any space or tab
        while (*p != ' ' && *p != 0x09) { p++; }  // skip color type (ignored)
        while (*p == ' ' || *p == 0x09) { p++; }  // skip any space or tab

        if (*p == '#') {
          // read RGB color
          return static_cast<color::value_type>(strtol(++p, nullptr, 16U));
        }
        else if (memcmp(p, "None", 4U)) {
          return color::none;
        }
      }
    }
    return color::none;   // color code not found, should not happen, return transparent
  }


  // get the xpm color out of the image data
  color::value_type xpm_get_clut_color(const char* pixel_code)
  {
    // try to find the color in clut
    for (std::uint_fast16_t n = 0U; n < VIC_CTRL_BITMAP_XPM_CLUT_SIZE; n++) {
      if (memcmp(clut_[n].code, pixel_code, xpm_data_.char_on_pixel) == 0) {
        // found
        return clut_[n].color;
      }
    }

    // store new color in clut
    color::value_type _color = xpm_get_image_color(pixel_code);
    memcpy(clut_[clut_idx_].code, pixel_code, xpm_data_.char_on_pixel);
    clut_[clut_idx_].color = _color;
    clut_idx_ = ++clut_idx_ % VIC_CTRL_BITMAP_XPM_CLUT_SIZE;
    return _color;
  }


  /**
   * Special fast string to long conversion, only pos values needed
   * \param str String to parse
   * \param endptr Pointer to one char after 
   * \param base Base
   * \return Parsed value
   */
  std::uint32_t strtol(const char* str, char** endptr, unsigned int base) const
  {
    std::size_t i = 0U;

    // skip whitspace(s)
    while (str[i] == ' ') {
      i++;
    }

    // parse value
    std::uint32_t res = 0U;
    for (;;) {
      std::uint8_t digit = static_cast<std::uint8_t>(str[i++]);
      if (digit >= 0x30U && digit <= 0x39U) {
        digit -= 0x30U;
      }
      else if ((digit & 0xDFU) >= 0x41U && (digit & 0xDFU) <= 0x5AU) {
        digit -= 0x37U;
      }
      else {
        if (endptr) {
          *endptr = (char*)&str[i-1];
        }
        return res;
      }
      res = res * base + digit;
    }
  }


private:

  // xpm color lookup table
  typedef struct tag_clut_type {
    char              code[VIC_CTRL_BITMAP_XPM_MAX_CHAR_ON_PIXEL];  // color code
    color::value_type color;                                        // assigned color
  } clut_type;

  // xpm data
  typedef struct tag_xpm_data_type {
    std::uint16_t colors;           // colors of the bitmap
    std::uint16_t char_on_pixel;    // chars per pixel
    const char**  image;            // pointer to the bitmap
  } xpm_data_type;

  std::uint16_t bmp_width_;         // bitmap width
  std::uint16_t bmp_height_;        // bitmap height
  vertex_type   bmp_origin_;        // top-left corner
  xpm_data_type xpm_data_;          // XPM data
 
  // XPM color lookup table
  clut_type     clut_[VIC_CTRL_BITMAP_XPM_CLUT_SIZE] = { 0 };
  std::size_t   clut_idx_;
};

} // namespace vic

#endif  // _VIC_CTRL_BITMAP_H_
