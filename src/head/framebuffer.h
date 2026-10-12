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
// \brief Framebuffer head, a RAM framebuffer (with one or more planes) in front
// of any other head. The user decides whether a display needs one:
// - displays which can't read their pixels back (pixel_get() returns color::none)
//   get alpha blending, anti aliasing, sprites and fill() this way
// - displays with an autoincrementing GRAM window get the "render in RAM, send the
//   changed window in one stream" model: all drawing goes into the buffer, present()
//   transfers only the dirty rect with blit() to the bound head
// The storage format selects the RAM usage: ARGB8888 (4 byte/pixel, lossless),
// RGB565 (2), RGB332 (1) or L1 (1 bit/pixel), e.g. a 128 x 64 L1 buffer takes 1 KB.
//
// Usage:
// vic::head::ILI9325<240, 320> lcd(...);                          // the display
// vic::head::framebuffer<240, 320, 1, vic::color::format_RGB565> fb(lcd);   // 150 KB buffer
// vic::dc _dc(fb);                                                // draw on the framebuffer
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_FRAMEBUFFER_H_
#define _VIC_HEAD_FRAMEBUFFER_H_

#include "../drv.h"


// defines the head name and version
#define VIC_HEAD_FRAMEBUFFER_VERSION   "Framebuffer head 3.00"

namespace vic {
namespace head {


/**
 * Framebuffer head
 * \param Screen_Size_X Screen (buffer) width, must match the bound head
 * \param Screen_Size_Y Screen (buffer) height, must match the bound head
 * \param Plane_Count Number of planes, drawing and display plane can differ (double buffering)
 * \param Format Storage format of the buffer: format_ARGB8888, format_RGB565, format_RGB332 or format_L1
 */
template<std::uint16_t Screen_Size_X, std::uint16_t Screen_Size_Y, std::size_t Plane_Count = 1U, color::format_type Format = color::format_ARGB8888>
class framebuffer : public drv
{
  static_assert((Format == color::format_ARGB8888) || (Format == color::format_RGB565) ||
                (Format == color::format_RGB332)   || (Format == color::format_L1), "unsupported framebuffer format");
  static_assert(Plane_Count > 0U, "at least one plane is needed");

  static const std::size_t PIXELS      = static_cast<std::size_t>(Screen_Size_X) * Screen_Size_Y;
  static const std::size_t PLANE_BYTES = (Format == color::format_L1)     ? ((PIXELS + 7U) / 8U) :
                                         (Format == color::format_RGB332) ? PIXELS :
                                         (Format == color::format_RGB565) ? PIXELS * 2U : PIXELS * 4U;
  static const std::size_t LINE_PIXELS = Screen_Size_X < 64U ? Screen_Size_X : 64U;   // transfer chunk

public:

  /**
   * ctor
   * \param head The bound head
   */
  framebuffer(drv& head)
    : drv(Screen_Size_X, Screen_Size_Y,
          Screen_Size_X, Screen_Size_Y)
    , head_(head)
    , plane_active_(0U)
    , plane_display_(0U)
  {
    for (std::size_t p = 0U; p < Plane_Count; ++p) {
      clear_plane(p, color::none);
      dirty_[p].clear();
    }
  }


