---
title: imagebutton
description: An XPM icon with a caption below, a soft key or icon button with a pressed and a latched look.
sidebar:
  order: 9
---

`imagebutton` is an XPM icon with a caption below it, a soft key or icon label. Like `button` it shows the pressed look
between `event_down` and `event_up`. In addition it has an active (latched) look, for example for the selected page of
a soft key bar.

![Input demo](/vic/media/demo_input.gif)

## Header

```cpp
#include "widget/imagebutton.h"
```

## Example

```cpp
#include "widget/imagebutton.h"
#include "font/LCD_6x10.h"

static vic::font::LCD_6x10 font;

// 12 x 10 icon
static const char* xpm_map[] = {
  "12 10 2 1", "  c None", ". c #40E0FF",
  "   ..       ", "  .  .      ", " .    .     ", ".      .    ", ".       .   ",
  " .       .  ", "  .       . ", "   .     .  ", "    .   .   ", "     ...    "
};

vic::widget::imagebutton::config_type cfg = { { 164, 170, 200, 196 }, 3U, xpm_map, &font, "MAP",
                                              vic::color::white, vic::color::gray04, vic::color::gray08,
                                              vic::color::darkblue, vic::color::gray08 };
vic::widget::imagebutton sk(_head, cfg);
sk.init();
sk.set_active(true);   // selected / latched look
```

## Configuration

| Member | Type | Meaning |
|---|---|---|
| `rect` | `rect_type` | Button rect (right/bottom exclusive) |
| `radius` | `std::uint8_t` | Corner radius, 0 for sharp corners |
| `xpm` | `const char**` | Icon in XPM3 format, `nullptr` for none, must stay valid |
| `font` | `const font::font_type*` | Caption font, must stay valid |
| `caption` | `const char*` | Caption (UTF-8), `nullptr` for none, must stay valid |
| `color_text` | `color::value_type` | Caption color |
| `color_bg` | `color::value_type` | Button color |
| `color_bg_pressed` | `color::value_type` | Button color while pressed |
| `color_bg_active` | `color::value_type` | Button color in active (latched) state |
| `color_border` | `color::value_type` | Border color, `color::none` for no border |

Icon and caption are stacked with a gap of 2 pixel and centered as a block in `rect`. Without an icon it is a plain
caption key, without a caption a plain icon. The icon is rendered by an internal [`bitmap`](/vic/reference/widgets/bitmap/),
which is excluded from the event routing.

## Member functions

### Constructor

```cpp
imagebutton(drv& head, const config_type& config);
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

### set_active

```cpp
void set_active(bool active);
```

Set the active (latched) state, redrawn on change.

| Parameter | Description |
|---|---|
| `active` | True for the active look |

### is_active

```cpp
bool is_active() const;
```

Returns the active state.

### is_pressed

```cpp
bool is_pressed() const;
```

Returns the pressed state: true while the button is pressed (between `event_down` and `event_up`).

### set_caption

```cpp
void set_caption(const char* caption);
```

Set a new caption and redraw.

| Parameter | Description |
|---|---|
| `caption` | New caption, `nullptr` for none, must stay valid |

### redraw

```cpp
void redraw();
```

Redraw the control. The background is `color_bg_pressed` while pressed, else `color_bg_active` if active, else
`color_bg`.

## Events

The hit area is `rect`.

| Event | Reaction |
|---|---|
| `event_down` | Pressed look |
| `event_up` | Released look |
| `event_tap`, `event_long_tap` | None, reported to the event handler |

The active state is not changed by events. A soft key bar sets it in the event handler, so the tapped key becomes the
active one:

```cpp
vic::widget::imagebutton* softkeys[4];

void on_softkey(vic::widget::base& w, vic::vertex_type pos, vic::widget::event_type ev, void* arg)
{
  (void)pos; (void)arg;
  if (ev == vic::widget::event_tap) {
    for (std::size_t i = 0U; i < 4U; ++i) {
      softkeys[i]->set_active(softkeys[i] == &w);
    }
  }
}
```

See [input routing](/vic/reference/widgets/#input-routing) and the [input example](/vic/examples/input/).
