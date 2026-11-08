///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// This file is part of the vic library.
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
// \brief main() for Linux (X11) demo programs
// Link this file together with the according demo_xxx.cpp file
//
///////////////////////////////////////////////////////////////////////////////

#include <time.h>

#include "head/X11.h"


// create a single head
vic::drv& get_singlehead()
{
  // create 320 x 200 pixel window with zoom factor 2
  static vic::head::x11<320, 200> _head( 0, 0, 0, 0, 2U, 2U );
  return _head;
}


///////////////////////////////////////////////////////////////////////////////
// I / O   A B S T R A C T I O N

void vic::io::delay(std::uint32_t time_to_delay)
{
  struct timespec ts;
  ts.tv_sec  = static_cast<time_t>(time_to_delay / 1000U);
  ts.tv_nsec = static_cast<long>(time_to_delay % 1000U) * 1000000L;
  ::nanosleep(&ts, nullptr);
}

///////////////////////////////////////////////////////////////////////////////


// demo
extern void demo(void);

// main
int main()
{
  // run the demo
  demo();

  // sleep
  vic::io::delay(100000U);

  return 0;
}
