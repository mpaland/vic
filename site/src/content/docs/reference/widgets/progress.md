---
title: progress
description: A progress bar in four orientations with delta drawing.
sidebar:
  order: 12
---

`progress` is a progress bar in four orientations. A position change draws only the changed part of the bar.

![Widget demo](/vic/media/demo_widget.gif)

## Header

```cpp
#include "widget/progress.h"
```

## Example

```cpp
#include "widget/progress.h"

vic::widget::progress::config_type cfg = { { 10, 10 }, 100U, 13U, vic::widget::progress::left_to_right,
                                           0, 100,
                                           vic::color::red,        // bar
                                           vic::color::darkgray,   // background
                                           vic::color::white };    // border
vic::widget::progress prog(_head, cfg);
prog.init();
prog.set_pos(50);

prog.set_step(5);
prog.step_it();            // 55
```

## Configuration

| Member | Type | Meaning |
|---|---|---|
| `origin` | `vertex_type` | Origin, always the left/top corner of the widget |
| `x_size` | `std::uint16_t` | Complete size in x direction |
| `y_size` | `std::uint16_t` | Complete size in y direction |
| `orientation` | `orientation_type` | Orientation of the bar: `left_to_right`, `right_to_left`, `bottom_to_top`, `top_to_bottom` |
| `range_lower` | `std::int16_t` | Initial lower range, e.g. 0 |
| `range_upper` | `std::int16_t` | Initial upper range, e.g. 100 |
| `color_bar` | `color::value_type` | Bar color |
| `color_bg` | `color::value_type` | Background color, `color::none` leaves the background untouched |
| `color_border` | `color::value_type` | Border color, `color::none` for no border |

If a border color is set, the border is one pixel wide and part of `x_size * y_size`. The initial position is
`range_lower`, the step is 1.

## Member functions

### Constructor

```cpp
progress(drv& head, const config_type& config);
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

### set_color

```cpp
void set_color(color::value_type color);
```

Set a new bar color, the control is redrawn.

| Parameter | Description |
|---|---|
| `color` | New bar color |

### set_range

```cpp
void set_range(std::int16_t lower, std::int16_t upper);
```

Set the new minimum and maximum range, the position is set to `lower`. The range is only taken if `lower < upper`, the
control is redrawn in any case.

| Parameter | Description |
|---|---|
| `lower` | Lower range limit including the `lower` value |
| `upper` | Upper range limit including the `upper` value |

### set_pos

```cpp
void set_pos(std::int16_t pos);
```

Set the new position of the progress bar and render it.

| Parameter | Description |
|---|---|
| `pos` | New position, clamped to the valid range |

### reset

```cpp
void reset();
```

Reset the progress bar to the lower range position (no progress).

### get_pos

```cpp
std::int16_t get_pos() const;
```

Returns the current position.

### set_step

```cpp
void set_step(std::int16_t step = 1);
```

Specifies the step increment for `step_it()`.

| Parameter | Description |
|---|---|
| `step` | New step increment, can be negative, default is 1 |

### step_it

```cpp
void step_it();
```

Advances the current position by the step increment (clamped to the range) and redraws the bar to reflect the new
position.

### redraw

```cpp
void redraw();
```

Redraw the control (border, bar and background).

## Events

The hit area is the complete widget rect. `progress` has no own event reaction, an event handler set with
`set_event_handler()` is called. See [input routing](/vic/reference/widgets/#input-routing).
