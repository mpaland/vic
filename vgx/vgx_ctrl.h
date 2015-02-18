///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2015, PALANDesign Hannover, Germany
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
// \brief Base class for complex controls like gauges, buttons, progress bars,
//        checkboxes, switches, etc.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_CTRL_H_
#define _VGX_CTRL_H_

#include "vgx_drv.h"


namespace vgx {

class ctrl
{
public:
  /**
   * ctor
   * \param driver Reference to driver
   */
  ctrl(drv& head)
    : head_(head)
  { };

  /**
   * Check if the given coordinate is inside the control
   * \param x X coordinate
   * \param y Y coordinate
   * \return true if coordinate is inside the control
   */
  virtual bool is_inside(std::int16_t x, std::int16_t y) = 0;

protected:
  drv& head_;

private:
  const ctrl& operator=(const ctrl& rhs) { return rhs; }  // non copyable
};

} // namespace vgx

#endif  // _VGX_CTRL_H
