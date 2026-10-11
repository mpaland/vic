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
// \brief Tracing io layer for the head demos
// There is no hardware behind it: every io::write() is printed as a hex dump
// (option, length, first bytes), io::read() returns zeros and io::delay() only
// prints. Use this file as the template for a real io layer: implement the four
// functions for your bus (SPI / I2C / parallel / UART) and keep the option
// convention of the head you use (see the header comment of the head), e.g.
//   option 0 = command / register select low, option 1 = data (the D/C line)
//
///////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstring>

#include "io.h"


namespace {
  const std::size_t MAX_DUMP = 16U;   // bytes shown per write
}


void vic::io::delay(std::uint32_t time_to_delay)
{
  // real io layer: sleep / busy wait for time_to_delay milliseconds
  std::printf("  delay %u ms\n", static_cast<unsigned>(time_to_delay));
}


void vic::io::init(handle_type device_handle)
{
  // real io layer: configure the bus, the control pins and pulse the reset line of the display
  std::printf("  init  handle %p (bus setup and display reset go here)\n", device_handle);
}


bool vic::io::write(handle_type device_handle,
                    std::uint32_t option,
                    const std::uint8_t* data_out, std::size_t data_out_length,
                    std::uint8_t* data_in, std::size_t data_in_length,
                    std::uint32_t timeout)
{
  // real io layer: set the register select / D/C line from option, then transfer data_out (and receive
  // data_in on a bidirectional bus like SPI)
  (void)device_handle; (void)timeout;
  std::printf("  write opt %u %4u byte:", static_cast<unsigned>(option), static_cast<unsigned>(data_out_length));
  for (std::size_t i = 0U; (i < data_out_length) && (i < MAX_DUMP); ++i) {
    std::printf(" %02X", data_out[i]);
  }
  if (data_out_length > MAX_DUMP) {
    std::printf(" ...");
  }
  std::printf("\n");
  if ((data_in != nullptr) && (data_in_length != 0U)) {
    std::memset(data_in, 0, data_in_length);
  }
  return true;
}


bool vic::io::read(handle_type device_handle,
                   std::uint32_t option,
                   std::uint8_t* data_in, std::size_t& data_in_length,
                   std::uint32_t timeout)
{
  // real io layer: read data_in_length bytes with the register select from option (include the dummy
  // read / dummy clocks the controller needs), return false if the bus can't read
  (void)device_handle; (void)timeout;
  std::memset(data_in, 0, data_in_length);
  if (data_in_length == 1U) {
    // busy flag reads (HD44780, SSD1680): not busy
    std::printf("  read  opt %u    1 byte: 00 (no device, not busy)\n", static_cast<unsigned>(option));
    return true;
  }
  if (data_in_length == 4U) {
    // BA6x status answer: positive
    data_in[0] = 0x04U;
    std::printf("  read  opt %u    4 byte: 04 00 00 00 (no device, status ok)\n", static_cast<unsigned>(option));
    return true;
  }
  // pixel reads and receive buffer flushes: nothing there
  std::printf("  read  opt %u %4u byte: no device\n", static_cast<unsigned>(option), static_cast<unsigned>(data_in_length));
  data_in_length = 0U;
  return false;
}
