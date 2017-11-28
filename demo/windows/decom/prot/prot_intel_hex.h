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
// \brief Intel HEX protocol class
//
// This class parses an INTEL_HEX format message stream and sends the extracted record data
// as a record block to the lower layer. The channel is used as address.
// The 'more'-bit is set until an EOF record is received
// see http://en.wikipedia.org/wiki/Intel_HEX for format description
//
// There are six record types, four are supported:
//
// 00  Data record
//     Contains data and 16-bit address. The format described above.
// 01  End Of File record
//     Must occur exactly once per file in the last line of the file.
//     The byte count is 00 and the data field is empty. Usually the address field is also 0000,
//     in which case the complete line is ':00000001FF'.
// 02  Extended Segment Address Record
//     Segment-base address (two hex digit pairs in big endian order).
//     Used when 16 bits are not enough, identical to 80x86 real mode addressing. The address specified
//     by the data field of the most recent 02 record is multiplied by 16 (shifted 4 bits left) and
//     added to the subsequent 00 record addresses. This allows addressing of up to a megabyte of address
//     space. The address field of this record has to be 0000, the byte count is 02 (the segment is 16-bit).
//     The least significant hex digit of the segment address is always 0.
// 03  Start Segment Address Record
//     For 80x86 processors, it specifies the initial content of the CS:IP registers.
//     UNSUPPORTED and ignored
// 04  Extended Linear Address Record
//     Allowing for fully 32 bit addressing (up to 4GiB). The address field is 0000, the byte count is 02.
//     The two data bytes (two hex digit pairs in big endian order) represent the upper 16 bits of the 32 bit
//     address for all subsequent 00 type records until the next 04 type record comes.
//     If there is not a 04 type record, the upper 16 bits default to 0000. To get the absolute address for
//     subsequent 00 type records, the address specified by the data field of the most recent 04 record is
//     added to the 00 record addresses.
// 05  Start Linear Address Record
//     UNSUPPORTED and ignored
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_PROT_INTEL_HEX_H_
#define _DECOM_PROT_INTEL_HEX_H_

#include "../prot.h"


/////////////////////////////////////////////////////////////////////

namespace decom {
namespace prot {


class intel_hex : public protocol
{
public:
  /**
   * Protocol ctor
   * \param lower Lower layer
   */
  intel_hex(layer* lower, const char* name = "prot_intel_hex")
    : protocol(lower, name)
  {
    hex_nibble_ = false;
    hex_[2] = 0U;
    address_ = 0U;
    state_ = start_code_st;
  }


  /**
   * dtor
   */
  virtual ~intel_hex()
  { }


  /**
   * Called by upper layer to open this layer
   * \param address The address to open
   * \param id The endpoint identifier to open
   * \return true if open is successful
   */
  virtual bool open(const char* address = "", eid const& id = eid_any)
  {
    // for security: check that upper protocol/device exists
    if (!upper_) {
      return false;
    }

    address_ = 0;
    state_ = start_code_st;

    return protocol::open(address, id);
  }


  /**
   * Called by upper layer to close this layer
   * \param id The endpoint identifier to close
   */
  virtual void close(eid const& id = eid_any)
  {
    protocol::close(id);
  }


