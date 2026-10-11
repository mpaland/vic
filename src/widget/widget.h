///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2017, PALANDesign Hannover, Germany
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
// \brief Base class for complex widgets like gauges, buttons, progress bars,
//        checkboxes, switches, etc.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_WIDGET_H_
#define _VIC_WIDGET_H_

#include "../tc.h"


namespace vic {
namespace widget {


/**
 * Colored value range (band) of a scale, used by the gauge widgets
 * A band covers the values from 'from' (inclusive) to 'to' (exclusive), e.g. { 2500, 2700, color::yellow }
 */
typedef struct tag_band_type {
  std::int16_t      from;     // first value of the band
  std::int16_t      to;       // end value of the band (exclusive)
  color::value_type color;    // band color
} band_type;


/**
 * Input event, e.g. from a touch controller, an encoder button or a mouse, routed by base::event()
 */
typedef enum tag_event_type {
  event_down = 0,   // button / finger down
  event_up,         // button / finger up
  event_tap,        // short tap (down and up)
  event_long_tap    // long tap
} event_type;


/**
 * Draw a ring sector from compass angle a0 to a1 (clockwise, 0 is 12 o'clock, a0 < a1)
 * gpr::sector() counts counter clockwise from 3 o'clock, so the compass end angle is the sector start.
 * The sector is half open: the start ray is part of it, the end ray is not, so adjacent sectors tile.
 * \param _dc Drawing context
 * \param center Center of the ring
 * \param ri Inner radius
 * \param ro Outer radius
 * \param a0 Start angle in compass degrees
 * \param a1 End angle in compass degrees
 */
inline void sector_cw(dc& _dc, vertex_type center, std::uint16_t ri, std::uint16_t ro, std::int32_t a0, std::int32_t a1)
{
  const std::int32_t sweep = a1 - a0;
  if (sweep <= 0) {
    return;
  }
  std::int32_t start = (450 - a1) % 360;
  if (start < 0) {
    start += 360;
  }
  _dc.sector(center, ri, ro, static_cast<std::uint16_t>(start), static_cast<std::uint16_t>(start + (sweep > 360 ? 360 : sweep)));
}


/**
 * Erase a ring including the halo of its anti aliased edges (up to two pixels, the sub pixel sampling
 * touches pixels whose center is up to 1.2 pixel outside the shape), used before an anti aliased ring
 * is redrawn, so nothing of the old edge pixels stays (anti aliased pixels blend over the old content)
 * \param _dc Drawing context
 * \param center Center of the ring
 * \param ri Inner radius
 * \param ro Outer radius
 * \param aa True if the ring is drawn anti aliased, false does nothing
 * \param bg Background color, color::none does nothing
 */
inline void erase_edges(dc& _dc, vertex_type center, std::uint16_t ri, std::uint16_t ro, bool aa, color::value_type bg)
{
  if (!aa || (bg == color::none)) {
    return;
  }
  _dc.anti_aliasing_enable(false);
  _dc.set_color(bg);
  _dc.sector(center, static_cast<std::uint16_t>(ri > 1U ? ri - 2U : 0U), static_cast<std::uint16_t>(ro + 2U), 0U, 360U);
  _dc.anti_aliasing_enable(true);
}


/**
 * Map a value of a range to an angle of a scale, rounded to the closest degree
 * \param value Value, must be within the range
 * \param range_lower Lower range limit
 * \param range_upper Upper range limit
 * \param angle_start Angle of range_lower
 * \param angle_end Angle of range_upper
 * \return The angle
 */
inline std::int16_t value_to_angle(std::int16_t value, std::int16_t range_lower, std::int16_t range_upper, std::int16_t angle_start, std::int16_t angle_end)
{
  const std::int32_t range = static_cast<std::int32_t>(range_upper) - range_lower;
  const std::int32_t sweep = static_cast<std::int32_t>(angle_end) - angle_start;
  if (range <= 0) {
    return angle_start;
  }
  return static_cast<std::int16_t>(angle_start + ((static_cast<std::int32_t>(value) - range_lower) * sweep + range / 2) / range);
}


/**
 * Widget base class
 */
class base
{
public:
  /**
   * Event handler, set with set_event_handler()
   * \param widget The widget which received the event
   * \param vertex Position of the event
   * \param ev The event
   * \param arg The argument given in set_event_handler()
   */
  typedef void (*event_handler_type)(base& widget, vertex_type vertex, event_type ev, void* arg);

protected:
  drv&                head_;            // head instance
  base*               next_;            // next widget in chain
  event_handler_type  event_handler_;   // event handler, nullptr for none
  void*               event_arg_;       // event handler argument
  bool                event_enabled_;   // false: the widget is skipped by event()

public:
  /**
   * ctor
   * \param head Reference to the head instance
   */
  base(drv& head)
    : head_(head)
    , next_(nullptr)
    , event_handler_(nullptr)
    , event_arg_(nullptr)
    , event_enabled_(true)
  {
    // register ctrl
    if (!*get_root()) {
      // set widget as root element
      *get_root() = this;
    }
    else {
      // append widget in the chain
      for (base* b = *get_root(); b != nullptr; b = b->next_) {
        if (!b->next_) {
          b->next_ = this;
          return;
        }
      }
    }
  }


