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
// \brief Base class for complex widgets like gauges, buttons, progress bars,
//        checkboxes, switches, etc.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_H_
#define _VIC_WIDGET_H_

#include "../drv.h"


namespace vic {
namespace widget {


/**
 * Widget base class
 */
class base
{
protected:
  drv&  head_;    // head instance
  base* next_;    // next widget in chain

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
      // set widget as root element
      *get_root() = this;
    }
    else {
      // append widget in the chain
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
    // remove widget from list
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
   * Init the widget
   * Initially draw the widget, init structures etc.
   */
  virtual void init(void) = 0;


  /**
   * Find a widget by vertex
   * This can be used to find the according widget if a touch event happend
   * \param vertex Vertex to check
   * \return Pointer to the widget containing the vertex, nullptr if no widget was found
   */
  base* find(vertex_type vertex)
  {
    for (base* b = *get_root(); b != nullptr; b = b->next_) {
      if (b->is_inside(vertex)) {
        // widget found
        return b;
      }
    }
    // no widget found
    return nullptr;
  }


protected:

  /**
   * Check if the given vertex is inside the widget
   * \param vertex Vertex to check
   * \return true if point is inside the widget
   */
  virtual bool is_inside(vertex_type vertex) const = 0;


  // get the root of the widget chain
  inline base** get_root()
  {
    static base* _root = nullptr;
    return &_root;
  }


private:
  const base& operator=(const base& rhs) { return rhs; }  // non copyable
};

} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_H_
