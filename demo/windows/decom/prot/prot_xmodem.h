///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2013-2017, PALANDesign Hannover, Germany
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
// \brief XMODEM protocol
// This protocol has two receive options:
// 1. buffer the entire message (file) until an <EOT> is received. If <EOT> is
// received, the complete message (file) is passed to the upper layer.
// If this is not possible due to small memory footprint, use option 2.:
// 2. buffer only one packet (128/1024 byte) and send its payload immediately
// to the upper layer with the 'more' flag set. If <EOT> is received the last
// message to the upper layer has a false 'more' flag, indicating that the whole
// message (file) is comlete. So, the upper layer has to deal with buffering,
// e.g. writing the fragments to disk or flashing parts into eeprom.
//
// Transmission:
// Incoming message data is split up into transmission blocks (128/1024 bytes)
// and passed to the lower layer, until the message data has been sent.
// Further the 'more' flag is evaluated. If the flag is false, the block is padded
// to 128/1024 bytes and an <EOT> char is sent after the last block.
// If the 'more' flag is set, the protocol expects more data to send.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_PROT_XMODEM_H_
#define _DECOM_PROT_XMODEM_H_

#include "prot.h"

/////////////////////////////////////////////////////////////////////

namespace decom {
namespace prot {

class xmodem : public protocol
{
private:
  // disable normal layer ctor
  xmodem(decom::layer* lower, const char* name);

public:
  // params
  typedef enum enum_protocol_type {
    xmodem_chk = 0,   // XMODEM  128 byte block and checksum (original)
    xmodem_crc,       // XMODEM  128 byte block and CRC
    xmodem_1k         // XMODEM 1024 byte block and CRC
  } protocol_type;


  /**
   * Protocol ctor
   * \param lower Lower layer
   */
  xmodem(decom::layer* lower, protocol_type protocol = xmodem_chk, const char* name = "prot_xmodem");

  /**
   * dtor
   */
  virtual ~xmodem();

  /**
   * Called by upper layer to open this layer
   * \param address The address to open
   * \param id The endpoint identifier to open
   * \return true if Open is successful
   */
  virtual bool open(const char* address = "", decom::eid const& id = eid_any);

  /**
   * Called by upper layer to close this layer
   * \param id The endpoint identifier to close
   */
  virtual void close(decom::eid const& id = eid_any);

  /**
   * Called by upper layer to transmit data (message) to this protocol
   * \param data The message to send
   * \param id The endpoint identifier
   * \param more true if message is a fragment which is followed by another msg. False if no/last fragment
   * \return true if Send is successful
   */
  virtual bool send(decom::msg& data, decom::eid const& id = eid_any, bool more = false);

  /**
   * Receive function for data from lower layer
   * \param data The message to receive
   * \param more true if message is a fragment which is followed by another msg. False if no/last fragment
   */
  virtual void receive(decom::msg& data, decom::eid const& id = eid_any, bool more = false);

  /**
   * Status indication from lower layer
   * \param code The status code which occurred on lower layer
   * \param id The endpoint identifier
   */
  virtual void indication(status_type code, decom::eid const& id = eid_any);

///////////////////////////////////////////////////////////////////////////////

  /**
   * Start the transfer
   * \param receive True to receive data, false to transmit data
   */
  bool start(bool receive);

private:
  bool send_block();      // helper function to send a single block
  std::uint8_t build_checksum(decom::msg const& data) const;
  std::uint16_t build_crc(decom::msg const& data) const;

  // xmodem state
  typedef enum enum_state_type {
    idle = 0,     // idle
    recv_wait,    // wait for reception start
    recv,         // receiving
    xmit_wait,    // wait for transmission start
    xmit_eot,     // wait for transmission end
    xmit          // transmitting
  } state_type;

  state_type state_;                    // layer state
  protocol_type protocol_type_;         // protocol type

  decom::msg rx_buffer_;                // holds the complete received msg
  decom::msg rx_msg_;

  decom::eid   tx_eid_;                 // tx eid
  std::size_t  tx_offset_;              // tx offset
  bool         tx_more_;                // tx more flag
  std::uint8_t tx_packet_number_;       // tx packet number
  std::uint8_t tx_retries_;             // number of block retries
  decom::msg   tx_buffer_;              // holds the complete transmit msg
  decom::msg   tx_block_;               // holds a single block

  static void timer_rx_func(void* arg); // RX timeout timer callback
  util::timer timer_rx_;                // RX timeout timer
};

} // namespace prot
} // namespace decom

#endif // _DECOM_PROT_XMODEM_H_
