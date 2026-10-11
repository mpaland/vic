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
// \brief Dummy head, use this as a non existent display replacement.
// All drawing functions are no-ops. If you need a head that stores pixels, use
// the memory head.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_DUMMY_H_
#define _VIC_HEAD_DUMMY_H_

#include "../drv.h"

// defines the head name and version
#define VIC_HEAD_DUMMY_VERSION   "Dummy head 2.00"


namespace vic {
namespace head {

/**
 * Dummy head
 * \param Screen_Size_X Screen (buffer) width
 * \param Screen_Size_Y Screen (buffer) height
 * \param Viewport_Size_X Viewport (window) width
 * \param Viewport_Size_Y Viewport (window) height
 */
template<std::uint16_t Screen_Size_X = 0U, std::uint16_t Screen_Size_Y = 0U,
         std::uint16_t Viewport_Size_X = Screen_Size_X, std::uint16_t Viewport_Size_Y = Screen_Size_Y>
class dummy : public drv
{
  bool is_graphic_;   // true if this dummy acts as graphic head, false for alpha numeric head

public:
  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S

  /**
   * ctor
   * \param viewport_x Viewport x position (left) on the screen, relative to top/left corner
   * \param viewport_y Viewport y position (top)  on the screen, relative to top/left corner
   * \param is_graphic True to act as graphic head, false to act as alpha numeric head
   */
  dummy(std::int16_t viewport_x = 0, std::int16_t viewport_y = 0, bool is_graphic = true)
    : drv(Screen_Size_X, Screen_Size_Y,
          Viewport_Size_X, Viewport_Size_Y,
          viewport_x, viewport_y)
    , is_graphic_(is_graphic)
  { }


  /**
   * dtor
   */
  ~dummy()
  {
    shutdown();
  }


  virtual void init()
  { }


  virtual void shutdown()
  { }


  inline virtual const char* version() const
  {
    return (const char*)VIC_HEAD_DUMMY_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    return is_graphic_;
  }

  // all other (drawing, text, present) functions use the drv no-op fallbacks
};

} // namespace head
} // namespace vic

#endif  // _VIC_HEAD_DUMMY_H_
