---
title: button
description: A push button with a centered caption and a pressed look.
sidebar:
  order: 4
---

`button` is a push button with a centered caption. It shows the pressed look between `event_down` and `event_up`, the
tap itself is reported to the event handler. The button has no state of its own.

![Input demo](/vic/media/demo_input.gif)

## Header

```cpp
#include "widget/button.h"
```

## Example

```cpp
#include "widget/button.h"
#include "font/LCD_6x10.h"

static vic::font::LCD_6x10 font;

void on_ok(vic::widget::base& w, vic::vertex_type pos, vic::widget::event_type ev, void* arg)
{
  (void)w; (void)pos; (void)arg;
  if (ev == vic::widget::event_tap) {
    // OK tapped
  }
}

vic::widget::button::config_type cfg = { { 10, 10, 70, 30 }, 4U, &font, "OK",
                                         vic::color::white, vic::color::gray04, vic::color::gray08, vic::color::gray10 };
vic::widget::button btn(_head, cfg);
btn.init();
btn.set_event_handler(on_ok);
```

## Configuration

| Member | Type | Meaning |
|---|---|---|
| `rect` | `rect_type` | Button rect (right/bottom exclusive) |
| `radius` | `std::uint8_t` | Corner radius, 0 for sharp corners |
| `font` | `const font::font_type*` | Caption font, must stay valid |
| `caption` | `const char*` | Caption (UTF-8), `nullptr` for a blank button, must stay valid |
| `color_text` | `color::value_type` | Caption color |
| `color_bg` | `color::value_type` | Button color |
| `color_bg_pressed` | `color::value_type` | Button color while pressed |
| `color_border` | `color::value_type` | Border color, `color::none` for no border |

## Member functions

### Constructor

```cpp
button(drv& head, const config_type& config);
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

### set_caption

```cpp
void set_caption(const char* caption);
```

Set a new caption and redraw.

| Parameter | Description |
|---|---|
| `caption` | New caption, `nullptr` for none, must stay valid |

### is_pressed

```cpp
bool is_pressed() const;
```

Returns the pressed state: true while the button is pressed (between `event_down` and `event_up`).

### redraw

```cpp
void redraw();
```

Redraw the control.

## Events

The hit area is `rect`.

| Event | Reaction |
|---|---|
| `event_down` | Pressed look (`color_bg_pressed`) |
| `event_up` | Released look (`color_bg`) |
| `event_tap`, `event_long_tap` | None, reported to the event handler |

The event handler is called for every event after the button reacted. See
[input routing](/vic/reference/widgets/#input-routing).
