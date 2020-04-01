///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2016-2018, PALANDesign Hannover, Germany
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
// \brief Canvas and sheet sprite support
//
// sprite usage:
//
// vic::sprite::canvas<400, 1> greenball(_head, 5);   // create 400 pixel sprite object with one frame, z-index 5
// greenball.set_color(vic::color::green);            // set drawing color to green
// greenball.disc({ 10, 10 }, 10);                    // draw a green filled circle
// greenball.render({ 50, 10 });                      // render the sprite to 50,10
//
// support:
// - unlimited amount of sprites
// - z-order of sprites
//
// todo:
// - collision detection
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_SPRITE_H_
#define _VIC_SPRITE_H_

#include "dc.h"
#include "lib/avl_array.h"


namespace vic {
namespace sprite {


/**
 * Base class for canvas and sheet sprites
 */
class base : public dc
{
public:
  drv&          head_;        // head instance
  base*         next_;        // next sprite (with lower z-index) in chain
  std::int16_t  z_index_;     // z-index of this sprite
  std::uint16_t frame_;       // actual displayed frame
  vertex_type   position_;    // actual rendered position

protected:

  /**
   * ctor
   * \param head Reference to the head instance
   * \param z_index, Z-index of the sprite, 0 = default, bigger z-index moves to front, lower z-index to back
   */
  base(drv& head, std::int16_t z_index)
    : dc(head)
    , head_(head)
    , next_(nullptr)
    , z_index_(z_index)
    , frame_(0xFFFFU)       // invalid frame as default
    , position_({ 0, 0 })
  {
    // register sprite
    if (!*get_root()) {
      // set as root element
      *get_root() = this;
    }
    else {
      // insert sprite in sprite chain, the highest z-index is first (root), the lowest z-index is last
      if ((*get_root())->z_index_ < z_index_) {
        // exchange root element
        base* tmp   = *get_root();
        *get_root() = this;
        next_       = tmp;
        return;
      }
      for (base* b = *get_root(); !!b; b = b->next_) {
        if ((b->z_index_ < z_index_) || (!b->next_)) {
          // exchange/append list element
          base* tmp = b->next_;
          b->next_  = this;
          next_     = tmp;
          return;
        }
      }
    }
  }


  ~base()
  {
    // remove from list
    if (this == *get_root()) {
      // set as root element
      *get_root() = next_;
    }
    else {
      for (base* b = *get_root(); !!b; b = b->next_) {
        if (this == b->next_) {
          b->next_ = next_;
          return;
        }
      }
    }
  }

protected:
  inline base** get_root()
  {
    static base* _root = nullptr;
    return &_root;
  }

  inline base* get_next() const
  {
    return next_;
  }

public:
  // 'pattern' buffer access
  virtual bool pattern_it_get(std::uint16_t frame, pixel_type& pixel) = 0;
  virtual void pattern_it_next(std::uint16_t frame, bool begin) = 0;
  virtual bool pattern_find(std::uint16_t frame, pixel_type& pixel) = 0;

  // 'restore' buffer access
  virtual bool restore_it_get(pixel_type& pixel) = 0;
  virtual void restore_it_next(bool begin) = 0;
  virtual void restore_it_delete(void) = 0;
  virtual bool restore_find(pixel_type& pixel) = 0;
  virtual void restore_set(const pixel_type& pixel) = 0;