  /**
   * dtor
   */
  ~framebuffer()
  {
    shutdown();
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   D R I V E R   F U N C T I O N S

  virtual void init()
  {
    head_.init();
  }


  virtual void shutdown()
  {
    head_.shutdown();
  }


  virtual inline const char* version() const
  {
    return (const char*)VIC_HEAD_FRAMEBUFFER_VERSION;
  }


  virtual inline bool is_graphic() const
  {
    return head_.is_graphic();
  }


  /////////////////////////////////////////////////////////////////////////////
  // F R A M E B U F F E R

  /**
   * Set the given framebuffer plane as active display, the plane is transferred to the head
   * \param plane The index of the plane to display, 0 for 1st
   * \param alpha Unused
   * \return True if successful, false if the plane is invalid
   */
  virtual bool framebuffer_set_display(std::size_t plane, std::uint8_t alpha = 0U)
  {
    (void)alpha;
    if (plane >= Plane_Count) {
      return false;
    }
    if (plane_display_ != plane) {
      plane_display_ = plane;
      // the whole plane has to be transferred
      dirty_[plane] = { 0, 0, static_cast<std::int16_t>(Screen_Size_X), static_cast<std::int16_t>(Screen_Size_Y) };
      present();
    }
    return true;
  }


  /**
   * Use the given framebuffer plane as read/write buffer
   * \param plane The index of the plane to access, 0 for 1st
   * \return True if successful, false if the plane is invalid
   */
  virtual bool framebuffer_set_access(std::size_t plane)
  {
    if (plane >= Plane_Count) {
      return false;
    }
    plane_active_ = plane;
    return true;
  }


  /**
   * Returns the number of available framebuffer planes
   * \return Number of planes
   */
  virtual std::size_t framebuffer_get_count() const
  {
    return Plane_Count;
  }


  /**
   * Get the active (drawing) plane
   * \return Index of the active plane
   */
  inline std::size_t framebuffer_get_access() const
  {
    return plane_active_;
  }


  /**
   * Get the displayed plane
   * \return Index of the displayed plane
   */
  inline std::size_t framebuffer_get_display() const
  {
    return plane_display_;
  }


  /**
   * Get the dirty rect of the active plane: the area which is changed since the last transfer
   * \return The dirty rect, empty if nothing changed
   */
  inline rect_type dirty() const
  {
    return dirty_[plane_active_];
  }


  /**
   * Get the storage size of one plane
   * \return Bytes per plane
   */
  static inline std::size_t plane_size()
  {
    return PLANE_BYTES;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

protected:

  virtual void cls(color::value_type bg_color = color::none)
  {
    clear_plane(plane_active_, bg_color);
    dirty_[plane_active_].clear();
    if (plane_display_ == plane_active_) {
      // the head clears itself, nothing to transfer
      head_.cls(bg_color);
    }
  }


  /**
   * Transfer the dirty rect of the display plane to the head and present it
   */
  virtual void present()
  {
    rect_type& d = dirty_[plane_display_];
    if (!d.empty()) {
      for (std::int16_t y = d.top; y < d.bottom; ++y) {
        for (std::int16_t x = d.left; x < d.right; x = static_cast<std::int16_t>(x + static_cast<std::int16_t>(LINE_PIXELS))) {
          const std::size_t n = static_cast<std::size_t>(d.right - x) < LINE_PIXELS ? static_cast<std::size_t>(d.right - x) : LINE_PIXELS;
          for (std::size_t i = 0U; i < n; ++i) {
            line_[i] = load(plane_display_, static_cast<std::size_t>(y) * Screen_Size_X + static_cast<std::size_t>(x) + i);
          }
          head_.blit({ x, y, static_cast<std::int16_t>(x + static_cast<std::int16_t>(n)), static_cast<std::int16_t>(y + 1) }, line_, n);
        }
      }
      d.clear();
    }
    head_.present();
  }


  ///////////////////////////////////////////////////////////////////////////////
  // G R A P H I C   F U N C T I O N S
  //

  virtual void pixel_set(vertex_type vertex, color::value_type color)
  {
    if (!screen_is_inside(vertex)) {
      return;
    }
    store(plane_active_, static_cast<std::size_t>(vertex.y) * Screen_Size_X + static_cast<std::size_t>(vertex.x), color);
    dirty_inflate({ vertex.x, vertex.y, static_cast<std::int16_t>(vertex.x + 1), static_cast<std::int16_t>(vertex.y + 1) });
  }


  virtual color::value_type pixel_get(vertex_type vertex)
  {
    if (!screen_is_inside(vertex)) {
      return color::none;
    }
    return load(plane_active_, static_cast<std::size_t>(vertex.y) * Screen_Size_X + static_cast<std::size_t>(vertex.x));
  }


  virtual void line_horz(vertex_type v0, vertex_type v1, color::value_type color)
  {
    util::min_max_swap(v0.x, v1.x);
    if ((v0.y < 0) || (v0.y >= static_cast<std::int16_t>(Screen_Size_Y)) || (v1.x < 0) || (v0.x >= static_cast<std::int16_t>(Screen_Size_X))) {
      return;
    }
    if (v0.x < 0) { v0.x = 0; }
    if (v1.x >= static_cast<std::int16_t>(Screen_Size_X)) { v1.x = static_cast<std::int16_t>(Screen_Size_X - 1U); }
    const std::size_t row = static_cast<std::size_t>(v0.y) * Screen_Size_X;
    for (std::int_fast16_t x = v0.x; x <= v1.x; ++x) {
      store(plane_active_, row + static_cast<std::size_t>(x), color);
    }
    dirty_inflate({ v0.x, v0.y, static_cast<std::int16_t>(v1.x + 1), static_cast<std::int16_t>(v0.y + 1) });
  }


  virtual void line_vert(vertex_type v0, vertex_type v1, color::value_type color)
  {
    util::min_max_swap(v0.y, v1.y);
    if ((v0.x < 0) || (v0.x >= static_cast<std::int16_t>(Screen_Size_X)) || (v1.y < 0) || (v0.y >= static_cast<std::int16_t>(Screen_Size_Y))) {
      return;
    }
    if (v0.y < 0) { v0.y = 0; }
    if (v1.y >= static_cast<std::int16_t>(Screen_Size_Y)) { v1.y = static_cast<std::int16_t>(Screen_Size_Y - 1U); }
    for (std::int_fast16_t y = v0.y; y <= v1.y; ++y) {
      store(plane_active_, static_cast<std::size_t>(y) * Screen_Size_X + static_cast<std::size_t>(v0.x), color);
    }
    dirty_inflate({ v0.x, v0.y, static_cast<std::int16_t>(v0.x + 1), static_cast<std::int16_t>(v1.y + 1) });
  }


  virtual void box(const rect_type& rect, color::value_type color)
  {
    rect_type r = rect;
    if (r.left < 0) { r.left = 0; }
    if (r.top  < 0) { r.top  = 0; }
    if (r.right  > static_cast<std::int16_t>(Screen_Size_X)) { r.right  = static_cast<std::int16_t>(Screen_Size_X); }
    if (r.bottom > static_cast<std::int16_t>(Screen_Size_Y)) { r.bottom = static_cast<std::int16_t>(Screen_Size_Y); }
    if ((r.left >= r.right) || (r.top >= r.bottom)) {
      return;
    }
    for (std::int_fast16_t y = r.top; y < r.bottom; ++y) {
      const std::size_t row = static_cast<std::size_t>(y) * Screen_Size_X;
      for (std::int_fast16_t x = r.left; x < r.right; ++x) {
        store(plane_active_, row + static_cast<std::size_t>(x), color);
      }
    }
    dirty_inflate(r);
  }


  virtual void move(const rect_type& source, vertex_type destination)
  {
    const std::int16_t w = source.width(), h = source.height();
    if ((w <= 0) || (h <= 0)) {
      return;
    }
    // copy in the direction which doesn't overwrite unread source pixels
    const bool down  = destination.y > source.top;
    const bool right = destination.x > source.left;
    for (std::int_fast16_t j = 0; j < h; ++j) {
      const std::int_fast16_t dy = down ? h - 1 - j : j;
      for (std::int_fast16_t i = 0; i < w; ++i) {
        const std::int_fast16_t dx = right ? w - 1 - i : i;
        const vertex_type s = { static_cast<std::int16_t>(source.left + dx), static_cast<std::int16_t>(source.top + dy) };
        const vertex_type d = { static_cast<std::int16_t>(destination.x + dx), static_cast<std::int16_t>(destination.y + dy) };
        if (screen_is_inside(d)) {
          store(plane_active_, static_cast<std::size_t>(d.y) * Screen_Size_X + static_cast<std::size_t>(d.x),
                screen_is_inside(s) ? load(plane_active_, static_cast<std::size_t>(s.y) * Screen_Size_X + static_cast<std::size_t>(s.x)) : color::none);
        }
      }
    }
    rect_type dst = { destination.x, destination.y, static_cast<std::int16_t>(destination.x + w), static_cast<std::int16_t>(destination.y + h) };
    if (dst.left < 0) { dst.left = 0; }
    if (dst.top  < 0) { dst.top  = 0; }
    if (dst.right  > static_cast<std::int16_t>(Screen_Size_X)) { dst.right  = static_cast<std::int16_t>(Screen_Size_X); }
    if (dst.bottom > static_cast<std::int16_t>(Screen_Size_Y)) { dst.bottom = static_cast<std::int16_t>(Screen_Size_Y); }
    if ((dst.left < dst.right) && (dst.top < dst.bottom)) {
      dirty_inflate(dst);
    }
  }


  virtual void blit(const rect_type& rect, const color::value_type* pixels, std::size_t stride)
  {
    rect_type r = rect;
    std::size_t skip_x = 0U, skip_y = 0U;
    if (r.left < 0) { skip_x = static_cast<std::size_t>(-r.left); r.left = 0; }
    if (r.top  < 0) { skip_y = static_cast<std::size_t>(-r.top);  r.top  = 0; }
    if (r.right  > static_cast<std::int16_t>(Screen_Size_X)) { r.right  = static_cast<std::int16_t>(Screen_Size_X); }
    if (r.bottom > static_cast<std::int16_t>(Screen_Size_Y)) { r.bottom = static_cast<std::int16_t>(Screen_Size_Y); }
    if ((r.left >= r.right) || (r.top >= r.bottom)) {
      return;
    }
    const color::value_type* row = pixels + skip_y * stride + skip_x;
    for (std::int16_t y = r.top; y < r.bottom; ++y, row += stride) {
      const std::size_t idx = static_cast<std::size_t>(y) * Screen_Size_X;
      for (std::int16_t x = r.left; x < r.right; ++x) {
        store(plane_active_, idx + static_cast<std::size_t>(x), row[x - r.left]);
      }
    }
    dirty_inflate(r);
  }


private:

  // store a pixel in the plane storage format
  inline void store(std::size_t plane, std::size_t idx, color::value_type color)
  {
    std::uint8_t* const b = buffer_[plane];
    // Format is a constant template parameter, so the switch is optimized away
    switch (Format) {
      case color::format_L1 : {
        const std::uint8_t mask = static_cast<std::uint8_t>(1U << (idx & 7U));
        b[idx >> 3U] = static_cast<std::uint8_t>(color::color_to_L1(color) ? (b[idx >> 3U] | mask) : (b[idx >> 3U] & ~mask));
        break;
      }
      case color::format_RGB332 :
        b[idx] = color::color_to_RGB332(color);
        break;
      case color::format_RGB565 : {
        const std::uint16_t c = color::color_to_RGB565(color);
        b[idx * 2U]      = static_cast<std::uint8_t>(c);
        b[idx * 2U + 1U] = static_cast<std::uint8_t>(c >> 8U);
        break;
      }
      default :
        b[idx * 4U]      = static_cast<std::uint8_t>(color);
        b[idx * 4U + 1U] = static_cast<std::uint8_t>(color >> 8U);
        b[idx * 4U + 2U] = static_cast<std::uint8_t>(color >> 16U);
        b[idx * 4U + 3U] = static_cast<std::uint8_t>(color >> 24U);
        break;
    }
  }


  // load a pixel from the plane storage format
  inline color::value_type load(std::size_t plane, std::size_t idx) const
  {
    const std::uint8_t* const b = buffer_[plane];
    switch (Format) {
      case color::format_L1 :
        return color::L1_to_color(static_cast<std::uint8_t>((b[idx >> 3U] >> (idx & 7U)) & 1U));
      case color::format_RGB332 :
        return color::RGB332_to_color(b[idx]);
      case color::format_RGB565 :
        return color::RGB565_to_color(static_cast<std::uint16_t>(b[idx * 2U] | (static_cast<std::uint16_t>(b[idx * 2U + 1U]) << 8U)));
      default :
        return static_cast<color::value_type>(b[idx * 4U]) |
               (static_cast<color::value_type>(b[idx * 4U + 1U]) << 8U) |
               (static_cast<color::value_type>(b[idx * 4U + 2U]) << 16U) |
               (static_cast<color::value_type>(b[idx * 4U + 3U]) << 24U);
    }
  }


  inline void clear_plane(std::size_t plane, color::value_type bg_color)
  {
    for (std::size_t i = 0U; i < PIXELS; ++i) {
      store(plane, i, bg_color);
    }
  }


  // extend the dirty rect of the active plane by the given (screen clipped) rect
  inline void dirty_inflate(const rect_type& r)
  {
    rect_type& d = dirty_[plane_active_];
    if (d.empty()) {
      d = r;
    }
    else {
      if (r.left   < d.left)   { d.left   = r.left; }
      if (r.top    < d.top)    { d.top    = r.top; }
      if (r.right  > d.right)  { d.right  = r.right; }
      if (r.bottom > d.bottom) { d.bottom = r.bottom; }
    }
  }

  std::uint8_t      buffer_[Plane_Count][PLANE_BYTES];   // planes in the storage format, row major
  rect_type         dirty_[Plane_Count];                 // changed area per plane since the last transfer
  color::value_type line_[LINE_PIXELS];                  // transfer chunk
  drv&              head_;                               // bound head
  std::size_t       plane_active_;                       // drawing plane
  std::size_t       plane_display_;                      // displayed plane
};

} // namespace head
} // namespace vic

#endif  // _VIC_HEAD_FRAMEBUFFER_H_
