---
title: slider
description: A track with a knob, the value is set by tapping or dragging on the track.
sidebar:
  order: 13
---

`slider` is a track with a knob. The value is set by tapping on the track, a touch driver which sends `event_down` on
every move gives a dragging knob.

![Input demo](/vic/media/demo_input.gif)

## Header

```cpp
#include "widget/slider.h"
```

## Example

```cpp
#include "widget/slider.h"

vic::widget::slider::config_type cfg = { { 10, 90 }, 136U, 16U, vic::widget::slider::horizontal, 0, 100, 4U, 6U,
                                         vic::color::gray04,   // track
                                         vic::color::cyan,     // fill
                                         vic::color::white,    // knob
                                         vic::color::black };  // background
vic::widget::slider sl(_head, cfg);
sl.init();
sl.set_value(60);
```

## Configuration

| Member | Type | Meaning |
|---|---|---|
| `origin` | `vertex_type` | Origin (left/top) |
| `x_size` | `std::uint16_t` | X size of the widget |
| `y_size` | `std::uint16_t` | Y size of the widget |
| `orientation` | `orientation_type` | `horizontal` (`range_lower` at the left) or `vertical` (`range_lower` at the bottom) |
| `range_lower` | `std::int16_t` | Lower range, e.g. 0 |
| `range_upper` | `std::int16_t` | Upper range, e.g. 100 |
| `track_width` | `std::uint8_t` | Thickness of the track |
| `knob_radius` | `std::uint8_t` | Radius of the knob |
| `color_track` | `color::value_type` | Track color |
| `color_fill` | `color::value_type` | Track color between `range_lower` and the knob, `color::none` for none |
| `color_knob` | `color::value_type` | Knob color |
| `color_bg` | `color::value_type` | Background color of the widget rect, erases the knob. Use the background color of the display |

The track is centered in the widget rect, the knob radius is kept free at both track ends (the hit margin at the ends).
The initial value is `range_lower`.

## Member functions

### Constructor

```cpp
slider(drv& head, const config_type& config);
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

Set a new value and move the knob, the widget is redrawn on change.

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

Set a new range, the value is set to `lower`. The range is only taken if `lower < upper`, the widget is redrawn in any
case.

| Parameter | Description |
|---|---|
| `lower` | Lower range limit |
| `upper` | Upper range limit |

### anti_aliasing_enable

```cpp
void anti_aliasing_enable(bool enable = true);
```

Enable anti aliasing for the knob, default is off. The widget is redrawn.

| Parameter | Description |
|---|---|
| `enable` | True to enable anti aliasing |

### redraw

```cpp
void redraw();
```

Redraw the control: background, track, filled part and knob.

## Events

The hit area is the widget rect.

| Event | Reaction |
|---|---|
| `event_down`, `event_tap` | The value is set to the event position, the knob jumps there |
| others | None |

The event handler is called after the value is set, `get_value()` returns the new value. See
[input routing](/vic/reference/widgets/#input-routing).
