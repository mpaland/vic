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
// \brief Null communicator
//
// This communicator is like a kind of null device in *nix sytems.
// All received data is silently discarded. No data is produced.
// It can be used for test and debugging purposes.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_COM_NULL_H_
#define _DECOM_COM_NULL_H_

#include "../com.h"

/////////////////////////////////////////////////////////////////////

namespace decom {
namespace com {

class null : public communicator
{
public:
  // standard com ctor
  null(const char* name = "com_null")
    : communicator(name)    // it's VERY IMPORTANT to call the base class ctor HERE!!!
  { }


  // dtor
  ~null()
  { }


  // called by upper layer
  virtual bool open(const char*, eid const&)
  { return true; }


  // called by upper layer
  virtual void close(eid const&)
  { }


  // transmit data
  virtual bool send(msg&, eid const&, bool)
  {
    communicator::indication(tx_done);
    return true;
  }
};

} // namespace com
} // namespace decom

#endif // _DECOM_COM_NULL_H_
