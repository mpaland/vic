---
title: Gauges
description: An aircraft engine page built from round gauges, bar gauges, labels and group boxes.
sidebar:
  order: 4
---

`demo_gauges.cpp` shows all gauge style widgets on one 320 x 200 screen: three round gauges with color bands and a
sprite needle, vertical bar gauges with one or two markers (one in fill mode), a horizontal strip, labels for names and
numeric readouts, and group boxes around the fuel and the electrical section. All values are animated with slow sine
waves.

![Gauges demo](/vic/media/demo_gauges.gif)

## Bands, markers and config helpers

The demo builds many widgets with the same look, so small helpers fill the `config_type` structs. Values with one
decimal are stored times 10 (manifold pressure 100 - 350 is 10.0 - 35.0 inHg).

```cpp
#include "dc.h"
#include "widget/gauge.h"
#include "widget/bargauge.h"
#include "widget/label.h"
#include "widget/groupbox.h"
#include "font/Montserrat_10_aa2.h"
#include "font/Montserrat_16_aa4.h"

typedef vic::widget::gauge<>    gauge_type;
typedef vic::widget::bargauge<> bar_type;
typedef vic::widget::label      label_type;
typedef vic::widget::groupbox   group_type;

const vic::color::value_type COL_BG     = vic::color::black;
const vic::color::value_type COL_RING   = vic::color::gray04;
const vic::color::value_type COL_TEXT   = vic::color::white;
const vic::color::value_type COL_GREEN  = vic::color::argb(0x20U, 0xD0U, 0x20U);
const vic::color::value_type COL_YELLOW = vic::color::argb(0xF0U, 0xD0U, 0x20U);
const vic::color::value_type COL_RED    = vic::color::argb(0xF0U, 0x20U, 0x20U);

vic::font::Montserrat_10_aa2 font_10;
vic::font::Montserrat_16_aa4 font_16;

// bands: { from, to (exclusive), color }
const vic::widget::band_type bands_rpm[] = { { 800, 2500, COL_GREEN }, { 2500, 2700, COL_YELLOW }, { 2700, 3000, COL_RED } };
const vic::widget::band_type bands_qty[] = { {   0,    5, COL_RED }, { 5, 10, COL_YELLOW }, { 10, 50, COL_GREEN } };

// markers: triangle pointers beside the strip
const bar_type::marker_type marker_left[] = { { COL_TEXT, bar_type::side_before, 4U } };

// a 240 degree gauge (-120 .. 120), radius 34, ring 6, needle 26 x 5, no background disc
gauge_type::config_type gauge_cfg(std::int16_t cx, std::int16_t lower, std::int16_t upper,
                                  const vic::widget::band_type* bands, std::uint8_t band_count, std::uint8_t ticks)
{
  const gauge_type::config_type cfg = { { cx, 38 }, 34U, 6U, -120, 120, lower, upper, bands, band_count, ticks, 26U, 5U,
                                        COL_RING, COL_TEXT, COL_TEXT, vic::color::gray12, vic::color::none };
  return cfg;
}

// a vertical bar, 14 pixel wide with a 6 pixel strip, no ticks
bar_type::config_type vbar_cfg(std::int16_t x, std::int16_t y, std::uint16_t h, std::int16_t lower, std::int16_t upper,
                               const vic::widget::band_type* bands, std::uint8_t band_count,
                               const bar_type::marker_type* markers, std::uint8_t marker_count, vic::color::value_type fill)
{
  const bar_type::config_type cfg = { { x, y }, 14U, h, bar_type::bottom_to_top, 6U, lower, upper, bands, band_count, 0U,
                                      markers, marker_count, fill, COL_RING, COL_BG, COL_BG };
  return cfg;
}

label_type::config_type label_cfg(std::int16_t x, std::int16_t y, std::uint16_t w, std::uint16_t h,
                                  const vic::font::font_type& font, vic::color::value_type color)
{
  const label_type::config_type cfg = { { x, y }, w, h, &font, label_type::align_center, color, COL_BG, vic::color::none };
  return cfg;
}

// slow sine wave between lo and hi, vic::util::sin() returns sin * 16384
std::int16_t wave(std::uint32_t t, std::uint32_t period, std::int16_t phase, std::int16_t lo, std::int16_t hi)
{
  const std::int16_t a = static_cast<std::int16_t>((t * 360U / period + static_cast<std::uint32_t>(phase)) % 360U);
  const std::int32_t s = vic::util::sin(a) + 16384;   // 0 .. 32768
  return static_cast<std::int16_t>(lo + (static_cast<std::int32_t>(hi - lo) * s) / 32768);
}
```

