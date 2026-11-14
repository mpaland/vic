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
// \brief Sprite classes, a canvas sprite is a dc where every rendered pixel is
// stored in a pattern buffer. Sprites restore their background when they are
// moved and respect each other's z-order. Unlimited amount of sprites are supported.
//
// vic::sprite::canvas<400, 1> greenball(_head, 5);   // create 400 pixel sprite object with one frame, z-index 5
// greenball.set_color(vic::color::green);            // set drawing color to green
// greenball.disc({ 10, 10 }, 10);                    // draw a green filled circle
// greenball.render({ 50, 10 });                      // render the sprite to 50,10
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_SPRITE_H_
#define _VIC_SPRITE_H_

#include "dc.h"
#include "lib/avl_array.h"


namespace vic {
namespace sprite {


/**
 * Sprite sheet description
 */
typedef struct tag_sheet_info_type {
  std::uint16_t         width;          // width of sprite sheet
  std::uint16_t         height;         // height of sprite sheet
  std::uint16_t         sprite_width;   // width of sprite
  std::uint16_t         sprite_height;  // height of sprite
  color::value_type     bg_color;       // background color (transparent), compared against the DECODED pixel color
  color::format_type    format;         // supported formats are RGB332, RBG565, RGB888, ARGB8888 and RGBA8888
  const std::uint8_t*   data;           // sprite sheet data
} sheet_info_type;


/**
 * Decode one pixel of a sprite sheet
 * The pixel is read byte wise (no alignment or endianess requirements), the least significant byte comes first
 * \param info The sheet info
 * \param p Pointer to the pixel data
 * \return The pixel color in ARGB format
 */
inline color::value_type sheet_pixel_color(const sheet_info_type& info, const std::uint8_t* p)
{
  switch (info.format)
  {
    case color::format_RGB332   :
      return color::RGB332_to_color(p[0]);
    case color::format_RGB565   :
      return color::RGB565_to_color(static_cast<std::uint16_t>(p[0] | (static_cast<std::uint16_t>(p[1]) << 8U)));
    case color::format_RGB888   :
      return color::argb(p[2], p[1], p[0]);           // B G R
    case color::format_RGBA8888 :
      return color::argb(p[3], p[2], p[1], p[0]);     // A B G R
    case color::format_ARGB8888 :
    default                     :
      return color::argb(p[2], p[1], p[0], p[3]);     // B G R A
  }
}


/**
 * Base class for canvas and sheet sprites
 */
class base : public dc
{
public:
  /**
   * Collision handler
   * \param self The sprite which is rendered and detected the collision
   * \param other The sprite which overlaps with self
   * \param arg The argument given in set_collision_handler()
   */
  typedef void (*collision_handler_type)(base& self, base& other, void* arg);

protected:

  drv&          head_;        // head instance
  base*         next_;        // next sprite (with lower z-index) in chain
  std::int16_t  z_index_;     // z-index of this sprite
  std::uint16_t frame_;       // actual displayed frame
  vertex_type   position_;    // actual rendered position

  collision_handler_type collision_handler_;   // collision handler, nullptr disables the collision detection
  void*                  collision_arg_;       // argument for the collision handler
  base*                  collision_;           // first sprite hit in the last render, nullptr if none
  bool                   collision_hit_;       // transient mark: this sprite overlaps with the sprite being rendered


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
    , collision_handler_(nullptr)
    , collision_arg_(nullptr)
    , collision_(nullptr)
    , collision_hit_(false)
  {
    // register the sprite in the chain: the highest z-index is first (root), the lowest z-index is last
    // sprites with the same z-index are placed behind the existing ones
    base* root = *get_root();
    if (!root || (root->z_index_ < z_index_)) {
      // new root element
      next_       = root;
      *get_root() = this;
      return;
    }
    for (base* b = root; ; b = b->next_) {
      if (!b->next_ || (b->next_->z_index_ < z_index_)) {
        // insert behind b
        next_    = b->next_;
        b->next_ = this;
        return;
      }
    }
  }


