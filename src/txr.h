///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2016-2017, PALANDesign Hannover, Germany
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
// \brief Text Renderer class to render text chars on graphic displays.
// The text renderer uses the fonts in the 'fonts' folder.
// Monospace and proportional fonts are supported.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_TXR_H_
#define _VIC_TXR_H_

#include "font.h"
#include "util.h"
#include "color.h"


namespace vic {


/**
 * TeXt Renderer
 */
class txr
{
  // pixel set function, defined in upper class
  virtual void pixel_set(vertex_type vertex, color::value_type color) = 0;

public:

  /**
   * ctor
   * Init vars
   */
  txr()
    : color_(color::white)
    , font_(nullptr)
    , x_set_(0)
    , x_act_(0)
    , y_act_(0)
    , inverse_(false)
  { }


///////////////////////////////////////////////////////////////////////////////
// T E X T   F U C T I O N S 
//

  /**
   * Select the font
   * \param Reference to font to use
   */
  inline void set_font(const font::font_type& font)
  {
    font_ = &font.info();
  }


  /**
   * Get the average char width of the actual selected font
   * \return Width (char 'A' is taken) in pixel units of the selected font, 1 on alpha numeric heads
   */
  std::uint8_t char_width() const
  {
    if (!font_) {
      return 0U;
    }
    if ((font_->attr & font::TYPE_MASK) != font::TYPE_PROP) {
      // mono font
      return font_->family.mono->xsize;
    }
    if ((font_->attr & font::ENCODING_MASK) == font::ENCODING_UNICODE) {
      // extended (UNICODE) prop font, take 'A' or the first available char
      const font::prop_ext_type* first = font_->family.prop_ext;
      for (const font::prop_ext_type* p = first; !!p; p = p->next) {
        if (('A' >= p->first) && ('A' <= p->last)) {
          return p->char_info_ext['A' - p->first].xdist;
        }
      }
      return first ? first->char_info_ext[0].xdist : 0U;
    }
    else {
      // prop font, take 'A' or the first available char
      const font::prop_type* first = font_->family.prop;
      for (const font::prop_type* p = first; !!p; p = p->next) {
        if (('A' >= p->first) && ('A' <= p->last)) {
          return p->char_info['A' - p->first].xdist;
        }
      }
      return first ? first->char_info[0].xdist : 0U;
    }
  }


  /**
   * Get the char height of the actual selected font
   * \return Height in pixel units of the selected font, 1 on alpha numeric heads
   */
  inline std::uint8_t char_height() const
  {
    return font_ ? font_->ysize : 0U;
  }


  /**
   * Set the new text position
   * \param pos Position (left/top) in pixel on graphic displays, position in chars on text displays
   */
  inline void set_pos(vertex_type pos)
  {
    x_act_ = x_set_ = pos.x;
    y_act_ = pos.y;
  }


  /**
   * Set inverse text mode
   * \param inverse Set normal or inverse video
   */
  inline void set_inverse(bool inverse = true)
  {
    inverse_ = inverse;
  }


  /**
   * Set the text color
   * \param color Set the text color
   */
  inline void set_color(color::value_type color)
  {
    color_ = color;
  }


