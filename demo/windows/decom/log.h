///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2011-2016, PALANDesign Hannover, Germany
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
// \brief Universal system debug logging class
//
// This class implements a single header only logger with the according macros
// The SYS_LOG_xxx macro is only expanded if the macro level is higher than the global SYS_LOG_LEVEL.
// This way a release version does not contain any SYS_LOG functions, if SYS_LOG_LEVEL is set to SYS_LOG_LEVEL_NONE
// This class avoids any STL (stream) usage to make it suitable for small micro controllers, too.
//
// Usage: DECOM_LOG(SYS_LOG_LEVEL_NOTICE, "Some text");
//        DECOM_LOG(SYS_LOG_LEVEL_ERROR, "Error returned, code: %X", err);
//        DECOM_LOG_WARN("This is a warning");
//        DECOM_LOG_INFO("The result is: ") << result;
//
// External implementation:
// You need to implement these two functions in your project:
//
// - void sys::log::out(time_type time, level_type level level, const char* name, const char* msg)
// The output interface to the console, serial port, file etc.
//
// - time_type sys::log::get_time()
// Returns a timestamp with [ms] resolution
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_UTIL_LOG_H_
#define _DECOM_UTIL_LOG_H_



#include "decom_cfg.h"

#include <cstdint>
#include <cstddef>

#include <stdio.h>


// log levels
#define DECOM_LOG_LEVEL_NONE   -1   // logging disabled
#define DECOM_LOG_LEVEL_EMERG   0   // module/stack is unusable, usually init problems
#define DECOM_LOG_LEVEL_ALERT   1   // module/stack may be unusable, action must be taken immediately
#define DECOM_LOG_LEVEL_CRIT    2   // critical module conditions
#define DECOM_LOG_LEVEL_ERROR   3   // error conditions
#define DECOM_LOG_LEVEL_WARN    4   // warning conditions
#define DECOM_LOG_LEVEL_NOTICE  5   // normal but significant condition
#define DECOM_LOG_LEVEL_INFO    6   // informational messages
#define DECOM_LOG_LEVEL_DEBUG   7   // debug-level messages

// defines the maximum debug message length
#ifndef DECOM_LOG_MAX_MSG_LEN
#define DECOM_LOG_MAX_MSG_LEN   1024U
#endif

// define the default log level (info)
#ifndef DECOM_LOG_LEVEL
#define DECOM_LOG_LEVEL         DECOM_LOG_LEVEL_INFO
#endif

// defines the output elements per dump line
#ifndef DECOM_LOG_DUMP_ELEMENTS
#define DECOM_LOG_DUMP_ELEMENTS 16U
#endif


#if DECOM_LOG_LEVEL >= DECOM_LOG_LEVEL_EMERG
  #define DECOM_LOG_EMERG(PARAM)          decom::log(DECOM_LOG_LEVEL_EMERG, this->name_) << PARAM
  #define DECOM_LOG_EMERG2(PARAM, NAME)   decom::log(DECOM_LOG_LEVEL_EMERG, NAME)        << PARAM
#else
  #define DECOM_LOG_EMERG(PARAM)          decom::log_noop()
  #define DECOM_LOG_EMERG2(PARAM)         decom::log_noop()
#endif
#if DECOM_LOG_LEVEL >= DECOM_LOG_LEVEL_ALERT
  #define DECOM_LOG_ALERT(PARAM)          decom::log(DECOM_LOG_LEVEL_ALERT, this->name_) << PARAM
  #define DECOM_LOG_ALERT2(PARAM, NAME)   decom::log(DECOM_LOG_LEVEL_ALERT, NAME)        << PARAM
#else
  #define DECOM_LOG_ALERT(PARAM)          decom::log_noop()
  #define DECOM_LOG_ALERT2(PARAM, NAME)   decom::log_noop()
#endif
#if DECOM_LOG_LEVEL >= DECOM_LOG_LEVEL_CRIT
  #define DECOM_LOG_CRIT(PARAM)           decom::log(DECOM_LOG_LEVEL_CRIT, this->name_) << PARAM
  #define DECOM_LOG_CRIT2(PARAM, NAME)    decom::log(DECOM_LOG_LEVEL_CRIT, NAME)        << PARAM