  ~base()
  {
    // remove from list
    if (this == *get_root()) {
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

  /**
   * Drawing on a sprite only modifies the pattern buffer, so the head is not
   * presented by the primitives. The head is presented once by render()
   */
  inline virtual void present()
  { }


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
   * Get the actual rendered position
   * \return The position of the sprite origin on the screen
   */
  inline vertex_type get_position() const
  {
    return position_;
  }


  /**
   * Get the z-index
   * \return The z-index of the sprite
   */
  inline std::int16_t get_z_index() const
  {
    return z_index_;
  }


  /**
   * Set the collision handler, this enables the pixel exact collision detection for this sprite
   * The detection is done in render(): the handler is called once for every other sprite which has at least one
   * (non background) pixel at the same screen position as this sprite, after the pixels are rendered and before the
   * head is presented. Translucent pixels count, the real (rotated) shape of both sprites is used.
   * Only the rendered sprite reports: when sprite A is rendered and overlaps with sprite B, the handler of A is
   * called with other = B, the handler of B is not called (B reports when B is rendered).
   * The handler may change colors, frames or own state, it must not destroy sprites or render the reporting sprite.
   * Without a handler the detection is off and costs nothing.
   * \param handler The collision handler, nullptr disables the detection
   * \param arg Argument which is passed to the handler
   */
  inline void set_collision_handler(collision_handler_type handler, void* arg = nullptr)
  {
    collision_handler_ = handler;
    collision_arg_     = arg;
    collision_         = nullptr;
  }


  /**
   * Get the collision of the last render() (polling alternative to the handler)
   * \return The first colliding sprite (the one with the highest z-index), nullptr if there was no collision or if
   *         no collision handler is set
   */
  inline base* get_collision() const
  {
    return collision_;
  }


  /**
   * Render the sprite on the head
   * \param position Screen vertex where the origin of the sprite is rendered
   * \param frame Frame of the sprite to render
   */
  void render(vertex_type position, std::uint16_t frame = 0U)
  {
    position_  = position;
    frame_     = frame;
    collision_ = nullptr;
    const bool detect_collision = collision_handler_ != nullptr;

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
      bool blend_done = false;    // true if the pixel color is complete, further sprites are only probed for collisions
      bool bg_found   = false;    // true if a sprite holds the background of the vertex (which may be color::none / transparent)
      for (base* sprite = *get_root(); !!sprite; sprite = sprite->next_) {
        pixel_type p = pt_pixel - sprite->position_;
        if (sprite->pattern_find(sprite->frame_, p)) {
          if (sprite != this) {
            // another sprite has a pixel here: mark it as collision partner (once per sprite, not per pixel)
            sprite->collision_hit_ = detect_collision;
          }
          if (blend_done) {
            continue;
          }
          // pixel needs to be rendered
          mixed_color = color::alpha_blend(mixed_color, p.color);
          // check if sprite has bg info of vertex
          if (!bg_found && sprite->restore_find(bg_pixel)) {
            // sprite got bg info, the stored background may be color::none (transparent), so don't test the color
            bg_found = true;
            // if color is opaque all stuff behind is hidden and if bg_info is available, stop processing
            if (color::is_opaque(mixed_color)) {
              if (!detect_collision) {
                break;
              }
              // sprites behind an opaque pixel are still collision partners
              blend_done = true;
            }
          }
        }
      }

      // set background
      if (!bg_found) {
        // get backgound
        bg_pixel.color = head_.pixel_get(bg_pixel.vertex);
        if (bg_pixel.color == color::none) {
          // the head can't read its pixels, take the background color of the sprite context
          bg_pixel.color = get_bg_color();
        }
      }
      restore_set(bg_pixel);

      // finally render the pixel on the head
      // the shader pipe here can't be used here cause the background is handled here
      head_.pixel_set(pt_pixel.vertex, color::alpha_blend(mixed_color, bg_pixel.color));
    }

    // C O L L I S I O N S  -  report every marked sprite once, the chain order is the z-order
    if (detect_collision) {
      for (base* sprite = *get_root(); !!sprite; sprite = sprite->next_) {
        if (sprite->collision_hit_) {
          sprite->collision_hit_ = false;
          if (!collision_) {
            collision_ = sprite;
          }
          collision_handler_(*this, *sprite, collision_arg_);
        }
      }
    }

    head_.present();
  }
};



///////////////////////////////////////////////////////////////////////////////
// C A N V A S   S P R I T E
//

/**
 * Output shader of a canvas sprite, stores all rendered pixels in the pattern buffer of the edit frame
 */
template <std::uint16_t Pixel>
class canvas_output : public shader::base
{
  typedef avl_array<vertex_type, color::value_type, std::uint16_t, Pixel, false> pattern_type;

  std::uint16_t&  frame_;
  pattern_type*   pattern_;
  rect_type*      pattern_bounding_;

public:
  /**
   * ctor
   * \param frame Reference to the edit frame
   * \param pattern Array of the pattern buffers
   * \param pattern_bounding Array of the pattern bounding boxes
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
    pattern_type& pattern = pattern_[frame_];
    typename pattern_type::iterator it = pattern.find(vertex);

    // delete pixel, if color is color::none, e.g. to punch out pixels
    if (color == color::none) {
      if (it != pattern.end()) {
        pattern.erase(it);
      }
      return;
    }

    if (it != pattern.end()) {
      // alpha blending and pixel modification
      *it = color::alpha_blend(color, *it);
    }
    else {
      // add pixel
      const bool first = pattern.empty();
      if (pattern.insert(vertex, color)) {
        // adjust bounding box
        if (first) {
          pattern_bounding_[frame_] = { vertex.x, vertex.y, static_cast<std::int16_t>(vertex.x + 1), static_cast<std::int16_t>(vertex.y + 1) };
        }
        else {
          pattern_bounding_[frame_].inflate(vertex);
        }
      }
    }
  }

  /**
   * Return the color of the pixel
   * \param vertex Vertex of the pixel
   * \return Color of pixel in ARGB format, color::none if the vertex is not part of the pattern
   */
  inline virtual color::value_type pixel_get(vertex_type vertex) final
  {
    color::value_type color = color::none;
    if (pattern_bounding_[frame_].contain(vertex)) {
      pattern_[frame_].find(vertex, color);
    }
    return color;
  }
};



/**
 * Canvas sprite
 * \param Pixel Maximum number of pixels per frame
 * \param Frames Number of frames
 */
template <std::uint16_t Pixel, std::uint16_t Frames>
class canvas : public base
{
  typedef avl_array<vertex_type, color::value_type, std::uint16_t, Pixel, false> pattern_type;
  typedef avl_array<vertex_type, color::value_type, std::uint16_t, Pixel, true>  restore_type;

  pattern_type pattern_[Frames];                // pattern (data) buffer
  restore_type restore_;                        // restore (background) buffer

  typename pattern_type::iterator pattern_it_;  // pattern iterator
  typename restore_type::iterator restore_it_;  // restore iterator

  rect_type pattern_bounding_[Frames];          // pattern bounding box
  std::uint16_t frame_edit_;                    // actual edit frame
  canvas_output<Pixel> output_shader_;          // shader output