  /**
   * Render the sprite on the head
   * \param position Screen vertex where the origin of the sprite is rendered
   * \param frame Frame of the sprite to render
   */
  void render(vertex_type position, std::uint16_t frame = 0U)
  {
    position_ = position;
    frame_    = frame;

    // R E S T O R E   T H E   B A C K G R O U N D
    pixel_type bg_pixel;
    for (restore_it_next(false); restore_it_get(bg_pixel);)
    {
      // check if this pixel is processed by the new pattern
      pixel_type p = bg_pixel - position;   // bg_pixel.vertex contains the absolute screen position
      if (pattern_find(frame, p)) {
        // yes, pixel is still part of new pattern, no need to restore yet, just take next pixel
        restore_it_next(true);
        continue;
      }

      color::value_type mixed_color = color::none;

      // check if this background pixel is part of any other sprite
      bool bg_pixel_copied = false;
      for (base* sprite = *get_root(); !!sprite; sprite = sprite->next_)
      {
        // iterate through all sprites except the own one
        if (sprite == this) {
          continue;
        }
        // if no sprite is rendered at the pixel position, just restore the background
        p = bg_pixel - sprite->position_;
        if (sprite->pattern_find(sprite->frame_, p)) {
          // TBD: collision detection here

          // sprite needs to be rendered
          mixed_color = color::alpha_blend(mixed_color, p.color);
          if (!bg_pixel_copied) {
            // copy the bg to another sprite once, so that there is only one bg info
            sprite->restore_set(bg_pixel);
            bg_pixel_copied = true;
          }
          if (color::is_opaque(p.color)) {
            // pixel color is opaque, all further stuff behind is hidden
            break;
          }
        }
      }
      // restore pixel and mix it with background color
      // the shader pipe here can't be used here cause the background is handled here 
      head_.pixel_set(bg_pixel.vertex, color::alpha_blend(mixed_color, bg_pixel.color));

      // and delete pixel from the restore buffer, the iterator is invalidated by erase, so set it to next pixel
      restore_it_delete();
    }

    // R E N D E R   T H E   P A T T E R N
    pixel_type pt_pixel;
    for (pattern_it_next(frame, false); pattern_it_get(frame, pt_pixel); pattern_it_next(frame, true))
    {
      pt_pixel += position;   // pt_pixel.vertex contains now the new absolute screen position

      // process all sprites for vertex
      bg_pixel = { pt_pixel.vertex, color::none };
      color::value_type mixed_color = color::none;
      for (base* sprite = *get_root(); !!sprite; sprite = sprite->next_) {
        pixel_type p = pt_pixel - sprite->position_;
        if (sprite->pattern_find(sprite->frame_, p)) {
          // pixel needs to be rendered
          mixed_color = color::alpha_blend(mixed_color, p.color);
          // check if sprite has bg info of vertex
          if ((bg_pixel.color == color::none) && sprite->restore_find(bg_pixel)) {
            // sprite got bg info
            // if color is opaque all stuff behind is hidden and if bg_info is available, stop processing
            if ((bg_pixel.color != color::none) && color::is_opaque(mixed_color)) {
              break;
            }
          }
        }
      }

      // set background
      if (bg_pixel.color == color::none) {
        // get backgound
        bg_pixel.color = head_.pixel_get(bg_pixel.vertex);
      }
      restore_set(bg_pixel);

      // finally render the pixel on the head
      // the shader pipe here can't be used here cause the background is handled here
      head_.pixel_set(pt_pixel.vertex, color::alpha_blend(mixed_color, bg_pixel.color));
    }
  }
};



///////////////////////////////////////////////////////////////////////////////
// C A N V A S   S P R I T E
//

template <std::uint16_t Pixel>
class canvas_output : public shader::base
{
  typedef typename avl_array<vertex_type, color::value_type, std::uint16_t, Pixel, false> pattern_type;

  std::uint16_t&  frame_;
  pattern_type*   pattern_;
  rect_type*      pattern_bounding_;

public:
  /**
   * ctor
   */
  canvas_output(std::uint16_t& frame, pattern_type* pattern, rect_type* pattern_bounding)
    : frame_(frame)
    , pattern_(pattern)
    , pattern_bounding_(pattern_bounding)
  { }

  /**
   * Set pixel in given color
   * \param vertex Pixel coordinates
   * \param color Color of pixel in ARGB format
   */
  inline virtual void pixel_set(vertex_type vertex, color::value_type color) final
  {
    avl_array<vertex_type, color::value_type, std::uint16_t, Pixel, false>::iterator it = pattern_[frame_].find(vertex);
    // delete pixel, if color is color::none, e.g. to punch out pixels
    if (color == color::none) {
      pattern_[frame_].erase(it);
      return;
    }

    // modify/add pixel
    if (it != pattern_->end()) {
      // alpha blending and pixel modification
      *it = color::alpha_blend(color, *it);
    }
    else {
      // add pixel
      pattern_[frame_].insert(vertex, color);

      // adjust bounding box
      pattern_bounding_[frame_].inflate(vertex);
    }
  }

  /**
   * Return the color of the pixel
   * \param point Vertex of the pixel
   * \return Color of pixel in ARGB format
   */
  inline virtual color::value_type pixel_get(vertex_type vertex) final
  {
    // get pixel color
    color::value_type color = color::none;  // color::none as default if vertex is not found
    if (pattern_bounding_[frame_].contain(vertex)) {
      pattern_[frame_].find(vertex, color);
    }
    return color;
  }
};



template <std::uint16_t Pixel, std::uint16_t Frames>
class canvas : public base
{
  typedef typename avl_array<vertex_type, color::value_type, std::uint16_t, Pixel, false> pattern_type;
  typedef typename avl_array<vertex_type, color::value_type, std::uint16_t, Pixel, true>  restore_type;

