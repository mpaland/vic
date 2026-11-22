---
title: battery
description: A battery symbol with a level from 0 to 100 %, a low level color, a charging symbol and four orientations.
sidebar:
  order: 3
---

`battery` is a battery symbol like in the status bar of a phone: a body with a terminal and a level from 0 (empty) to
100 % (full). The level color is selectable, a low level can have its own color, the level can be shown as blocks and
a bolt shows the charging state.

![vumeter and battery widgets](/vic/media/demo_widget_page4.png)

## Header

```cpp
#include "widget/battery.h"
```

## Example

```cpp
#include "widget/battery.h"

vic::widget::battery::config_type cfg = { { 290, 4 }, 24U, 11U, vic::widget::battery::left_to_right,
                                          0U,                    // continuous level bar
                                          20U,                   // low level up to 20 %
                                          vic::color::green,     // level
                                          vic::color::red,       // low level
                                          vic::color::white,     // outline and terminal
                                          vic::color::white,     // charging symbol
                                          vic::color::black };   // background
vic::widget::battery bat(_head, cfg);
bat.init();
bat.set_level(75U);         // 0 = empty, 100 = full
bat.set_charging(true);
```

## Configuration

| Member | Type | Meaning |
|---|---|---|
| `origin` | `vertex_type` | Origin, always the left/top corner of the widget |
| `x_size` | `std::uint16_t` | Complete size in x direction including the terminal |
| `y_size` | `std::uint16_t` | Complete size in y direction including the terminal |
| `orientation` | `orientation_type` | Fill direction: `left_to_right`, `right_to_left`, `bottom_to_top`, `top_to_bottom` |
| `segment_count` | `std::uint8_t` | 0 or 1 is a continuous level bar, any other count shows the level as blocks |
| `level_low` | `std::uint8_t` | Low level threshold in percent, 0 for none |
| `color_level` | `color::value_type` | Level color |
| `color_low` | `color::value_type` | Level color if the level is equal or below `level_low` |
| `color_frame` | `color::value_type` | Color of the body outline and the terminal |
| `color_charge` | `color::value_type` | Color of the charging symbol |
| `color_bg` | `color::value_type` | Background color inside the body, `color::none` leaves it untouched |

The terminal is at the full end of the fill direction: `left_to_right` is the phone style battery with the terminal on
the right, `bottom_to_top` is an upright battery with the terminal on top. The symbol scales with the size: the
terminal is a fifth of the size across the fill direction long (at least one pixel) and half of it wide, the body has
a one pixel outline with rounded corners. The level is drawn with one pixel clearance to the outline, so the widget
should be at least 7 pixel across.

Any level above 0 shows at least one pixel (or the first block), only level 0 is empty. Blocks have one pixel gap.
The initial level is 0, not charging.

With `color_bg` set to `color::none` a falling level is not erased, use it only on a level which is drawn once.

## Member functions

### Constructor

```cpp
battery(drv& head, const config_type& config);
```

| Parameter | Description |
|---|---|
| `head` | Reference to the head driver |
| `config` | Configuration params |

### init

```cpp
virtual void init() final;
```

Draw the widget (empty).

### set_level

```cpp
void set_level(std::uint8_t level);
```

Set the level. The inside of the body is redrawn only if the level changed.

| Parameter | Description |
|---|---|
| `level` | New level in percent, 0 is empty, 100 is full, bigger values are clamped to 100 |

### get_level

```cpp
std::uint8_t get_level() const;
```

Returns the actual level in percent.

### set_color

```cpp
void set_color(color::value_type color);
```

Set a new level color, drawn only on change. The low level color stays.

| Parameter | Description |
|---|---|
| `color` | New level color |

### set_charging

```cpp
void set_charging(bool charging = true);
```

Show or hide the charging symbol, an upright bolt in `color_charge` in the middle of the body, drawn over the level.

| Parameter | Description |
|---|---|
| `charging` | True to show the charging symbol |

### is_charging

```cpp
bool is_charging() const;
```

Returns true if the charging symbol is shown.

### redraw

```cpp
void redraw();
```

Redraw the control (outline, terminal, background, level and charging symbol).

## Events

The hit area is the complete widget rect including the terminal. `battery` has no own event reaction, an event handler
set with `set_event_handler()` is called. See [input routing](/vic/reference/widgets/#input-routing).
