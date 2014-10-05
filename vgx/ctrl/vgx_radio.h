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

#ifndef _VGX_CTRL_RADIO_H_
#define _VGX_CTRL_RADIO_H_

#include "../vgx_ctrl.h"


namespace vgx {

class radio : public ctrl
{
public:

  typedef enum enum_state_type {
    inactive = 0,
    active,
    hover,
    disabled
  } state_type;

  typedef struct struct_config_type {
    std::int16_t      x;              // x coordinate
    std::int16_t      y;              // y coordinate
    std::uint16_t     radius;         // radius
    std::uint32_t     color_active;   // active color
    std::uint32_t     color_hover;    // hover color
    std::uint32_t     color_disabled; // disabled color
    std::uint32_t     color_frame;    // frame color
    std::uint32_t     color_bg;       // background color
  } config_type;

  /**
   * ctor
   * \param head Reference to the head driver
   * \param config Configuration params
   */
  radio(drv& head, config_type& config)
    : ctrl(head)
    , config_(config)
    , state_(inactive)
  { render(); };

  /**
   * Set new radio state
   * \param state New color for control, control is redrawn
   */
  void set_state(state_type state)
  { state_ = state; redraw(); }

  /**
   * Redraw the control
   */
  void redraw()
  { render(); }

  virtual bool is_inside(std::int16_t x, std::int16_t y)
  { return (config_.x - x > 0 ? config_.x - x : x - config_.x) * (config_.y - y > 0 ? config_.y - y : y - config_.y) <= config_.radius * config_.radius; }

private:
  // render the control
  void render()
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

  config_type config_;  // configuration
  state_type  state_;   // radio state
};

} // namespace vgx

#endif  // _VGX_CTRL_RADIO_H_
