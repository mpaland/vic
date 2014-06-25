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
// \brief WSkeleton driver, use this as a start for own drivers
//
///////////////////////////////////////////////////////////////////////////////

#include "vgx_drv_skeleton.h"

// defines the driver name and version
#define VGX_DRV_VERSION   "Skeleton driver 1.00"


namespace vgx {


void drv_skeleton::init()
{

}


void drv_skeleton::deinit()
{

}


void drv_skeleton::brightness_set(std::uint8_t)
{

}


const char* drv_skeleton::version() const
{
  return (const char*)VGX_DRV_VERSION;
}


void drv_skeleton::primitive_done()
{

}


void drv_skeleton::cls()
{

}


bool drv_skeleton::pixel_set(int16_t x, int16_t y)
{
  return true;
}


bool drv_skeleton::pixel_set_color(int16_t x, int16_t y, std::uint32_t color)
{
  return true;
}


std::uint32_t drv_skeleton::pixel_get(int16_t x, int16_t y) const
{
  return 0U;
}


/*
// define all native supported primitives here
void drv_skeleton::drv_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
}

*/

} // namespace vgx
