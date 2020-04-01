///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2016-2017, PALANDesign Hannover, Germany
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
// \brief BA63 USB text display demo
//
///////////////////////////////////////////////////////////////////////////////

#include <Windows.h>
#include <chrono>

// use decom for USB interface
#include "decom/com/com_usbhid.h"
#include "decom/dev/dev_generic.h"

// BA63 driver
#include "../src/drv/BA6x.h"

// multihead driver
#include "../src/drv/multihead.h"



// create two BA63 heads
static vic::drv& head()
{
  static vic::head::BA6x<vic::head::BA63> _head0((vic::io::handle_type)0U);
  static vic::head::BA6x<vic::head::BA63> _head1((vic::io::handle_type)1U);
  static vic::head::multihead<40U, 2U, 2U> _multihead = { { _head0, { 0, 0 }}, { _head1, { 20, 0 }} };
  return _multihead;
}


//////////////////////////////////////////////////////////////////////////

// snake test
static void test_text_snake1(vic::drv& head)
{
  vic::tc _tc(head);
  for (std::int16_t y = 0; y < head.screen_height(); ++y) {
    for (std::int16_t x = 0; x < head.screen_width(); ++x) {
      // write position
      _tc.set_pos({ x, y });
      _tc.out((x % 10U) + 0x30U);
      _tc.present();

      _tc.set_pos({ x, y });
      _tc.out(' ');
      _tc.present();
    }
  }
}




int main()
{
  // init head
  head().init();

  test_text_snake1(head());

  vic::tc _tc(head());
  _tc.out({ 0, 0 }, (const std::uint8_t*)"Test01234567890 This is a test!");
  _tc.out({ 8, 1 }, (const std::uint8_t*)"Another test");

  ::Sleep(10000);

  return 0;
}


//////////////////////////////////////////////////////////////////////////

// decom mini USB stack
static decom::com::usbhid  usb1, usb2;
static decom::dev::generic gen1(&usb1);
static decom::dev::generic gen2(&usb2);



void vic::io::delay(std::uint32_t time_to_delay)
{
  ::Sleep(time_to_delay);
}


void vic::io::init(vic::io::handle_type device_handle)
{
  std::vector<decom::com::usbhid::hid_device_info_type> hid_dev;
  switch ((int)device_handle) {
    case 0:
      hid_dev = usb1.enumerate();
      // open BA63 - this has usage_page 0xFF45 with usage 0x2400
      for (auto it = hid_dev.begin(); it != hid_dev.end(); ++it) {
        if ((it->usage_page == 0xFF45U) && (it->usage == 0x2400U)) {
          gen1.open(it->path.c_str());
          break;
        }
      }
      break;
    case 1:
      hid_dev = usb2.enumerate();
      // open BA63 - this has usage_page 0xFF45 with usage 0x2400
      for (auto it = hid_dev.begin(); it != hid_dev.end(); ++it) {
        if ((it->usage_page == 0xFF45U) && (it->usage == 0x2400U)) {
          gen2.open(it->path.c_str());
          break;
        }
      }
      break;
    default:
      break;
  }
}


bool vic::io::write(vic::io::handle_type device_handle,
                    std::uint32_t,
                    const std::uint8_t* data_out, std::size_t data_out_length,
                    std::uint8_t*, std::size_t,
                    std::uint32_t)
{
  switch ((int)device_handle) {
    case 0:
      return gen1.write(data_out, data_out_length);
    case 1:
      return gen2.write(data_out, data_out_length);
    default:
      break;
  }
  return false;
}


bool vic::io::read(vic::io::handle_type device_handle,
                   std::uint32_t,
                   std::uint8_t* data_in, std::size_t& data_in_length,
                   std::uint32_t timeout)
{
  std::vector<std::uint8_t> data;
  decom::eid id((std::uint32_t)device_handle);

  switch ((int)device_handle) {
    case 0:
      if (gen1.read(data, id, std::chrono::milliseconds(timeout))) {
        data_in_length = data.size() < data_in_length ? data.size() : data_in_length;
        memcpy(data_in, &data[0], data_in_length);
        return true;
      }
      break;
    case 1:
      if (gen2.read(data, id, std::chrono::milliseconds(timeout))) {
        data_in_length = data.size() < data_in_length ? data.size() : data_in_length;
        memcpy(data_in, &data[0], data_in_length);
        return true;
      }
      break;
    default:
      break;
   }
   return false;
}
