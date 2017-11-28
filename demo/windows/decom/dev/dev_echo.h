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
// \brief Echo device class
//
// This device class receives messages and sends a copy of the message on the same eid
// Can be used as a small echo test server.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_DEV_ECHO_H_
#define _DECOM_DEV_ECHO_H_

#include "../dev.h"

/////////////////////////////////////////////////////////////////////

namespace decom {
namespace dev {


class echo : public device
{
public:
  /**
   * Device ctor
   * \param lower Lower layer
   */
  echo(layer* lower)
    : device(lower, "dev_echo")    // it's VERY IMPORTANT to call the base class ctor HERE!!!
  { }


  /**
   * dtor
   */
  ~echo()
  {
    close();
  }


  /**
   * Receive function for data from lower layer
   * \param data The message to receive
   * \param id The endpoint identifier
   * \param more true if message is a fragment which is followed by another msg. False if no/last fragment
   */
  void receive(msg& data, eid const& id = eid_any, bool more = false)
  {
    // echo message
    DECOM_LOG_DEBUG("Echo msg from eid ") << format_eid(id).str().c_str();
    device::send(data, id, more);
  }


  /**
   * Status/Error indication from lower layer
   */
  void indication(status_type, eid const&)
  { }


private:
  std::stringstream format_eid(eid const& id) const
  {
    std::stringstream eid_str;
    if (id.is_any()) {
      eid_str << "ANY";
    }
    else {
      eid_str << std::hex << id.addr().addr32[0] << "."
        << std::hex << id.addr().addr32[1] << "."
        << std::hex << id.addr().addr32[2] << "."
        << std::hex << id.addr().addr32[3] << ":"
        << std::dec << id.port();
    }
    return eid_str;
  }
};

} // namespace dev
} // namespace decom

#endif //_DECOM_DEV_ECHO_H_
