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
// \brief Base class for all communicators
//
// This class is used for communication (layer 2) with hardware interfaces like serial ports,
// USB/I²C/CAN-drivers etc or used for hardware abstraction by OS API functions.
// Communicators have no lower layer, only an upper layer.
// All communicators are derived from this class
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_COMMUNICATOR_H_
#define _DECOM_COMMUNICATOR_H_

#include "layer.h"


namespace decom {
namespace com {

// decom communication base class
class communicator : public layer
{
private:
  /**
   * NO default ctor!
   * Derived class MUST use param ctor below
   */
  communicator()
  { }


public:
  /**
   * Param ctor - called by any derived class
   * \param lower Lower layer
   * \param name Layer name
   */
  communicator(const char* name)
    : layer(name) { }


  /**
   * default dtor
   */
  virtual ~communicator() { }


  /**
   * Called by upper layer to open this layer
   * Pure virtual - override this and use own open() function
   * \param address The address to open
   * \param id The endpoint identifier
   * \return true if open is successful
   */
  virtual bool open(const char* address = "", eid const& id = eid_any) = 0;


  /**
   * Called by upper layer to close this layer
   * Pure virtual - override this and use own close() function
   * \param id The endpoint identifier
   */
  virtual void close(eid const& id = eid_any) = 0;


  /**
   * Called by upper layer to transmit data interface
   * Pure virtual - override this and use own send() function
   * \param data The message to send
   * \param id The endpoint identifier
   * \param more true if message is a fragment - basically unused, ignored and not necessary on this layer
   * \return true if Send is successful
   */
  virtual bool send(msg& data, eid const& id = eid_any, bool more = false) = 0;
};

} // namespace com
} // namespace decom

#endif // _DECOM_COMMUNICATOR_H_
