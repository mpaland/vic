///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2017, PALANDesign Hannover, Germany
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
// \brief Base class for complex controls like gauges, buttons, progress bars,
//        checkboxes, switches, etc.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_CTRL_H_
#define _VIC_CTRL_H_

#include "../drv.h"


namespace vic {
namespace ctrl {


/**
 * Ctrl base class
 */
class base
{
protected:
  drv&  head_;    // head instance
  base* next_;    // next ctrl in chain

public:
  /**
   * ctor
   * \param head Reference to the head instance
   */
  base(drv& head)
    : head_(head)
    , next_(nullptr)
  {
    // register ctrl
    if (!*get_root()) {
      // set ctrl as root element
      *get_root() = this;
    }
    else {
      // append ctrl in the chain
      for (base* b = *get_root(); b != nullptr; b = b->next_) {
        if (!b->next_) {
          b->next_ = this;
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
      for (base* b = *get_root(); b != nullptr; b = b->next_) {
        if (this == b->next_) {
          b->next_ = next_;
          return;
        }
      }
    }
  }


  /**
   * Check if the given vertex is inside the control
   * \param point Point to check
   * \return true if point is inside the control
   */
  virtual bool is_inside(vertex_type vertex) = 0;
  

protected:
  inline base** get_root()
  {
    static base* _root = nullptr;
    return &_root;
  }


private:
  const base& operator=(const base& rhs) { return rhs; }  // non copyable
};

} // namespace ctrl
} // namespace vic

#endif  // _VIC_CTRL_H
