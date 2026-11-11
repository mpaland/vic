---
title: compass
description: A heading up compass rose which rotates below a fixed pointer.
sidebar:
  order: 6
---

`compass` is a heading up compass rose: the rose rotates below a fixed pointer at the top. It has ticks every 10
degree (longer every 30, longest every 90) and the cardinal letters N E S W inside the ring.

![Input demo](/vic/media/demo_input.gif)

## Header

```cpp
#include "widget/compass.h"
```

## Example

```cpp
#include "widget/compass.h"
#include "font/LCD_6x10.h"

static vic::font::LCD_6x10 font;

vic::widget::compass::config_type cfg = { { 60, 60 }, 40U, &font,
                                          vic::color::black, vic::color::gray08, vic::color::white, vic::color::white, vic::color::magenta };
vic::widget::compass cp(_head, cfg);
cp.anti_aliasing_enable();
cp.init();
cp.set_heading(215);
```

## Configuration

| Member | Type | Meaning |
|---|---|---|
| `center` | `vertex_type` | Center of the rose |
| `radius` | `std::uint16_t` | Outer radius of the rose |
| `font` | `const font::font_type*` | Font of the cardinal letters, `nullptr` for no letters |
| `color_bg` | `color::value_type` | Disc background |
| `color_ring` | `color::value_type` | Outer ring |
| `color_tick` | `color::value_type` | Ticks |
| `color_letter` | `color::value_type` | Cardinal letters |
| `color_pointer` | `color::value_type` | Heading pointer and center dot |

The heading is the direction at the pointer, the rose turns counter clockwise when it increases. Every heading change
redraws the whole disc within a present lock: there is no sprite RAM, but it costs the disc pixels on slow displays.

## Member functions

### Constructor

```cpp
compass(drv& head, const config_type& config);
```

| Parameter | Description |
|---|---|
| `head` | Reference to the head driver |
| `config` | Configuration params |

The initial heading is 0.

### init

```cpp
virtual void init() final;
```

Draw the widget.

### set_heading

```cpp
void set_heading(std::int16_t heading);
```

Set a new heading, the rose is redrawn on change.

| Parameter | Description |
|---|---|
| `heading` | Heading in degree, any value, normalized to 0 - 359 |

### get_heading

```cpp
std::int16_t get_heading() const;
```

Returns the heading in degree, 0 - 359.

### anti_aliasing_enable

```cpp
void anti_aliasing_enable(bool enable = true);
```

Enable anti aliasing for the ring, the ticks and the pointer, default is off. The widget is redrawn.

| Parameter | Description |
|---|---|
| `enable` | True to enable anti aliasing |

### redraw

```cpp
void redraw();
```

Redraw the control.

## Events

The hit area is the disc (`radius`). `compass` has no own event reaction, an event handler set with
`set_event_handler()` is called. See [input routing](/vic/reference/widgets/#input-routing).
