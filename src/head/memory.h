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
// \brief Memory (RAM) head
// Stores every pixel as full 32 bit ARGB value (the alpha channel is kept) and
// counts present(), cls() and pixel_set() calls.
// Use this head as off-screen rendering buffer or as reference head in unit tests.
// It deliberately implements only the mandatory pixel_set()/pixel_get() functions,
// so that all drv fallback functions (line_horz, box, move ...) are exercised.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_MEMORY_H_
#define _VIC_HEAD_MEMORY_H_

#include "../drv.h"


// defines the head name and version
#define VIC_HEAD_MEMORY_VERSION   "Memory head 1.00"


namespace vic {
namespace head {


/**
 * Memory head
 * \param Screen_Size_X Screen (buffer) width
 * \param Screen_Size_Y Screen (buffer) height
 * \param Viewport_Size_X Viewport width
 * \param Viewport_Size_Y Viewport height
 */
template<std::uint16_t Screen_Size_X, std::uint16_t Screen_Size_Y,
         std::uint16_t Viewport_Size_X = Screen_Size_X, std::uint16_t Viewport_Size_Y = Screen_Size_Y>
class memory : public drv
{
public:
  typedef std::uint32_t counter_type;

  /**
   * ctor
   * \param viewport_x Viewport x position (left) on the screen, relative to top/left corner
   * \param viewport_y Viewport y position (top)  on the screen, relative to top/left corner
   */
  memory(std::int16_t viewport_x = 0, std::int16_t viewport_y = 0)
    : drv(Screen_Size_X, Screen_Size_Y,
          Viewport_Size_X, Viewport_Size_Y,
          viewport_x, viewport_y,
          orientation_0)
    , present_count_(0U)
    , cls_count_(0U)
    , pixel_set_count_(0U)
    , oob_count_(0U)
    , blit_count_(0U)
    , blit_pixels_(0U)
    , cls_color_(color::none)
  {
    // define the buffer content from the start
    for (std::size_t i = 0U; i < sizeof(frame_buffer_) / sizeof(frame_buffer_[0]); ++i) {
      frame_buffer_[i] = color::none;
    }
  }


  /**
   * dtor
   */
  ~memory()
  {
    shutdown();
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   D R I V E R   F U N C T I O N S

  virtual void init()
  {
    cls();
  }


  virtual void shutdown()
  { }


  inline virtual const char* version() const
  {
    return (const char*)VIC_HEAD_MEMORY_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    return true;
  }


  /////////////////////////////////////////////////////////////////////////////
  // I N S P E C T I O N   I N T E R F A C E
  //
  // These functions are public and const, they don't modify any counter

  /**
   * Access the raw frame buffer, row major, Screen_Size_X * Screen_Size_Y entries
   * \return Pointer to the ARGB frame buffer
   */
  inline const color::value_type* buffer() const
  {
    return frame_buffer_;
  }


  /**
   * Get the color of a pixel
   * \param vertex Pixel coordinates
   * \return Color of pixel in ARGB format, color::none if the vertex is outside the screen
   */
  inline color::value_type pixel(vertex_type vertex) const
  {
    return screen_is_inside(vertex) ? frame_buffer_[static_cast<std::size_t>(vertex.y) * Screen_Size_X + static_cast<std::size_t>(vertex.x)] : color::none;
  }


  // number of present() calls
  inline counter_type present_count() const
  { return present_count_; }

  // number of cls() calls
  inline counter_type cls_count() const
  { return cls_count_; }

  // number of accepted (inside screen) pixel_set() calls
  inline counter_type pixel_set_count() const
  { return pixel_set_count_; }

  // number of rejected (outside screen) pixel_set() calls
  inline counter_type oob_count() const
  { return oob_count_; }

  // number of blit() calls
  inline counter_type blit_count() const
  { return blit_count_; }

  // number of pixels written by blit() calls (inside the screen)
  inline counter_type blit_pixels() const
  { return blit_pixels_; }

  // background color of the last cls() call
  inline color::value_type cls_color() const
  { return cls_color_; }

  // reset all counters
  inline void reset_counters()
  {
    present_count_ = cls_count_ = pixel_set_count_ = oob_count_ = blit_count_ = blit_pixels_ = 0U;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

protected:

  virtual void cls(color::value_type bg_color = color::none)
  {
    for (std::size_t i = 0U; i < sizeof(frame_buffer_) / sizeof(frame_buffer_[0]); ++i) {
      frame_buffer_[i] = bg_color;
    }
    cls_color_ = bg_color;
    ++cls_count_;
  }


  virtual void present()
  {
    ++present_count_;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // G R A P H I C   F U N C T I O N S
  //

  virtual void pixel_set(vertex_type vertex, color::value_type color)
  {
    // check limits
    if (!screen_is_inside(vertex)) {
      // out of bounds
      ++oob_count_;
      return;
    }

    frame_buffer_[static_cast<std::size_t>(vertex.y) * Screen_Size_X + static_cast<std::size_t>(vertex.x)] = color;
    ++pixel_set_count_;
  }


  virtual color::value_type pixel_get(vertex_type vertex)
  {
    return pixel(vertex);
  }


  virtual void blit(const rect_type& rect, const color::value_type* pixels, std::size_t stride)
  {
    ++blit_count_;
    // clip to the screen and copy the rows
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
      color::value_type* dst = &frame_buffer_[static_cast<std::size_t>(y) * Screen_Size_X + static_cast<std::size_t>(r.left)];
      for (std::int16_t x = r.left; x < r.right; ++x) {
        *dst++ = row[x - r.left];
      }
      blit_pixels_ += static_cast<counter_type>(r.right - r.left);
    }
  }


private:
  color::value_type frame_buffer_[static_cast<std::size_t>(Screen_Size_X) * Screen_Size_Y];  // ARGB frame buffer
  counter_type      present_count_;     // number of present() calls
  counter_type      cls_count_;         // number of cls() calls
  counter_type      pixel_set_count_;   // number of accepted pixel_set() calls
  counter_type      oob_count_;         // number of rejected (out of bounds) pixel_set() calls
  counter_type      blit_count_;        // number of blit() calls
  counter_type      blit_pixels_;       // number of pixels written by blit()
  color::value_type cls_color_;         // background color of the last cls()
};

} // namespace head
} // namespace vic

#endif  // _VIC_HEAD_MEMORY_H_