  // rotation, the pattern is rotated at render time by mapping every destination pixel back to the pattern
  std::int16_t      angle_;                     // rotation angle in degree, 0 = no rotation
  vertex_type       rot_center_;                // rotation center in canvas coordinates
  std::int32_t      inv_sin_;                   // sin(-angle), destination to source mapping
  std::int32_t      inv_cos_;                   // cos(-angle)
  rect_type         it_box_;                    // iteration box of the rotated pattern (bounding box of the rotated bounding box)
  vertex_type       it_pos_;                    // rotated pattern iterator position (destination coordinates)
  color::value_type it_color_;                  // pattern color at it_pos_

public:
  /**
   * ctor
   * \param head Reference to the head instance
   * \param z_index Z-index of the sprite, bigger z-index moves to front
   */
  canvas(drv& head, std::int16_t z_index = 0)
    : base(head, z_index)
    , frame_edit_(0U)
    , output_shader_(frame_edit_, pattern_, pattern_bounding_)
    , angle_(0)
    , rot_center_({ 0, 0 })
    , inv_sin_(0)
    , inv_cos_(16384)
    , it_box_({ 0, 0, 0, 0 })
    , it_pos_({ 0, 0 })
    , it_color_(color::none)
  {
    // use own canvas output shader as shader pipeline output
    shader_pipe_ = &output_shader_;

    // init bounding boxes
    for (std::uint_fast16_t n = 0; n < Frames; ++n) {
      pattern_bounding_[n].clear();
    }
  }


  /**
   * Set the rotation of the sprite
   * The pattern is not modified, it is rotated when the sprite is rendered (and when other sprites
   * test against it). Without rotation (angle 0) rendering is as fast as before. A rotated pattern
   * covers about as many destination pixels as the pattern has (+/- 20 % by rasterization), so
   * size Pixel with some headroom when rotating, the restore buffer has Pixel entries.
   * \param angle Rotation angle in degree, math positive (clockwise on screen), 0 to disable rotation
   * \param center Rotation center in canvas coordinates, the sprite stays anchored at its origin
   */
  void set_rotation(std::int16_t angle, vertex_type center = { 0, 0 })
  {
    angle_      = static_cast<std::int16_t>(angle % 360);
    rot_center_ = center;
    inv_sin_    = util::sin(static_cast<std::int16_t>(-angle_));
    inv_cos_    = util::cos(static_cast<std::int16_t>(-angle_));
  }


  /**
   * Get the rotation angle
   * \return Rotation angle in degree, 0 if the sprite is not rotated
   */
  inline std::int16_t get_rotation() const
  {
    return angle_;
  }


  /**
   * Get the rotation center
   * \return Rotation center in canvas coordinates
   */
  inline vertex_type get_rotation_center() const
  {
    return rot_center_;
  }


  /**
   * Select the actual frame for editing, invalid frames are ignored
   * \param frame Edit frame
   */
  inline void select_frame(std::uint16_t frame)
  {
    if (frame < Frames) {
      frame_edit_ = frame;
    }
  }


  /**
   * Get the actual edit frame
   * \return The edit frame
   */
  inline std::uint16_t get_frame() const
  {
    return frame_edit_;
  }


  /**
   * Clear the actual edit frame
   */
  void cls()
  {
    pattern_[frame_edit_].clear();
    pattern_bounding_[frame_edit_].clear();
  }


  /**
   * Get the number of pixels in the actual edit frame
   * \return Pixel count of the edit frame
   */
  inline std::uint16_t pixel_count() const
  {
    return pattern_[frame_edit_].size();
  }


  virtual inline void pattern_it_next(std::uint16_t frame, bool next) final
  {
    if (frame >= Frames) {
      // invalid frame: empty iteration
      pattern_it_ = pattern_[0].end();
      it_box_.clear();
      it_pos_ = { 0, 0 };
      return;
    }

    if (angle_ == 0) {
      // not rotated: iterate the pattern directly
      if (next) {
        ++pattern_it_;
      }
      else {
        pattern_it_ = pattern_[frame].begin();
      }
      return;
    }

    // rotated: iterate the destination box and take every pixel which maps back into the pattern
    if (next) {
      it_pos_.x++;
    }
    else {
      it_box_ = rotated_box(frame);
      it_pos_ = it_box_.top_left();
    }
    for (; it_pos_.y < it_box_.bottom; ++it_pos_.y, it_pos_.x = it_box_.left) {
      for (; it_pos_.x < it_box_.right; ++it_pos_.x) {
        pixel_type p = { it_pos_, color::none };
        if (pattern_find(frame, p)) {
          it_color_ = p.color;
          return;
        }
      }
    }
    // end reached, it_pos_.y == it_box_.bottom
  }

  virtual inline bool pattern_it_get(std::uint16_t frame, pixel_type& pixel) final
  {
    if (frame >= Frames) {
      return false;
    }
    if (angle_ == 0) {
      if (pattern_it_ != pattern_[frame].end()) {
        pixel = { pattern_it_.key(), pattern_it_.val() };
        return true;
      }
      return false;
    }
    if (it_pos_.y < it_box_.bottom) {
      pixel = { it_pos_, it_color_ };
      return true;
    }
    return false;
  }

