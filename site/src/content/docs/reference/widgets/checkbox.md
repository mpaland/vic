---
title: checkbox
description: A box with a check mark and a caption at the right, toggled by a tap.
sidebar:
  order: 5
---

`checkbox` is a square box with a check mark and a caption at the right. A tap toggles the state.

![Input demo](/vic/media/demo_input.gif)

## Header

```cpp
#include "widget/checkbox.h"
```

## Example

```cpp
#include "widget/checkbox.h"
#include "font/LCD_6x10.h"

static vic::font::LCD_6x10 font;

vic::widget::checkbox::config_type cfg = { { 10, 10 }, 12U, &font, "Sound",
                                           vic::color::white, vic::color::green, vic::color::gray04, vic::color::gray10 };
vic::widget::checkbox cb(_head, cfg);
cb.init();
cb.set_checked(true);
```

## Configuration

| Member | Type | Meaning |
|---|---|---|
| `origin` | `vertex_type` | Origin (left/top) of the box |
| `size` | `std::uint8_t` | Box size in pixel (square), 8 or more |
| `font` | `const font::font_type*` | Caption font, must stay valid |
| `caption` | `const char*` | Caption (UTF-8), `nullptr` for none, must stay valid |
| `color_text` | `color::value_type` | Caption color |
| `color_check` | `color::value_type` | Check mark color |
| `color_box` | `color::value_type` | Box fill color |
| `color_border` | `color::value_type` | Box border color, `color::none` for no border |

The caption starts 4 pixel right of the box and is vertically centered to it. The initial state is unchecked.

## Member functions

### Constructor

```cpp
checkbox(drv& head, const config_type& config);
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

### set_checked

```cpp
void set_checked(bool checked);
```

Set the state, the box is redrawn only on change.

| Parameter | Description |
|---|---|
| `checked` | True for checked |

### toggle

```cpp
void toggle();
```

Toggle the state.

### is_checked

```cpp
bool is_checked() const;
```

Returns the state, true if checked.

### redraw

```cpp
void redraw();
```

Redraw the control (box and caption).

## Events

The hit area is the box and the caption (the caption width is measured by `redraw()`).

| Event | Reaction |
|---|---|
| `event_tap` | Toggles the state |
| others | None |

The event handler is called after the state is toggled, so `is_checked()` returns the new state. See
[input routing](/vic/reference/widgets/#input-routing).
