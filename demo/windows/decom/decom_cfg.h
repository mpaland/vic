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
// \brief decom configuration
//
// Setup configuration parameter
// This file CAN/MUST be touched by YOU to setup/define your environment
// RENAME this file to 'decom_cfg.h' then. When you update this lib, decom_cfg.h
// is not overwritten, BUT check it against 'decom_cfg.h.dist' for necessary changes!
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_CFG_H_
#define _DECOM_CFG_H_

// decom version information
#include "version.h"


//////////////////////////////////////////////////////////////////////////
// P L A T F O R M   D E F I N I T I O N S

// defines the which platform specific implementation is used
// this is the name of the subdirectory in 'impl'
// don't use '' or "" for the name
#define PLATFORM       windows

// Windows platform specific
#if (PLATFORM == windows)
// undefine macros which are defined by some platform headers (e.g. Windows.h) to
// avoid obvious conflicts like with std::min(), std::max() etc.
#if defined(_MSC_VER)
#undef min
#undef max
#endif  // _MSC_VER

// define _WINSOCKAPI_ to stop windows.h including old winsock.h
#define _WINSOCKAPI_

#endif // PLATFORM == windows


//////////////////////////////////////////////////////////////////////////
// B Y T E   O R D E R

// define byte order here
// possible values __ORDER_LITTLE_ENDIAN__, __ORDER_BIG_ENDIAN__ or __ORDER_PDP_ENDIAN__
#define DECOM_BYTE_ORDER  __ORDER_LITTLE_ENDIAN__

// Ms uses little endian
#if defined(_MSC_VER)
  #undef DECOM_BYTE_ORDER
  #define DECOM_BYTE_ORDER  __ORDER_LITTLE_ENDIAN__
#endif

// check
#ifndef DECOM_BYTE_ORDER
  #error System byte order is not defined
#elif DECOM_BYTE_ORDER !=  __ORDER_BIG_ENDIAN__ && DECOM_BYTE_ORDER !=  __ORDER_LITTLE_ENDIAN__ && DECOM_BYTE_ORDER != __ORDER_PDP_ENDIAN__
  #error Unknown byte order useed
#endif


//////////////////////////////////////////////////////////////////////////
// M E S S A G E   P O O L

// defines the page size in bytes
// modify this value according to your belongings
#define DECOM_MSG_POOL_PAGE_SIZE    128U

// defines the number of pages in the pool
// POOL_PAGE_SIZE * POOL_PAGES is the total required memory size
#define DECOM_MSG_POOL_PAGES        128U

// defines the start offset of a new message within the first page
// set this value according to expected header length of all lower layer protocols, so that
// allocation of a new page is not necessary
// depends on the used protocols, a good start is a quarter of the page
#define DECOM_MSG_POOL_PAGE_BEGIN   (DECOM_MSG_POOL_PAGE_SIZE / 4U)


//////////////////////////////////////////////////////////////////////////
// S T A T I S T I C S

// define this to turn on layer statistics (mostly for debugging purpose)
#define DECOM_STATS

// disable for MS release
#if defined(_MSC_VER) && !defined(_DEBUG)
#undef DECOM_STATS
#endif


//////////////////////////////////////////////////////////////////////////
// L O G G I N G

// logging level
// available levels are NONE, DEBUG, INFO, NOTICE, WARN, ERROR, CRIT, ALERT, EMERG
// NONE or undefined DECOM_LOG_LEVEL will disable logging
// messages below the defined logging level are suppressed
#define DECOM_LOG_LEVEL   DECOM_LOG_LEVEL_WARN

// disable for MS release
#if defined(_MSC_VER) && !defined(_DEBUG)
#undef DECOM_LOG_LEVEL
#define DECOM_LOG_LEVEL   DECOM_LOG_LEVEL_NONE
#endif


#endif // _DECOM_CFG_H_