  virtual inline bool pattern_find(std::uint16_t frame, pixel_type& pixel) final
  {
    if (frame >= Frames) {
      // frame invalid
      return false;
    }
    // map the (rotated) destination vertex back to the pattern
    const vertex_type source = angle_ ? util::vertex_rotate(pixel.vertex, rot_center_, inv_sin_, inv_cos_) : pixel.vertex;
    if (!pattern_bounding_[frame].contain(source)) {
      // pixel is out of canvas
      return false;
    }
    return pattern_[frame].find(source, pixel.color);
  }


private:
  /**
   * Get the destination box of the rotated pattern bounding box
   * \param frame The (valid) frame
   * \return Box which contains all rotated pattern pixels, empty if the pattern is empty
   */
  rect_type rotated_box(std::uint16_t frame) const
  {
    const rect_type& b = pattern_bounding_[frame];
    if (b.empty()) {
      return { 0, 0, 0, 0 };
    }
    // rotate the four corners forward (sin(angle) = -sin(-angle)) and take the extremes,
    // one pixel margin on every side covers the rounding
    const vertex_type corners[4] = { { b.left, b.top }, { static_cast<std::int16_t>(b.right - 1), b.top },
                                     { b.left, static_cast<std::int16_t>(b.bottom - 1) }, { static_cast<std::int16_t>(b.right - 1), static_cast<std::int16_t>(b.bottom - 1) } };
    rect_type box = { 0, 0, 0, 0 };
    for (std::size_t n = 0U; n < 4U; ++n) {
      const vertex_type r = util::vertex_rotate(corners[n], rot_center_, -inv_sin_, inv_cos_);
      if (n == 0U) {
        box = { r.x, r.y, static_cast<std::int16_t>(r.x + 1), static_cast<std::int16_t>(r.y + 1) };
      }
      else {
        box.inflate(r);
      }
    }
    return { static_cast<std::int16_t>(box.left - 1), static_cast<std::int16_t>(box.top - 1), static_cast<std::int16_t>(box.right + 1), static_cast<std::int16_t>(box.bottom + 1) };
  }

public:
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
    typename restore_type::iterator it = restore_it_++;
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

/**
 * Sheet sprite, the frames are taken directly out of a sprite sheet
 * The restore (background) buffer is a two dimensional array of the frame size with x and y as keys: a sheet only
 * stores background pixels where its frame is, so the keys are the screen position modulo the frame size. Every
 * pixel inside the frame has its own entry then, wherever the sprite is
 * \param Frame_Width Width of one frame in pixel, at least the sprite width of the sheet
 * \param Frame_Height Height of one frame in pixel, at least the sprite height of the sheet
 */
template <std::uint16_t Frame_Width, std::uint16_t Frame_Height>
class sheet : public base
{
  static_assert((Frame_Width > 0U) && (Frame_Height > 0U), "sheet frame size must be positive");

  const sheet_info_type*  sheet_info_;          // sheet info
  std::uint16_t           frames_per_row_;      // frames per sheet row
  std::uint16_t           frame_count_;         // number of frames in the sheet
  std::uint32_t           pattern_size_;        // pattern (frame) size in pixel
  std::uint32_t           pattern_it_;          // pattern iterator
  rect_type               pattern_bounding_;    // pattern bounding box

  color::value_type       restore_[Frame_Height][Frame_Width];                    // restore (background) buffer [y][x]
  std::uint8_t            restore_used_[Frame_Height][(Frame_Width + 7U) / 8U];   // one bit per buffer entry: entry is used
  vertex_type             restore_origin_;      // sprite position of the buffered pixels
  vertex_type             restore_it_;          // restore iterator, x/y keys

public:
  /**
   * ctor
   * \param head Reference to the head instance
   * \param sheet_info The sheet info
   * \param z_index Z-index of the sprite, bigger z-index moves to front
   */
  sheet(drv& head, const sheet_info_type* sheet_info, std::int16_t z_index = 0)
    : base(head, z_index)
    , sheet_info_(sheet_info)
    , frames_per_row_(0U)
    , frame_count_(0U)
    , pattern_size_(0U)
    , pattern_it_(0U)
    , restore_used_()
    , restore_origin_({ 0, 0 })
    , restore_it_({ 0, Frame_Height })
  {
    pattern_bounding_.clear();
    if (sheet_info_ && sheet_info_->sprite_width && sheet_info_->sprite_height) {
      frames_per_row_   = static_cast<std::uint16_t>(sheet_info_->width  / sheet_info_->sprite_width);
      frame_count_      = static_cast<std::uint16_t>(frames_per_row_ * (sheet_info_->height / sheet_info_->sprite_height));
      pattern_bounding_ = { 0, 0, static_cast<std::int16_t>(sheet_info_->sprite_width), static_cast<std::int16_t>(sheet_info_->sprite_height) };
      pattern_size_     = static_cast<std::uint32_t>(sheet_info_->sprite_width) * sheet_info_->sprite_height;
    }
  }


  /**
   * Get the number of frames of the sheet
   * \return Frame count
   */
  inline std::uint16_t frame_count() const
  {
    return frame_count_;
  }


  virtual inline void pattern_it_next(std::uint16_t frame, bool next) final
  {
    if (frame >= frame_count_) {
      pattern_it_ = pattern_size_;
      return;
    }
    if (next) {
      ++pattern_it_;
    }
    else {
      pattern_it_ = 0U;
    }
    // skip background pixels
    for (; pattern_it_ < pattern_size_; ++pattern_it_) {
      if (pixel_color(frame, pattern_it_) != sheet_info_->bg_color) {
        return;
      }
    }
  }

  virtual inline bool pattern_it_get(std::uint16_t frame, pixel_type& pixel) final
  {
    if ((frame < frame_count_) && (pattern_it_ < pattern_size_)) {
      pixel.vertex.x = static_cast<std::int16_t>(pattern_it_ % sheet_info_->sprite_width);
      pixel.vertex.y = static_cast<std::int16_t>(pattern_it_ / sheet_info_->sprite_width);
      pixel.color    = pixel_color(frame, pattern_it_);
      return true;
    }
    return false;
  }

  virtual inline bool pattern_find(std::uint16_t frame, pixel_type& pixel) final
  {
    if ((frame >= frame_count_) || !pattern_bounding_.contain(pixel.vertex)) {
      // frame invalid or pixel is out of sheet frame
      return false;
    }
    pixel.color = pixel_color(frame, static_cast<std::uint32_t>(pixel.vertex.y) * sheet_info_->sprite_width + static_cast<std::uint32_t>(pixel.vertex.x));
    return pixel.color != sheet_info_->bg_color;
  }


