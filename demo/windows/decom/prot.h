///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2011-2017, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// This file is part of the decom library.
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
// \brief Base class for protocols
//
// This class is used for protocols (layer 3-6). Protocols have always a
// lower and an upper layer.
// All protocols are derived from this class.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_PROTOCOL_H_
#define _DECOM_PROTOCOL_H_

#include "layer.h"


namespace decom {
namespace prot {

// decom transport protocol base class
class protocol : public layer
{
private:
  /**
   * NO default ctor!
   * Derived class MUST use param ctor below
   */
  protocol()
  { }


public:
  /**
   * Param ctor - called by any derived class
   * \param lower Lower layer
   * \param name Layer name
   */
  protocol(layer* lower, const char* name)
    : layer(lower, name)
  { }


  /**
   * dtor
   */
  virtual ~protocol()
  { }
};

} // namespace protocol
} // namespace decom

#endif // _DECOM_PROTOCOL_H_
