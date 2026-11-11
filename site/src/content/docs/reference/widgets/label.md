---
title: label
description: A text or numeric readout in a fixed cell, redrawn only when the text changes.
sidebar:
  order: 10
---

`label` is a text or numeric readout in a fixed cell. The text is redrawn only when it changes, so `set_text()` and
`set_value()` can be called every frame. Numbers are formatted without floating point.

![Gauges demo](/vic/media/demo_gauges.gif)

## Header

```cpp
#include "widget/label.h"
```

## Example

```cpp
#include "widget/label.h"
#include "font/Montserrat_16_aa4.h"

static vic::font::Montserrat_16_aa4 font;

vic::widget::label::config_type cfg = { { 10, 10 }, 60U, 20U, &font, vic::widget::label::align_center,
                                        vic::color::white, vic::color::black, vic::color::none };
vic::widget::label lbl(_head, cfg);
lbl.init();
lbl.set_text("RPM");
lbl.set_value(2100);             // "2100"
lbl.set_value(270, 1U, " PSI");  // "27.0 PSI"
lbl.set_text("\aFF6060HOT");     // inline color escape
```

## Configuration

| Member | Type | Meaning |
|---|---|---|
| `origin` | `vertex_type` | Origin (left/top) |
| `x_size` | `std::uint16_t` | X size of the cell |
| `y_size` | `std::uint16_t` | Y size of the cell |
| `font` | `const font::font_type*` | Font, must stay valid during the lifetime of the widget |
| `align` | `align_type` | Horizontal text alignment: `align_left`, `align_center` or `align_right` |
| `color_text` | `color::value_type` | Text color |
| `color_bg` | `color::value_type` | Background color, `color::none` leaves the background untouched |
| `color_border` | `color::value_type` | Border color, `color::none` for no border |

`x_size * y_size` is the complete cell, the one pixel border (if any) is part of it. The text is vertically centered in
the cell and horizontally aligned by `align`. The cell is cleared with `color_bg` before the text is rendered. With
`color::none` the new text is drawn over the old one, use it for static texts only. Inline color escapes
(`"\aRRGGBB"`) are supported.

| Define | Default | Meaning |
|---|---|---|
| `VIC_WIDGET_LABEL_MAX_LEN` | `63U` | Maximum text length in bytes without the terminating zero, an inline color escape takes 7 bytes. Can be defined before the include |

## Member functions

### Constructor

```cpp
label(drv& head, const config_type& config);
```

| Parameter | Description |
|---|---|
| `head` | Reference to the head driver |
| `config` | Configuration params |

The initial text is empty.

### init

```cpp
virtual void init() final;
```

Draw the (empty) widget.

### set_text

```cpp
void set_text(const char* utf8);
```

Set a new text, the widget is redrawn if the text differs from the actual one. The text is copied.

| Parameter | Description |
|---|---|
| `utf8` | Zero terminated UTF-8 text, truncated to `VIC_WIDGET_LABEL_MAX_LEN` bytes, `nullptr` is an empty text |

### set_value

```cpp
void set_value(std::int32_t value, std::uint8_t decimals = 0U, const char* suffix = nullptr);
```

Set a numeric value as text, no floating point is used.

| Parameter | Description |
|---|---|
| `value` | The value, e.g. 270 with `decimals = 1` is rendered as `"27.0"` |
| `decimals` | Number of decimal places (0 - 9) |
| `suffix` | Optional text appended to the number, e.g. `" PSI"`, `nullptr` for none |

### set_color

```cpp
void set_color(color::value_type color);
```

Set a new text color, the widget is redrawn.

| Parameter | Description |
|---|---|
| `color` | New text color |

### get_text

```cpp
const char* get_text() const;
```

Returns the actual text, zero terminated.

### redraw

```cpp
void redraw();
```

Redraw the control.

### format

```cpp
static std::size_t format(char* buf, std::int32_t value, std::uint8_t decimals);
```

Format a value as decimal number, like `"-27.05"`. Used by other widgets as well (`spinbox`).

| Parameter | Description |
|---|---|
| `buf` | Output buffer, at least 14 chars |
| `value` | Value to format |
| `decimals` | Decimal places, 0 - 9 |

Returns the length of the string in `buf`, the string is not terminated.

## Events

The hit area is the cell. `label` has no own event reaction, an event handler set with `set_event_handler()` is called.
See [input routing](/vic/reference/widgets/#input-routing).
