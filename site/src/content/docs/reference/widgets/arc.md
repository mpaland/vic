---
title: arc
description: A ring or a part of it which fills clockwise with the value, like a round progress bar.
sidebar:
  order: 1
---

`arc` is a ring (or a part of it) which fills clockwise with the value, a round progress bar. A value change draws only
the changed ring sector.

![Widget demo](/vic/media/demo_widget.gif)

## Header

```cpp
#include "widget/arc.h"
```

## Example

```cpp
#include "widget/arc.h"

// 270 degree arc from the lower left over the top to the lower right
vic::widget::arc::config_type cfg = { { 60, 60 }, 30U, 8U, -135, 135, 0, 100,
                                      vic::color::green, vic::color::gray04, vic::color::black };
vic::widget::arc a(_head, cfg);
a.anti_aliasing_enable();   // optional, full redraw on every change
a.init();
a.set_value(42);
```

## Configuration

Angles are compass degrees: 0 is 12 o'clock, positive is clockwise, `angle_start < angle_end`. A full ring is
`-180 .. 180` (or `0 .. 360`). The ring spans from `radius - ring_width` to `radius`.

| Member | Type | Meaning |
|---|---|---|
| `center` | `vertex_type` | Center of the ring |
| `radius` | `std::uint16_t` | Outer radius of the ring |
| `ring_width` | `std::uint8_t` | Width of the ring |
| `angle_start` | `std::int16_t` | Angle of `range_lower` in compass degrees |
| `angle_end` | `std::int16_t` | Angle of `range_upper`, must be greater than `angle_start` |
| `range_lower` | `std::int16_t` | Lower range, e.g. 0 |
| `range_upper` | `std::int16_t` | Upper range, e.g. 100 |
| `color_arc` | `color::value_type` | Arc (value) color |
| `color_track` | `color::value_type` | Color of the unfilled part of the ring. Use the background color for an invisible track, it is needed to erase the arc when the value decreases |
| `color_bg` | `color::value_type` | Background around the ring, used to erase the anti aliased edges, `color::none` for none |

The initial value is `range_lower`.

## Member functions

### Constructor

```cpp
arc(drv& head, const config_type& config);
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

Set a new value, only the changed part of the ring is drawn (the sector between the old and the new angle in
`color_arc` or `color_track`).

| Parameter | Description |
|---|---|
| `value` | New value, clamped to the valid range |

### get_value

```cpp
std::int16_t get_value() const;
```

Returns the actual value.

### set_color

```cpp
void set_color(color::value_type color);
```

Set a new arc color and redraw.

| Parameter | Description |
|---|---|
| `color` | New arc color |

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

Enable anti aliasing, default is off. The ring is then completely redrawn on every value change, as anti aliased edges
can't be delta drawn: the ring and two pixel around it are erased with `color_bg` first, so keep two pixel clearance to
other content.

| Parameter | Description |
|---|---|
| `enable` | True to enable anti aliasing |

### redraw

```cpp
void redraw();
```

Redraw the control.

## Events

The hit area is the disc of the ring (`radius`). `arc` has no own event reaction, an event handler set with
`set_event_handler()` is called. See [input routing](/vic/reference/widgets/#input-routing).
