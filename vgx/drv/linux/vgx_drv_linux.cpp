///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2014, PALANDesign Hannover, Germany
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
// \brief Linux driver, uses OpenGL for rendering
//
///////////////////////////////////////////////////////////////////////////////

#include "vgx_drv_linux.h"

// defines the driver name and version
#define VGX_DRV_VERSION   "Linux driver 1.00"


namespace vgx {
namespace head {


void linux::init()
{

}


void linux::deinit()
{

}


void linux::brightness_set(std::uint8_t)
{

}


const char* linux::version() const
{
  return (const char*)VGX_DRV_VERSION;
}


void linux::primitive_done()
{

}


void linux::cls()
{

}


void linux::pixel_set(int16_t x, int16_t y)
{
  return true;
}


void linux::pixel_set_color(int16_t x, int16_t y, std::uint32_t color)
{
  return true;
}


std::uint32_t linux::pixel_get(int16_t x, int16_t y) const
{
  return 0U;
}

} // namespace head
} // namespace vgx
