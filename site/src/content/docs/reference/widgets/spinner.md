---
title: spinner
description: A ring segment which rotates on every step(), a busy indicator.
sidebar:
  order: 15
---

`spinner` is a ring segment which rotates by a fixed angle on every `step()`, a busy indicator. A step draws only the
changed sectors.

![Widget demo](/vic/media/demo_widget.gif)

## Header

```cpp
#include "widget/spinner.h"
```

## Example

```cpp
#include "widget/spinner.h"

vic::widget::spinner::config_type cfg = { { 60, 60 }, 16U, 4U, 90U, 10,
                                          vic::color::white, vic::color::gray04, vic::color::black };
vic::widget::spinner s(_head, cfg);
s.init();

for (;;) {
  s.step();               // 10 degree clockwise
  vic::io::delay(40U);
}
```

## Configuration

| Member | Type | Meaning |
|---|---|---|
| `center` | `vertex_type` | Center of the ring |
| `radius` | `std::uint16_t` | Outer radius of the ring |
| `ring_width` | `std::uint8_t` | Width of the ring |
| `sweep` | `std::uint16_t` | Length of the segment in degrees, 1 - 359, other values are set to 90 |
| `step` | `std::int16_t` | Rotation per `step()` in degrees, positive is clockwise |
| `color_arc` | `color::value_type` | Segment color |
| `color_track` | `color::value_type` | Track color (rest of the ring). Use the background color for an invisible track, it is needed to erase the segment |
| `color_bg` | `color::value_type` | Background around the ring, used to erase the anti aliased edges, `color::none` for none |

Angles are compass degrees (0 is 12 o'clock, clockwise). The segment starts at 0.

## Member functions

### Constructor

```cpp
spinner(drv& head, const config_type& config);
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

### step

```cpp
void step();
```

Rotate the segment by the configured step, only the changed sectors are drawn. Call it periodically, e.g. every 40 ms.

### set_pos

```cpp
void set_pos(std::int16_t angle);
```

Set the segment start angle and redraw.

| Parameter | Description |
|---|---|
| `angle` | Start angle in compass degrees, normalized to 0 - 359 |

### get_pos

```cpp
std::int16_t get_pos() const;
```

Returns the segment start angle in compass degrees, 0 - 359.

### anti_aliasing_enable

```cpp
void anti_aliasing_enable(bool enable = true);
```

Enable anti aliasing, default is off. The ring is then completely redrawn on every step: the ring and two pixel around
it are erased with `color_bg` first.

| Parameter | Description |
|---|---|
| `enable` | True to enable anti aliasing |

### redraw

```cpp
void redraw();
```

Redraw the control.

## Events

The hit area is the disc of the ring (`radius`). `spinner` has no own event reaction, an event handler set with
`set_event_handler()` is called. See [input routing](/vic/reference/widgets/#input-routing).
