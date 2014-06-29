///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2001-2014, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// This file is part of the vgx library.
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
// \brief Global library configuration
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_CFG_H_
#define _VGX_CFG_H_

//////////////////////////////////////////////////////////////////////////
// L I B R A R Y   S U P P O R T

// library font and text rendering support (include fonts and text rendering functions)
// This can be undefined when using alpha char displays
#define VGX_CFG_FONT

// library anti aliasing support (include AA functions)
#define VGX_CFG_ANTIALIASING

// library clipping support (slower)
//#define VGX_CFG_CLIPPING

// theme support for controls
// Define this if you are using controls
#define VGX_CFG_THEME


//////////////////////////////////////////////////////////////////////////
// H A R D W A R E   I N T E R F A C E

// defines for your platform dependent hardware access

// memory access
#define VGX_MEM_8(address, data)      mcal::mem< 8U>(address, data)
#define VGX_MEM_16(address, data)     mcal::mem<16U>(address, data)
#define VGX_MEM_32(address, data)     mcal::mem<32U>(address, data)
#define VGX_MEM_64(address, data)     mcal::mem<64U>(address, data)

// SPI bus access
#define VGX_SPI(bus, outdata, outlen, indata, inlen)

// I²C bus access
#define VGX_I2C(address, outdata, outlen, indata, inlen)

// UART access
#define VGX_UART_TX(port, outdata, outlen)
#define VGX_UART_RX(port, indata, inlen)

#endif  // _VGX_CFG_H_