#else
  #define DECOM_LOG_CRIT(PARAM)           decom::log_noop()
  #define DECOM_LOG_CRIT2(PARAM, NAME)    decom::log_noop()
#endif
#if DECOM_LOG_LEVEL >= DECOM_LOG_LEVEL_ERROR
  #define DECOM_LOG_ERROR(PARAM)          decom::log(DECOM_LOG_LEVEL_ERROR, this->name_) << PARAM
  #define DECOM_LOG_ERROR2(PARAM, NAME)   decom::log(DECOM_LOG_LEVEL_ERROR, NAME)        << PARAM
#else
  #define DECOM_LOG_ERROR(PARAM)          decom::log_noop()
  #define DECOM_LOG_ERROR2(PARAM, NAME)   decom::log_noop()
#endif
#if DECOM_LOG_LEVEL >= DECOM_LOG_LEVEL_WARN
  #define DECOM_LOG_WARN(PARAM)           decom::log(DECOM_LOG_LEVEL_WARN, this->name_) << PARAM
  #define DECOM_LOG_WARN2(PARAM, NAME)    decom::log(DECOM_LOG_LEVEL_WARN, NAME)        << PARAM
#else
  #define DECOM_LOG_WARN(PARAM)           decom::log_noop()
  #define DECOM_LOG_WARN2(PARAM, NAME)    decom::log_noop()
#endif
#if DECOM_LOG_LEVEL >= DECOM_LOG_LEVEL_NOTICE
  #define DECOM_LOG_NOTICE(PARAM)         decom::log(DECOM_LOG_LEVEL_NOTICE, this->name_) << PARAM
  #define DECOM_LOG_NOTICE2(PARAM, NAME)  decom::log(DECOM_LOG_LEVEL_NOTICE, NAME)        << PARAM
#else
  #define DECOM_LOG_NOTICE(PARAM)         decom::log_noop()
  #define DECOM_LOG_NOTICE2(PARAM, NAME)  decom::log_noop()
#endif
#if DECOM_LOG_LEVEL >= DECOM_LOG_LEVEL_INFO
  #define DECOM_LOG_INFO(PARAM)           decom::log(DECOM_LOG_LEVEL_INFO, this->name_) << PARAM
  #define DECOM_LOG_INFO2(PARAM, NAME)    decom::log(DECOM_LOG_LEVEL_INFO, NAME)        << PARAM
#else
  #define DECOM_LOG_INFO(PARAM)           decom::log_noop()
  #define DECOM_LOG_INFO2(PARAM, NAME)    decom::log_noop()
#endif
#if DECOM_LOG_LEVEL >= DECOM_LOG_LEVEL_DEBUG
  #define DECOM_LOG_DEBUG(PARAM)          decom::log(DECOM_LOG_LEVEL_DEBUG, this->name_) << PARAM
  #define DECOM_LOG_DEBUG2(PARAM, NAME)   decom::log(DECOM_LOG_LEVEL_DEBUG, NAME)        << PARAM
#else
  #define DECOM_LOG_DEBUG(PARAM)          decom::log_noop()
  #define DECOM_LOG_DEBUG2(PARAM, NAME)   decom::log_noop()
#endif


#if DECOM_LOG_LEVEL >= DECOM_LOG_LEVEL_EMERG
  // assert
  #define DECOM_LOG_ASSERT(expr)          (void)(                                                                               \
                                            (!!(expr)) ||                                                                       \
                                            (decom::log(DECOM_LOG_LEVEL_EMERG, __FILE__) << "ASSERT in line " << __LINE__, 0)   \
                                          )
  // verify
  #define DECOM_LOG_VERIFY(expr)          DECOM_LOG_ASSERT(expr)
#else
  #define DECOM_LOG_ASSERT(expr)          ((void)0)
  #define DECOM_LOG_VERIFY(expr)          ((void)(expr))
#endif


