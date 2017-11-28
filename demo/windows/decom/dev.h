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
// \brief Base class for devices
//
// This class is used for devices (layer 7). Devices have no upper layer.
// All devices are derived from this class.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_DEVICE_H_
#define _DECOM_DEVICE_H_

#include "layer.h"


namespace decom {
namespace dev {

// decom device (application layer) class
class device : public layer
{
private:
  /**
   * NO default ctor!
   * Derived class MUST use param ctor below
   */
  device()
  { }


public:
  /**
   * Param ctor - called by any derived class
   * \param lower Lower layer
   * \param name Layer name
   */
  device(layer* lower, const char* name)
    : layer(lower, name)
  { }


  /**
   * dtor
   */
  virtual ~device()
  { }


  /**
   * Receive function for data from lower layer
   * Pure virtual - override this and use own receive function
   * \param data The message to receive
   * \param id The endpoint identifier
   * \param more true if message is a fragment which is followed by another msg. False if no/last fragment
   */
  virtual void receive(msg& data, eid const& id = eid_any, bool more = false) = 0;


  /**
   * Indication from lower layer
   * Pure virtual - override this and use own indication function
   * \param code The indication which occurred on lower layer
   * \param id The endpoint identifier
   */
  virtual void indication(status_type code, eid const& id = eid_any) = 0;
};

} // namespace dev
} // namespace decom

#endif // _DECOM_DEVICE_H_
