///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2011-2016, PALANDesign Hannover, Germany
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
// \brief Disturb protocol
//
// This class can be inserted anywhere in the stack and disturbs data.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_PROT_DISTURB_H_
#define _DECOM_PROT_DISTURB_H_

#include "../prot.h"


/////////////////////////////////////////////////////////////////////

namespace decom {
namespace prot {


class disturb : public protocol
{
public:
  /**
   * Protocol ctor
   * \param lower Lower layer
   */
  disturb(layer* lower)
   : protocol(lower, "prot_disturb")
  { }


  /**
   * Called by upper layer to open this layer
   * \param address The address to open
   * \param id The endpoint identifier, normally not used in open()
   * \return true if open is successful
   */
  bool open(const char* address = "", eid const& id = eid_any)
  {
    return protocol::open(address, id);
  }


  /**
   * Called by upper layer to transmit data (message) to this protocol
   * \param data The message to send
   * \param id The endpoint identifier
   * \return true if Send is successful
   */
  bool send(msg& data, eid const& id = eid_any)
  {
    static std::uint32_t count = 0U;

    if (++count == 3U) {
      // drop
      //return true;

      // copy
 //     protocol::send(data, channel);
 //     Sleep(50);

      // disturb
      //data[10] = ~data[10];
    }

    return protocol::send(data, id);
  }


  /**
   * Receive function for data from lower layer
   * \param data The message to receive
   * \param id The endpoint identifier
   */
  void receive(msg& data, eid const& id = eid_any)
  {
    protocol::receive(data, id);
  }
};

} // namespace prot
} // namespace decom

#endif // _DECOM_PROT_DISTURB_H_
