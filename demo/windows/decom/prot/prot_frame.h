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
// \brief Frame protocol
//
// WIP - for testing only
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_PROT_FRAME_H_
#define _DECOM_PROT_FRAME_H_

#include "../prot.h"


/////////////////////////////////////////////////////////////////////

namespace decom {
namespace prot {


class frame : public protocol
{
public:
  /**
   * Protocol ctor
   * \param lower Lower layer
   */
  frame(layer* lower)
   : protocol(lower, "prot_frame")  // it's VERY IMPORTANT to call the base class ctor HERE!!!
  { }


  /**
   * Called by upper layer to open this layer
   * \param address The address to open
   * \param channel The message/channel ID, normally not used in open()
   * \return true if open is successful
   */
  bool open(std::string const& address = "", eid const& id = eid_any)
  {
    return protocol::open(address, channel);
  }


  /**
   * Called by upper layer to transmit data (message) to this protocol
   * \param data The message to send
   * \param id The endpoint identifier
   * \return true if Send is successful
   */
  bool send(msg& data, eid const& id = eid_any)
  {
    while ((data.size() % 8U) != 0U) {
      data.push_back(0U);
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
/*
    static std::deque<std::uint8_t> buffer;

    buffer.insert(buffer.end(), data.begin(), data.end());

    if (buffer.size() >= 8) {
      // frame complete
      std::deque<std::uint8_t> frame(buffer.begin(), buffer.begin() + 8);
      upper_->receive(frame, channel);
      buffer.erase(buffer.begin(), buffer.begin() + 8);
*/
    protocol::receive(data, id);
  }
};

} // namespace prot
} // namespace decom

#endif _DECOM_PROT_FRAME_H_