#if DECOM_LOG_LEVEL > DECOM_LOG_LEVEL_NONE
  #define DECOM_LOG(LEVEL, ...)                     { bool exp = LEVEL <= DECOM_LOG_LEVEL; if (exp) { decom::log _log(LEVEL, this->name); _log.msg_len_ = (std::uint16_t)snprintf(&_log.msg_[0], SYS_LOG_MAX_MSG_LEN, __VA_ARGS__); } }
  #define DECOM_LOG2(LEVEL, NAME, PARAM)            { bool exp = LEVEL <= DECOM_LOG_LEVEL; if (exp) { decom::log _log(LEVEL, NAME); _log << PARAM << "\n"; } }
  #define DECOM_LOG_DUMP(LEVEL, PARAM, BEGIN, END)  { bool exp = LEVEL <= DECOM_LOG_LEVEL; if (exp) { decom::log _log(LEVEL, this->name_); _log << PARAM << "\n"; _log.dump(BEGIN, END); } }
#else
  #define DECOM_LOG(LEVEL, PARAM)                   ((void)0)
  #define DECOM_LOG2(LEVEL, NAME, PARAM)            ((void)0)
  #define DECOM_LOG_DUMP(LEVEL, PARAM, PTR, LEN)    ((void)0)
#endif


namespace decom {

// noop class if actual level isn't active
class log_noop
{
public:
  template <typename T>
  inline log_noop& operator<< (T) { return *this; }
};


class log
{
private:
  // no public default ctor
  log()
    : level_(DECOM_LOG_LEVEL_EMERG)
    , name_("")
  { }

public:
  // level type
  typedef std::int_fast8_t    level_type;

  // time type (32 bit as default, can be 64 bit for extended range)
  typedef std::uint32_t       time_type;


  /**
   * param ctor
   * \param level Log level
   * \param name The module name
   * \param msg The (initial) message string for output
   */
  log(level_type level, const char* name = "", const char* msg = "")
    : level_(level)
    , name_(name)
    , msg_len_(0U)
  {
    if (level_ <= DECOM_LOG_LEVEL) {
      time_ = get_time();
      msgcat(msg);
    }
  }


  /**
   * dtor
   */
  ~log()
  {
    if (level_ <= DECOM_LOG_LEVEL) {
      msgcat("\n");         // end line
      msg_[msg_len_] = 0;   // terminate string
      out(time_, level_, strip_file_name(name_), msg_);
    }
  }


  /**
   * Declaration of decom::log::out() function, must be platform dependent defined
   * This function should be thread safe and use a mutex to lock the output
   */
  void out(time_type time, level_type level, const char* module_name, const char* msg) const;


  /**
   * Declaration of decom::log::get_time() function, must be platform dependent defined
   * \return The actual time after system start in milliseconds [ms]
   */
  time_type get_time() const;