  ~base()
  {
    // remove widget from list
    if (this == *get_root()) {
      // set as root element
      *get_root() = next_;
    }
    else {
      for (base* b = *get_root(); b != nullptr; b = b->next_) {
        if (this == b->next_) {
          b->next_ = next_;
          return;
        }
      }
    }
  }


  /**
   * Init the widget
   * Initially draw the widget, init structures etc.
   */
  virtual void init(void) = 0;


  /**
   * Find a widget by vertex
   * This can be used to find the according widget if a touch event happend
   * \param vertex Vertex to check
   * \return Pointer to the widget containing the vertex, nullptr if no widget was found
   */
  base* find(vertex_type vertex)
  {
    return find_widget(vertex);
  }


  /**
   * Route an input event to the widget at the given vertex
   * Call this from the touch / mouse / button driver of the application with the position and the
   * event. The widget reacts first (on_event(), e.g. a future button changes its state), then the
   * handler set with set_event_handler() is called. With overlapping widgets the one created last
   * receives the event, as it is drawn on top (e.g. a LED inside a group box).
   * \param vertex Position of the event
   * \param ev The event
   * \return The widget which received the event, nullptr if no widget is at the vertex
   */
  static base* event(vertex_type vertex, event_type ev)
  {
    base* b = find_widget(vertex);
    if (!!b) {
      b->on_event(vertex, ev);
      if (!!b->event_handler_) {
        b->event_handler_(*b, vertex, ev, b->event_arg_);
      }
    }
    return b;
  }


  /**
   * Set the event handler of the widget, called by event() after the widget processed the event
   * \param handler Handler function, nullptr for none
   * \param arg Argument passed to the handler
   */
  void set_event_handler(event_handler_type handler, void* arg = nullptr)
  {
    event_handler_ = handler;
    event_arg_     = arg;
  }


  /**
   * Enable or disable the event routing to this widget, a disabled widget is transparent for
   * event(), so the widget below it receives the event (e.g. a decoration inside a button)
   * \param enable False to skip the widget in event()
   */
  inline void event_enable(bool enable = true)
  {
    event_enabled_ = enable;
  }


protected:

  /**
   * Check if the given vertex is inside the widget
   * \param vertex Vertex to check
   * \return true if point is inside the widget
   */
  virtual bool is_inside(vertex_type vertex) const = 0;


  /**
   * Event reaction of the widget, called by event() before the event handler, default does nothing
   * \param vertex Position of the event
   * \param ev The event
   */
  virtual void on_event(vertex_type vertex, event_type ev)
  {
    (void)vertex;
    (void)ev;
  }


  // find the widget which contains the vertex, the last created one (drawn on top) wins
  static base* find_widget(vertex_type vertex)
  {
    base* found = nullptr;
    for (base* b = *get_root(); b != nullptr; b = b->next_) {
      if (b->event_enabled_ && b->is_inside(vertex)) {
        found = b;
      }
    }
    return found;
  }


  // get the root of the widget chain
  static inline base** get_root()
  {
    static base* _root = nullptr;
    return &_root;
  }


private:
  const base& operator=(const base& rhs) { return rhs; }  // non copyable
};

} // namespace widget
} // namespace vic

#endif  // _VIC_WIDGET_H_
