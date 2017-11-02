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
// \brief Sprite support - WIP - DON'T USE YET!
//
// sprite usage:
//
// vic::sprite<100, 8> my_sprite(_head0);   // create a sprite with 100 points and 8 planes on head0
// my_sprite.triangle_solid(10,10,20,20,30,30);
// my_sprite.render( 0, 0, 0);
// my_sprite.render(10, 0, 1);
//
// support:
// - configurable amount of sprites
// - z-order of sprites
// - collision detection
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_SPRITE_H_
#define _VIC_SPRITE_H_

#include "dc.h"
#include "util/avl_array.h"


namespace vic {
namespace sprite {


/**
 * Base class for canvas and sheet sprites
 */
class base
{
public:
  drv&  head_;                // head instance
  base* next_;                // next sprite (with lower z-index) in chain
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
    : head_(head)
    , next_(nullptr)
    , z_index_(z_index)
    , frame_(0U)
    , position_({ 0,0 })
  {
    // register sprite
    if (!*get_root()) {
      // set as root element
      *get_root() = this;
    }
    else {
      // insert sprite in sprite chain, the highest z-index is first (root), the lowest z-index is last
      for (base* b = *get_root(); b != nullptr; b = b->next_) {
        if ((b->z_index_ < z_index_) || (!b->next_)) {
          if (b == *get_root()) {
            // exchange root element
            base* tmp   = *get_root();
            *get_root() = this;
            next_       = tmp;
            return;
          }
          else {
            // exchange/append list element
            base* tmp = b->next_;
            b->next_  = this;
            next_     = tmp;
            return;
          }
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
      for (base* b = *get_root(); b != nullptr; b = b->next_) {
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

  inline base* get_next()
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


  // select the actual frame
  inline void select_frame(std::uint16_t frame)
  {
    frame_ = frame;
  }


  /**
   * Render the sprite
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
      for (base* sprite = *get_root(); sprite != nullptr; sprite = sprite->next_) {
        // iterate through all sprites except the own one
        if (sprite == this) {
          continue;
        }
        // if no sprite is rendered at the pixel position, just restore the background
        p = bg_pixel - sprite->position_;
        if (sprite->pattern_find(sprite->frame_, p)) {
          // collision

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
      for (base* sprite = *get_root(); sprite != nullptr; sprite = sprite->next_) {
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
  std::uint16_t& frame_;
  avl_array<vertex_type, color::value_type, std::uint16_t, Pixel, false>* pattern_;
  rect_type* pattern_bounding_;

public:
  /**
   * ctor
   */
  canvas_output(std::uint16_t& frame, avl_array<vertex_type, color::value_type, std::uint16_t, Pixel, false>* pattern, rect_type* pattern_bounding)
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
    if (it != pattern_->end()) {
      // alpha blending and pixel modification
      *it = color::alpha_blend(color, *it);
    }
    else {
      // add pixel
      pattern_[frame_].insert(vertex, color);

      // adjust bounding box
      if (vertex.x > pattern_bounding_[frame_].right) {
        pattern_bounding_[frame_].right = vertex.x;
      }
      else if (vertex.x < pattern_bounding_[frame_].left) {
        pattern_bounding_[frame_].left = vertex.x;
      }
      if (vertex.y > pattern_bounding_[frame_].bottom) {
        pattern_bounding_[frame_].bottom = vertex.y;
      }
      else if (vertex.y < pattern_bounding_[frame_].top) {
        pattern_bounding_[frame_].top = vertex.y;
      }
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
class canvas : public base, public dc
{
  typedef typename avl_array<vertex_type, color::value_type, std::uint16_t, Pixel, false>::iterator pattern_iterator_type;
  typedef typename avl_array<vertex_type, color::value_type, std::uint16_t, Pixel, true>::iterator  restore_iterator_type;

  avl_array<vertex_type, color::value_type, std::uint16_t, Pixel, false>pattern_[Frames];   // pattern (data) buffer
  avl_array<vertex_type, color::value_type, std::uint16_t, Pixel, true> restore_;           // restore (background) buffer

  pattern_iterator_type pattern_it_;    // pattern iterator
  restore_iterator_type restore_it_;    // restore iterator

  rect_type pattern_bounding_[Frames];  // pattern bounding box

  canvas_output<Pixel> output_shader_;

public:
  /**
   * ctor
   */
  canvas(drv& head, std::int16_t z_index = 0)
    : base(head, z_index)
    , dc(head)
    , output_shader_(frame_, pattern_, pattern_bounding_)
  {
    // use own canvas output shader as shader pipeline output
    shader_pipe_ = &output_shader_;

    // init bounding boxes
    for (std::size_t n = 0; n < Frames; ++n) {
      pattern_bounding_[n].clear();
    }
  }


  void cls()
  {
    pattern_[frame_].clear();
    pattern_bounding_[frame_].clear();
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
    if (!pattern_bounding_[frame].contain(pixel.vertex)) {
      // pixel is out of canvas
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
    restore_iterator_type it = restore_it_++;
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
  std::uint16_t           width;              // width of sprite sheet
  std::uint16_t           height;             // height of sprite sheet
  std::uint16_t           sprite_width;       // width of sprite
  std::uint16_t           sprite_height;      // height of sprite
  color::value_type       bg_color;           // background color (transparent)
  color::format_type      format;             // supported formats are RBG565, RGB888, ARGB8888 and RGBA8888
  const std::uint8_t*     data;               // sprite sheet data
} sheet_info_type;


template <std::uint16_t Frame_Width, std::uint16_t Frame_Height>
class sheet : public base, public dc
{
  std::size_t pattern_it_;      // pattern iterator
  std::size_t pattern_size_;    // pattern size (vertexes)

  typedef typename avl_array<vertex_type, color::value_type, std::uint16_t, Frame_Width * Frame_Height, true>::iterator  restore_iterator_type;
  avl_array<vertex_type, color::value_type, std::uint16_t, Frame_Width * Frame_Height, true> restore_;           // restore (background) buffer
  restore_iterator_type restore_it_;    // restore iterator

  const sheet_info_type* sheet_info_;

  std::uint16_t frames_per_row;
  std::uint8_t  byte_per_pixel;

  rect_type pattern_bounding_;        // pattern bounding box

public:
  /**
   * ctor
   */
  sheet(drv& head, const sheet_info_type* sheet_info, std::int16_t z_index = 0)
    : base(head, z_index)
    , dc(head)
    , sheet_info_(sheet_info)
  {
    // sheet constants
    byte_per_pixel = 2;  // sheet_info_->format;
    frames_per_row    = sheet_info_->width / sheet_info_->sprite_width;
    pattern_bounding_ = { 0, 0, static_cast<std::int16_t>(sheet_info_->sprite_width - 1U), static_cast<std::int16_t>(sheet_info_->sprite_height - 1U) };
    pattern_size_     = sheet_info_->sprite_width * sheet_info_->sprite_height;
  }


  virtual inline void pattern_it_next(std::uint16_t frame, bool next) final
  {
    if (next) {
      for (++pattern_it_; pattern_it_ != pattern_size_; ++pattern_it_) {
        const std::uint16_t x   = static_cast<std::uint16_t>(pattern_it_ % sheet_info_->sprite_width);
        const std::uint16_t y   = static_cast<std::uint16_t>(pattern_it_ / sheet_info_->sprite_width);
        const std::uint16_t row = frame / frames_per_row * sheet_info_->sprite_height + y;
        const std::uint16_t col = frame % frames_per_row * sheet_info_->sprite_width  + x;
        const std::uint8_t* pos = sheet_info_->data + (row * sheet_info_->width + col) * byte_per_pixel;
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
    if (!pattern_bounding_.contain(pixel.vertex)) {
      // pixel is out of sheet frame
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
    restore_iterator_type it = restore_it_++;
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
  color::value_type format_to_color(const std::uint8_t* format_color)
  {
    switch (sheet_info_->format)
    {
      case color::format_RGB332   : return color::argb(static_cast<std::uint8_t>(*format_color & 0xE0U), static_cast<std::uint8_t>((*format_color & 0x1CU) << 3U), static_cast<std::uint8_t>((*format_color & 0x03U) << 6U));
      case color::format_RGB565   : return color::argb(static_cast<std::uint8_t>((*reinterpret_cast<const std::uint16_t*>(format_color) & 0xF800U) >> 8U), static_cast<std::uint8_t>((*reinterpret_cast<const std::uint16_t*>(format_color) & 0x07E0U) >> 3U), static_cast<std::uint8_t>((*reinterpret_cast<const std::uint16_t*>(format_color) & 0x001FU) << 3U));
      case color::format_RGB888   : return color::set_alpha(*reinterpret_cast<const color::value_type*>(format_color), 255);
      case color::format_RGBA8888 : return *reinterpret_cast<const color::value_type*>(format_color) >> 8U | ((*reinterpret_cast<const color::value_type*>(format_color) & 0xFFU) << 24U);
      default                     : return *reinterpret_cast<const color::value_type*>(format_color);
    }
  }

};


} // namespace sprite
} // namespace vic

#endif  // _VIC_SPRITE_H