  virtual inline void restore_it_next(bool next) final
  {
    if (next) {
      restore_it_.x++;
    }
    else {
      restore_it_ = { 0, 0 };
    }
    // take the next used entry
    for (; restore_it_.y < Frame_Height; ++restore_it_.y, restore_it_.x = 0) {
      for (; restore_it_.x < Frame_Width; ++restore_it_.x) {
        if (restore_used_[restore_it_.y][restore_it_.x / 8] & (1U << (restore_it_.x % 8))) {
          return;
        }
      }
    }
  }

  virtual inline bool restore_it_get(pixel_type& pixel) final
  {
    if (restore_it_.y < Frame_Height) {
      // get the screen position back out of the keys: it's the one inside the frame at the sprite position of the buffered pixels
      pixel.vertex.x = static_cast<std::int16_t>(restore_origin_.x + (restore_it_.x + Frame_Width  - static_cast<std::uint16_t>(restore_origin_.x + 32768) % Frame_Width)  % Frame_Width);
      pixel.vertex.y = static_cast<std::int16_t>(restore_origin_.y + (restore_it_.y + Frame_Height - static_cast<std::uint16_t>(restore_origin_.y + 32768) % Frame_Height) % Frame_Height);
      pixel.color    = restore_[restore_it_.y][restore_it_.x];
      return true;
    }
    return false;
  }

  virtual inline void restore_it_delete() final
  {
    restore_used_[restore_it_.y][restore_it_.x / 8] &= static_cast<std::uint8_t>(~(1U << (restore_it_.x % 8)));
    restore_it_next(true);
  }

  virtual inline void restore_set(const pixel_type& pixel) final
  {
    // the keys are the screen position modulo the frame size (+ 32768 for positive values)
    const std::int32_t x = static_cast<std::uint16_t>(pixel.vertex.x + 32768) % Frame_Width;
    const std::int32_t y = static_cast<std::uint16_t>(pixel.vertex.y + 32768) % Frame_Height;
    restore_[y][x] = pixel.color;
    restore_used_[y][x / 8] |= static_cast<std::uint8_t>(1U << (x % 8));
    // pixels are only set while the sprite is at its place: remember it, render() changes the position before it restores
    restore_origin_ = position_;
  }

  virtual inline bool restore_find(pixel_type& pixel) final
  {
    const std::int32_t x = static_cast<std::uint16_t>(pixel.vertex.x + 32768) % Frame_Width;
    const std::int32_t y = static_cast<std::uint16_t>(pixel.vertex.y + 32768) % Frame_Height;
    if (restore_used_[y][x / 8] & (1U << (x % 8))) {
      pixel.color = restore_[y][x];
      return true;
    }
    return false;
  }

private:
  /**
   * Get the color of a frame pixel
   * \param frame The (valid) frame
   * \param index Pixel index within the frame, row major
   * \return Decoded pixel color
   */
  inline color::value_type pixel_color(std::uint16_t frame, std::uint32_t index) const
  {
    const std::uint32_t x   = (frame % frames_per_row_) * sheet_info_->sprite_width  + index % sheet_info_->sprite_width;
    const std::uint32_t y   = (frame / frames_per_row_) * sheet_info_->sprite_height + index / sheet_info_->sprite_width;
    const std::uint32_t bpp = static_cast<std::uint8_t>(sheet_info_->format) >> 4U;     // upper nibble of format is 'bytes/pixel'
    return sheet_pixel_color(*sheet_info_, sheet_info_->data + (y * sheet_info_->width + x) * bpp);
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
  const sheet_info_type* sheet_info_;   // sheet info
  std::uint16_t          frames_per_row_;
  std::uint16_t          frame_count_;

public:
  /**
   * ctor
   * \param head Reference to the head instance
   * \param sheet_info The sheet info
   * \param z_index Z-index of the sprite, bigger z-index moves to front
   */
  canvas_sheet(drv& head, const sheet_info_type* sheet_info, std::int16_t z_index = 0)
    : canvas<Pixel, 1U>(head, z_index)
    , sheet_info_(sheet_info)
    , frames_per_row_(0U)
    , frame_count_(0U)
  {
    if (sheet_info_ && sheet_info_->sprite_width && sheet_info_->sprite_height) {
      frames_per_row_ = static_cast<std::uint16_t>(sheet_info_->width / sheet_info_->sprite_width);
      frame_count_    = static_cast<std::uint16_t>(frames_per_row_ * (sheet_info_->height / sheet_info_->sprite_height));
    }
  }


  /**
   * Get the number of frames of the sheet
   * \return Frame count
   */
  inline std::uint16_t frame_count() const
  {
    return frame_count_;
  }


  /**
   * Set the rotation of the sprite around an explicit center, see canvas::set_rotation()
   * \param angle Rotation angle in degree, math positive (clockwise on screen), 0 to disable rotation
   * \param center Rotation center in frame coordinates
   */
  void set_rotation(std::int16_t angle, vertex_type center)
  {
    canvas<Pixel, 1U>::set_rotation(angle, center);
  }


  /**
   * Set the rotation of the sprite around the center of the sheet frame
   * \param angle Rotation angle in degree, math positive (clockwise on screen), 0 to disable rotation
   */
  void set_rotation(std::int16_t angle)
  {
    const vertex_type center = sheet_info_ ? vertex_type{ static_cast<std::int16_t>(sheet_info_->sprite_width / 2U), static_cast<std::int16_t>(sheet_info_->sprite_height / 2U) } : vertex_type{ 0, 0 };
    canvas<Pixel, 1U>::set_rotation(angle, center);
  }


  /**
   * Render the sprite rotated around the frame center on the head
   * \param position Screen vertex where the origin of the sprite is rendered
   * \param frame Frame of the sheet to render, an invalid frame renders an empty sprite
   * \param angle Rotation angle in degree, math positive (clockwise on screen)
   */
  void render(vertex_type position, std::uint16_t frame, std::int16_t angle)
  {
    set_rotation(angle);
    render(position, frame);
  }


  /**
   * Render the sprite on the head
   * \param position Screen vertex where the origin of the sprite is rendered
   * \param frame Frame of the sheet to render, an invalid frame renders an empty sprite
   */
  void render(vertex_type position, std::uint16_t frame = 0U)
  {
    // clear buffer
    this->cls();

    if (frame < frame_count_) {
      // load the sheet frame into the canvas
      const std::uint32_t bpp = static_cast<std::uint8_t>(sheet_info_->format) >> 4U;     // upper nibble of format is 'bytes/pixel'
      const std::uint32_t row = (frame / frames_per_row_) * sheet_info_->sprite_height;
      const std::uint32_t col = (frame % frames_per_row_) * sheet_info_->sprite_width;
      for (std::uint32_t y = 0U; y < sheet_info_->sprite_height; ++y) {
        for (std::uint32_t x = 0U; x < sheet_info_->sprite_width; ++x) {
          const color::value_type c = sheet_pixel_color(*sheet_info_, sheet_info_->data + ((row + y) * sheet_info_->width + (col + x)) * bpp);
          if (c != sheet_info_->bg_color) {
            this->pixel_set({ static_cast<std::int16_t>(x), static_cast<std::int16_t>(y) }, c);
          }
        }
      }
    }

    // call the base class
    canvas<Pixel, 1U>::render(position, 0U);
  }
};



///////////////////////////////////////////////////////////////////////////////
// C A N V A S   S H E E T   X Y   S P R I T E
//

/**
 * Canvas sheet sprite of a fixed size
 * It is used like a canvas_sheet, but the pattern and the restore buffer are two dimensional arrays with x and y as
 * keys instead of AVL trees, which need about half of the RAM and are faster.
 * The pattern is limited to the canvas of Width x Height, pixels which a shader (zoom, offset etc.) moves out of the
 * canvas are dropped. The restore buffer has the size of the canvas, or of the rotated canvas if the sprite can be
 * rotated. Its keys are the screen position modulo this size, so every pixel of the sprite has its own entry,
 * wherever the sprite is
 * \param Width Width of the canvas, at least "sheet_info_->sprite_width" to hold the sprite
 * \param Height Height of the canvas, at least "sheet_info_->sprite_height" to hold the sprite
 * \param Rotate true if the sprite can be rotated. Set it to false if rotation is not needed, the restore buffer
 *               has the size of the canvas then instead of the (about two times bigger) rotated canvas
 */
template <std::uint16_t Width, std::uint16_t Height, bool Rotate = true>
class canvas_sheet_xy : public base
{
  static_assert((Width > 0U) && (Height > 0U), "canvas size must be positive");

