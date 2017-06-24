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
// \brief Text Renderer to render text on graphic displays.
// The text renderer uses the fonts in the font folder.
// Monospace and proportional fonts are supported.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_TXR_H_
#define _VIC_TXR_H_

#include "base.h"
#include "font.h"


namespace vic {


typedef enum tag_text_mode_type {
  text_mode_normal = 0,
  text_mode_inverse
} text_mode_type;


/**
 * TeXt Renderer
 */
class txr : virtual public base
{
public:

  /**
   * ctor
   * Init vars
   */
  txr()
    : text_font_(nullptr)
    , text_x_set_(0)
    , text_x_act_(0)
    , text_y_act_(0)
    , text_mode_(text_mode_normal)
  { }


///////////////////////////////////////////////////////////////////////////////
// T E X T   F U C T I O N S 
//

  /**
   * Select the font
   * \param Reference to font to use
   */
  inline virtual void text_font_select(const font::font_type& font)
  {
    text_font_ = &font;
  }


  /**
   * Get the height of the actual selected font
   * \return Height in pixel units of the selected font, 1 on alpha numeric heads
   */
  inline std::uint8_t text_line_height() const
  {
    return drv_is_graphic() ? text_font_->ysize : 1U;
  }


  /**
   * Set the new text position
   * \param pos Position (left/top) in pixel on graphic displays, position in chars on text displays
   */
  virtual void text_pos(vertex_type pos)
  {
    text_x_act_ = text_x_set_ = pos.x;
    text_y_act_ = pos.y;
  }


  /**
   * Set the text mode
   * \param mode Set normal or inverse video
   */
  virtual void text_mode(text_mode_type mode)
  {
    text_mode_ = mode;
  }


  /**
   * Clear actual line from cursor pos to end
   */
  virtual void text_clear_eol()
  {
  }


  /**
   * Clear actual line from start to cursor pos
   */
  virtual void text_clear_sol()
  {
  }


  /**
   * Clear the actual line
   */
  virtual void text_clear_line()
  {
  }


