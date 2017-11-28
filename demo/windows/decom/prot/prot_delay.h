///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2012-2017, PALANDesign Hannover, Germany
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
// \brief Delay protocol class
//
// This class delays outgoing messages for a configurable time.
// It's very useful in testing communication protocols and simulating long distance
// connections.
// Mostly used as lowest protocol level in stack.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_PROT_DELAY_H_
#define _DECOM_PROT_DELAY_H_

#include <chrono>

#include "../prot.h"


/////////////////////////////////////////////////////////////////////

namespace decom {
namespace prot {


class delay : public protocol
{
public:
  /**
   * Protocol ctor
   * \param lower Lower layer
   * \param delay Time of the message delay
   */
  delay(layer* lower, std::chrono::milliseconds delay = std::chrono::milliseconds(0U))
    : protocol(lower, "prot_delay")  // it's VERY IMPORTANT to call the base class ctor HERE!!!
    , tx_delay_(delay)
  { }


  /**
   * dtor
   */
  virtual ~delay()
  { }


  /**
   * Called by upper layer to open this layer
   * \param address The address to open
   * \param id The endpoint identifier, normally not used in open()
   * \return true if open is successful
   */
  virtual bool open(const char* address = "", eid const& id = eid_any)
  {
    // for security: check that upper protocol/device exists
    if (!upper_) {
      return false;
    }

    return protocol::open(address, id);
  }


  /**
   * Called by upper layer to close this layer
   */
  virtual void close()
  {
    protocol::close();
  }


  /**
   * Called by upper layer to transmit data (message) to this protocol
   * \param data The message to send
   * \param id The endpoint identifier
   * \param more true if message is a fragment which is followed by another msg. False if no/last fragment
   * \return true if Send is successful
   */
  virtual bool send(msg& data, eid const& id = eid_any, bool more = false)
  {
    if (!tx_buffer_.empty()) {
      // buffer is in use
      return false;
    }

    // store message data
    tx_buffer_ = data;
    tx_eid_    = id;
    tx_more_   = more;

    // trigger timer
    tx_timer_.start(tx_delay_, false, &delay::timer_callback, this);

    return true;
  }


private:
  /**
   * Timer callback routing which send the stored message to the lower layer
   * \param arg The message/channel ID
   */
  static void delay::timer_callback(void* arg)
  {
    delay* d = static_cast<delay*>(arg);

    // send data to lower layer
    d->protocol::send(d->tx_buffer_, d->tx_eid_, d->tx_more_);
    d->tx_buffer_.clear();
  }


  ////////////////////////////////////////////////////////////////////////
  // L A Y E R   A P I

public:
  /**
   * Set the delay time
   * \param delay Time of the message delay
   */
  void set_delay(std::chrono::milliseconds delay)
  {
    tx_delay_ = delay;
  }

private:
  msg                       tx_buffer_;   // tx buffer
  eid                       tx_eid_;      // tx eid
  bool                      tx_more_;     // tx more flag
  util::timer               tx_timer_;    // tx timer
  std::chrono::milliseconds tx_delay_;    // tx delay time
};

} // namespace prot
} // namespace decom

#endif // _DECOM_PROT_DELAY_H_
