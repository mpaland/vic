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
//
//
/*
                XMODEM PROTOCOL OVERVIEW

1/1/82 by Ward Christensen.

  I will maintain a master copy of this.  Please pass on
changes or suggestions via CBBS/Chicago at (312) 545-8086, or
by voice at (312) 849-6279.


NOTE: This does not include things which I am not familiar
with, such as the CRC option implemented by John Mahr.




Last Rev: (none)

At the request of Rick Malinak on behalf of the guys at
Standard Oil with IBM P.C.s, as well as several previous
requests, I finally decided to put my modem protocol into
writing.  It had been previously formally published only in the
AMRAD newsletter.


 --------    Table of Contents

 1. DEFINITIONS
 2. TRANSMISSION MEDIUM LEVEL PROTOCOL
 3. MESSAGE BLOCK LEVEL PROTOCOL
 4. FILE LEVEL PROTOCOL
 5. DATA FLOW EXAMPLE INCLUDING ERROR RECOVERY
 6. PROGRAMMING TIPS.


 -------- 1. DEFINITIONS.

 <soh> 01H
 <eot> 04H
 <ack> 06H
 <nak> 15H
 <can> 18H



 -------- 2. TRANSMISSION MEDIUM LEVEL PROTOCOL

     Asynchronous, 8 data bits, no parity, one stop bit.

    The protocol imposes no restrictions on the contents of the
data being transmitted.  No control characters are looked for
in the 128-byte data messages.  Absolutely any kind of data may
be sent - binary, ASCII, etc.  The protocol has not formally
been adopted to a 7-bit environment for the transmission of
ASCII-only (or unpacked-hex) data , although it could be simply
by having both ends agree to AND the protocol-dependent data
with 7F hex before validating it.  I specifically am referring
to the checksum, and the block numbers and their ones-
complement.
    Those wishing to maintain compatibility of the CP/M file
structure, i.e. to allow modemming ASCII files to or from CP/M
systems should follow this data format:
  * ASCII tabs used (09H); tabs set every 8.
  * Lines terminated by CR/LF (0DH 0AH)
  * End-of-file indicated by ctl/z, 1AH.  (one or more)
  * Data is variable length, i.e. should be considered a
    continuous stream of data bytes, broken into 128-byte
    chunks purely for the purpose of transmission.

  * A CP/M "peculiarity": If the data ends exactly on a
    128-byte boundary, i.e. CR in 127, and LF in 128, a
    subsequent sector containing the ctl/z EOF character(s)
    is optional, but is preferred.  Some utilities or
    user programs still do not handle EOF without ctl/z's.
  * The last block sent is no different from others, i.e.
    there is no "short block".  


 -------- 3. MESSAGE BLOCK LEVEL PROTOCOL

 Each block of the transfer looks like:
 <SOH><blk #><255-blk #><--128 data bytes--><cksum>
    in which:

 <SOH>       = 01 hex
 <blk #>     = binary number, starts at 01 increments by 1, and
               wraps 0FFH to 00H (not to 01)
 <255-blk #> = blk # after going thru 8080 "CMA" instr.
               Formally, this is the "ones complement".
 <cksum>     = the sum of the data bytes only.  Toss any carry.



 -------- 4. FILE LEVEL PROTOCOL

 ---- 4A. COMMON TO BOTH SENDER AND RECEIVER:

    All errors are retried 10 times.  For versions running with
an operator (i.e. NOT with XMODEM), a message is typed after 10
errors asking the operator whether to "retry or quit".
    Some versions of the protocol use <can>, ASCII ctl/x, to
cancel transmission.  This was never adopted as a standard, as
having a single "abort" character makes the transmission
susceptible to false termination due to an <ack> <nak> or <soh>
being corrupted into a <can> and canceling transmission.
    The protocol may be considered "receiver driven", that is,
the sender need not automatically re-transmit, although it does
in the current implementations.

 ---- 4B. RECEIVE PROGRAM CONSIDERATIONS:

    The receiver has a 10-second timeout.  It sends a <nak>
every time it times out.  The receiver's first timeout, which
sends a <nak>, signals the transmitter to start.  Optionally,
the receiver could send a <nak> immediately, in case the sender
was ready.  This would save the initial 10 second timeout. 
However, the receiver MUST continue to timeout every 10 seconds
in case the sender wasn't ready.
    Once into a receiving a block, the receiver goes into a
one-second timeout for each character and the checksum.  If the
receiver wishes to <nak> a block for any reason (invalid
header, timeout receiving data), it must wait for the line to
clear.  See "programming tips" for ideas
    Synchronizing:  If a valid block number is received, it
will be: 1) the expected one, in which case everything is fine;
or 2) a repeat of the previously received block.  This should
be considered OK, and only indicates that the receivers <ack>
got glitched, and the sender re-transmitted; 3) any other block
number indicates a fatal loss of synchronization, such as the
rare case of the sender getting a line-glitch that looked like
an <ack>.  Abort the transmission, sending a <can>



 ---- 4C. SENDING PROGRAM CONSIDERATIONS.

    While waiting for transmission to begin, the sender has
only a single very long timeout, say one minute.  In the
current protocol, the sender has a 10 second timeout before
retrying.  I suggest NOT doing this, and letting the protocol
be completely receiver-driven.  This will be compatible with
existing programs.
    When the sender has no more data, it sends an <eot>, and
awaits an <ack>, resending the <eot> if it doesn't get one. 
Again, the protocol could be receiver-driven, with the sender
only having the high-level 1-minute timeout to abort.


 -------- 5. DATA FLOW EXAMPLE INCLUDING ERROR RECOVERY

Here is a sample of the data flow, sending a 3-block message.
It includes the two most common line hits - a garbaged block,
and an <ack> reply getting garbaged.  <xx> represents the
checksum byte.

  SENDER                           RECIEVER
                                   Times out after 10 seconds,
                           <---    <nak>
  <soh> 01 FE -data- <xx>   --->
                           <---    <ack>
  <soh> 02 FD -data- <xx>   --->   (data gets line hit)
                           <---    <nak>
  <soh> 02 FD -data- <xx>   --->
                           <---    <ack>
  <soh> 03 FC -data- <xx>   --->
    (ack gets garbaged)    <---    <ack>
  <soh> 03 FC -data- <xx>   --->
                           <---    <ack>
  <eot>                     --->
                           <---    <ack>



-------- 6. PROGRAMMING TIPS.

* The character-receive subroutine should be called with a
parameter specifying the number of seconds to wait.  The
receiver should first call it with a time of 10, then <nak> and
try again, 10 times.
  After receiving the <soh>, the receiver should call the
character receive subroutine with a 1-second timeout, for the
remainder of the message and the <cksum>.  Since they are sent
as a continuous stream, timing out of this implies a serious
like glitch that caused, say, 127 characters to be seen instead
of 128.

* When the receiver wishes to <nak>, it should call a "PURGE"
subroutine, to wait for the line to clear.  Recall the sender
tosses any characters in its UART buffer immediately upon
completing sending a block, to ensure no glitches were mis-
interpreted.
  The most common technique is for "PURGE" to call the
character receive subroutine, specifying a 1-second timeout,
and looping back to PURGE until a timeout occurs.  The <nak> is
then sent, ensuring the other end will see it.

* You may wish to add code recommended by Jonh Mahr to your
character receive routine - to set an error flag if the UART
shows framing error, or overrun.  This will help catch a few
more glitches - the most common of which is a hit in the high
bits of the byte in two consecutive bytes.  The <cksum> comes
out OK since counting in 1-byte produces the same result of
adding 80H + 80H as with adding 00H + 00H.

===============================================================

  *THE FOLLOWING IS NOT PART OF THE ORIGINAL SPECIFICATION*


A summary of two of my own modifications to the specification
to make the protocol less susceptible to failure due to delays
encountered in large networks like CIS.

I do not use a 1-second per character timeout once into
receiving a block.  I consider this overkill and not favorable
to the protocol.

On entry to Xmodem Receive initial setup is done, an initial
"nak" is sent, and a 22 second timeout clock is started.

After the first timeout this clock is then kept in 10 second
intervals until 9 more consecutive timeouts occur.  After 10
consecutive timeouts the protocol is abandoned as it is assumed
that the transmitter is not ready or the communications link
has failed.

Each character received resets the timeout clock to zero.  This
provides for the extreme possibility that the transmitter may
stop sending several times in mid-block for considerable
periods of time.  I would agree that this may be overkill also
but I believe that it is in favor of the protocol rather than
against it.

If a received block does not have the expected attributes a
"nak" is sent and a "retry count" is incremented.  After 10
consecutive retries of the same block the protocol is abandoned
as it is assumed that syncronization has been lost.

The reception of a good block resets the "retry counter" to
zero.

Naturally, if all goes well, the data will eventually be
completely received intact.

If a bad block is received a "purge" routine is invoked.  The
"purge" routine monitors the RS-232 interface until there has
been no activity for 3 seconds.  When this requirement is
satisfied the bad block just received is discarded and a "nak"
is sent.

Mike Ward
*/
///////////////////////////////////////////////////////////////////////////////