  pattern_type pattern_[Frames];                // pattern (data) buffer
  restore_type restore_;                        // restore (background) buffer

  typename pattern_type::iterator pattern_it_;  // pattern iterator
  typename restore_type::iterator restore_it_;  // restore iterator

  rect_type pattern_bounding_[Frames];          // pattern bounding box
  canvas_output<Pixel> output_shader_;          // shader output
  std::uint16_t frame_edit_;                    // actual edit frame

public:
  /**
   * ctor
   */
  canvas(drv& head, std::int16_t z_index = 0)
    : base(head, z_index)
    , output_shader_(frame_edit_, pattern_, pattern_bounding_)
    , frame_edit_(0U)
  {
    // use own canvas output shader as shader pipeline output
    shader_pipe_ = &output_shader_;

    // init bounding boxes
    for (std::uint_fast16_t n = 0; n < Frames; ++n) {
      pattern_bounding_[n].clear();
    }
  }


  /**
   * Select the actual frame for editing
   * \param frame Edit frame
   */
  inline void select_frame(std::uint16_t frame)
  {
    frame_edit_ = frame;
  }


  /**
   * Clear the actual frame
   */
  void cls()
  {
    pattern_[frame_edit_].clear();
    pattern_bounding_[frame_edit_].clear();
  }


  virtual inline void pattern_it_next(std::uint16_t frame, bool next) final
  {
    if (next) {
      ++pattern_it_;
    }
    else {
      pattern_it_ = pattern_[frame].begin();
    }
  }

  virtual inline bool pattern_it_get(std::uint16_t frame, pixel_type& pixel) final
  {
    if (pattern_it_ != pattern_[frame].end()) {
      pixel = { pattern_it_.key(), pattern_it_.val() };
      return true;
    }
    return false;
  }

  virtual inline bool pattern_find(std::uint16_t frame, pixel_type& pixel) final
  {
    if ((frame == 0xFFFFU) || !pattern_bounding_[frame].contain(pixel.vertex)) {
      // frame invalid or pixel is out of canvas
      return false;
    }
    return pattern_[frame].find(pixel.vertex, pixel.color);
  }


  virtual inline void restore_it_next(bool next) final
  {
    if (next) {
      ++restore_it_;
    }
    else {
      restore_it_ = restore_.begin();
    }
  }

  virtual inline bool restore_it_get(pixel_type& pixel) final
  {    
    if (restore_it_ != restore_.end()) {
      pixel = { restore_it_.key(), restore_it_.val() };
      return true;
    }
    return false;
  }

  virtual inline void restore_it_delete() final
  {
    restore_type::iterator it = restore_it_++;
    if (restore_it_ != restore_.end()) {
      const vertex_type key = restore_it_.key();
      restore_.erase(it);
      restore_it_ = restore_.find(key);    // restore old iterator position
    }
    else {
      restore_.erase(it);
    }
  }

  virtual inline void restore_set(const pixel_type& pixel) final
  {
    restore_.insert(pixel.vertex, pixel.color);
  }

  virtual inline bool restore_find(pixel_type& pixel) final
  {
    return restore_.find(pixel.vertex, pixel.color);
  }
};



///////////////////////////////////////////////////////////////////////////////
// S H E E T   S P R I T E
//

typedef struct tag_sheet_info_type {
  std::uint16_t         width;          // width of sprite sheet
  std::uint16_t         height;         // height of sprite sheet
  std::uint16_t         sprite_width;   // width of sprite
  std::uint16_t         sprite_height;  // height of sprite
  color::value_type     bg_color;       // background color (transparent)
  color::format_type    format;         // supported formats are RBG565, RGB888, ARGB8888 and RGBA8888
  const std::uint8_t*   data;           // sprite sheet data
} sheet_info_type;


template <std::uint16_t Frame_Width, std::uint16_t Frame_Height>
class sheet : public base
{
  typedef typename avl_array<vertex_type, color::value_type, std::uint16_t, Frame_Width * Frame_Height, true> restore_type;

  restore_type restore_;                        // restore (background) buffer

  typename restore_type::iterator restore_it_;  // restore iterator

