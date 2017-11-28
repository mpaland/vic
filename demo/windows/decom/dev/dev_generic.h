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
// \brief Generic device class
//
// This generic device can be used standalone device or it can be used as
// base class for more sophisticated devices.
// Basically it receives messages and stores them in a receive buffer which
// can be read via read() function with timeout param.
// Bytes and strings can be send as messages via the write() function.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_DEV_GENERIC_H_
#define _DECOM_DEV_GENERIC_H_

#include <cstring>

#include "../dev.h"


/////////////////////////////////////////////////////////////////////

namespace decom {
namespace dev {


class generic : public device
{
public:
  // device ctor
  generic(layer* lower, const char* name = "dev_generic")
    : device(lower, name)
    , is_open_(false)
    , is_connected_(false)
    , tx_status_(disconnected)
    , callback_(nullptr)
    , callback_arg_(nullptr)
  {
    tx_ev_.set();   // set event initially
  }


  // default dtor
  ~generic()
  {
    close();
  }


  /**
   * Called by the application to open this stack. Device is the highest layer
   * \param address The address to open
   * \param id The endpoint identifier to open, if != eid_any it is used as receive filter
   * \return true if open is successful
   */
  virtual bool open(const char* address = "", eid const& id = eid_any)
  {
    if (is_open_) {
      // device already open
      DECOM_LOG_WARN("Device already open");
      return false;
    }

    eid_     = id;
    is_open_ = device::open(address, id);

    return is_open_;
  };


  /**
   * Called by the application to close stack
   * \param id The endpoint identifier to close
   */
  virtual void close(eid const& id = eid_any)
  {
    is_open_ = false;
    device::close(id);
  }


  /**
   * Receive function for data from lower layer
   * \param data The message to receive
   * \param id The endpoint identifier
   * \param more true if message is a fragment which is followed by another msg. False if no/last fragment
   */
  virtual void receive(msg& data, eid const& id = eid_any, bool more = false)
  {
    if (!is_open_) {
      // layer is closed
      return;
    }

    if (id != eid_ && !id.is_any()) {
      // msg not for us, ignore it
      return;
    }

    static bool last_more = false;

    // acquire buffer lock and append / replace received data
    std::lock_guard<std::mutex> lock(rx_mutex_);
    if (last_more) {
      // more data expected - append data
      rx_msg_.append(data);
    }
    else {
      // copy data (cheap copy)
      rx_msg_.ref_copy(data);
    }
    last_more = more;
    if (!more) {
      if (callback_) {
        callback_(callback_arg_, rx_msg_, id);
      }

      rx_ev_.set();   // trigger receive event
    }
  }


  /**
   * Status/Error indication from lower layer
   * \param code The status code which occurred on lower layer
   * \param id The endpoint identifier
   */
  virtual void indication(status_type code, eid const& id = eid_any)
  {
    if (id != eid_ && !id.is_any()) {
      // not for us, ignore indication
      return;
    }

    switch (code) {
    case connected:
      is_connected_ = true;
      con_ev_.set();
      break;
    case disconnected:
      is_connected_ = false;
      con_ev_.reset();
      break;
    case tx_done:
    case tx_error:
    case tx_timeout:
    case tx_overrun:
      tx_status_ = code;
      tx_ev_.set();
      break;
    default:
      break;
    }
  }


  ////////////////////////////////////////////////////////////////////////
  // D E V I C E   A P I

  inline bool is_connected(std::chrono::milliseconds timeout = std::chrono::milliseconds(0))
  {
    if (!is_connected_ && timeout != std::chrono::milliseconds(0)) {
      // wait for connection
      con_ev_.wait_for(timeout);
    }
    return is_connected_;
  }


  inline bool is_open() const
  { return is_open_; }


  /**
   * Zero copy write (msg buffer is passed by reference)
   * \param data Data written to device
   * \param id The endpoint identifier
   * \param blocking if true write() blocks until data is send or an error/timeout happened
   * \return true if write was successful
   */
  bool write(msg& data, eid const& id = eid_any, bool more = false, bool blocking = true)
  {
    // checks
    if (!is_open_) {
      DECOM_LOG_ERROR("Device is not opened, sending not possible");
      return false;
    }
    if (!is_connected_) {
      DECOM_LOG_ERROR("Device is not connected, sending not possible");
      return false;
    }

    if (!tx_ev_.get()) {
      // sending is still in progress
      DECOM_LOG_ERROR("Transmission in progress, sending not possible");
      return false;
    }

    tx_ev_.reset();                         // clear indication
    if (!blocking) {
      // return without waiting for tx done
      return device::send(data, id, more);  // send data to lower layer
    }
    else {
      if (device::send(data, id, more)) {   // send data to lower layer
        tx_ev_.wait();                      // wait for notification
        // return when tx indication received
        return tx_status_ == tx_done;
      }
      return false;
    }
  }