  /**
   * Output a single ASCII/UNICODE char at the actual cursor position
   * The cursor position is moved by the char width (distance)
   * \param ch Output character in 16 bit ASCII/UNICODE (NOT UTF-8) format, 00-7F is compatible with ASCII
   */
  virtual void text_char(std::uint16_t ch)
  {
    const std::uint8_t color_depth = (text_font_->attr & font::AA_MASK);
    const std::uint8_t color_mask = (1U << color_depth) - 1U;
    const std::uint8_t color_shift = 8U - color_depth;

    if (ch < 0x20U) {
      // ignore non characters
      return;
    }

    if ((text_font_->attr & font::ENCODING_MASK) == font::ENCODING_UNICODE) {
      // extended (UNICODE) font
      const font::prop_ext_type* font_prop_ext = text_font_->font_type_type.prop_ext;
      do {
        if (ch >= font_prop_ext->first && ch <= font_prop_ext->last) {
          // found char
          const font::charinfo_ext_type* info = &font_prop_ext->char_info_ext[ch - font_prop_ext->first];
          for (std::uint_fast8_t y = 0U; y < info->ysize; ++y) {
            std::uint16_t d = (1U + ((info->xsize - 1U) * color_depth / 8U)) * y;
            for (std::uint_fast8_t x = 0U; x < info->xsize; ++x) {
              std::uint16_t intensity = (info->data[d + ((x * color_depth) >> 3U)] >> ((8U - (x + 1U) * color_depth) % 8U)) & color_mask;
              if (intensity) {
                intensity = ((intensity + 1U) << color_shift) - 1U;
                drv_pixel_set_color({ static_cast<std::int16_t>(text_x_act_ + info->xpos + (std::int16_t)x), static_cast<std::int16_t>(text_y_act_ + info->ypos + (std::int16_t)y) }, color::set_alpha(pen_get_color(), static_cast<std::uint8_t>((std::uint16_t)color::get_alpha(pen_get_color()) * (255U - intensity) / 255U)));
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
      if ((text_font_->attr & font::TYPE_MASK) == font::TYPE_PROP) {
        // prop font
        const font::prop_type* font_prop = text_font_->font_type_type.prop;
        do {
          if (ch >= font_prop->first && ch <= font_prop->last) {
            // found char
            const font::charinfo_type* info = &font_prop->char_info[ch - font_prop->first];
            for (std::uint_fast8_t y = 0U; y < text_font_->ysize; ++y) {
              std::uint16_t d = (1U + ((info->xsize - 1U) * color_depth / 8U)) * y;
              for (std::uint_fast8_t x = 0U; x < info->xsize; ++x) {
                std::uint16_t intensity = (info->data[d + ((x * color_depth) >> 3U)] >> ((8U - (x + 1U) * color_depth) % 8U)) & color_mask;
                if (intensity) {
                  intensity = ((intensity + 1U) << color_shift) - 1U;
                  drv_pixel_set_color({ static_cast<std::int16_t>(text_x_act_ + (std::int16_t)x), static_cast<std::int16_t>(text_y_act_ + (std::int16_t)y) }, color::set_alpha(pen_get_color(), 255U - static_cast<std::uint8_t>((255U - color::get_alpha(pen_get_color())) * intensity / 255U)));
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
        const font::mono_type* font_mono = text_font_->font_type_type.mono;
        if (ch >= font_mono->first && ch <= font_mono->last) {
          for (std::uint_fast8_t y = 0U; y < text_font_->ysize; ++y) {
            std::uint16_t d = (ch - font_mono->first) * text_font_->ysize * font_mono->bytes_per_line + (std::int16_t)y * font_mono->bytes_per_line;
            for (std::uint_fast8_t x = 0U; x < font_mono->xsize; ++x) {
              std::uint16_t intensity = (font_mono->data[d + ((x * color_depth) >> 3U)] >> ((8U - (x + 1U) * color_depth) % 8U)) & color_mask;
              if (intensity) {
                intensity = ((intensity + 1U) << color_shift) - 1U;
                drv_pixel_set_color({ static_cast<std::int16_t>(text_x_act_ + (std::int16_t)x), static_cast<std::int16_t>(text_y_act_ + (std::int16_t)y) }, color::set_alpha(pen_get_color(), 255U - static_cast<std::uint8_t>((255U - color::get_alpha(pen_get_color())) * intensity / 255U)));
              }
            }
          }
          text_x_act_ += font_mono->xsize;
          return;
        }
      }
    }
  }


  /**
   * Render an UTF-8 / ASCII coded string
   * \param string Output string in UTF-8/ASCII format, zero terminated
   * \return Number of written characters, not bytes (as an UTF-8 character may consist out of more bytes)
   */
  virtual std::uint16_t text_string(const std::uint8_t* string)
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
        text_x_act_ = text_x_set_;
        text_y_act_ = text_y_act_ + text_font_->ysize;
      }
      else if ((char)ch == '\r') {
        // CR: X = 0
        text_x_act_ = text_x_set_;
      }
      else {
        text_char(ch);
      }
      cnt++;
    }
    present();
    return cnt;
  }


  /**
   * Output a string at x/y position (combines text_set_pos and text_string)
   * \param pos Position in pixel on graphic displays, position in chars on text displays
   * \param string Output string in ASCII/UTF-8 format, zero terminated
   * \return Number of written characters, not bytes (as an UTF-8 character may consist out of more bytes)
   */
  inline virtual std::uint16_t text_string_pos(vertex_type pos, const std::uint8_t* string)
  {
    text_pos(pos);
    return text_string(string);
  }


  /**
   * Output a rotated string
   * \param pos Position in pixel on graphic displays, position in chars on text displays
   * \param angle Angle in degree, 0� is horizontal right, counting anticlockwise
   * \param string Output string in ASCII/UTF-8 format, zero terminated
   * \return Number of written characters, not bytes (as a character may consist out of two bytes)
   */
  std::uint16_t text_string_rotate(vertex_type pos, std::uint16_t angle, const std::uint8_t* string)
  {
    text_pos(pos);
//  TBD
    (void)angle; (void)string;
    return 0U;
  }


  /**
   * Returns the width and height the rendered string would take.
   * The string is not rendered on screen
   * \param width Width the rendered string would take
   * \param height Height the rendered string would take
   * \param string String in UTF-8 format, zero terminated
   * \return Number of string characters, not bytes (as a character may consist out of two bytes)
   */
  std::uint16_t text_string_get_extend(std::uint16_t& width, std::uint16_t& height, const std::uint8_t* string) const
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

      if (drv_is_graphic())
      {
        if ((text_font_->attr & font::ENCODING_MASK) == font::ENCODING_UNICODE) {
          // extended (UNICODE) font
          const font::prop_ext_type* font_prop_ext = text_font_->font_type_type.prop_ext;
          do {
            if (ch >= font_prop_ext->first && ch <= font_prop_ext->last) {
              // found char
              const font::charinfo_ext_type* info = &font_prop_ext->char_info_ext[ch - font_prop_ext->first];
              width += info->xdist;
            }
            font_prop_ext = font_prop_ext->next;
          } while (font_prop_ext);
          // char not found
        }
        else {
          // normal (ASCII) font
          if ((text_font_->attr & font::TYPE_MASK) == font::TYPE_PROP) {
            // prop font
            const font::prop_type* font_prop = text_font_->font_type_type.prop;
            do {
              if (ch >= font_prop->first && ch <= font_prop->last) {
                // found char
                const font::charinfo_type* info = &font_prop->char_info[ch - font_prop->first];
                width += info->xdist;
              }
              font_prop = font_prop->next;
            } while (font_prop);
            // char not found
          }
          else {
            // mono font
            const font::mono_type* font_mono = text_font_->font_type_type.mono;
            if (ch >= font_mono->first && ch <= font_mono->last) {
              width += font_mono->xsize;   // x-distance is xsize
            }
          }
        }
      }
      cnt++;
    }
    width  = cnt;
    height = 1U;
    return cnt;
  }


///////////////////////////////////////////////////////////////////////////////

protected:
  const font::font_type*  text_font_;       // actual selected font
  std::int16_t            text_x_set_;      // x cursor position for new line
  std::int16_t            text_x_act_;      // actual x cursor position
  std::int16_t            text_y_act_;      // actual y cursor position
  text_mode_type          text_mode_;       // text mode

private:

  // non copyable
  const txr& operator=(const txr& rhs)
  { return rhs; }
};

} // namespace vic

#endif  // _VIC_TXR_H_