  // overloaded << operators
  inline log& operator<< (char value)
  {
    if (level_ <= DECOM_LOG_LEVEL) {
      msg_len_ += (std::uint16_t)snprintf(&msg_[msg_len_], DECOM_LOG_MAX_MSG_LEN - msg_len_, "%d", value);
    }
    return *this;
  }
  inline log& operator<< (unsigned char value)
  {
    if (level_ <= DECOM_LOG_LEVEL) {
      msg_len_ += (std::uint16_t)snprintf(&msg_[msg_len_], DECOM_LOG_MAX_MSG_LEN - msg_len_, "%u", value);
    }
    return *this;
  }
  inline log& operator<< (int value)
  {
    if (level_ <= DECOM_LOG_LEVEL) {
      msg_len_ += (std::uint16_t)snprintf(&msg_[msg_len_], DECOM_LOG_MAX_MSG_LEN - msg_len_, "%d", value);
    }
    return *this;
  }
  inline log& operator<< (unsigned int value)
  {
    if (level_ <= DECOM_LOG_LEVEL) {
      msg_len_ += (std::uint16_t)snprintf(&msg_[msg_len_], DECOM_LOG_MAX_MSG_LEN - msg_len_, "%u", value);
    }
    return *this;
  }
  inline log& operator<< (long value)
  {
    if (level_ <= DECOM_LOG_LEVEL) {
      msg_len_ += (std::uint16_t)snprintf(&msg_[msg_len_], DECOM_LOG_MAX_MSG_LEN - msg_len_, "%ld", value);
    }
    return *this;
  }
  inline log& operator<< (unsigned long value)
  {
    if (level_ <= DECOM_LOG_LEVEL) {
      msg_len_ += (std::uint16_t)snprintf(&msg_[msg_len_], DECOM_LOG_MAX_MSG_LEN - msg_len_, "%lu", value);
    }
    return *this;
  }
  inline log& operator<< (long long value)
  {
    if (level_ <= DECOM_LOG_LEVEL) {
      msg_len_ += (std::uint16_t)snprintf(&msg_[msg_len_], DECOM_LOG_MAX_MSG_LEN - msg_len_, "%lld", value);
    }
    return *this;
  }
  inline log& operator<< (unsigned long long value)
  {
    if (level_ <= DECOM_LOG_LEVEL) {
      msg_len_ += (std::uint16_t)snprintf(&msg_[msg_len_], DECOM_LOG_MAX_MSG_LEN - msg_len_, "%llu", value);
    }
    return *this;
  }
  inline log& operator<< (float value)
  {
    if (level_ <= DECOM_LOG_LEVEL) {
      msg_len_ += (std::uint16_t)snprintf(&msg_[msg_len_], DECOM_LOG_MAX_MSG_LEN - msg_len_, "%f", (double)value);
    }
    return *this;
  }
  inline log& operator<< (double value)
  {
    if (level_ <= DECOM_LOG_LEVEL) {
      msg_len_ += (std::uint16_t)snprintf(&msg_[msg_len_], DECOM_LOG_MAX_MSG_LEN - msg_len_, "%a", value);
    }
    return *this;
  }
  inline log& operator<< (const char* value)
  {
    if (level_ <= DECOM_LOG_LEVEL) {
      msgcat(value);
    }
    return *this;
  }


  /**
   * Dumps the given range as hex byte dump
   * \param first Start iterator
   * \param last End iterator
   * \param elements_per_line Defines the hex bytes per line, 16 is default
   */
  template <typename T>
  void dump(T first, T last, std::size_t elements_per_line = DECOM_LOG_DUMP_ELEMENTS)
  {
    if (level_ <= DECOM_LOG_LEVEL) {
      for (std::size_t pos = 0U; (first != last) && (msg_len_ + 3U < DECOM_LOG_MAX_MSG_LEN); ++first) {
        snprintf(&msg_[msg_len_], DECOM_LOG_MAX_MSG_LEN - msg_len_, "%02X ", static_cast<const std::uint8_t>(*first));
        msg_len_ += 3U;
        if (++pos == elements_per_line / 2U) {
          msgcat(" ");
        }
        if ((pos >= elements_per_line) && (first + 1U != last)) {
          pos = 0U;
          msgcat("\n");   // end line
        }
      }
    }
  }


private:
  // concat given msg to message buffer
  inline void msgcat(const char* msg)
  {
    std::size_t m = 0U;
    while (msg[m] && msg_len_ < DECOM_LOG_MAX_MSG_LEN) {
      msg_[msg_len_++] = msg[m++];
    }
  }


  // return only the module name (cutoff the path)
  const char* strip_file_name(const char* name)
  {
    std::size_t pos = 0U, mod = 0U;
    do {
      if (name[pos] == '/' || name[pos] == '\\') {
        mod = pos + 1U;
      }
    } while (name[pos++]);
    return &name[mod];
  }


  const level_type  level_;                       // actual level
  const char*       name_;                        // module name
  time_type         time_;                        // message time
  char              msg_[DECOM_LOG_MAX_MSG_LEN];  // message buffer
  std::size_t       msg_len_;                     // actual message length

  // non copyable
  log(const log&);
  const log& operator=(const log&);
};

} // namespace decom

#endif // _DECOM_UTIL_LOG_H_
