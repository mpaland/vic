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
// \brief Loopback interface
// This class implements a loopback interface on the lowest layer
// Two stacks may be connected together for testing purpose
// Incoming messages are 
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_COM_LOOPBACK_H_
#define _DECOM_COM_LOOPBACK_H_

#include <queue>
#include <thread>

#include "../com.h"

/////////////////////////////////////////////////////////////////////

namespace decom {
namespace com {


class loopback : public communicator
{
public:

  /**
   * Normal com ctor
   */
  loopback(const char* name = "com_loopback")
    : communicator(name)   // it's VERY IMPORTANT to call the base class ctor HERE!!!
    , counter_loopback_(nullptr)
    , is_open_(false)
    , worker_end_(false)
  {
    // create worker thread
    std::thread t(&worker, this);
    thread_.swap(t);
  }


  /**
   * dtor
   */
  ~loopback()
  {
    is_open_    = false;
    worker_end_ = true;
    data_ind_.notify_all();
    thread_.join();
  }


  static void worker(void* arg)
  {
    loopback* l = static_cast<loopback*>(arg);
    std::mutex m;

    do {
      std::unique_lock<std::mutex> lk(m);
      l->data_ind_.wait(lk);

      // send data
      if (l->is_open_ && !l->send_queue_.empty()) {
        l->mutex_.lock();
        txdata_type data = l->send_queue_.front();
        l->send_queue_.pop();
        l->mutex_.unlock();
        l->counter_loopback_->receive(data.data, data.id, data.more);
      }
    } while (!l->worker_end_);
  }


  /**
   * Called by upper layer to open the loopback interface
   * \param address The address to open
   * \param id The endpoint identifier to open
   * \return true if open is successful
   */
  virtual bool open(const char*, eid const&)
  {
    // for security: check that upper protocol/device exists
    if (!upper_ || !counter_loopback_) {
      return false;
    }

    is_open_ = true;

    // send open indication
    communicator::indication(connected);

    return true;
  };


  /**
   * Called by upper layer to close the loopback interface
   * \param id The endpoint identifier to close
   */
  virtual void close(eid const&)
  {
    is_open_ = false;
  }


  /**
   * Called by upper layer to transmit data to this loopback interface
   * \param data The message to send
   * \param id The endpoint identifier
   * \param more true if message is a fragment - mostly unused on this layer
   * \return true if Send is successful
   */
  virtual bool send(msg& data, eid const& id = eid_any, bool more = false)
  {
    communicator::indication(tx_done, id);
    // pass the data to the other stack / counter loopback part
    if (is_open_ && counter_loopback_) {
      mutex_.lock();
      send_queue_.push({ data, id, more });
      mutex_.unlock();
      data_ind_.notify_all();
      return true;
    }
    return false;
  }


  /**
   * A complete loopback interface consists of two loopback interfaces,
   * one for each stack.
   * The loopback interfaces MUST be registered one another before usage
   * \param loopback The counter loopback interface
   */
  void register_loopback(loopback* counter_loopback)
  { counter_loopback_ = counter_loopback; };


private:
  bool is_open_;                      // true if open
  std::condition_variable data_ind_;  // data indication
  std::thread thread_;                // worker thread
  volatile bool worker_end_;          // terminate thread
  std::mutex mutex_;                  // queue lock

  typedef struct struct_txdata_type {
    msg  data;
    eid  id;
    bool more;
  } txdata_type;

  std::queue<txdata_type> send_queue_;  // send queue

  loopback* counter_loopback_;          // pointer to the counter loopback part
};

} // namespace com
} // namespace decom

#endif  // _DECOM_COM_LOOPBACK_H_