  /**
   * Render a single ASCII/UNICODE char at the actual cursor position
   * The cursor position is moved by the char width (distance)
   * \param ch Output character in 16 bit ASCII/UNICODE (NOT UTF-8) format, 00-7F is compatible with ASCII
   */
  void out(std::uint16_t ch)
  {
    if (!font_ || (ch < 0x20U)) {
      // no font selected or non character
      return;
    }

    // font color depth: 1, 2, 4 or 8 bit per pixel
    const std::uint8_t color_depth = (font_->attr & font::AA_MASK);
    const std::uint8_t color_mask  = static_cast<std::uint8_t>((1U << color_depth) - 1U);
    const std::uint8_t color_shift = static_cast<std::uint8_t>(8U - color_depth);
    // alpha of the pen color, the pixel intensity of anti aliased fonts is multiplied with it
    const std::uint16_t color_alpha = color::get_alpha(color_);

    // The intensity of a pixel is extracted with:  (data[d + ((x * color_depth) >> 3)] >> ((8 - (x + 1) * color_depth) % 8)) & color_mask
    // The shift expression relies on unsigned wrap around for x > 8 / color_depth, e.g. 2 bpp, x = 4: (8 - 10) mod 2^32 % 8 = 6
    // The intensity is expanded to 8 bit with: ((intensity + 1) << color_shift) - 1, so the max intensity is always 255

    if ((font_->attr & font::ENCODING_MASK) == font::ENCODING_UNICODE) {
      // extended (UNICODE) font
      const font::prop_ext_type* font_prop_ext = font_->family.prop_ext;
      do {
        if (ch >= font_prop_ext->first && ch <= font_prop_ext->last) {
          // found char
          const font::charinfo_ext_type* info = &font_prop_ext->char_info_ext[ch - font_prop_ext->first];
          for (std::uint_fast8_t y = 0U; y < info->ysize; ++y) {
            std::uint16_t d = (1U + ((info->xsize - 1U) * color_depth / 8U)) * y;
            for (std::uint_fast8_t x = 0U; x < info->xsize; ++x) {
              std::uint16_t intensity = (info->data[d + ((x * color_depth) >> 3U)] >> ((8U - (x + 1U) * color_depth) % 8U)) & color_mask;
              if (intensity) {
                intensity = static_cast<std::uint16_t>(((intensity + 1U) << color_shift) - 1U);
                pixel_set({ static_cast<std::int16_t>(x_act_ + info->xpos + (std::int16_t)x), static_cast<std::int16_t>(y_act_ + info->ypos + (std::int16_t)y) },
                          color::set_alpha(color_, static_cast<std::uint8_t>(color_alpha * intensity / 255U)));
              }
            }
          }
          x_act_ = static_cast<std::int16_t>(x_act_ + info->xdist);
          return;
        }
        font_prop_ext = font_prop_ext->next;
      } while (font_prop_ext);
      // char not found
      return;
    }
    else {
      // normal (ASCII) font
      if ((font_->attr & font::TYPE_MASK) == font::TYPE_PROP) {
        // prop font
        const font::prop_type* font_prop = font_->family.prop;
        do {
          if (ch >= font_prop->first && ch <= font_prop->last) {
            // found char
            const font::charinfo_type* info = &font_prop->char_info[ch - font_prop->first];
            for (std::uint_fast8_t y = 0U; y < font_->ysize; ++y) {
              std::uint16_t d = (1U + ((info->xsize - 1U) * color_depth / 8U)) * y;
              for (std::uint_fast8_t x = 0U; x < info->xsize; ++x) {
                std::uint16_t intensity = (info->data[d + ((x * color_depth) >> 3U)] >> ((8U - (x + 1U) * color_depth) % 8U)) & color_mask;
                if (intensity) {
                  intensity = static_cast<std::uint16_t>(((intensity + 1U) << color_shift) - 1U);
                  pixel_set({ static_cast<std::int16_t>(x_act_ + (std::int16_t)x), static_cast<std::int16_t>(y_act_ + (std::int16_t)y) },
                            color::set_alpha(color_, static_cast<std::uint8_t>(color_alpha * intensity / 255U)));
                }
              }
            }
            x_act_ = static_cast<std::int16_t>(x_act_ + info->xdist);
            return;
          }
          font_prop = font_prop->next;
        } while (font_prop);
        // char not found
        return;
      }
      else {
        // mono font
        const font::mono_type* font_mono = font_->family.mono;
        if (ch >= font_mono->first && ch <= font_mono->last) {
          for (std::uint_fast8_t y = 0U; y < font_->ysize; ++y) {
            std::uint16_t d = (ch - font_mono->first) * font_->ysize * font_mono->bytes_per_line + (std::int16_t)y * font_mono->bytes_per_line;
            for (std::uint_fast8_t x = 0U; x < font_mono->xsize; ++x) {
              std::uint16_t intensity = (font_mono->data[d + ((x * color_depth) >> 3U)] >> ((8U - (x + 1U) * color_depth) % 8U)) & color_mask;
              if (intensity) {
                intensity = static_cast<std::uint16_t>(((intensity + 1U) << color_shift) - 1U);
                pixel_set({ static_cast<std::int16_t>(x_act_ + (std::int16_t)x), static_cast<std::int16_t>(y_act_ + (std::int16_t)y) },
                          color::set_alpha(color_, static_cast<std::uint8_t>(color_alpha * intensity / 255U)));
              }
            }
          }
          x_act_ = static_cast<std::int16_t>(x_act_ + font_mono->xsize);
          return;
        }
      }
    }
  }


  /**
   * Render an UTF-8 / ASCII coded string at the actual cursor position
   * \param string Output string in UTF-8/ASCII format, zero terminated
   * \return Number of written characters, not bytes (as an UTF-8 character may consist out of more bytes)
   */
  virtual std::uint16_t out(const std::uint8_t* string)
  {
    std::uint16_t cnt = 0U;
    for (std::uint16_t ch = util::utf8_decode(string); ch != 0U; ch = util::utf8_decode(string)) {
      // handling of special chars
      if (ch == 0x0AU) {
        // LF: X = 0, Y = next line
        x_act_ = x_set_;
        y_act_ = static_cast<std::int16_t>(y_act_ + (font_ ? font_->ysize : 0U));
      }
      else if (ch == 0x0DU) {
        // CR: X = 0
        x_act_ = x_set_;
      }
      else if (ch == 0x07U) {
        // handling of color control: \aRRGGBB
        // the color is only changed if all six hex digits are present
        std::uint8_t r, g, b;
        const std::uint8_t* p = string;
        if (hex2byte(p, r) && hex2byte(p, g) && hex2byte(p, b)) {
          set_color(color::argb(r, g, b));
          string = p;
        }
      }
      else {
        out(ch);
      }
      cnt++;
    }
    return cnt;
  }


