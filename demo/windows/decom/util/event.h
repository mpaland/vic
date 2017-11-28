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
// \brief event class
//
// This class abstracts waitable events (signals) based on std::condition_variable and std::mutex
// Usage: decom::util::event ev;
//        if (ev.wait_for(std::chrono::milliseconds(1000)) == std::no_timeout) {   // wait for one second to get event set
//          // event was set
//          ev.reset();
//              :    // do something
//        }
//
//        ev.set();  // somewhere else to set event...
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_UTIL_EVENT_H_
#define _DECOM_UTIL_EVENT_H_

#include <cstdint>
#include <mutex>
#include <ratio>
#include <chrono>
#include <condition_variable>   // std::condition_variable


namespace decom {
namespace util {


class event
{
public:
  event()
    : state_(false)
  { }


  /**
   * Set the event
   */
  inline void set()
  {
    std::lock_guard<std::mutex> lk(m_);
    state_ = true;
    c_.notify_one();
  }


  /**
   * Reset the event
   */
  inline void reset()
  {
    state_ = false;
  }


  /**
   * Return the event status
   */
  inline bool get() const
  {
    return state_;
  }


  /**
   * Wait forever for the event to get signaled.
   * If the event was previously set, wait() returns immediately.
   * Does NOT reset the event.
   */
  inline void wait()
  {
    std::unique_lock<std::mutex> lk(m_);
    while (!state_) {
      c_.wait(lk);
    }
  }


  /**
   * Wait for the event for given duration to get signaled.
   * If the event was previously set, wait_for() returns immediately with std::no_timeout.
   * Does NOT reset the event.
   * \param rel_time Time to wait for the event
   * \return std::cv_status::no_timeout or std::cv_status::timeout if the event wasn't signaled in given duration
   */
  template<typename Rep, typename Period>
  std::cv_status wait_for(const std::chrono::duration<Rep, Period>& rel_time)
  {
    std::unique_lock<std::mutex> lk(m_);
    if (!state_) {
      return c_.wait_for<Rep, Period>(lk, rel_time);
    }
    return std::cv_status::no_timeout;
  }


private:
  std::mutex m_;
  std::condition_variable c_;
  volatile bool state_;
};


} // namespace util
} // namespace decom

#endif // _DECOM_UTIL_EVENT_H_
