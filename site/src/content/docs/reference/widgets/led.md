---
title: led
description: A round or square indicator with an on and an off color.
sidebar:
  order: 11
---

`led` is a round or square indicator with an on and an off color.

![Widget demo](/vic/media/demo_widget.gif)

## Header

```cpp
#include "widget/led.h"
```

## Example

```cpp
#include "widget/led.h"

vic::widget::led::config_type cfg = { { 20, 20 }, 6U, vic::widget::led::shape_round,
                                      vic::color::green, vic::color::darkgreen, vic::color::gray08 };
vic::widget::led l(_head, cfg);
l.anti_aliasing_enable();
l.init();
l.set_state(true);
l.toggle();
```

## Configuration

| Member | Type | Meaning |
|---|---|---|
| `center` | `vertex_type` | Center of the LED |
| `radius` | `std::uint8_t` | Radius (half size) of the LED |
| `shape` | `shape_type` | `shape_round` or `shape_square` |
| `color_on` | `color::value_type` | Color in on state |
| `color_off` | `color::value_type` | Color in off state |
| `color_border` | `color::value_type` | One pixel border color, `color::none` for no border |

A square LED is `2 * radius + 1` wide with slightly rounded corners (corner radius `radius / 3`). The initial state is
off.

## Member functions

### Constructor

```cpp
led(drv& head, const config_type& config);
```

| Parameter | Description |
|---|---|
| `head` | Reference to the head driver |
| `config` | Configuration params |

### init

```cpp
virtual void init() final;
```

Draw the widget (off).

### set_state

```cpp
void set_state(bool on);
```

Set the LED state, redrawn only on change.

| Parameter | Description |
|---|---|
| `on` | True for on |

### toggle

```cpp
void toggle();
```

Toggle the LED state.

### is_on

```cpp
bool is_on() const;
```

Returns the LED state, true if on.

### set_color

```cpp
void set_color(color::value_type color_on, color::value_type color_off);
```

Set new on / off colors and redraw.

| Parameter | Description |
|---|---|
| `color_on` | New on color |
| `color_off` | New off color |

### anti_aliasing_enable

```cpp
void anti_aliasing_enable(bool enable = true);
```

Enable anti aliasing for the round shape and the border, default is off. The widget is redrawn.

| Parameter | Description |
|---|---|
| `enable` | True to enable anti aliasing |

### redraw

```cpp
void redraw();
```

Redraw the control.

## Events

The hit area is the bounding square (`2 * radius + 1`). `led` has no own event reaction, an event handler set with
`set_event_handler()` is called, so a LED can act as a simple button. See
[input routing](/vic/reference/widgets/#input-routing).
