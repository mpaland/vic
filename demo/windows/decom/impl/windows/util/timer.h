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
// \brief Timer implementation for Windows(tm) system API
//
// This class implements a high precision timer on the Windows(tm) platform.
// The problem is that Windows timers are unprecise. Sleep() and WaitForSingleObject()
// functions have an accuracy of 10-20ms. New timer queues are pretty much the same.
// Accurary of the high precision counter is fine but the counter needs active polling.
// This timer class uses a dual approach: Waiting for long intervals is done by WaitForSingleObject()
// and short intervals (<30ms) are done by active polling QueryPerformanceCounter
// in worker thread.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_UTIL_TIMER_H_
#define _DECOM_UTIL_TIMER_H_

#include <windows.h>
#include <process.h>
#include <chrono>

#include "../../../decom_cfg.h"

/////////////////////////////////////////////////////////////////////

namespace decom {
namespace util {


class timer
{
public:
  typedef enum tag_priority_type {
    //                        windows mappings
    priority_idle           = THREAD_BASE_PRIORITY_IDLE,
    priority_low            = THREAD_BASE_PRIORITY_MIN,
    priority_normal         = THREAD_PRIORITY_NORMAL,      // use this if every timer resolution is > 100ms
    priority_high           = THREAD_BASE_PRIORITY_MAX,    // default, sufficient for most scenarios
    priority_time_critical  = THREAD_BASE_PRIORITY_LOWRT   // use this if high precision is necessary, e.g. for LIN/CAN scheduling etc.
  } priority_type;

  /**
   * param ctor
   * \param prio Defines the timer thread priority - ignored if platform doesn't support thread priority
   */
  timer(priority_type prio = priority_high)
  {
    running_    = false;    // initially not running
    worker_end_ = false;
    callback_   = nullptr;
    trigger_handle_ = ::CreateEvent(NULL, FALSE, FALSE, NULL);

    // create timer thread
    thread_handle_ = reinterpret_cast<HANDLE>(::_beginthreadex(
      NULL,
      0U,
      reinterpret_cast<unsigned(__stdcall *)(void*)>(worker),
      reinterpret_cast<void*>(this),
      CREATE_SUSPENDED,
      NULL
    ));

    if (thread_handle_) {
      // set prio and start thread
      (void)::SetThreadPriority(thread_handle_, (int)prio);   // set priority
      (void)::SetThreadAffinityMask(thread_handle_, 0x01U);   // avoid problems with multi core CPU clocks
      (void)::ResumeThread(thread_handle_);                   // start worker thread
    }
  }


  /**
   * dtor
   */
  ~timer()
  {
    // gracefully stop and kill the timer thread
    stop();
    worker_end_ = true;
    (void)::SetEvent(trigger_handle_);                        // trigger thread
    (void)::WaitForSingleObject(thread_handle_, INFINITE);    // join thread
    (void)::CloseHandle(thread_handle_);
    (void)::CloseHandle(trigger_handle_);
  }


  /**
   * Worker thread
   */
  static void worker(void* arg)
  {
    timer* t = static_cast<decom::util::timer*>(arg);

    LARGE_INTEGER llFrequency, llElapsed;
    (void)::QueryPerformanceFrequency(&llFrequency);

    do {
      (void)::WaitForSingleObject(t->trigger_handle_, INFINITE);

      while (t->running_) {
        (void)::QueryPerformanceCounter(&llElapsed);
        llElapsed.QuadPart = llElapsed.QuadPart - t->trigger_time_.QuadPart;
        llElapsed.QuadPart *= 1000000UL;
        llElapsed.QuadPart /= llFrequency.QuadPart;
        if (llElapsed.QuadPart >= t->period_.count())
        {
          // time elapsed - set new trigger time
          t->trigger_time_.QuadPart += (t->period_.count() * llFrequency.QuadPart) / 1000000UL;

          // execute callback function
          if (t->callback_) {
            if (!t->periodic_) {
              // execute only once, disable before callback execution
              t->running_ = false;
            }
            // execute callback here - CAUTION: timer may be restarted in callback!
            t->callback_(t->arg_);
          }
        }

        // time to next trigger
        const LONGLONG next_trg = llElapsed.QuadPart < t->period_.count() ? t->period_.count() - llElapsed.QuadPart : 0U;

        // WaitForSingleObject() has a resolution of 10ms, so a WaitForSingleObject(xx, 20) may return after max 30ms
        if ((next_trg >= 30000U) && (!t->worker_end_)) {
          (void)::WaitForSingleObject(t->trigger_handle_, static_cast<DWORD>((next_trg / 1000U) - 10U));
        }
      }
    } while (!t->worker_end_);
  }