  std::uint32_t pattern_it_;                    // pattern iterator
  std::uint32_t pattern_size_;                  // pattern size
  rect_type     pattern_bounding_;              // pattern bounding box

  std::uint16_t frames_per_row;
  std::uint8_t  byte_per_pixel;

  const sheet_info_type* sheet_info_;           // sheet info

public:
  /**
   * ctor
   */
  sheet(drv& head, const sheet_info_type* sheet_info, std::int16_t z_index = 0)
    : base(head, z_index)
    , pattern_it_(0U)
    , sheet_info_(sheet_info)
  {
    // sheet constants
    byte_per_pixel    = static_cast<std::uint8_t>(sheet_info_->format) >> 4U;     // upper nibble of format is 'bytes/pixel'
    frames_per_row    = sheet_info_->width / sheet_info_->sprite_width;
    pattern_bounding_ = { 0, 0, static_cast<std::int16_t>(sheet_info_->sprite_width), static_cast<std::int16_t>(sheet_info_->sprite_height) };
    pattern_size_     = sheet_info_->sprite_width * sheet_info_->sprite_height;
  }


  virtual inline void pattern_it_next(std::uint16_t frame, bool next) final
  {
    if (next) {
      const std::uint16_t row = frame / frames_per_row * sheet_info_->sprite_height;
      const std::uint16_t col = frame % frames_per_row * sheet_info_->sprite_width;
      for (++pattern_it_; pattern_it_ != pattern_size_; ++pattern_it_) {
        const std::uint16_t x   = static_cast<std::uint16_t>(pattern_it_ % sheet_info_->sprite_width);
        const std::uint16_t y   = static_cast<std::uint16_t>(pattern_it_ / sheet_info_->sprite_width);
        const std::uint8_t* pos = sheet_info_->data + ((row + y) * sheet_info_->width + (col + x)) * byte_per_pixel;
        if (format_to_color(pos) != sheet_info_->bg_color) {
          return;
        }
      }
    }
    else {
      pattern_it_ = 0U;
    }
  }

  virtual inline bool pattern_it_get(std::uint16_t frame, pixel_type& pixel) final
  {
    if (pattern_it_ < pattern_size_) {
      // get pixel
      pixel.vertex.x          = static_cast<std::uint16_t>(pattern_it_ % sheet_info_->sprite_width);
      pixel.vertex.y          = static_cast<std::uint16_t>(pattern_it_ / sheet_info_->sprite_width);
      const std::uint16_t row = frame / frames_per_row * sheet_info_->sprite_height + pixel.vertex.y;
      const std::uint16_t col = frame % frames_per_row * sheet_info_->sprite_width  + pixel.vertex.x;
      const std::uint8_t* pos = sheet_info_->data + (row * sheet_info_->width + col) * byte_per_pixel;
      pixel.color = format_to_color(pos);
      return true;
    }
    return false;
  }

  virtual inline bool pattern_find(std::uint16_t frame, pixel_type& pixel) final
  {
    if ((frame == 0xFFFFU) || !pattern_bounding_.contain(pixel.vertex)) {
      // frame invalid or pixel is out of sheet frame
      return false;
    }

    const std::uint16_t row = frame / frames_per_row * sheet_info_->sprite_height + pixel.vertex.y;
    const std::uint16_t col = frame % frames_per_row * sheet_info_->sprite_width  + pixel.vertex.x;
    const std::uint8_t* pos = sheet_info_->data + (row * sheet_info_->width + col) * byte_per_pixel;
    pixel.color = format_to_color(pos);
    return pixel.color != sheet_info_->bg_color;
  }


  virtual inline void restore_it_next(bool next) final
  {
    if (next) {
      ++restore_it_;
    }
    else {
      restore_it_ = restore_.begin();
    }
  }

  virtual inline bool restore_it_get(pixel_type& pixel) final
  {    
    if (restore_it_ != restore_.end()) {
      pixel = { restore_it_.key(), restore_it_.val() };
      return true;
    }
    return false;
  }

  virtual inline void restore_it_delete() final
  {
    restore_type::iterator it = restore_it_++;
    if (restore_it_ != restore_.end()) {
      const vertex_type key = restore_it_.key();
      restore_.erase(it);
      restore_it_ = restore_.find(key);    // restore old iterator position
    }
    else {
      restore_.erase(it);
    }
  }

  virtual inline void restore_set(const pixel_type& pixel) final
  {
    restore_.insert(pixel.vertex, pixel.color);
  }

