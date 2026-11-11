---
title: bargauge
description: A linear scale strip with colored bands, ticks, an optional fill bar and up to two triangle markers.
sidebar:
  order: 2
---

`bargauge` is a linear scale: a strip with colored bands and ticks, an optional fill bar (bar graph style) and up to two
triangle markers (pointers) beside the strip. The markers are canvas sprites, moving a marker costs only its pixels.

![Gauges demo](/vic/media/demo_gauges.gif)

## Header

```cpp
#include "widget/bargauge.h"
```

## Example

```cpp
#include "widget/bargauge.h"

typedef vic::widget::bargauge<> bar_type;

static const vic::widget::band_type bands[] = { { 0, 10, vic::color::red }, { 10, 20, vic::color::yellow }, { 20, 50, vic::color::green } };
static const bar_type::marker_type markers[] = { { vic::color::white, bar_type::side_before, 4U },
                                                 { vic::color::cyan,  bar_type::side_after,  4U } };

bar_type::config_type cfg = { { 20, 100 }, 14U, 80U, bar_type::bottom_to_top, 4U,
                              0, 50, bands, 3U, 6U, markers, 2U,
                              vic::color::none, vic::color::gray08, vic::color::white, vic::color::black };
bar_type qty(_head, cfg);
qty.init();
qty.set_value(0U, 23);   // move marker 0 to 23
qty.set_value(1U, 31);   // move marker 1 to 31
```

## Template parameter

```cpp
template <std::uint16_t Marker_Pixels = 32U>
class bargauge;
```

`Marker_Pixels` is the maximum number of pixels of one marker sprite, a marker of size `s` needs `s * s` pixels.
`VIC_WIDGET_BARGAUGE_MAX_MARKERS` (2) is the maximum number of markers per bar gauge.

## Configuration

`origin` is the left/top corner, `x_size * y_size` is the complete widget including the marker lanes. The strip
(`bar_width` wide) is centered across the orientation axis, the markers live in the lanes beside it.

| Member | Type | Meaning |
|---|---|---|
| `origin` | `vertex_type` | Origin (left/top) |
| `x_size` | `std::uint16_t` | X size of the widget |
| `y_size` | `std::uint16_t` | Y size of the widget |
| `orientation` | `orientation_type` | Orientation of the scale (direction of increasing values) |
| `bar_width` | `std::uint8_t` | Width of the strip across the orientation axis |
| `range_lower` | `std::int16_t` | Lower range, e.g. 0 |
| `range_upper` | `std::int16_t` | Upper range, e.g. 100 |
| `bands` | `const band_type*` | Colored bands, `nullptr` for none, must stay valid |
| `band_count` | `std::uint8_t` | Number of bands |
| `tick_count` | `std::uint8_t` | Number of ticks including both ends, 0 for none |
| `markers` | `const marker_type*` | Marker definitions, must stay valid |
| `marker_count` | `std::uint8_t` | Number of markers, 0 - `VIC_WIDGET_BARGAUGE_MAX_MARKERS`, larger values are limited |
| `color_fill` | `color::value_type` | `color::none` shows the band strip only. Any other color fills the strip from the `range_lower` end up to the value of marker 0 (bar graph style), the bands show in the rest of the strip. The fill is delta drawn |
| `color_bar` | `color::value_type` | Strip color where no band / fill is |
| `color_tick` | `color::value_type` | Tick color |
| `color_bg` | `color::value_type` | Background color of the marker lanes, `color::none` leaves the background untouched |

### orientation_type

| Value | Meaning |
|---|---|
| `left_to_right` | Horizontal, values increase to the right |
| `right_to_left` | Horizontal, values increase to the left |
| `bottom_to_top` | Vertical, values increase upwards |
| `top_to_bottom` | Vertical, values increase downwards |

### marker_type

| Member | Type | Meaning |
|---|---|---|
| `color` | `color::value_type` | Marker color |
| `side` | `marker_side_type` | `side_before`: left of a vertical strip, above a horizontal strip. `side_after`: right of a vertical strip, below a horizontal strip |
| `size` | `std::uint8_t` | Triangle height in pixel (tip to base), 3 - 7 |

The marker is a solid triangle whose tip points at the value on the strip.

## Member functions

### Constructor

```cpp
bargauge(drv& head, const config_type& config);
```

| Parameter | Description |
|---|---|
| `head` | Reference to the head driver |
| `config` | Configuration params |

All marker values start at `range_lower`.

### init

```cpp
virtual void init() final;
```

Create the marker patterns and draw the widget.

### set_value

```cpp
void set_value(std::uint8_t marker, std::int16_t value);
```

Set a new value for a marker, the marker is moved. In fill mode the bar of marker 0 is updated (delta drawn). Invalid
marker indices are ignored.

| Parameter | Description |
|---|---|
| `marker` | Marker index |
| `value` | New value, clamped to the valid range |

### get_value

```cpp
std::int16_t get_value(std::uint8_t marker = 0U) const;
```

Returns the actual value of a marker.

| Parameter | Description |
|---|---|
| `marker` | Marker index |

### set_range

```cpp
void set_range(std::int16_t lower, std::int16_t upper);
```

Set a new range, all values are set to `lower`. The range is only taken if `lower < upper`, the widget is redrawn in
any case.

| Parameter | Description |
|---|---|
| `lower` | Lower range limit |
| `upper` | Upper range limit |

### redraw

```cpp
void redraw();
```

Redraw the control: the markers are hidden, lanes, strip, bands, fill and ticks are drawn and the markers are rendered
again.

## Events

The hit area is the complete widget rect. `bargauge` has no own event reaction, an event handler set with
`set_event_handler()` is called. See [input routing](/vic/reference/widgets/#input-routing).

The [gauges example](/vic/examples/gauges/) uses vertical bar gauges in band, fill and dual marker mode and a
horizontal strip.
