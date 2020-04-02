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
// \brief Debug protocol class
//
// This class can be inserted anywhere in the stack and logs input and output data.
// All data is passed transparently.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_PROT_DEBUG_H_
#define _DECOM_PROT_DEBUG_H_

#include <sstream>

#include "../prot.h"


/////////////////////////////////////////////////////////////////////

namespace decom {
namespace prot {


class debug : public protocol
{
public:
  /**
   * Protocol ctor
   * \param lower Lower layer
   * \param name The name which will be logged as layer name, if empty, "debug" is default
   */
  debug(layer* lower, const char* name = "")
    : protocol(lower, *name ? name : "debug")
  { }


  /**
   * Called by upper layer to open this layer
   * \param address The address to open
   * \param eid The endpoint identifier
   * \return true if open is successful
   */
  virtual bool open(const char* address = "", eid const& id = eid_any)
  {
    DECOM_LOG_INFO("OPEN '" << address << "', eid " << format_eid(id).str().c_str());
    return protocol::open(address, id);
  }


  /**
   * Called by upper layer to close this layer
   * \param id The endpoint identifier to close
   */
  virtual void close(eid const& id = eid_any)
  {
    DECOM_LOG_INFO("CLOSE eid " << format_eid(id).str().c_str());
    protocol::close(id);
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
    DECOM_LOG_DUMP(DECOM_LOG_LEVEL_DEBUG, upper_->name_ << " -> " << lower_->name_ << ", eid " << format_eid(id).str().c_str() << (more ? ", more" : ", last") << ", len " << data.size(), data.begin(), data.end());
    return protocol::send(data, id, more);
  }


  /**
   * Receive function for data from lower layer
   * \param data The message to receive
   * \param id The endpoint identifier
   * \param more true if message is a fragment which is followed by another msg. False if no/last fragment
   */
  virtual void receive(msg& data, eid const& id = eid_any, bool more = false)
  {
    DECOM_LOG_DUMP(DECOM_LOG_LEVEL_DEBUG, lower_->name_ << " -> " << upper_->name_ << ", eid " << format_eid(id).str().c_str() << (more ? ", more" : ", last") << ", len " << data.size(), data.begin(), data.end());
    protocol::receive(data, id, more);
  }


  /**
   * Error indication from lower layer
   * \param code The error code which occurred on lower layer
   * \param id The endpoint identifier
   */
  virtual void indication(status_type code, eid const& id = eid_any)
  {
    switch (code) {
      case connected :
        DECOM_LOG_INFO("CONNECTED indication on eid " << format_eid(id).str().c_str());
        break;
      case disconnected :
        DECOM_LOG_INFO("DISCONNECTED indication on eid " << format_eid(id).str().c_str());
        break;
      case tx_done :
        DECOM_LOG_INFO("TX_DONE indication on eid " << format_eid(id).str().c_str());
        break;
      case tx_error   :
        DECOM_LOG_INFO("TX_ERROR indication on eid " << format_eid(id).str().c_str());
        break;
      case tx_timeout :
        DECOM_LOG_INFO("TX_TIMEOUT indication on eid " << format_eid(id).str().c_str());
        break;
      case tx_overrun :
        DECOM_LOG_INFO("TX_OVERRUN indication on eid " << format_eid(id).str().c_str());
        break;
      case rx_error   :
        DECOM_LOG_INFO("RX_ERROR indication on eid " << format_eid(id).str().c_str());
        break;
      case rx_timeout :
        DECOM_LOG_INFO("RX_TIMEOUT indication on eid " << format_eid(id).str().c_str());
        break;
      case rx_overrun :
        DECOM_LOG_INFO("RX_OVERRUN indication on eid " << format_eid(id).str().c_str());
        break;
      default :
        DECOM_LOG_INFO("UNKNOWN indication on eid " << format_eid(id).str().c_str());
    }
    protocol::indication(code, id);
  }


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

} // namespace prot
} // namespace decom

#endif // _DECOM_PROT_DEBUG_H_
