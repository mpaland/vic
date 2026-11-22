---
title: vumeter
description: A segmented level meter in four orientations, colored by hard zones or a gradient.
sidebar:
  order: 19
---

`vumeter` is a level meter out of segments, like the LED bargraph of an audio device. The segment colors come from an
array of any length: one color gives a single colored meter, more colors are hard zones or a gradient. Unlit segments
show their color dimmed. A value change draws only the segments which change their state.

![vumeter and battery widgets](/vic/media/demo_widget_page4.png)

## Header

```cpp
#include "widget/vumeter.h"
```

## Example

```cpp
#include "widget/vumeter.h"

static const vic::color::value_type zones[] = { vic::color::green, vic::color::yellow, vic::color::red };

vic::widget::vumeter::config_type cfg = { { 10, 10 }, 120U, 8U, vic::widget::vumeter::left_to_right,
                                          20U, 1U,              // 20 segments, one pixel gap
                                          0, 100,               // range
                                          zones, 3U, true,      // three colors as gradient
                                          64U,                  // unlit segments at 25 % luminance
                                          vic::color::black };  // gaps
vic::widget::vumeter vu(_head, cfg);
vu.init();
vu.set_value(70);
```

## Configuration

| Member | Type | Meaning |
|---|---|---|
| `origin` | `vertex_type` | Origin, always the left/top corner of the widget |
| `x_size` | `std::uint16_t` | Complete size in x direction |
| `y_size` | `std::uint16_t` | Complete size in y direction |
| `orientation` | `orientation_type` | Direction of increasing values: `left_to_right`, `right_to_left`, `bottom_to_top`, `top_to_bottom` |
| `segment_count` | `std::uint8_t` | Number of segments |
| `segment_gap` | `std::uint8_t` | Gap between two segments in pixel |
| `range_lower` | `std::int16_t` | Lower range, e.g. 0 |
| `range_upper` | `std::int16_t` | Upper range, e.g. 100 |
| `colors` | `const color::value_type*` | Colors from the first to the last segment, must stay valid |
| `color_count` | `std::uint8_t` | Number of colors, 1 for a single colored meter |
| `gradient` | `bool` | `true`: the colors blend from one to the next, `false`: hard zones |
| `off_lum` | `std::uint8_t` | Luminance of the unlit segments, 0 is black, 255 is the full color |
| `color_bg` | `color::value_type` | Gap color, `color::none` leaves the gaps untouched |

The segments are spread evenly along the orientation axis, the last segment ends at the widget edge. The initial value
is `range_lower`. The number of lit segments is rounded to the closest segment.

### Colors

The colors are spread evenly over the meter, the same array works in both modes:

| `gradient` | Segment colors |
|---|---|
| `false` | `color_count` zones of equal size, segment `i` has `colors[i * color_count / segment_count]` |
| `true` | The colors are stops from the first to the last segment and the segments blend between them. The first segment has exactly `colors[0]`, the last exactly the last color |

A wider zone is made by repeating its color:

```cpp
// 60 % green, 20 % yellow, 20 % red
static const vic::color::value_type zones[] = { vic::color::green, vic::color::green, vic::color::green,
                                                vic::color::yellow, vic::color::red };
```

A gradient of two colors is blended in RGB, so green to red passes a dark olive. A third color in the middle (green,
yellow, red) keeps it bright.

## Member functions

### Constructor

```cpp
vumeter(drv& head, const config_type& config);
```

| Parameter | Description |
|---|---|
| `head` | Reference to the head driver |
| `config` | Configuration params |

### init

```cpp
virtual void init() final;
```

Draw the widget.

### set_value

```cpp
void set_value(std::int16_t value);
```

Set a new value. Only the segments which change their state are drawn, nothing is drawn if the number of lit segments
stays the same.

| Parameter | Description |
|---|---|
| `value` | New value, clamped to the valid range |

### get_value

```cpp
std::int16_t get_value() const;
```

Returns the actual value.

### set_range

```cpp
void set_range(std::int16_t lower, std::int16_t upper);
```

Set a new range, the value is set to `lower`. The range is only taken if `lower < upper`, the control is redrawn in
any case.

| Parameter | Description |
|---|---|
| `lower` | Lower range limit |
| `upper` | Upper range limit |

### redraw

```cpp
void redraw();
```

Redraw the control (gaps and all segments).

## Events

The hit area is the complete widget rect. `vumeter` has no own event reaction, an event handler set with
`set_event_handler()` is called. See [input routing](/vic/reference/widgets/#input-routing).
