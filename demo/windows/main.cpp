///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2020, paland consult, Hannover, Germany
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
// \brief main() for Windows demo programs
// Link this file together with the according demo_xxx.cpp file
//
///////////////////////////////////////////////////////////////////////////////

#include <Windows.h>

#include "head/windows.h"
#include "head/multihead.h"


// create a single head
vic::drv& get_singlehead()
{
  // create 320 x 200 pixel window with zoom factor 2
  static vic::head::windows<320, 200> _head( 0, 0, 0, 0, 2U, 2U );
  return _head;
}


// create a two windows as multihead
template<std::uint16_t Screen_Size_X, std::uint16_t Screen_Size_Y>
vic::drv& get_multihead()
{
  static vic::head::windows<250, 100> _head0( 0, 0, 0,   0, 4U, 4U );
  static vic::head::windows<250, 100> _head1( 0, 0, 0, 440, 4U, 4U );
  static vic::head::multihead<250U, 400U, 2U> _multihead = { { _head0, { 0, 0 }}, { _head1, { 0, 100 }} };
  return _multihead;
}


// create a text single head
vic::drv& get_text_singlehead()
{
  static vic::head::windows_text<16, 4> _head( 0, 0, 0, 0 );
  return _head;
}


// create a text multihead
vic::drv& get_text_multihead()
{
  static vic::head::windows_text<12, 4> _head0( 0, 0,   0, 0 );
  static vic::head::windows_text<12, 4> _head1( 0, 0, 450, 0 );
  static vic::head::multihead<24U, 4U, 2U> _multihead = { { _head0, { 0U, 0U }}, { _head1, { 12U, 0U }} };
  return _multihead;
}


///////////////////////////////////////////////////////////////////////////////
// I / O   A B S T R A C T I O N

void vic::io::delay(std::uint32_t time_to_delay)
{
  ::Sleep(time_to_delay);
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
