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
// \brief Skeleton head, use this as a template for your own head/display driver
// Copy this file, rename the class and implement the functions your display supports.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_SKELETON_H_
#define _VIC_HEAD_SKELETON_H_

#include "../drv.h"


// defines the heads name and version
#define VIC_HEAD_SKELETON_VERSION   "Skeleton head 1.01"

namespace vic {
namespace head {


/**
 * Skeleton driver
 * \param Screen_Size_X Screen (buffer) width
 * \param Screen_Size_Y Screen (buffer) height
 * \param Viewport_Size_X Viewport (window / physical display) width
 * \param Viewport_Size_Y Viewport (window / physical display) height
 * \param Orientation Orientation of the display
 */
template<std::uint16_t Screen_Size_X,   std::uint16_t Screen_Size_Y,    // Screen (buffer) size in pixel on graphic displays or chars on text displays, must match the orientation
         std::uint16_t Viewport_Size_X = Screen_Size_X,                 // Viewport (physical) size in pixel on graphic displays or chars on text displays
         std::uint16_t Viewport_Size_Y = Screen_Size_Y,
         drv::orientation_type Orientation = drv::orientation_0         // Orientation of the display
>
class skeleton : public drv
{
  // vars
  const io::handle_type device_handle_;   // logical device handle of the display interface

// ctor(), dtor(), init(), shutdown(), version() and is_graphic() are the only public functions
public:

  /**
   * ctor
   * \param device_handle Logical device handle of the display interface (SPI, I²C, UART ...)
   * \param viewport_x X offset of the viewport, relative to top/left corner of the screen
   * \param viewport_y Y offset of the viewport, relative to top/left corner of the screen
   */
  skeleton(io::handle_type device_handle,
           std::int16_t viewport_x = 0, std::int16_t viewport_y = 0)
    : drv(Screen_Size_X,   Screen_Size_Y,
          Viewport_Size_X, Viewport_Size_Y,
          viewport_x,      viewport_y,
          Orientation)
    , device_handle_(device_handle)
  {
    // init of memory structures, normally empty
  }