## The page

Condensed to one gauge and two bars, the demo has three gauges, five bars and a horizontal strip:

```cpp
extern vic::drv& get_singlehead(void);   // demo/windows/main.cpp or demo/linux/main.cpp

void demo()
{
  vic::drv& head = get_singlehead();
  head.init();
  vic::dc _dc(head);
  _dc.cls(COL_BG);

  // round gauge with its name and readout
  gauge_type rpm     (head, gauge_cfg(54, 0, 3000, bands_rpm, 3U, 7U));
  label_type rpm_name(head, label_cfg(34, 46, 40U, 10U, font_10, vic::color::gray10));
  label_type rpm_val (head, label_cfg(24, 58, 60U, 18U, font_16, COL_TEXT));

  // group box with a bar gauge inside: created first, so it is drawn below
  const group_type::config_type fuel_cfg = { { 2, 90, 150, 198 }, 4U, &font_10, "FUEL", vic::color::gray08, COL_TEXT, COL_BG };
  group_type fuel(head, fuel_cfg);
  bar_type   qty (head, vbar_cfg(25, 118, 74U, 0, 50, bands_qty, 3U, marker_left, 1U, vic::color::none));
  bar_type   gph (head, vbar_cfg(69, 118, 74U, 0, 200, nullptr, 0U, marker_left, 1U, COL_GREEN));   // fill mode

  rpm.anti_aliasing_enable();   // anti aliased scale

  // init: group boxes first, then the widgets inside
  fuel.init();
  rpm.init(); rpm_name.init(); rpm_val.init();
  qty.init(); gph.init();
  rpm_name.set_text("RPM");

  for (std::uint32_t t = 0U; ; ++t) {
    const std::int16_t v_rpm = wave(t, 400U, 0, 600, 2900);   // sine between 600 and 2900, period 400 frames
    rpm.set_value(v_rpm);                                     // moves the needle sprite
    rpm_val.set_value(v_rpm);                                 // redrawn only if the text changes
    qty.set_value(0U, wave(t, 900U, 45, 2, 48));             // moves marker 0
    gph.set_value(0U, wave(t, 250U, 180, 30, 195));          // delta draws the fill and moves the marker
    vic::io::delay(40U);
  }
}
```

## Key points

- The scale of a round gauge is drawn once by `init()`, the needle is a canvas sprite which restores the scale below
  it. A value change costs only the needle pixels.
- A bar gauge with `color_fill != color::none` fills the strip up to marker 0 (bar graph style) and delta draws the
  fill. The electrical section of the demo uses two markers per bar (`side_before` and `side_after`) for two engines.
- `label::set_value(value, decimals, suffix)` formats fixed point values without floating point: `man_val.set_value(v_man, 1U)`
  shows 254 as `"25.4"`. Labels can be updated every frame, they redraw only on a changed text.
- Widgets created later are on top for the [input routing](/vic/reference/widgets/#input-routing), so the group box is
  created before the widgets inside it.

See [gauge](/vic/reference/widgets/gauge/), [bargauge](/vic/reference/widgets/bargauge/),
[label](/vic/reference/widgets/label/) and [groupbox](/vic/reference/widgets/groupbox/).