  /**
   * Get the size of the restore buffer of the rotated canvas at compile time: the diagonal of the canvas
   * \param size Start value 1, the recursion is used to stay C++11 compliant
   * \return Biggest width and height of the box of the rotated canvas (the diagonal and the 3 pixel margin of the box)
   */
  static constexpr std::uint16_t rotated_size(std::uint32_t size)
  {
    return (size * size >= static_cast<std::uint32_t>(Width) * Width + static_cast<std::uint32_t>(Height) * Height) ? static_cast<std::uint16_t>(size + 3U) : rotated_size(size + 1U);
  }

  static const std::uint16_t RESTORE_WIDTH  = Rotate ? rotated_size(1U) : Width;
  static const std::uint16_t RESTORE_HEIGHT = Rotate ? rotated_size(1U) : Height;

  /**
   * Output shader, stores all rendered pixels in the pattern buffer
   */
  class output : public shader::base
  {
    color::value_type (&pattern_)[Height][Width];

  public:
    output(color::value_type (&pattern)[Height][Width])
      : pattern_(pattern)
    { }

    inline virtual void pixel_set(vertex_type vertex, color::value_type color) final
    {
      if ((vertex.x >= 0) && (vertex.x < Width) && (vertex.y >= 0) && (vertex.y < Height)) {
        // color::none deletes the pixel (e.g. to punch out pixels), an existing pixel is alpha blended
        color::value_type& pattern = pattern_[vertex.y][vertex.x];
        pattern = ((color == color::none) || (pattern == color::none)) ? color : color::alpha_blend(color, pattern);
      }
    }

    inline virtual color::value_type pixel_get(vertex_type vertex) final
    {
      return ((vertex.x >= 0) && (vertex.x < Width) && (vertex.y >= 0) && (vertex.y < Height)) ? pattern_[vertex.y][vertex.x] : static_cast<color::value_type>(color::none);
    }
  };

  const sheet_info_type*  sheet_info_;          // sheet info
  std::uint16_t           frames_per_row_;      // frames per sheet row
  std::uint16_t           frame_count_;         // number of frames in the sheet

  color::value_type       pattern_[Height][Width];                                  // pattern buffer [y][x], color::none is no pixel
  output                  output_shader_;                                           // shader output

  color::value_type       restore_[RESTORE_HEIGHT][RESTORE_WIDTH];                  // restore (background) buffer [y][x]
  std::uint8_t            restore_used_[RESTORE_HEIGHT][(RESTORE_WIDTH + 7U) / 8U]; // one bit per buffer entry: entry is used
  vertex_type             restore_origin_;      // top left screen position of the box of the buffered pixels
  vertex_type             restore_it_;          // restore iterator, x/y keys

