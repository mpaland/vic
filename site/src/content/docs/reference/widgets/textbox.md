---
title: textbox
description: A multi line text with word wrap, explicit line breaks and line scrolling.
sidebar:
  order: 17
---

`textbox` renders a multi line text in a fixed cell. The text is wrapped at spaces to the cell width, `'\n'` forces a
line break, and the visible part can be scrolled line by line.

![Widget demo](/vic/media/demo_widget.gif)

## Header

```cpp
#include "widget/textbox.h"
```

## Example

```cpp
#include "widget/textbox.h"
#include "font/Montserrat_12_aa2.h"

static vic::font::Montserrat_12_aa2 font;
static const char text[] = "The quick brown fox jumps over the lazy dog.\nSecond \aFFD040paragraph.";

vic::widget::textbox::config_type cfg = { { 10, 10 }, 120U, 60U, &font, vic::widget::textbox::align_left, 1U,
                                          vic::color::white, vic::color::black, vic::color::none };
vic::widget::textbox tb(_head, cfg);
tb.init();
const std::uint16_t lines = tb.set_text(text);   // text is not copied
if (lines > tb.get_visible_lines()) {
  tb.scroll(1);                                  // scroll one line down
}
```

## Configuration

| Member | Type | Meaning |
|---|---|---|
| `origin` | `vertex_type` | Origin (left/top) |
| `x_size` | `std::uint16_t` | X size of the cell |
| `y_size` | `std::uint16_t` | Y size of the cell |
| `font` | `const font::font_type*` | Font, must stay valid during the lifetime of the widget |
| `align` | `align_type` | Horizontal alignment of the lines: `align_left`, `align_center` or `align_right` |
| `line_spacing` | `std::uint8_t` | Extra pixel between the lines |
| `color_text` | `color::value_type` | Text color |
| `color_bg` | `color::value_type` | Background color, `color::none` leaves the background untouched |
| `color_border` | `color::value_type` | Border color, `color::none` for no border |

`x_size * y_size` is the complete cell, the one pixel border (if any) is part of it. Lines are rendered from the top,
lines which don't fit into the cell are not drawn. Inline color escapes (`"\aRRGGBB"`) are supported, the color is
reset to `color_text` at every redraw.

| Define | Default | Meaning |
|---|---|---|
| `VIC_WIDGET_TEXTBOX_LINE_LEN` | `127U` | Maximum line length in bytes without the terminating zero, longer words are cut. Can be defined before the include |

## Member functions

### Constructor

```cpp
textbox(drv& head, const config_type& config);
```

| Parameter | Description |
|---|---|
| `head` | Reference to the head driver |
| `config` | Configuration params |

### init

```cpp
virtual void init() final;
```

Draw the (empty) widget.

### set_text

```cpp
std::uint16_t set_text(const char* utf8);
```

Set a new text and render it from the first line. The text is not copied, the pointer must stay valid as long as the
widget shows it.

| Parameter | Description |
|---|---|
| `utf8` | Zero terminated UTF-8 text, `nullptr` clears the box |

Returns the number of lines the text takes.

### set_first_line

```cpp
void set_first_line(std::uint16_t line);
```

Set the first visible line and redraw (on change).

| Parameter | Description |
|---|---|
| `line` | First visible line, clamped to the last line |

### scroll

```cpp
void scroll(std::int16_t lines);
```

Scroll the text by a number of lines.

| Parameter | Description |
|---|---|
| `lines` | Lines to scroll, positive scrolls down (shows later lines) |

### get_line_count

```cpp
std::uint16_t get_line_count() const;
```

Returns the number of lines of the text after wrapping, valid after `set_text()` / `redraw()`.

### get_first_line

```cpp
std::uint16_t get_first_line() const;
```

Returns the first visible line.

### get_visible_lines

```cpp
std::uint16_t get_visible_lines() const;
```

Returns the number of lines which fit into the cell.

### set_color

```cpp
void set_color(color::value_type color);
```

Set a new text color and redraw.

| Parameter | Description |
|---|---|
| `color` | New text color |

### redraw

```cpp
void redraw();
```

Redraw the control: clear the cell, wrap and render the text.

## Events

The hit area is the cell. `textbox` has no own event reaction, an event handler set with `set_event_handler()` is
called, for example to scroll on a tap. See [input routing](/vic/reference/widgets/#input-routing).