  virtual inline bool restore_find(pixel_type& pixel) final
  {
    return restore_.find(pixel.vertex, pixel.color);
  }

private:
  // format conversion
  // the format color is expected to have the LSB (least significant byte) FIRST
  color::value_type format_to_color(const std::uint8_t* format_color)
  {
    switch (sheet_info_->format)
    {
      case color::format_RGB332   :
        return color::RGB332_to_color(*format_color);
      case color::format_RGB565   : {
        const std::uint16_t clr = (static_cast<std::uint16_t>(*format_color)) |
                                  (static_cast<std::uint16_t>(*(format_color + 1U)) << 8U);
        return color::RGB565_to_color(clr);
      }
      case color::format_RGB888   : return color::set_alpha(*reinterpret_cast<const color::value_type*>(format_color), 255);
      case color::format_RGBA8888 : return *reinterpret_cast<const color::value_type*>(format_color) >> 8U | ((*reinterpret_cast<const color::value_type*>(format_color) & 0xFFU) << 24U);
      default                     : return *reinterpret_cast<const color::value_type*>(format_color);
    }
  }
};



///////////////////////////////////////////////////////////////////////////////
// C A N V A S   S H E E T   S P R I T E
//

/**
 * Canvas sheet class that combines a canvas sprite with support for sheet rendering
 * \param Pixel At least "sheet_info_->sprite_width * sheet_info_->sprite_height" to hold the sprite. Increase value for zoom, rotation etc.
 */
template <std::uint16_t Pixel>
class canvas_sheet : public canvas<Pixel, 1U>
{
  std::uint16_t frames_per_row;
  std::uint8_t  byte_per_pixel;

  const sheet_info_type* sheet_info_;   // sheet info

public:
  /**
   * ctor
   */
  canvas_sheet(drv& head, const sheet_info_type* sheet_info, std::int16_t z_index = 0)
    : canvas(head, z_index)
    , sheet_info_(sheet_info)
  {
    // sheet constants
    byte_per_pixel = static_cast<std::uint8_t>(sheet_info_->format) >> 4U;     // upper nibble of format is 'bytes/pixel'
    frames_per_row = sheet_info_->width / sheet_info_->sprite_width;
  }


  /**
   * Render the sprite on the head
   * \param position Screen vertex where the origin of the sprite is rendered
   * \param frame Frame of the sheet to render
   */
  void render(vertex_type position, std::uint16_t frame = 0U)
  {
    // clear buffer
    cls();

    // load the sheet in the sprite
    const std::uint16_t row = frame / frames_per_row * sheet_info_->sprite_height;
    const std::uint16_t col = frame % frames_per_row * sheet_info_->sprite_width;
    for (std::int_fast16_t y = 0; y < sheet_info_->sprite_height; ++y) {
      for (std::int_fast16_t x = 0; x < sheet_info_->sprite_width; ++x) {
        const std::uint8_t* pos = sheet_info_->data + ((row + y) * sheet_info_->width + (col + x)) * byte_per_pixel;
        const color::value_type c = format_to_color(pos);
        if (c != sheet_info_->bg_color) {
          pixel_set(vertex_type({ static_cast<std::int16_t>(x), static_cast<std::int16_t>(y) }), c);
        }
      }
    }

    // call the base class
    canvas::render(position, 0U);
  }

private:
  // format conversion
  // the format color is expected to have the LSB (least significant byte) FIRST
  color::value_type format_to_color(const std::uint8_t* format_color)
  {
    switch (sheet_info_->format)
    {
      case color::format_RGB332   :
        return color::RGB332_to_color(*format_color);
      case color::format_RGB565   : {
        const std::uint16_t clr = (static_cast<std::uint16_t>(*format_color)) |
                                  (static_cast<std::uint16_t>(*(format_color + 1U)) << 8U);
        return color::RGB565_to_color(clr);
      }
      case color::format_RGB888   : return color::set_alpha(*reinterpret_cast<const color::value_type*>(format_color), 255);
      case color::format_RGBA8888 : return *reinterpret_cast<const color::value_type*>(format_color) >> 8U | ((*reinterpret_cast<const color::value_type*>(format_color) & 0xFFU) << 24U);
      default                     : return *reinterpret_cast<const color::value_type*>(format_color);
    }
  }
};


} // namespace sprite
} // namespace vic

#endif  // _VIC_SPRITE_H
