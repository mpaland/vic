---
title: groupbox
description: A rectangular or rounded frame with a title cut into the top border.
sidebar:
  order: 8
---

`groupbox` is a (rounded) frame with a title cut into the top border. It is static, draw it before the widgets inside
it.

![Widget demo](/vic/media/demo_widget.gif)

## Header

```cpp
#include "widget/groupbox.h"
```

## Example

```cpp
#include "widget/groupbox.h"
#include "font/Montserrat_10_aa2.h"

static vic::font::Montserrat_10_aa2 font;

vic::widget::groupbox::config_type cfg = { { 2, 84, 150, 196 }, 4U, &font, "FUEL",
                                           vic::color::gray, vic::color::white, vic::color::black };
vic::widget::groupbox grp(_head, cfg);
grp.init();   // first, then the widgets inside
```

## Configuration

| Member | Type | Meaning |
|---|---|---|
| `rect` | `rect_type` | Frame rect (right/bottom exclusive) |
| `radius` | `std::uint8_t` | Corner radius, 0 for sharp corners |
| `font` | `const font::font_type*` | Title font, must stay valid during the lifetime of the widget |
| `title` | `const char*` | Title text (UTF-8), `nullptr` for no title, must stay valid |
| `color_border` | `color::value_type` | Frame color |
| `color_title` | `color::value_type` | Title text color |
| `color_bg` | `color::value_type` | Background color of the frame interior, `color::none` leaves it untouched |

The title starts `radius + 4` pixel right of the left edge and is vertically centered on the top border line. Its area
is cleared with `color_bg`, so it cuts the border. With `color::none` the background and the border under the title
stay untouched.

## Member functions

### Constructor

```cpp
groupbox(drv& head, const config_type& config);
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

### set_title

```cpp
void set_title(const char* title);
```

Set a new title and redraw.

| Parameter | Description |
|---|---|
| `title` | New title, `nullptr` for none, must stay valid |

### redraw

```cpp
void redraw();
```

Redraw the control. With `color_bg` set this clears the interior, so the widgets inside have to be redrawn as well.

## Events

The hit area is `rect`. `groupbox` has no own event reaction. Widgets inside the group box are created after it, so
they receive the events at their position, the group box receives the events on its free area. See
[input routing](/vic/reference/widgets/#input-routing).
