---
title: gauge
description: A round gauge with an arc scale, colored bands, major ticks and a sprite needle.
sidebar:
  order: 7
---

`gauge` is a round gauge: an arc scale with colored bands and major ticks and a needle. The needle is a canvas sprite
which restores the background by itself, so a value change costs only the needle pixels.

![Gauges demo](/vic/media/demo_gauges.gif)

## Header

```cpp
#include "widget/gauge.h"
```

## Example

```cpp
#include "widget/gauge.h"

static const vic::widget::band_type bands[] = { {  800, 2500, vic::color::green },
                                                { 2500, 2700, vic::color::yellow },
                                                { 2700, 3000, vic::color::red } };

vic::widget::gauge<>::config_type cfg = { { 60, 50 }, 34U, 6U, -120, 120, 0, 3000,
                                          bands, 3U, 7U, 28U, 5U,
                                          vic::color::gray08, vic::color::white, vic::color::white,
                                          vic::color::gray12, vic::color::black };
vic::widget::gauge<> rpm(_head, cfg);
rpm.anti_aliasing_enable();   // smooth scale, the needle is always anti aliased
rpm.init();
rpm.set_value(2100);
```

## Template parameter

```cpp
template <std::uint16_t Needle_Pixels = 256U>
class gauge;
```

`Needle_Pixels` is the maximum number of pixels of the needle sprite. An anti aliased needle of length `L` and width `W`
needs about `L * (W + 3)` pixels.

## Configuration

Angles are compass degrees: 0 is 12 o'clock, positive is clockwise. A scale from the lower left to the lower right over
the top is `angle_start = -120`, `angle_end = 120`. The ring spans from `radius - ring_width` to `radius`, the bands are
drawn on the ring.

| Member | Type | Meaning |
|---|---|---|
| `center` | `vertex_type` | Center of the gauge |
| `radius` | `std::uint16_t` | Outer radius of the ring |
| `ring_width` | `std::uint8_t` | Width of the ring (scale) |
| `angle_start` | `std::int16_t` | Angle of `range_lower` in compass degrees |
| `angle_end` | `std::int16_t` | Angle of `range_upper`, must be greater than `angle_start` |
| `range_lower` | `std::int16_t` | Lower range, e.g. 0 |
| `range_upper` | `std::int16_t` | Upper range, e.g. 100 |
| `bands` | `const band_type*` | Colored bands on the ring, `nullptr` for none, must stay valid |
| `band_count` | `std::uint8_t` | Number of bands |
| `tick_count` | `std::uint8_t` | Number of major ticks including both ends, spread evenly from `angle_start` to `angle_end`, 0 or 1 for none |
| `needle_length` | `std::uint8_t` | Needle length in pixel from the center |
| `needle_width` | `std::uint8_t` | Needle width at the hub in pixel (tapers to the tip), 2 - 8 |
| `color_ring` | `color::value_type` | Ring color where no band is |
| `color_tick` | `color::value_type` | Tick color |
| `color_needle` | `color::value_type` | Needle color |
| `color_hub` | `color::value_type` | Hub (needle center) color, `color::none` for no hub |
| `color_bg` | `color::value_type` | Background disc color, `color::none` leaves the background untouched |

The initial value is `range_lower`.

## Member functions

### Constructor

```cpp
gauge(drv& head, const config_type& config);
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

Set a new value and move the needle.

| Parameter | Description |
|---|---|
| `value` | New value, clamped to the valid range |

### get_value

```cpp
std::int16_t get_value() const;
```

Returns the actual value.

### anti_aliasing_enable

```cpp
void anti_aliasing_enable(bool enable = true);
```

Enable anti aliasing for the scale (ring, bands and ticks), default is off. The needle is always anti aliased. The
scale is redrawn if the widget is already initialized. Like `dc::anti_aliasing_enable()` this needs a head which can
read its pixels back or a background color set in the drawing context.

| Parameter | Description |
|---|---|
| `enable` | True to enable anti aliasing |

### set_range

```cpp
void set_range(std::int16_t lower, std::int16_t upper);
```

Set a new range, the value is set to `lower`. The range is only taken if `lower < upper`, the widget is redrawn in any
case.

| Parameter | Description |
|---|---|
| `lower` | Lower range limit |
| `upper` | Upper range limit |

### redraw

```cpp
void redraw();
```

Redraw the control (scale and needle). The needle is hidden first, so the background below it is redrawn too.

## Events

The hit area is the gauge disc (`radius`). `gauge` has no own event reaction, an event handler set with
`set_event_handler()` is called. See [input routing](/vic/reference/widgets/#input-routing).

The [gauges example](/vic/examples/gauges/) shows three round gauges on an engine page.