  /**
   * dtor
   * Basically shut the driver down
   */
  ~skeleton()
  {
    // normally a head is not deconstructed. But if so, shutdown the driver
    shutdown();
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   D R I V E R   F U N C T I O N S

  // Implement all functions in this section, they are mandatory for any display

  /**
   * Driver init
   */
  virtual void init()
  {
    // init the driver and hardware here, e.g.
    // io::init(device_handle_);
  }


  /**
   * Driver shutdown
   */
  virtual void shutdown()
  {
    // shutdown the driver and hardware here
    // or enter sleep / powersave mode
  }


  /**
   * Returns the driver version and name
   * \return Driver version and name
   */
  virtual inline const char* version() const
  {
    // return the driver version, like
    return (const char*)VIC_HEAD_SKELETON_VERSION;
  }


  /**
   * Returns the display capability: graphic or alpha numeric
   * \return True if graphic display type
   */
  virtual inline bool is_graphic() const
  {
    // return if the display is a graphic display (true) or alpha numeric (false)
    return true;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

// ALL following functions must not be accessed directly, therefore they are 'protected'
protected:

  /**
   * Clear screen, set all pixels off, delete all characters or fill screen with background/blank color
   * \param bg_color Backgound/erase color, defines normally the default color of the display
   */
  virtual void cls(color::value_type bg_color = color::none)
  {
    // clear the entire screen / buffer
    // implement if available
    (void)bg_color;
  }


  /**
   * Primitive rendering is done, copy RAM / frame buffer to screen
   */
  virtual void present()
  {
    // implement if available
  }


  ///////////////////////////////////////////////////////////////////////////////
  // G R A P H I C   F U N C T I O N S
  //

  // Implement the functions in this section, if it's a graphic display. Delete this
  // section completely, if it's an alphanumeric display.
  // Mandatory are only pixel_set() and pixel_get(), implement all other functions (line_horz etc.)
  // if the the display has native or fast support for it. If the according function is not implemented
  // here, the 'drv' base class will provide a fallback function.

  /**
   * Set pixel in given color
   * \param vertex Pixel coordinates
   * \param color Color of pixel in ARGB format, alpha channel is/maybe ignored
   */
  virtual void pixel_set(vertex_type vertex, color::value_type color)
  {
    // check limits
    if (!screen_is_inside(vertex)) {
      // out of bounds
      return;
    }

    // set the pixel in the display buffer
    (void)color;
  }


  /**
   * Return the color of the pixel
   * \param vertex Vertex of the pixel
   * \return Color of pixel in ARGB format, alpha channel must be set to opaque if unsupported (default)
   */
  virtual color::value_type pixel_get(vertex_type vertex)
  {
    // check limits
    if (!screen_is_inside(vertex)) {
      // out of bounds
      return color::none;
    }

    // return the pixel color at the given position
    return color::black;
  }


  /**
   * Draw a horizontal line in the given color, width is one pixel
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, y component is ignored
   * \param color Line color
   */
  virtual void line_horz(vertex_type v0, vertex_type v1, color::value_type color)
  {
    // implement native horizontal line drawing or remove this function to use the fallback
    drv::line_horz(v0, v1, color);
  }


  /**
   * Draw a vertical line in the given color, width is one pixel
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, x component is ignored
   * \param color Line color
   */
  virtual void line_vert(vertex_type v0, vertex_type v1, color::value_type color)
  {
    // implement native vertical line drawing or remove this function to use the fallback
    drv::line_vert(v0, v1, color);
  }


  /**
   * Draw a box (filled rectangle) in given color
   * \param rect Box bounding, left/top is inside, right/bottom is outside the box
   * \param color Box color
   */
  virtual void box(const rect_type& rect, color::value_type color)
  {
    // implement a native filled rectangle drawing function or remove this function to use the fallback
    drv::box(rect, color);
  }


  /**
   * Move display area
   * \param source Source rect
   * \param destination Destination top/left vertex
   */
  virtual void move(const rect_type& source, vertex_type destination)
  {
    // implement a native move area function or remove this function to use the fallback
    drv::move(source, destination);
  }


  /**
   * Write a block of pixels (bit block transfer), used by the framebuffer head and for bitmaps
   * \param rect Destination rect, right and bottom are outside the rect
   * \param pixels Pixel block in ARGB format, row major
   * \param stride Distance between two rows in pixels
   */
  virtual void blit(const rect_type& rect, const color::value_type* pixels, std::size_t stride)
  {
    // if the controller has an autoincrementing GRAM window: clip the rect to the screen, set the
    // window once and stream the converted pixels. Or remove this function to use the per pixel fallback
    drv::blit(rect, pixels, stride);
  }


  ///////////////////////////////////////////////////////////////////////////////
  // A L P H A   T E X T   F U N C T I O N S
  //

  // Implement the functions in this section, if it's an alpha numeric display. Delete this
  // section completely, if it's a graphic display.
  // Mandatory is text_out(), implement all other functions (text_set_pos etc.) that the display supports

  /**
   * Output a single ASCII/UNICODE char at the actual cursor position
   * The cursor position is moved by the char width (distance)
   * \param ch Output character in 16 bit ASCII/UNICODE (NOT UTF-8) format, 00-7F is compatible with ASCII
   */
  virtual void text_out(std::uint16_t ch)
  {
    // implement the text output function which writes the character to the display
    (void)ch;
  }


  /**
   * Set the new text position
   * \param pos Position in chars on text displays (0/0 is left/top)
   */
  virtual void text_set_pos(vertex_type pos)
  {
    // implement a cursor set position function
    (void)pos;
  }


  /**
   * Set inverse text mode
   * \param inverse Set normal or inverse video
   */
  virtual void text_set_inverse(bool inverse)
  {
    // implement a set inverse text function
    (void)inverse;
  }


  /**
   * Clear actual line from cursor pos (included) to end of line
   */
  virtual void text_clear_eol()
  {
    // implement a clear the actual line from cursor pos (included) to the end of line function
  }


  /**
   * Clear actual line from start to cursor pos
   */
  virtual void text_clear_sol()
  {
    // implement a clear the actual line from start to cursor pos function
  }


  /**
   * Clear the actual line
   */
  virtual void text_clear_line()
  {
    // implement a function to clear the actual line
  }


  /////////////////////////////////////////////////////////////////////////////
  // O P T I O N A L   D I S P L A Y   C O N T R O L
  //

  // Implement the functions that the display supports, delete what is unsupported
public:

  /**
   * Enable/disable the display
   * \param enable True to switch the display on, false to switch it off (standby, powersave)
   */
  virtual void display_enable(bool enable = true)
  {
    // implement a display enable/disable function
    (void)enable;
  }


  /**
   * Set display or backlight brightness
   * \param level 0: dark, backlight off; 255: maximum brightness, backlight full on
   */
  virtual void display_brightness(std::uint8_t level)
  {
    // implement a function to set the display brightness
    (void)level;
  }


  /**
   * Set display contrast brightness
   * \param level 0: minimum; 255: maximum
   */
  virtual void display_contrast(std::uint8_t level)
  {
    // implement a function to set the display contrast
    (void)level;
  }

};

} // namespace head
} // namespace vic

#endif  // _VIC_HEAD_SKELETON_H_
