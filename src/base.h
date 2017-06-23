///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2017, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// This file is part of the vgx library.
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
// \brief vgx base class
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_BASE_H_
#define _VGX_BASE_H_

#include <cstdint>
#include <cstddef>

#include <vgx_cfg.h>    // use < > here, cause vgx_cfg.h may be in some platform folder
#include "util.h"
#include "color.h"


namespace vgx {


// callback function definition for dynamic pen color
typedef color::value_type (*pen_color_function_type)(vertex_type vertex);


/**
 * vgx base class
 */
class base
{
  color::value_type         pen_color_;           // drawing color
  color::value_type         bg_color_;            // background color
  pen_color_function_type   pen_color_function_;  // function for dynamic pen color
  bool                      present_lock_;        // true if present is locked


public:

  // standard ctor
  base()
    : pen_color_(color::white)
    , bg_color_(color::black)
    , pen_color_function_(nullptr)
    , present_lock_(false)
  { }

///////////////////////////////////////////////////////////////////////////////
// C O L O R   F U N C T I O N S
//
public:

  /**
   * Set the pen (drawing) color
   * \param pen_color New drawing color in ARGB format
   */
  inline virtual void pen_set_color(color::value_type pen_color)
  {
    pen_color_          = pen_color;
    pen_color_function_ = nullptr;
  }

  /**
   * Set the callback function for dynamic pen color
   * \param pen_color_function Function for dynamic pen color
   */
  inline virtual void pen_set_color(pen_color_function_type pen_color_function)
  { pen_color_function_ = pen_color_function; }

  /**
   * Get the actual pen (drawing) color
   * \return Actual drawing color in ARGB format
   */
  inline virtual color::value_type pen_get_color() const
  { return pen_color_; }

  /**
   * Get the actual pen (drawing) color
   * \param point Point for which the color is needed, color is given by the defined pen function
   * \return Actual drawing color in ARGB format
   */
  inline virtual color::value_type pen_get_color(vertex_type point) const
  { return !!pen_color_function_ ? pen_color_function_(point) : pen_color_; }

  /**
   * Return true if pen color is defined by function
   * \return True if pen color is a function, false if the pen color is solid
   */
  inline bool pen_color_is_function() const
  { return !!pen_color_function_; }

  /**
   * Set the background color (e.g. for cls)
   * \param background_color New background color in ARGB format
   */
  inline virtual void bg_set_color(color::value_type background_color)
  { bg_color_ = background_color; }

  /**
   * Get the actual background color
   * \return Actual background color in ARGB format
   */
  inline virtual color::value_type bg_get_color() const
  { return bg_color_; }


///////////////////////////////////////////////////////////////////////////////
// C O M M O N   F U N C T I O N S
//

  /**
   * Primitive rendering is done. Display (frame) buffer is copied to real screen.
   * Not all devices may support this function, especially not, when there's no separate
   * screen / frame buffer
   */
  inline void present()
  {
    if (!present_lock_) {
      drv_present();
    }
  }


  /**
   * Prevent calling drv_present after a single primitive drawing.
   * Use this to draw complex objects like controls. After all primitives are drawn call
   * present_lock(false) which releases the lock and calls drv_present().
   * \param lock True for engaging a drv_present() lock.
   */
  inline void present_lock(bool lock = true)
  {
    present_lock_ = lock;
    if (!lock) {
      // primitive is done when lock is released
      drv_present();
    }
  }


///////////////////////////////////////////////////////////////////////////////
// M A N D A T O R Y   D R I V E R   F U N C T I O N S
//
// All functions in this section (marked with 'drv_' prefix are MANDATORY driver functions!
// Every driver MUST implement them - even if not supported
//
protected:

  /**
   * Driver init
   */
  virtual void drv_init(void) = 0;


  /**
   * Driver shutdown
   */
  virtual void drv_shutdown(void) = 0;


  /**
   * Returns the driver version and name
   * \return Driver version and name
   */
  virtual const char* drv_version() const = 0;


  /**
   * Returns the display capability: graphic or alpha numeric
   * \return True if graphic display
   */
  virtual bool drv_is_graphic(void) const = 0;


  /**
   * Set pixel in given color, the color doesn't change the actual drawing color
   * \param point Pixel coordinates
   * \param color Color of pixel in ARGB format
   */
  virtual void drv_pixel_set_color(vertex_type point, color::value_type color) = 0;


  /**
   * Return the color of the pixel
   * \param point Vertex of the pixel
   * \return Color of pixel in ARGB format
   */
  virtual color::value_type drv_pixel_get(vertex_type point) = 0;


  /**
   * Clear the entire screen with the background color or delete all characters on text display
   */
  virtual void drv_cls(void) = 0;


  /**
   * Primitive rendering is done. May be overridden by driver to update display,
   * frame buffer or something else (like copy RAM / rendering buffer to screen)
   */
  virtual void drv_present(void) = 0;

};

} // namespace vgx

#endif  // _VGX_BASE_H_