  /**
   * Set process to idle for the given time of milliseconds
   * Standard implementation is an OS wrapper in most cases
   * \param milliseconds The time to sleep in [ms] or other units (converted by chrono)
   */
  static inline void sleep(std::chrono::milliseconds milliseconds)
  {
    // call OS sleep - CAUTION: only accuracy of 10-20ms
    ::Sleep(static_cast<DWORD>(milliseconds.count()));
  }


  inline void wait(std::chrono::microseconds microseconds)
  {
    start(microseconds, false, nullptr, nullptr);
    while (running_) {
      if (microseconds.count() >= 20000U) {
        ::Sleep(10U);
      }
    };
  }


  /**
   * Start/restart the a periodic timer. If the timer is running it is restarted.
   * \param microseconds Cycle time of the timer in [µs] or other chrono units (converted by chrono)
   * \param periodic true for a periodic timer, false for a singleshot timer
   * \param func Callback function which is called when timer fires
   * \param arg Argument which is passed to the callback function
   * \return true if timer started successfully
   */
  bool start(std::chrono::microseconds period, bool periodic, void(*func)(void* arg), void* arg)
  {
    (void)::QueryPerformanceCounter(&trigger_time_);
    period_   = period;
    periodic_ = periodic;
    arg_      = arg;
    callback_ = func;
    running_  = true;
    (void)::SetEvent(trigger_handle_);

    return true;
  }


  /**
   * Stop the timer
   * \return true if stopped successfully
   */
  inline bool stop()
  {
    running_ = false;
    (void)::SetEvent(trigger_handle_);

    return true;
  }


  /**
   * Returns the time since the last (re)start of the timer
   * \return The time since the lass (re)start of the timer, -1 if timer is not running
   */
  std::chrono::microseconds get_elapsed() const
  {
    if (!running_) {
      return std::chrono::microseconds(-1);
    }

    LARGE_INTEGER llFrequency, llElapsed;
    (void)::QueryPerformanceFrequency(&llFrequency);
    (void)::QueryPerformanceCounter(&llElapsed);
    llElapsed.QuadPart = llElapsed.QuadPart - trigger_time_.QuadPart;
    llElapsed.QuadPart *= 1000000UL;
    llElapsed.QuadPart /= llFrequency.QuadPart;

    return std::chrono::microseconds(llElapsed.QuadPart);
  }


  /**
   * Status of the timer
   * \return true if timer is running
   */
  inline bool is_running() const
  { return running_; }


  /////////////////////////////////////////////////////////////////////////////
  // P R I V A T E
private:
  std::chrono::microseconds period_;    // timer period
  volatile bool running_;               // timer status
  volatile bool worker_end_;            // end worker thread
  bool periodic_;                       // true if periodic timer
  void* arg_;                           // argument for callback function
  void (*callback_)(void* arg);         // callback function
  HANDLE thread_handle_;                // thread handle
  HANDLE trigger_handle_;               // timer start handle
  LARGE_INTEGER trigger_time_;          // time of last trigger
};

} // namespace util
} // namespace decom

#endif // _DECOM_UTIL_TIMER_H_
