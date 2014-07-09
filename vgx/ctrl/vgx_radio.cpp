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
// \brief Radio button control
//
///////////////////////////////////////////////////////////////////////////////

#include "vgx_radio.h"


namespace vgx {

void radio::render()
{
  std::uint32_t color_old = head_.color_get();

  head_.color_set(state_ == disabled ? config_.color_disabled : config_.color_frame);
  head_.circle(config_.x, config_.y, config_.radius);

  head_.color_set(config_.color_bg);
  head_.circle(config_.x, config_.y, config_.radius - 1U);

  switch (state_) {
    case inactive : head_.color_set(config_.color_bg);     break;
    case active   : head_.color_set(config_.color_active); break;
    case hover    : head_.color_set(config_.color_hover);  break;
    case disabled : head_.color_set(config_.color_bg);     break;
    default: break;
  }
  head_.disc(config_.x, config_.y, config_.radius - 2U);
  head_.color_set(color_old);
}

} // namespace vgx
