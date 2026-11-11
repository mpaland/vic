---
title: toggle
description: An on / off switch with a sliding knob, the class toggle in widget/switch.h.
sidebar:
  order: 16
---

`toggle` is an on / off switch: a rounded track with a knob at the left (off) or right (on) end. A tap toggles the
state.

:::note
The widget lives in `widget/switch.h`, the class is named `toggle` because `switch` is a C++ keyword.
:::

![Input demo](/vic/media/demo_input.gif)

## Header

```cpp
#include "widget/switch.h"
```

## Example

```cpp
#include "widget/switch.h"

vic::widget::toggle::config_type cfg = { { 10, 10 }, 30U, 14U,
                                         vic::color::green, vic::color::gray04, vic::color::white, vic::color::gray10 };
vic::widget::toggle sw(_head, cfg);
sw.init();
sw.set_state(true);
```

## Configuration

| Member | Type | Meaning |
|---|---|---|
| `origin` | `vertex_type` | Origin (left/top) |
| `x_size` | `std::uint16_t` | Width of the track, at least `2 * y_size` |
| `y_size` | `std::uint16_t` | Height of the track |
| `color_on` | `color::value_type` | Track color in on state |
| `color_off` | `color::value_type` | Track color in off state |
| `color_knob` | `color::value_type` | Knob color |
| `color_border` | `color::value_type` | Track border color, `color::none` for no border |

The track is a box with a corner radius of `y_size / 2`, the knob a disc with a radius of `y_size / 2 - 2`. The initial
state is off.

## Member functions

### Constructor

```cpp
toggle(drv& head, const config_type& config);
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

### set_state

```cpp
void set_state(bool on);
```

Set the state, redrawn only on change.

| Parameter | Description |
|---|---|
| `on` | True for on |

### toggle_state

```cpp
void toggle_state();
```

Toggle the state.

### is_on

```cpp
bool is_on() const;
```

Returns the state, true if on.

### anti_aliasing_enable

```cpp
void anti_aliasing_enable(bool enable = true);
```

Enable anti aliasing for the rounded track and the knob, default is off. The widget is redrawn.

| Parameter | Description |
|---|---|
| `enable` | True to enable anti aliasing |

### redraw

```cpp
void redraw();
```

Redraw the control.

## Events

The hit area is the track rect.

| Event | Reaction |
|---|---|
| `event_tap` | Toggles the state |
| others | None |

The event handler is called after the state is toggled, `is_on()` returns the new state. See
[input routing](/vic/reference/widgets/#input-routing).