  /**
   * STL container write
   * \param data Data written to device
   * \param id The endpoint identifier
   * \param blocking if true write() blocks until data is send or an error/timeout happened
   * \return true if write was successful
   */
  template<typename Container>
  bool write(Container const& data, eid const& id = eid_any, bool more = false, bool blocking = true)
  {
    msg buf(data.begin(), data.end());
    return write(buf, id, more, blocking);
  }


  /**
   * STL iterator write
   * \param first An input iterator addressing the position of the first element in the source
   * \param last An input iterator addressing the position of the last element in the source
   * \param id The endpoint identifier
   * \return true if write was successful
   */
  template<typename Iterator>
  bool write(Iterator first, Iterator last, eid const& id = eid_any, bool more = false, bool blocking = true)
  {
    msg buf(first, last);
    return write(buf, id, more, blocking);
  }


  /**
   * String write
   * \param data An input string to send
   * \param id The endpoint identifier
   * \return true if write was successful
   */
  bool write(const char* data, eid const& id = eid_any, bool more = false, bool blocking = true)
  {
    msg buf;
    buf.put((std::uint8_t*)data, ::strlen(data));
    return write(buf, id, more, blocking);
  }


  /**
   * Scalar write
   * \param data An input scalar (uint8_t) to send
   * \param id The endpoint identifier
   * \return true if write was successful
   */
  bool write(std::uint8_t data, eid const& id = eid_any, bool more = false, bool blocking = true)
  {
    msg buf;
    buf.push_back(data);
    return write(buf, id, more, blocking);
  }


  /**
   * Array write
   * \param data An array (uint8_t) to send
   * \param id The endpoint identifier
   * \return true if write was successful
   */
  bool write(const std::uint8_t* data, std::size_t count, eid const& id = eid_any, bool more = false, bool blocking = true)
  {
    msg buf;
    buf.put(data, count);
    return write(buf, id, more, blocking);
  }


  /**
   * Read data from the device
   * \param data Read data
   * \param channel Read message/channel ID, returns the received channel
   * \param timeout Wait for [ms] to get data
   * \return true if read was successful, false if error/timeout
   */
  template<typename Container>
  bool read(Container& data, eid& id, std::chrono::milliseconds timeout)
  {
    if (rx_msg_.empty()) {
      // wait for new received data
      if (rx_ev_.wait_for(timeout) == std::cv_status::timeout) {
        // no data available (timeout)
        return false;
      }
    }
    std::unique_lock<std::mutex> lock(rx_mutex_);
    data.clear();
    data.insert(data.end(), rx_msg_.begin(), rx_msg_.end());
    id = rx_eid_;
    rx_msg_.clear();
    rx_ev_.reset();
    return true;
  }


  /**
   * Set the callback function which is called after a message has been received
   * This can be used as notification function or e.g. for message injection
   * \param rx_callback Callback function
   */
  void set_receive_callback(void* arg, void(*callback)(void* arg, msg& data, eid const& id))
  {
    callback_     = callback;
    callback_arg_ = arg;
  }


protected:
  bool          is_open_;       // true if device is open
  bool          is_connected_;  // true if device is connected
  eid           eid_;           // opened eid

  msg           rx_msg_;        // receive buffer
  eid           rx_eid_;        // receive eid
  bool          rx_more_;       // last more flag
  std::mutex    rx_mutex_;      // receive buffer mutex
  util::event   rx_ev_;         // receive event
  util::event   tx_ev_;         // transmit event
  util::event   con_ev_;        // connection event
  status_type   tx_status_;     // actual tx status

  void (*callback_)(void* arg, msg& data, eid const& id);   // receive callback function
  void*         callback_arg_;  // callback arg
};

} // namespace dev
} // namespace decom

#endif // _DECOM_DEV_GENERIC_H_