  /**
   * Output a string at x/y position (combines text_set_pos and text_string)
   * \param pos Position in pixel on graphic displays, position in chars on text displays
   * \param string Output string in ASCII/UTF-8 format, zero terminated
   * \return Number of written characters, not bytes (as an UTF-8 character may consist out of more bytes)
   */
  inline virtual std::uint16_t out(vertex_type pos, const std::uint8_t* string)
  {
    set_pos(pos);
    return out(string);
  }


  /**
   * Returns the width and height the rendered string would take.
   * The string is not rendered on screen
   * \param rect Width and height that the rendered string would take
   * \param string String in UTF-8 format, zero terminated
   * \return Number of string characters, not bytes (as a character may consist out of two or three bytes in UTF-8)
   */
  std::uint16_t get_extend(rect_type& rect, const std::uint8_t* string) const
  {
    std::uint16_t cnt = 0U;
    rect.clear();
    if (!font_) {
      // no font selected
      return 0U;
    }
    for (std::uint16_t ch = util::utf8_decode(string); ch != 0U; ch = util::utf8_decode(string)) {
      if (ch == '\a') {
        // inline color escape \aRRGGBB: not rendered, skip the 6 hex digits (NUL safe)
        for (std::uint_fast8_t n = 0U; (n < 6U) && (*string != 0U); ++n) {
          ++string;
        }
        continue;
      }
      if ((ch == '\n') || (ch == '\r')) {
        // control chars have no extent
        continue;
      }
      if ((font_->attr & font::ENCODING_MASK) == font::ENCODING_UNICODE) {
        // extended (UNICODE) font
        const font::prop_ext_type* font_prop_ext = font_->family.prop_ext;
        do {
          if (ch >= font_prop_ext->first && ch <= font_prop_ext->last) {
            // found char
            const font::charinfo_ext_type* info = &font_prop_ext->char_info_ext[ch - font_prop_ext->first];
            rect.right += info->xdist;
            rect.bottom = font_->ysize;
            break;
          }
          font_prop_ext = font_prop_ext->next;
        } while (font_prop_ext);
        // char not found
      }
      else {
        // normal (ASCII) font
        if ((font_->attr & font::TYPE_MASK) == font::TYPE_PROP) {
          // prop font
          const font::prop_type* font_prop = font_->family.prop;
          do {
            if (ch >= font_prop->first && ch <= font_prop->last) {
              // found char
              const font::charinfo_type* info = &font_prop->char_info[ch - font_prop->first];
              rect.right += info->xdist;
              rect.bottom = font_->ysize;
              break;
            }
            font_prop = font_prop->next;
          } while (font_prop);
          // char not found
        }
        else {
          // mono font
          const font::mono_type* font_mono = font_->family.mono;
          if (ch >= font_mono->first && ch <= font_mono->last) {
            rect.right += font_mono->xsize;   // x-distance is xsize
            rect.bottom = font_->ysize;
          }
        }
      }

      cnt++;
    }
    return cnt;
  }


///////////////////////////////////////////////////////////////////////////////

private:
  color::value_type       color_;      // actual text color
  const font::info_type*  font_;       // actual selected font
  std::int16_t            x_set_;      // x cursor position for new line
  std::int16_t            x_act_;      // actual x cursor position
  std::int16_t            y_act_;      // actual y cursor position
  bool                    inverse_;    // inverse text mode

private:

  // non copyable
  const txr& operator=(const txr& rhs)
  { return rhs; }


  /**
   * Hex to byte conversion helper, parses exactly two hex digits
   * \param string Reference to the string pointer, advanced by two chars on success, unchanged on failure
   * \param bin The parsed byte
   * \return True on success, false if the string ends or contains a non hex char
   */
  static bool hex2byte(const std::uint8_t*& string, std::uint8_t& bin)
  {
    std::uint8_t val = 0U;
    for (std::uint_fast8_t n = 0U; n < 2U; n++) {
      const std::uint8_t ch = string[n];
      if (ch >= '0' && ch <= '9') {
        val = static_cast<std::uint8_t>((val << 4U) + (ch - '0'));
      }
      else if (ch >= 'A' && ch <= 'F') {
        val = static_cast<std::uint8_t>((val << 4U) + (ch - 'A' + 10U));
      }
      else if (ch >= 'a' && ch <= 'f') {
        val = static_cast<std::uint8_t>((val << 4U) + (ch - 'a' + 10U));
      }
      else {
        // NUL or no hex digit
        return false;
      }
    }
    string += 2U;
    bin = val;
    return true;
  }

};

} // namespace vic

#endif  // _VIC_TXR_H_