  /**
   * Called by upper layer to transmit data (message) to this protocol
   * \param data The message to send
   * \param id The endpoint identifier
   * \param more true if message is a fragment which is followed by another msg. False if no/last fragment
   * \return true if Send is successful
   */
  virtual bool send(msg& data, eid const& id = eid_any)
  {
    for (decom::msg::const_iterator it = data.begin(); it != data.end(); ++it)
    {
      std::uint8_t val = 0U;

      // rebuild hex value out of two chars
      hex_[0] = hex_[1];
      hex_[1] = *it;
      hex_nibble_ = !hex_nibble_;
      if ((!hex_nibble_) || (*it == ':')) {
        val = util::hex2int<std::uint8_t>(hex_);
        checksum_ += val;
      }
      else {
        continue;
      }

      switch (state_) {
      case start_code_st:
        if (*it == ':') {
          // start code found
          hex_nibble_ = false;
          checksum_ = 0U;
          state_ = byte_count_st;
        }
        break;
      case byte_count_st:
        byte_count_ = val;
        state_ = offset1_st;
        break;
      case offset1_st:
        offset_ = val;
        state_ = offset2_st;
        break;
      case offset2_st:
        offset_ = (offset_ << 8) | val;
        state_ = record_type_st;
        break;
      case record_type_st:
        record_type_ = val;
        state_ = data_st;
        break;
      case data_st:
        if (byte_count_-- == 0) {
          checksum_ -= val;
          state_ = checksum_st;
          // no break, continue in checksum
        }
        else {
          data_.push_back(val);
          break;
        }
        // NO break here!
      case checksum_st:
        if (checksum_ == static_cast<std::uint8_t>((val ^ 0xFFU) + 1)) {
          // checksum correct - process record type
          switch (record_type_) {
          case 0x00U:
            // data record
            tx_eof_ = false;
            tx_ev_.reset();     // clear indication
            if (!protocol::send(data_, address_ + offset_, true)) {
              return false;
            }
            tx_ev_.wait();
            if (tx_status_ != tx_done) {
              return false;
            }
            data_.clear();
            break;
          case 0x01U:
            // End Of File - notify lower layer
            tx_eof_ = true;
            return protocol::send(data_, 0, false);
            break;
          case 0x02U:
            // Extended segment address record
            address_ = ((std::uint32_t)data_[0] << 12U) + ((std::uint32_t)data_[1] << 4U);
            data_.clear();
            break;
          case 0x03U:
            // Start Segment Address Record - IGNORED
            break;
          case 0x04U:
            // Extended linear address record
            address_ = ((std::uint32_t)data_[0] << 24U) + ((std::uint32_t)data_[1] << 16U);
            data_.clear();
            break;
          case 0x05U:
            // Start Linear Address Record - IGNORED
            break;
          default:
            // unknown or unsupported record - discard
            DECOM_LOG_NOTICE("Unknown/unsupported record type: " << (int)record_type_);
            break;
          }
        }
        else {
          DECOM_LOG_WARN("Checksum error - record discarded");
        }
        state_ = start_code_st;
        break;
      default:
        break;
      }
    }

    // message processed
    protocol::indication(tx_done, id);

    return true;
  }


  /**
   * Status indication from lower layer
   * \param code The status code which occurred on lower layer
   * \param id The endpoint identifier
   */
  virtual void indication(status_type code, eid const& id = eid_any)
  {
    if (code == tx_done || code == tx_error || code == tx_timeout || code == tx_overrun) {
      tx_status_ = code;
      tx_ev_.set();
    }

    if (tx_eof_) {
      protocol::indication(code, id);
    }
  }


  ////////////////////////////////////////////////////////////////////////

private:
  // state
  typedef enum enum_state_type {
    start_code_st = 0,        // wait for start code ':'
    byte_count_st,            // byte count
    offset1_st,               // address offset high
    offset2_st,               // address offset low
    record_type_st,           // record type
    data_st,                  // data
    checksum_st               // checksum
  } state_type;

  state_type state_;          // state

  decom::msg data_;           // data buffer
  char hex_[3];               // hex conversion buffer for two digits
  bool hex_nibble_;           // active hex nibble

  std::uint32_t address_;     // absolute address of extended/linear address record
  std::uint32_t offset_;      // offset
  std::uint8_t byte_count_;   // byte count
  std::uint8_t record_type_;  // record type

  std::uint8_t checksum_;     // checksum

  bool tx_eof_;               // EOF record detected
  util::event tx_ev_;         // transmit event
  status_type tx_status_;
};

} // namespace prot
} // namespace decom

#endif // _DECOM_PROT_INTEL_HEX_H_