  // rotation, the pattern is rotated at render time by mapping every destination pixel back to the pattern
  std::int16_t            angle_;               // rotation angle in degree, 0 = no rotation
  vertex_type             rot_center_;          // rotation center in canvas coordinates
  std::int32_t            inv_sin_;             // sin(-angle), destination to source mapping
  std::int32_t            inv_cos_;             // cos(-angle)
  rect_type               it_box_;              // iteration box of the (rotated) canvas
  vertex_type             it_pos_;              // pattern iterator position (destination coordinates)
  color::value_type       it_color_;            // pattern color at it_pos_

public:
  /**
   * ctor
   * \param head Reference to the head instance
   * \param sheet_info The sheet info
   * \param z_index Z-index of the sprite, bigger z-index moves to front
   */
  canvas_sheet_xy(drv& head, const sheet_info_type* sheet_info, std::int16_t z_index = 0)
    : base(head, z_index)
    , sheet_info_(sheet_info)
    , frames_per_row_(0U)
    , frame_count_(0U)
    , pattern_()
    , output_shader_(pattern_)
    , restore_used_()
    , restore_origin_({ 0, 0 })
    , restore_it_({ 0, RESTORE_HEIGHT })
    , angle_(0)
    , rot_center_({ 0, 0 })
    , inv_sin_(0)
    , inv_cos_(16384)
    , it_box_({ 0, 0, 0, 0 })
    , it_pos_({ 0, 0 })
    , it_color_(color::none)
  {
    // use own output shader as shader pipeline output
    shader_pipe_ = &output_shader_;

    if (sheet_info_ && sheet_info_->sprite_width && sheet_info_->sprite_height) {
      frames_per_row_ = static_cast<std::uint16_t>(sheet_info_->width / sheet_info_->sprite_width);
      frame_count_    = static_cast<std::uint16_t>(frames_per_row_ * (sheet_info_->height / sheet_info_->sprite_height));
    }
  }


  /**
   * Get the number of frames of the sheet
   * \return Frame count
   */
  inline std::uint16_t frame_count() const
  {
    return frame_count_;
  }


  /**
   * Set the rotation of the sprite around an explicit center, see canvas::set_rotation()
   * The template parameter Rotate must be true
   * \param angle Rotation angle in degree, math positive (clockwise on screen), 0 to disable rotation
   * \param center Rotation center in frame coordinates
   */
  void set_rotation(std::int16_t angle, vertex_type center)
  {
    static_assert(Rotate, "rotation is disabled by the template parameter Rotate");
    angle_      = static_cast<std::int16_t>(angle % 360);
    rot_center_ = center;
    inv_sin_    = util::sin(static_cast<std::int16_t>(-angle_));
    inv_cos_    = util::cos(static_cast<std::int16_t>(-angle_));
  }


  /**
   * Set the rotation of the sprite around the center of the sheet frame
   * \param angle Rotation angle in degree, math positive (clockwise on screen), 0 to disable rotation
   */
  void set_rotation(std::int16_t angle)
  {
    set_rotation(angle, sheet_info_ ? vertex_type{ static_cast<std::int16_t>(sheet_info_->sprite_width / 2U), static_cast<std::int16_t>(sheet_info_->sprite_height / 2U) } : vertex_type{ 0, 0 });
  }


  /**
   * Get the rotation angle
   * \return Rotation angle in degree, 0 if the sprite is not rotated
   */
  inline std::int16_t get_rotation() const
  {
    return angle_;
  }


  /**
   * Get the rotation center
   * \return Rotation center in canvas coordinates
   */
  inline vertex_type get_rotation_center() const
  {
    return rot_center_;
  }


  /**
   * Render the sprite rotated around the frame center on the head
   * \param position Screen vertex where the origin of the sprite is rendered
   * \param frame Frame of the sheet to render, an invalid frame renders an empty sprite
   * \param angle Rotation angle in degree, math positive (clockwise on screen)
   */
  void render(vertex_type position, std::uint16_t frame, std::int16_t angle)
  {
    set_rotation(angle);
    render(position, frame);
  }


  /**
   * Render the sprite on the head
   * \param position Screen vertex where the origin of the sprite is rendered
   * \param frame Frame of the sheet to render, an invalid frame renders an empty sprite
   */
  void render(vertex_type position, std::uint16_t frame = 0U)
  {
    // clear the pattern
    for (std::uint_fast16_t y = 0U; y < Height; ++y) {
      for (std::uint_fast16_t x = 0U; x < Width; ++x) {
        pattern_[y][x] = color::none;
      }
    }

    if (frame < frame_count_) {
      // load the sheet frame into the pattern
      const std::uint32_t bpp = static_cast<std::uint8_t>(sheet_info_->format) >> 4U;     // upper nibble of format is 'bytes/pixel'
      const std::uint32_t row = (frame / frames_per_row_) * sheet_info_->sprite_height;
      const std::uint32_t col = (frame % frames_per_row_) * sheet_info_->sprite_width;
      for (std::uint32_t y = 0U; y < sheet_info_->sprite_height; ++y) {
        for (std::uint32_t x = 0U; x < sheet_info_->sprite_width; ++x) {
          const color::value_type c = sheet_pixel_color(*sheet_info_, sheet_info_->data + ((row + y) * sheet_info_->width + (col + x)) * bpp);
          if (c != sheet_info_->bg_color) {
            pixel_set({ static_cast<std::int16_t>(x), static_cast<std::int16_t>(y) }, c);
          }
        }
      }
    }

    // call the base class
    base::render(position, 0U);
  }


