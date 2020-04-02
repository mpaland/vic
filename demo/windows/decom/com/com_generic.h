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
// \brief Generic com interface
// This communicator can be used as generic low level interface
// Data written to this com is directly send to the upper layer.
// Data received by this com can be read by the read() function or can be
// passed to the given rx callback function
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_COM_GENERIC_H_
#define _DECOM_COM_GENERIC_H_

#include "../com.h"

/////////////////////////////////////////////////////////////////////

namespace decom {
namespace com {


class generic : public communicator
{
public:
  /**
   * Normal com ctor
   */
  generic(const char* name = "com_generic")
    : communicator(name)   // it's VERY IMPORTANT to call the base class ctor HERE!!!
    , is_open_(false)
    , rx_more_(false)
    , rx_callback_(nullptr)
    , rx_callback_arg_(nullptr)
  { }


  /**
   * dtor
   */
  virtual ~generic()
  {
    // close the layer gracefully - you should/can call close() here because
    // it's the lowest layer
    close();
  }


  /**
   * Called by upper layer to open this layer
   * \param address The address to open
   * \param id The endpoint identifier to open
   * \return true if open is successful
   */
  virtual bool open(const char*, eid const& id = eid_any)
  {
    is_open_ = true;
    rx_buffer_.clear();

    // notify upper layer
    communicator::indication(connected, id);

    return true;
  };


  /**
   * Called by upper layer to close this layer
   * \param id The endpoint identifier to close
   */
  virtual void close(eid const& id = eid_any)
  {
    is_open_ = false;

    // notify upper layer
    communicator::indication(disconnected, id);
  }


  /**
   * Called by upper layer to transmit data to this communication endpoint / physical layer
   * \param data The message to send
   * \param id The endpoint identifier
   * \param more true if message is a fragment - mostly unused on this layer
   * \return true if Send is successful
   */
  virtual bool send(msg& data, eid const& id = eid_any, bool more = false)
  {
    if (!is_open_) {
      return false;
    }

    {
      // lock scope - acquire buffer lock and replace data
      std::lock_guard<std::mutex> lock(rx_mutex_);
      rx_buffer_ = data;
      rx_eid_    = id;
      rx_more_   = more;
    }

    // notify upper layer
    communicator::indication(tx_done, id);

    if (rx_callback_) {
      rx_callback_(rx_callback_arg_, rx_buffer_, rx_eid_, rx_more_);
      rx_buffer_.clear();
    }

    return true;
  }


  ////////////////////////////////////////////////////////////////////////
  // C O M M U N I C A T O R   A P I

  /**
   * Specialized zero copy write (buffer is passed by reference)
   * \param data Data written to communicator
   * \param id The endpoint identifier
   * \return true if write was successful
   */
  bool write(msg& data, eid const& id = eid_any)
  {
    if (!is_open_) {
      return false;
    }

    communicator::receive(data, id);   // send data to upper layer
    return true;
  }


  /**
   * STL container write
   * data is passed to the upper layer
   * \param data Data written to communicator
   * \param id The endpoint identifier
   * \return true if write was successful
   */
  template<typename Container>
  bool write(Container const& data, eid const& id = eid_any)
  {
    msg buf(data.begin(), data.end());
    return write(buf, id);
  }


  /**
   * STL iterator write
   * data is passed to the upper layer
   * \param first An input iterator addressing the position of the first element in the source
   * \param last An input iterator addressing the position of the last element in the source
   * \param id The endpoint identifier
   * \return true if write was successful
   */
  template<typename Iterator>
  bool write(Iterator first, Iterator last, eid const& id = eid_any)
  {
    decom::msg buf(first, last);
    return write(buf, id);
  }


  /**
   * Scalar write
   * byte is passed to the upper layer
   * \param data An input scalar to send
   * \param id The endpoint identifier
   * \return true if write was successful
   */
  bool write(std::uint8_t data, eid const& id = eid_any)
  {
    msg buf;
    buf.push_back(data);
    return write(buf, id);
  }


  /**
   * Wait and read data from upper layer
   * \param data Read data
   * \param id Read endpoint identifier
   * \param more State of the more bit
   * \return true if read was successful, false if error/timeout
   */
  bool read(msg& data, eid& id, bool& more)
  {
    std::lock_guard<std::mutex> lock(rx_mutex_);
    // buffer is locked here
    if (rx_callback_ || !is_open_ || rx_buffer_.empty()) {
      // nothing to do
      return false;
    }
    data = rx_buffer_;  // make a real copy
    id   = rx_eid_;
    more = rx_more_;
    rx_buffer_.clear();
    return true;
  }


  /**
   * Set the callback function which is called after a message has been received from upper layer.
   * This can be used as notification function or e.g. for message injection
   * \param rx_callback Callback function
   */
  void set_receive_callback(void* arg, void(*rx_callback)(void* arg, msg& data, eid const& id, bool more))
  {
    rx_callback_     = rx_callback;
    rx_callback_arg_ = arg;
  }


private:
  bool         is_open_;            // true if layer is open
  msg          rx_buffer_;          // rx buffer
  eid          rx_eid_;             // rx eid
  bool         rx_more_;            // rx more flag
  std::mutex   rx_mutex_;           // rx mutex
  std::condition_variable rx_ind_;  // rx indication
  void (*rx_callback_)(void* arg, msg& data, eid const& id, bool more);   // rx callback function
  void* rx_callback_arg_;           // rx callback arg
};

} // namespace com
} // namespace decom

#endif    // _DECOM_COM_GENERIC_H_
