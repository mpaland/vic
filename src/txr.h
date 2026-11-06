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

#include "vic_cfg.h"
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
    , size_(0U)
    , x_set_(0)
    , x_act_(0)
    , x_sub_(0U)
    , y_act_(0)
    , inverse_(false)
  { }


///////////////////////////////////////////////////////////////////////////////
// T E X T   F U C T I O N S 
//

  /**
   * Select the font
   * \param font Reference to font to use
   * \param size Line height in pixel of a vector font, 0 selects the default size of the font, ignored by bitmap fonts
   */
  inline void set_font(const font::font_type& font, std::uint8_t size = 0U)
  {
    font_ = &font.info();
    size_ = (is_vector() && (size > 0U)) ? size : font_->ysize;
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
    if (is_vector()) {
      // vector font, take 'A'
      const font::vector_charinfo_type* g = vector_glyph('A');
      const std::int16_t width = g ? vector_scaled(g->xdist) : 0;
      return static_cast<std::uint8_t>(width > 255 ? 255 : width);
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
    return font_ ? size_ : 0U;
  }


  /**
   * Set the new text position
   * \param pos Position (left/top) in pixel on graphic displays, position in chars on text displays
   */
  inline void set_pos(vertex_type pos)
  {
    x_act_ = x_set_ = pos.x;
    x_sub_ = 0U;
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

    if (is_vector()) {
      vector_out(ch);
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
            std::uint16_t d = static_cast<std::uint16_t>((1U + ((info->xsize - 1U) * color_depth / 8U)) * y);
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
              std::uint16_t d = static_cast<std::uint16_t>((1U + ((info->xsize - 1U) * color_depth / 8U)) * y);
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
            std::uint16_t d = static_cast<std::uint16_t>((ch - font_mono->first) * font_->ysize * font_mono->bytes_per_line + y * font_mono->bytes_per_line);
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
        x_sub_ = 0U;
        y_act_ = static_cast<std::int16_t>(y_act_ + (font_ ? static_cast<std::int16_t>(size_) : 0));
      }
      else if (ch == 0x0DU) {
        // CR: X = 0
        x_act_ = x_set_;
        x_sub_ = 0U;
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
    std::int32_t  sub = 0;    // width of a vector font string in 1/64 pixel, the cursor keeps the sub-pixels
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
      if (is_vector()) {
        // vector font
        const font::vector_charinfo_type* g = vector_glyph(ch);
        if (g) {
          sub        += vec_div(g->xdist * vector_scale(), 1024);
          rect.right  = static_cast<std::int16_t>((sub + 32) / 64);
          rect.bottom = size_;
        }
      }
      else if ((font_->attr & font::ENCODING_MASK) == font::ENCODING_UNICODE) {
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
  std::uint8_t            size_;       // line height of the actual font in pixel (vector fonts are scalable)
  std::int16_t            x_set_;      // x cursor position for new line
  std::int16_t            x_act_;      // actual x cursor position
  std::uint8_t            x_sub_;      // sub-pixel part of the x cursor position in 1/64 pixel (vector fonts)
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
        val = static_cast<std::uint8_t>((static_cast<unsigned>(val) << 4U) + static_cast<unsigned>(ch - 'A' + 10));
      }
      else if (ch >= 'a' && ch <= 'f') {
        val = static_cast<std::uint8_t>((static_cast<unsigned>(val) << 4U) + static_cast<unsigned>(ch - 'a' + 10));
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


///////////////////////////////////////////////////////////////////////////////
// V E C T O R   F O N T   R A S T E R I Z E R
//
// The outline is scaled to 26.6 fixed point pixel coordinates (1/64 pixel) and rasterized row by
// row: every line segment of the (flattened) outline adds its exact signed area to a row buffer,
// the running sum over the row is the coverage of the pixels (nonzero winding, overlapping contours
// are clamped to full coverage). No float, no heap, one row buffer on the stack.
// Vertical hinting: the baseline and the horizontal edges and stems of the glyph (the hints of the
// font data) are snapped to pixel rows, the stem thickness is rounded to whole pixels (min. 1), the y
// of the other points is interpolated between them, so horizontal strokes become crisp pixel rows.
// Horizontal hinting: the same for the vertical edges and stems in x, they become crisp pixel columns.
// The cursor keeps the sub-pixels and a glyph is hinted at its sub-pixel position: every stem is
// snapped to the pixel column nearest to its exact position, no rounding of the advances adds up.

  // true if the selected font is a vector font
  inline bool is_vector() const
  {
    return (font_->attr & font::FORMAT_MASK) == font::FORMAT_VECTOR;
  }


  // glyph of a char in the selected vector font, nullptr if the font doesn't contain it
  const font::vector_charinfo_type* vector_glyph(std::uint16_t ch) const
  {
    for (const font::vector_block_type* b = font_->family.vector->block; !!b; b = b->next) {
      if ((ch >= b->first) && (ch <= b->last)) {
        return &b->char_info[ch - b->first];
      }
    }
    return nullptr;
  }


  // scale of the selected vector font: pixel per font unit in 16.16 fixed point
  inline std::int32_t vector_scale() const
  {
    return (static_cast<std::int32_t>(size_) << 16) / static_cast<std::int32_t>(font_->family.vector->units);
  }


  // font units of the selected vector font in pixel, rounded
  inline std::int16_t vector_scaled(std::uint16_t units) const
  {
    return static_cast<std::int16_t>((static_cast<std::int32_t>(units) * vector_scale() + 32768) / 65536);
  }


  // a point in 26.6 fixed point pixel coordinates, relative to the glyph box
  struct vec_point_type {
    std::int32_t x;
    std::int32_t y;
  };

  // max number of hinted edges (knots) of a glyph per axis
  static const std::int32_t VEC_KNOTS = 24;

  // hinted edges (knots) of an axis, the points between them are interpolated
  struct vec_knots_type {
    std::int32_t  o;              // sub-pixel offset of the axis, 26.6 (x: the sub-pixel part of the cursor)
    std::int32_t  n;              // number of knots
    std::int16_t  u[VEC_KNOTS];   // knot position in font units, increasing
    std::int16_t  d[VEC_KNOTS];   // knot position in pixel (x, height above the baseline), 26.6, whole pixels
  };

  // rasterizer state of a pixel row
  struct vec_row_type {
    std::int32_t  scale;      // pixel per font unit, 16.16
    std::int32_t  base;       // baseline from the top of the line, 26.6, whole pixels
    vec_knots_type kx;        // horizontal hints (vertical stems)
    vec_knots_type ky;        // vertical hints (horizontal stems)
    const std::int16_t* contours; // contours of the glyph
    const std::int16_t* end;  // end of the glyph data
    std::int32_t  ox;         // left of the glyph box, 26.6
    std::int32_t  oy;         // top of the glyph box, 26.6
    std::int32_t  top;        // top of the actual pixel row relative to the glyph box, 26.6
    std::int32_t  width;      // width of the row in pixel
    std::int16_t* cover;      // row buffer, width + 1 entries
  };


  // the contours of a glyph one by one: a full contour, or a moved copy of the last full contour
  // (the data: point count and points, or a negative count -k and the moves of k copies)
  struct vec_contour_type {
    const std::int16_t* d;        // next data word
    const std::int16_t* end;      // end of the glyph data
    const std::int16_t* p;        // points of the contour
    std::int32_t        n;        // number of points
    std::int32_t        copies;   // remaining copies, negative
    std::int32_t        dx;       // move of the contour in font units
    std::int32_t        dy;

    // advance to the next contour, false at the end of the glyph
    bool next()
    {
      if (copies < 0) {
        dx = d[0];
        dy = d[1];
        d += 2;
        ++copies;
        return true;
      }
      if (d >= end) {
        return false;
      }
      const std::int32_t count = *d++;
      if (count > 0) {
        p  = d;
        n  = count;
        d += 2 * count;
        dx = dy = 0;
        return true;
      }
      copies = count;
      return next();
    }
  };


  // floor(v / 64)
  static inline std::int32_t vec_floor(std::int32_t v)
  {
    return v >= 0 ? v / 64 : -((63 - v) / 64);
  }


  // decode point k of a contour to 26.6 pixel coordinates relative to the glyph box, returns the on-curve flag
  static inline bool vec_point(const vec_row_type& r, const vec_contour_type& c, std::int32_t k, vec_point_type& v)
  {
    const std::int32_t x  = c.p[2 * k];
    const std::int32_t on = x & 1;
    v.x = vec_map(r.kx, r.scale, (x - on) / 2 + c.dx) - r.ox;
    v.y = r.base - vec_map(r.ky, r.scale, c.p[2 * k + 1] + c.dy) - r.oy;
    return on != 0;
  }


  // 16.16 to whole pixel, rounded to nearest
  static inline std::int32_t vec_round(std::int32_t v)
  {
    v += 32768;
    return v >= 0 ? v / 65536 : -((65535 - v) / 65536);
  }


  // pixel position (26.6, whole pixels) of a hinted edge, aligned to its zone (-32768: no zone)
  static inline std::int32_t vec_snap(const vec_knots_type& k, std::int32_t scale, std::int32_t u, std::int32_t zone)
  {
    return zone == -32768 ? vec_round(u * scale + k.o * 1024) * 64
                          : (vec_round(zone * scale + k.o * 1024) + vec_round((u - zone) * scale)) * 64;
  }


  // add a knot (hinted edge), the knots keep their order after snapping
  static inline void vec_knot(vec_knots_type& k, std::int32_t u, std::int32_t d)
  {
    if ((k.n < VEC_KNOTS) && ((k.n == 0) || (u > k.u[k.n - 1]))) {
      k.u[k.n] = static_cast<std::int16_t>(u);
      k.d[k.n] = static_cast<std::int16_t>(((k.n > 0) && (d < k.d[k.n - 1])) ? k.d[k.n - 1] : d);
      ++k.n;
    }
  }


  // the hints of an axis to knots: edges are snapped to whole pixels, stems get a whole pixel thickness,
  // zones: the entries have zone words (vertical hints), returns the end of the hints
  static const std::int16_t* vec_hints(vec_knots_type& k, std::int32_t scale, const std::int16_t* h, bool zones)
  {
    const std::int16_t* const end = h + 1 + h[0];
    const std::int32_t w = zones ? 2 : 1;   // words of an edge
    k.n = 0;
    for (++h; h < end; ) {
      const std::int32_t b  = (h[0] - (h[0] & 1)) / 2;
      const std::int32_t zb = zones ? h[1] : -32768;
      if (!(h[0] & 1)) {
        // edge
        vec_knot(k, b, vec_snap(k, scale, b, zb));
        h += w;
        continue;
      }
      // stem: anchored at its zone edge or centered
      const std::int32_t e  = h[w];
      const std::int32_t ze = zones ? h[w + 1] : -32768;
      std::int32_t t = vec_round((e - b) * scale);
      t = (t < 1 ? 1 : t) * 64;
      std::int32_t db;
      if (zb != -32768) {
        db = vec_snap(k, scale, b, zb);
      }
      else if (ze != -32768) {
        db = vec_snap(k, scale, e, ze) - t;
      }
      else {
        db = vec_round((b + e) * scale / 2 - t * 512 + k.o * 1024) * 64;
      }
      vec_knot(k, b, db);
      vec_knot(k, e, k.d[k.n - 1] + t);   // the thickness is kept if the bottom moved behind the previous knot
      h += 2 * w;
    }
    return end;
  }


  // pixel position (26.6) of a font unit position: piecewise linear between the knots, scaled outside
  static std::int32_t vec_map(const vec_knots_type& k, std::int32_t scale, std::int32_t u)
  {
    if (k.n == 0) {
      return k.o + vec_div(u * scale, 1024);
    }
    if (u <= k.u[0]) {
      return k.d[0] + vec_div((u - k.u[0]) * scale, 1024);
    }
    for (std::int32_t i = 1; i < k.n; ++i) {
      if (u <= k.u[i]) {
        return k.d[i - 1] + vec_div((u - k.u[i - 1]) * (k.d[i] - k.d[i - 1]), k.u[i] - k.u[i - 1]);
      }
    }
    return k.d[k.n - 1] + vec_div((u - k.u[k.n - 1]) * scale, 1024);
  }


  // n / d rounded to nearest, d > 0
  static inline std::int32_t vec_div(std::int32_t n, std::int32_t d)
  {
    return (n >= 0 ? n + d / 2 : n - d / 2) / d;
  }


  // integral of clamp(u, 0, 1) * 128, 26.6, the scale keeps it exact for short (almost vertical) segments
  static inline std::int32_t vec_h(std::int32_t u)
  {
    return u <= 0 ? 0 : (u < 64 ? u * u : 128 * u - 4096);
  }


  // add the area of a line segment to the row buffer
  static void vec_line(const vec_row_type& r, vec_point_type p0, vec_point_type p1)
  {
    std::int32_t dir = 1;
    if (p0.y > p1.y) {
      const vec_point_type t = p0; p0 = p1; p1 = t;
      dir = -1;
    }
    const std::int32_t bottom = r.top + 64;
    if ((p0.y == p1.y) || (p1.y <= r.top) || (p0.y >= bottom)) {
      // horizontal or not in this row
      return;
    }
    // clip the segment to the row
    const std::int32_t ys = p0.y < r.top ? r.top : p0.y;
    const std::int32_t ye = p1.y > bottom ? bottom : p1.y;
    const std::int32_t xs = p0.x + vec_div((ys - p0.y) * (p1.x - p0.x), p1.y - p0.y);
    const std::int32_t xe = p0.x + vec_div((ye - p0.y) * (p1.x - p0.x), p1.y - p0.y);
    const std::int32_t dy = ye - ys;
    const std::int32_t lo = xs < xe ? xs : xe;
    const std::int32_t hi = xs < xe ? xe : xs;
    const std::int32_t dx = hi - lo;
    // f is the area of the pixels from the segment up to the right edge of column i,
    // the buffer gets the increments, so that the running sum is the coverage
    std::int32_t i    = lo / 64;
    std::int32_t prev = 0;
    for (; (i <= hi / 64) && (i < r.width); ++i) {
      const std::int32_t t = (i + 1) * 64;
      std::int32_t f = dx ? vec_div(dy * (vec_h(t - lo) - vec_h(t - hi)), 128 * dx) : vec_div(dy * (t - lo), 64);
      f = f > dy ? dy : f;
      r.cover[i] = static_cast<std::int16_t>(r.cover[i] + dir * (f - prev));
      prev = f;
    }
    if (i <= r.width) {
      r.cover[i] = static_cast<std::int16_t>(r.cover[i] + dir * (dy - prev));
    }
  }


  // add the area of a quadratic bezier curve to the row buffer, the curve is flattened to lines
  static void vec_quad(const vec_row_type& r, const vec_point_type& p0, const vec_point_type& p1, const vec_point_type& p2)
  {
    // the control points enclose the curve, skip curves outside of the row
    const std::int32_t ymin = p0.y < p1.y ? (p0.y < p2.y ? p0.y : p2.y) : (p1.y < p2.y ? p1.y : p2.y);
    const std::int32_t ymax = p0.y > p1.y ? (p0.y > p2.y ? p0.y : p2.y) : (p1.y > p2.y ? p1.y : p2.y);
    if ((ymax <= r.top) || (ymin >= r.top + 64)) {
      return;
    }
    // n segments, the flattening error |p0 - 2p1 + p2| / 4n^2 is below 1/16 pixel
    const std::int32_t ddx = p0.x - 2 * p1.x + p2.x;
    const std::int32_t ddy = p0.y - 2 * p1.y + p2.y;
    const std::int32_t dd  = (ddx < 0 ? -ddx : ddx) + (ddy < 0 ? -ddy : ddy);
    std::int32_t n = 1;
    while (n * n * 16 < dd) {
      ++n;
    }
    vec_point_type a = p0;
    for (std::int32_t k = 1; k <= n; ++k) {
      const std::int32_t u = n - k;
      // the coordinates are positive (relative to the glyph box), round to nearest
      const vec_point_type b = { (p0.x * u * u + 2 * p1.x * u * k + p2.x * k * k + n * n / 2) / (n * n),
                                 (p0.y * u * u + 2 * p1.y * u * k + p2.y * k * k + n * n / 2) / (n * n) };
      vec_line(r, a, b);
      a = b;
    }
  }


  // add the area of all contours of a glyph to the row buffer
  static void vec_contours(const vec_row_type& r)
  {
    vec_contour_type c = { r.contours, r.end, nullptr, 0, 0, 0, 0 };
    while (c.next()) {
      // start at an on-curve point: the first, the last or the implied one between them
      vec_point_type first, start;
      std::int32_t k = 0, k_end = c.n;
      if (vec_point(r, c, 0, first)) {
        start = first;
        k = 1;
      }
      else if (vec_point(r, c, c.n - 1, start)) {
        k_end = c.n - 1;
      }
      else {
        start.x = (first.x + start.x) / 2;
        start.y = (first.y + start.y) / 2;
      }
      // two consecutive off-curve points have an implied on-curve point in the middle
      vec_point_type cur = start, ctrl = start;
      bool has_ctrl = false;
      for (; k < k_end; ++k) {
        vec_point_type pt;
        if (vec_point(r, c, k, pt)) {
          if (has_ctrl) { vec_quad(r, cur, ctrl, pt); } else { vec_line(r, cur, pt); }
          has_ctrl = false;
          cur = pt;
        }
        else {
          if (has_ctrl) {
            const vec_point_type m = { (ctrl.x + pt.x) / 2, (ctrl.y + pt.y) / 2 };
            vec_quad(r, cur, ctrl, m);
            cur = m;
          }
          ctrl = pt;
          has_ctrl = true;
        }
      }
      if (has_ctrl) { vec_quad(r, cur, ctrl, start); } else { vec_line(r, cur, start); }
    }
  }


  // move the cursor by the advance of a glyph, the cursor keeps the sub-pixels (no rounding error adds up)
  void vector_advance(const font::vector_charinfo_type& g)
  {
    const std::int32_t x = x_sub_ + vec_div(g.xdist * vector_scale(), 1024);
    x_act_ = static_cast<std::int16_t>(x_act_ + x / 64);
    x_sub_ = static_cast<std::uint8_t>(x % 64);
  }


  // render a char of the selected vector font at the actual cursor position, move the cursor
  void vector_out(std::uint16_t ch)
  {
    const font::vector_charinfo_type* g = vector_glyph(ch);
    if (!g) {
      return;
    }
    if (g->size == 0U) {
      // space
      vector_advance(*g);
      return;
    }
    // the glyph is hinted at its sub-pixel position: the stems are snapped to the pixel columns nearest to them
    vec_row_type r;
    r.scale    = vector_scale();
    r.base     = vec_round(font_->family.vector->ascent * r.scale) * 64;
    r.kx.o     = x_sub_;
    r.ky.o     = 0;
    r.contours = vec_hints(r.kx, r.scale, vec_hints(r.ky, r.scale, g->data, true), false);
    r.end      = g->data + g->size;
    r.ox       = 0;
    r.oy       = 0;

    // glyph box in pixel, the control points enclose the curves
    std::int32_t xmin = 0x3FFFFFFF, ymin = 0x3FFFFFFF, xmax = -0x3FFFFFFF, ymax = -0x3FFFFFFF;
    for (vec_contour_type c = { r.contours, r.end, nullptr, 0, 0, 0, 0 }; c.next(); ) {
      for (std::int32_t k = 0; k < c.n; ++k) {
        vec_point_type v;
        (void)vec_point(r, c, k, v);
        xmin = v.x < xmin ? v.x : xmin;
        xmax = v.x > xmax ? v.x : xmax;
        ymin = v.y < ymin ? v.y : ymin;
        ymax = v.y > ymax ? v.y : ymax;
      }
    }
    const std::int32_t px0 = vec_floor(xmin), px1 = vec_floor(xmax - 1);
    const std::int32_t py0 = vec_floor(ymin), py1 = vec_floor(ymax - 1);

    if ((px1 >= px0) && (py1 >= py0)) {
      std::int16_t cover[VIC_TXR_VECTOR_MAX_WIDTH + 1];
      r.ox    = px0 * 64;
      r.oy    = py0 * 64;
      r.width = px1 - px0 + 1;
      r.width = r.width > VIC_TXR_VECTOR_MAX_WIDTH ? VIC_TXR_VECTOR_MAX_WIDTH : r.width;
      r.cover = cover;
      const std::uint32_t color_alpha = color::get_alpha(color_);
      for (std::int32_t row = 0; row <= py1 - py0; ++row) {
        for (std::int32_t x = 0; x <= r.width; ++x) {
          cover[x] = 0;
        }
        r.top = row * 64;
        vec_contours(r);
        // running sum: coverage of the pixels
        std::int32_t s = 0;
        for (std::int32_t x = 0; x < r.width; ++x) {
          s += cover[x];
          const std::int32_t c = s < 0 ? (s < -64 ? 64 : -s) : (s > 64 ? 64 : s);
          if (c) {
            pixel_set({ static_cast<std::int16_t>(x_act_ + px0 + x), static_cast<std::int16_t>(y_act_ + py0 + row) },
                      color::set_alpha(color_, static_cast<std::uint8_t>(color_alpha * static_cast<std::uint32_t>(c) / 64U)));
          }
        }
      }
    }
    vector_advance(*g);
  }

};

} // namespace vic

#endif  // _VIC_TXR_H_
