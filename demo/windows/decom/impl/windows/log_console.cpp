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
// \brief Logging output
//
// This module is the platform dependent log output to the Windows console.
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <stdio.h>

#include "../../log.h"


// windows console colors
typedef enum enum_log_color_type
{
  black = 0,
  dark_blue = 1,
  dark_green = 2,
  dark_aqua = 3,
  dark_red = 4,
  dark_purple = 5,
  dark_yellow = 6,
  dark_white = 7,
  gray = 8,
  blue = 9,
  green = 10,
  aqua = 11,
  red = 12,
  purple = 13,
  yellow = 14,
  white = 15
} log_color_type;


// colors of log levels
static const log_color_type log_level_color[8][2] = {
//  background  foreground             level
  { red,         yellow       },    // emerg
  { red,         white        },    // alert
  { red,         white        },    // crit
  { black,       red          },    // error
  { black,       yellow       },    // warn
  { black,       purple       },    // notice
  { black,       green        },    // info
  { black,       dark_white   }     // debug
};


// level names in plain text
static const char* log_level_name[8] = {
  "EMERG",    // 0
  "ALERT",    // 1
  "CRIT ",    // 2
  "ERROR",    // 3
  "WARN ",    // 4
  "NOTE ",    // 5
  "INFO ",    // 6
  "DEBUG"     // 7
};


class console_log
{
  CRITICAL_SECTION crit_sec_;

public:
  console_log()
  {
    ::InitializeCriticalSection(&crit_sec_);
  }


  ~console_log()
  {
    ::DeleteCriticalSection(&crit_sec_);
  }


  void out(decom::log::time_type time,decom::log::level_type level, const char* name, const char* msg)
  {
    DWORD cCharsWritten;

    // obtain exclusive console access
    ::EnterCriticalSection(&crit_sec_);

    // init console
    (void)::AllocConsole();
    (void)::SetConsoleTitleA("decom log");
    HANDLE hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);

    // get default color
    CONSOLE_SCREEN_BUFFER_INFO info;
    (void)::GetConsoleScreenBufferInfo(hStdOut, &info);

    COORD co = { 80, 3000 };
    (void)::SetConsoleScreenBufferSize(hStdOut, co);

    // timestamp
    char timestamp[16];
    snprintf(timestamp, 16U, "%02u:%02u:%02u.%03u ", (time / 3600000UL) % 24U, (time / 60000UL) % 60U, (time / 1000UL) % 60U, time % 1000UL);
    (void)::WriteConsoleA(hStdOut, timestamp, static_cast<DWORD>(strlen(timestamp)), &cCharsWritten, NULL);

    // level
    (void)::WriteConsoleA(hStdOut, "[", 1U, &cCharsWritten, NULL);
    (void)::SetConsoleTextAttribute(hStdOut, static_cast<WORD>((log_level_color[level][0] << 4U) | log_level_color[level][1U]));
    (void)::WriteConsoleA(hStdOut, log_level_name[level], static_cast<DWORD>(strlen(log_level_name[level])), &cCharsWritten, NULL);
    (void)::SetConsoleTextAttribute(hStdOut, info.wAttributes);
    (void)::WriteConsoleA(hStdOut, "] ", 2U, &cCharsWritten, NULL);

    // name
    DWORD name_len = 0U;
    for (; name[name_len] && name[name_len] != '.'; name_len++);  // cutoff name suffix
    (void)::SetConsoleTextAttribute(hStdOut, static_cast<WORD>((black << 4U) | white));
    (void)::WriteConsoleA(hStdOut, name, name_len, &cCharsWritten, NULL);
    (void)::SetConsoleTextAttribute(hStdOut, info.wAttributes);
    (void)::WriteConsoleA(hStdOut, ": ", 2U, &cCharsWritten, NULL);

    // msg
    (void)::WriteConsoleA(hStdOut, msg, static_cast<DWORD>(strlen(msg)), &cCharsWritten, NULL);

    // release exclusive access
    ::LeaveCriticalSection(&crit_sec_);
  }
};


static console_log& get_console_log()
{
  static console_log _log;
  return _log;
}


/**
* Platform dependent output implementation of logging class
* \param time Relative time of message in [ms] after system start
* \param level Log level, see log.h for definitions
* \param name Module name
* \param msg The message string for output
*/
void decom::log::out(decom::log::time_type time, decom::log::level_type level, const char* name, const char* msg) const
{
  get_console_log().out(time, level, name, msg);
}


// returns the performance counter in microseconds [µs]
static std::uint64_t get_high_resolution_clock()
{
  LARGE_INTEGER count, frequency;
  if (!::QueryPerformanceFrequency(&frequency)) {
    // error - return 0
    return 0U;
  }
  if (!::QueryPerformanceCounter(&count)) {
    // error - return 0
    return 0U;
  }

  // calculate in double, because calculation doesn't fit in uint64
  #define TICK_COUNT (static_cast<std::uint64_t>((double)count.QuadPart * (double)1000000 / (double)frequency.QuadPart))
  volatile std::uint64_t tick = TICK_COUNT;
  while (tick != TICK_COUNT) {
    tick = TICK_COUNT;
  }
  return tick;
}


static const std::uint64_t sys_tick_start = get_high_resolution_clock();  // store program start value as zero reference

/**
 * Platform dependent time implementation of logging class
 * \return Time since program start in [ms]
 */
decom::log::time_type decom::log::get_time() const
{
  return static_cast<decom::log::time_type>((get_high_resolution_clock() - sys_tick_start) / 1000U);
}