  virtual inline void pattern_it_next(std::uint16_t frame, bool next) final
  {
    if (next) {
      it_pos_.x++;
    }
    else {
      // iterate the box of the (rotated) canvas and take every pixel which maps back into the pattern
      it_box_ = canvas_box();
      it_pos_ = it_box_.top_left();
    }
    for (; it_pos_.y < it_box_.bottom; ++it_pos_.y, it_pos_.x = it_box_.left) {
      for (; it_pos_.x < it_box_.right; ++it_pos_.x) {
        pixel_type p = { it_pos_, color::none };
        if (pattern_find(frame, p)) {
          it_color_ = p.color;
          return;
        }
      }
    }
    // end reached, it_pos_.y == it_box_.bottom
  }

  virtual inline bool pattern_it_get(std::uint16_t, pixel_type& pixel) final
  {
    if (it_pos_.y < it_box_.bottom) {
      pixel = { it_pos_, it_color_ };
      return true;
    }
    return false;
  }

  virtual inline bool pattern_find(std::uint16_t frame, pixel_type& pixel) final
  {
    // map the (rotated) destination vertex back to the pattern, x and y are the keys of the pattern
    const vertex_type source = angle_ ? util::vertex_rotate(pixel.vertex, rot_center_, inv_sin_, inv_cos_) : pixel.vertex;
    if ((frame != 0U) || (source.x < 0) || (source.x >= Width) || (source.y < 0) || (source.y >= Height)) {
      // frame invalid or pixel is out of canvas
      return false;
    }
    pixel.color = pattern_[source.y][source.x];
    return pixel.color != color::none;
  }


  virtual inline void restore_it_next(bool next) final
  {
    if (next) {
      restore_it_.x++;
    }
    else {
      restore_it_ = { 0, 0 };
    }
    // take the next used entry
    for (; restore_it_.y < RESTORE_HEIGHT; ++restore_it_.y, restore_it_.x = 0) {
      for (; restore_it_.x < RESTORE_WIDTH; ++restore_it_.x) {
        if (restore_used_[restore_it_.y][restore_it_.x / 8] & (1U << (restore_it_.x % 8))) {
          return;
        }
      }
    }
  }

  virtual inline bool restore_it_get(pixel_type& pixel) final
  {
    if (restore_it_.y < RESTORE_HEIGHT) {
      // get the screen position back out of the keys: it's the one inside the box of the buffered pixels
      pixel.vertex.x = static_cast<std::int16_t>(restore_origin_.x + (restore_it_.x + RESTORE_WIDTH  - static_cast<std::uint16_t>(restore_origin_.x + 32768) % RESTORE_WIDTH)  % RESTORE_WIDTH);
      pixel.vertex.y = static_cast<std::int16_t>(restore_origin_.y + (restore_it_.y + RESTORE_HEIGHT - static_cast<std::uint16_t>(restore_origin_.y + 32768) % RESTORE_HEIGHT) % RESTORE_HEIGHT);
      pixel.color    = restore_[restore_it_.y][restore_it_.x];
      return true;
    }
    return false;
  }

  virtual inline void restore_it_delete() final
  {
    restore_used_[restore_it_.y][restore_it_.x / 8] &= static_cast<std::uint8_t>(~(1U << (restore_it_.x % 8)));
    restore_it_next(true);
  }

  virtual inline void restore_set(const pixel_type& pixel) final
  {
    // the keys are the screen position modulo the buffer size (+ 32768 for positive values)
    const std::int32_t x = static_cast<std::uint16_t>(pixel.vertex.x + 32768) % RESTORE_WIDTH;
    const std::int32_t y = static_cast<std::uint16_t>(pixel.vertex.y + 32768) % RESTORE_HEIGHT;
    restore_[y][x] = pixel.color;
    restore_used_[y][x / 8] |= static_cast<std::uint8_t>(1U << (x % 8));
    // pixels are only set while the sprite is at its place: remember it, render() changes the position before it restores
    restore_origin_ = position_ + it_box_.top_left();
  }

  virtual inline bool restore_find(pixel_type& pixel) final
  {
    const std::int32_t x = static_cast<std::uint16_t>(pixel.vertex.x + 32768) % RESTORE_WIDTH;
    const std::int32_t y = static_cast<std::uint16_t>(pixel.vertex.y + 32768) % RESTORE_HEIGHT;
    if (restore_used_[y][x / 8] & (1U << (x % 8))) {
      pixel.color = restore_[y][x];
      return true;
    }
    return false;
  }

private:
  /**
   * Get the destination box of the (rotated) canvas
   * \return Box which contains all pixels of the rotated pattern
   */
  rect_type canvas_box() const
  {
    if (angle_ == 0) {
      return { 0, 0, static_cast<std::int16_t>(Width), static_cast<std::int16_t>(Height) };
    }
    // rotate the four corners forward (sin(angle) = -sin(-angle)) and take the extremes,
    // one pixel margin on every side covers the rounding
    const vertex_type corners[4] = { { 0, 0 }, { static_cast<std::int16_t>(Width - 1), 0 },
                                     { 0, static_cast<std::int16_t>(Height - 1) }, { static_cast<std::int16_t>(Width - 1), static_cast<std::int16_t>(Height - 1) } };
    rect_type box = { 0, 0, 0, 0 };
    for (std::size_t n = 0U; n < 4U; ++n) {
      const vertex_type r = util::vertex_rotate(corners[n], rot_center_, -inv_sin_, inv_cos_);
      if (n == 0U) {
        box = { r.x, r.y, static_cast<std::int16_t>(r.x + 1), static_cast<std::int16_t>(r.y + 1) };
      }
      else {
        box.inflate(r);
      }
    }
    return { static_cast<std::int16_t>(box.left - 1), static_cast<std::int16_t>(box.top - 1), static_cast<std::int16_t>(box.right + 1), static_cast<std::int16_t>(box.bottom + 1) };
  }
};


} // namespace sprite
} // namespace vic

#endif  // _VIC_SPRITE_H