#include "prot_xmodem.h"

/////////////////////////////////////////////////////////////////////

namespace decom {
namespace prot {

// special characters in transfer protocol
#define SOH   0x01U   // CTRL-A
#define STX   0x02U   // CTRL-B
#define EOT   0x04U   // CTRL-D - End Of Transmission
#define ACK   0x06U   // CTRL-F - Positive Acknowledgement
#define NAK   0x15U   // CTRL-U - Negative Acknowledgement
#define CAN   0x18U   // CTRL-X - Cancel
#define CTRLZ 0x1AU   // CTRL-Z
#define CHARC 0x43U   // C

// maximum retransmissions
#define MAX_RETRIES 10U

// block sizes
#define BLK_SIZE      128U
#define BLK_SIZE_1K   1024U

#define TIMEOUT_ACK   10U

xmodem::xmodem(decom::layer* lower, protocol_type protocol, const char* name)
 : protocol(lower, name)    // it's VERY IMPORTANT to call the base class ctor HERE!!!
 , protocol_type_(protocol)
{
  state_ = idle;
}


xmodem::~xmodem()
{
  // close the layer gracefully BUT DO NOT CALL close() here
}


// called by upper layer
bool xmodem::open(const char* address, decom::eid const& id)
{
  // for security: check that upper protocol/device exists
  if (!upper_) {
    return false;
  }

  bool result = protocol::open(address, id);

  // open THIS layer HERE

  return result;
}


void xmodem::close(decom::eid const& id)
{
  // FIRST close THIS layer HERE
  state_ = idle;
  timer_rx_.stop();

  // Close the lower layer after closing THIS layer - closing is done TOP-DOWN
  // in layer stack
  // the highest layer is closed first, the lowest layer last
  protocol::close(id);
}


// transmit data to lower layer
bool xmodem::send(decom::msg& data, decom::eid const& id, bool more)
{
/// Incoming message data is split up into transmission blocks (128/1024 bytes)
/// and passed to the lower layer, until the message data has been sent.
/// Further the 'more' flag is evaluated. If the flag is false (data complete), the block is padded
/// to 128/1024 bytes and an <EOT> char is sent after the block.
/// If the 'more' flag is set, the protocol expects more data to send.

  // check that no transmission is in progress
  if (state_ != xmit_wait) {
    // should not happen - did you wait for tx_done?
    DECOM_LOG_ERROR("Xmodem not ready for transmission");
    return false;
  }

  // start transmission
  tx_buffer_        = data;       // store a cheap copy
  tx_offset_        = 0U;
  tx_packet_number_ = 1U;         // start with packet #1
  tx_eid_           = id;
  tx_more_          = more;
  state_            = xmit_wait;  // start transmission

  return true;
}


bool xmodem::send_block()
{
  // block structure
  // XMODEM:     | SOH | packet number | 1's cpl packet number | data 128 byte  | checksum    |
  // XMODEM-CRC: | SOH | packet number | 1's cpl packet number | data 128 byte  | CRC-16 high | CRC-16 low |
  // XMODEM-1K:  | STX | packet number | 1's cpl packet number | data 1024 byte | CRC-16 high | CRC-16 low |
 
  std::uint16_t blk_size = (protocol_type_ == xmodem_1k ? BLK_SIZE_1K : BLK_SIZE);
  std::size_t   length   = tx_buffer_.size() - tx_offset_ > blk_size ? blk_size : tx_buffer_.size() - tx_offset_;

  tx_block_.clear();
  tx_block_.insert(tx_block_.end(), tx_buffer_.begin() + tx_offset_, tx_buffer_.begin() + tx_offset_ + length);
  tx_offset_ += length;
  tx_retries_ = 0U;

  // padding
  if (length < blk_size) {
    if (tx_more_) {
      // padding is necessary, but more flag is set - this is BAD! Messages from upper layer must be multiples of block sizes!
      DECOM_LOG_CRIT("Padding orccurs but more flag is set - XMIT data gets corrupted now!");
    }
    tx_block_.insert(tx_block_.end(), blk_size - length, (decom::msg::value_type)0U);
  }

  std::uint16_t crc;
  switch (protocol_type_) {
    case xmodem_1k :
      // use XMODEM-1K
      crc = build_crc(tx_block_);                       // use crc
      tx_block_.push_back(decom::util::hi_part<std::uint16_t, std::uint8_t>(crc));
      tx_block_.push_back(decom::util::lo_part<std::uint16_t, std::uint8_t>(crc));
      tx_block_.push_front(0xFFU - tx_packet_number_);  // 1 complement of packet number
      tx_block_.push_front(tx_packet_number_);          // packet number
      tx_block_.push_front(STX);
      break;
    case xmodem_crc :
      // use XMODEM-CRC
      crc = build_crc(tx_block_);                       // use crc
      tx_block_.push_back(decom::util::hi_part<std::uint16_t, std::uint8_t>(crc));
      tx_block_.push_back(decom::util::lo_part<std::uint16_t, std::uint8_t>(crc));
      tx_block_.push_front(0xFFU - tx_packet_number_);  // 1 complement of packet number
      tx_block_.push_front(tx_packet_number_);          // packet number
      tx_block_.push_front(SOH);
      break;
    default :
      // use XMODEM
      tx_block_.push_back(build_checksum(tx_block_));   // use checksum
      tx_block_.push_front(0xFFU - tx_packet_number_);  // 1 complement of packet number
      tx_block_.push_front(tx_packet_number_);          // packet number
      tx_block_.push_front(SOH);
      break;
  }

  // start timeout timer
  timer_rx_.start(std::chrono::seconds(TIMEOUT_ACK), false, timer_rx_func, this);

  return protocol::send(tx_block_, tx_eid_);
}


// receive function for data from lower layer
void xmodem::receive(decom::msg& data, decom::eid const& id, bool)
{
  timer_rx_.stop();   // something received, stop the timer

  switch (state_)
  {
    case recv_wait :

    case recv :
      // RECEIVE
      rx_msg_.append(data);
      // msg complete?
      //if (rx_msg_.size() == xx)
        // check

      break;
    case xmit_wait :
      // wait for starting <NAK> from receiver
      if (data.size() == 1U && data[0] == NAK) {
        // start transmission
        DECOM_LOG_DEBUG("Start transmission");
        state_ = xmit;
        send_block();
      }
      break;
    case xmit :
    case xmit_eot :
      // TRANSMIT
      if (data.size() == 1U && data[0] == ACK) {
        // ACK received - process next block
        if (tx_buffer_.size() == tx_offset_) {
          // message completely sent
          if (!tx_more_ && state_ == xmit) {
            // done, no more data from upper layer, so send <EOT>
            state_ = xmit_eot;
            tx_block_.clear();
            tx_block_.push_back(EOT);
            timer_rx_.start(std::chrono::seconds(TIMEOUT_ACK), false, timer_rx_func, this);
            protocol::send(tx_block_, tx_eid_);
            break;
          }
          // notify upper layer
          protocol::indication(tx_done, tx_eid_);
          break;
        }
        else {
          // send next block
          tx_packet_number_ = static_cast<std::uint8_t>(tx_packet_number_++);
          send_block();
        }
      }
      else {
        // wrong size or NAK etc. received - resend block
        DECOM_LOG_DEBUG("NAK or invalid data, retransmit");
        if (++tx_retries_ < MAX_RETRIES) {
          timer_rx_.start(std::chrono::seconds(TIMEOUT_ACK), false, timer_rx_func, this);
          protocol::send(tx_block_, tx_eid_);
        }
        else {
          // abort, set state to idle and notify upper layer
          DECOM_LOG_WARN("Too many retries, aborting");
          state_ = idle;
          protocol::indication(tx_error, tx_eid_);
        }
      }
      break;
    default :
      // IDLE
      break;
  }
}


// error indication from lower layer
void xmodem::indication(status_type code, decom::eid const& id)
{
  // if necessary and recover is not possible, inform the upper layer
  protocol::indication(code, id);
}


bool xmodem::start(bool receive)
{
  // check that no transmission is in progress
  if (state_ != idle) {
    // should not happen - did you wait for tx_done?
    DECOM_LOG_ERROR("Xmodem not idle");
    return false;
  }
  
  if (receive) {
    state_ = recv_wait;
    timer_rx_.start(std::chrono::seconds(1U), false, timer_rx_func, this);
  }
  else {
    state_ = xmit_wait;
  }
  return true;
}


void xmodem::timer_rx_func(void* arg)
{
  xmodem* x = static_cast<xmodem*>(arg);

  switch (x->state_) {
    case recv_wait :
      {
        // send periodic NAKs to sender
        decom::msg nak(1, NAK);
        x->timer_rx_.start(std::chrono::seconds(TIMEOUT_ACK), false, timer_rx_func, arg);
        x->protocol::send(nak, x->tx_eid_);
      }
      break;
    default:
      break;
  }
}


std::uint8_t xmodem::build_checksum(decom::msg const& data) const
{
  std::uint8_t checksum = 0;
  for (decom::msg::const_iterator it = data.begin(); it != data.end(); ++it) {
    checksum = static_cast<std::uint8_t>(checksum + *it);
  }

  return checksum;
}


std::uint16_t xmodem::build_crc(decom::msg const& data) const
{
  std::uint16_t crc = 0;
  for (decom::msg::const_iterator it = data.begin(); it != data.end(); ++it) {
  }
  return crc;
}


} // namespace prot
} // namespace decom
