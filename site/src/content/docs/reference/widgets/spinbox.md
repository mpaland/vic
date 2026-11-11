---
title: spinbox
description: A numeric value with a minus and a plus button, changed by taps and long taps.
sidebar:
  order: 14
---

`spinbox` shows a numeric value between a minus and a plus button. A tap on a button changes the value by `step`, a
long tap by `10 * step`. The value is shown with a fixed number of decimal places, without floating point.

![Input demo](/vic/media/demo_input.gif)

## Header

```cpp
#include "widget/spinbox.h"
```

## Example

```cpp
#include "widget/spinbox.h"
#include "font/LCD_6x10.h"

static vic::font::LCD_6x10 font;

// altitude 0 - 20000 in steps of 100
vic::widget::spinbox::config_type cfg = { { 70, 108, 146, 126 }, 3U, &font, 0, 20000, 100, 0U,
                                          vic::color::white, vic::color::black, vic::color::gray04, vic::color::gray08 };
vic::widget::spinbox sb(_head, cfg);
sb.init();
sb.set_value(3500);
```

## Configuration

| Member | Type | Meaning |
|---|---|---|
| `rect` | `rect_type` | Widget rect (right/bottom exclusive) |
| `radius` | `std::uint8_t` | Corner radius of the buttons, 0 for sharp corners |
| `font` | `const font::font_type*` | Value font, must stay valid |
| `range_lower` | `std::int16_t` | Lower range |
| `range_upper` | `std::int16_t` | Upper range |
| `step` | `std::int16_t` | Step per tap |
| `decimals` | `std::uint8_t` | Decimal places of the shown value, e.g. 275 with `decimals = 1` is `"27.5"` |
| `color_text` | `color::value_type` | Value and sign color |
| `color_bg` | `color::value_type` | Background of the value cell |
| `color_button` | `color::value_type` | Background of the minus / plus cells |
| `color_border` | `color::value_type` | Border color of the buttons, `color::none` for no border |

The minus and plus cells are squares at the left and right end, as wide as the rect is high. The signs are drawn as
boxes, so the font needs no sign glyphs. The initial value is `range_lower`.

## Member functions

### Constructor

```cpp
spinbox(drv& head, const config_type& config);
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

Set a new value, only the value cell is redrawn (on change).

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

Set a new range, the value is clamped to it. The range is only taken if `lower < upper`, the widget is redrawn in any
case.

| Parameter | Description |
|---|---|
| `lower` | Lower range limit |
| `upper` | Upper range limit |

### set_step

```cpp
void set_step(std::int16_t step);
```

Set a new step.

| Parameter | Description |
|---|---|
| `step` | Step per tap |

### redraw

```cpp
void redraw();
```

Redraw the control: buttons, signs and value cell.

## Events

The hit area is `rect`.

| Event | Reaction |
|---|---|
| `event_tap` | On the minus cell: value - `step`, on the plus cell: value + `step` |
| `event_long_tap` | On the minus cell: value - `10 * step`, on the plus cell: value + `10 * step` |
| others, or a tap on the value cell | None |

The event handler is called after the value changed. See [input routing](/vic/reference/widgets/#input-routing).
