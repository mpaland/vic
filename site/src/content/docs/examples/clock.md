---
title: Clock
description: An analog clock with a gradient filled face and three hands as canvas sprites drawn with a brush shader.
sidebar:
  order: 7
---

`demo_clock.cpp` draws an analog clock: the face is drawn once with a gradient fill, tick marks and numbers, the three
hands are canvas sprites which are redrawn every 100 ms. The sprites restore the face below them, so the face is never
redrawn.

## The face

```cpp
#include <cstdio>
#include "dc.h"
#include "tc.h"
#include "sprite.h"
#include "shader/gradient.h"
#include "shader/brush.h"
#include "font/LCD_8x8.h"

#define CLOCK_SIZE 180

// end point of a hand: 0 degree is 12 o'clock, clockwise
inline vic::vertex_type get_hand(std::uint16_t radius, std::uint16_t degree)
{
  return { static_cast<std::int16_t>(static_cast<std::int32_t>(radius) * vic::util::sin(static_cast<std::int16_t>(degree)) / 16384),
           static_cast<std::int16_t>(-static_cast<std::int32_t>(radius) * vic::util::cos(static_cast<std::int16_t>(degree)) / 16384) };
}

void render_face(vic::drv& head, vic::vertex_type center)
{
  const std::int16_t radius = CLOCK_SIZE / 2;
  vic::dc _dc(head);
  vic::tc _tc(_dc);

  // the bounding circle
  _dc.set_color(vic::color::gray);
  _dc.circle(center, radius);

  // fill the inside with a 4 point gradient, the fill stops at the gray circle
  vic::shader::gradient<4> gr = { { center.x, static_cast<std::int16_t>(center.y - radius), vic::color::darkred },
                                  { static_cast<std::int16_t>(center.x + radius), center.y, vic::color::darkgreen },
                                  { center.x, static_cast<std::int16_t>(center.y + radius), vic::color::darkblue },
                                  { static_cast<std::int16_t>(center.x - radius), center.y, vic::color::darkyellow } };
  _dc.shader_register(&gr);
  _dc.fill(center, vic::color::gray);
  _dc.shader_remove(&gr);

  // tick marks and numbers
  vic::font::LCD_8x8 lcd;
  _tc.set_font(lcd);
  _tc.set_color(vic::color::gray12);
  _dc.set_color(vic::color::white);
  for (std::uint16_t n = 0U; n < 12U; ++n) {
    const std::uint16_t a = static_cast<std::uint16_t>(360U * n / 12U);
    _dc.line(center + get_hand(radius * 5U / 6U, a), center + get_hand(radius, a));

    char str[4];
    std::snprintf(str, sizeof(str), "%u", static_cast<unsigned int>(n));
    vic::rect_type ext;
    _tc.get_extend(ext, reinterpret_cast<const std::uint8_t*>(str));
    const vic::vertex_type p = center + get_hand(radius * 6U / 8U, a);
    _tc.out({ static_cast<std::int16_t>(p.x - ext.width() / 2), static_cast<std::int16_t>(p.y - ext.height() / 2) },
            reinterpret_cast<const std::uint8_t*>(str));
  }
}
```

## The hands

Each hand is a canvas sprite with its origin at the clock center. A brush shader on the sprite gives the lines a two
pixel pen:

```cpp
void render_time(vic::drv& head, vic::vertex_type center, std::uint16_t radius, std::uint8_t hour, std::uint8_t min, std::uint8_t sec)
{
  // one frame each, the z-index puts the seconds hand on top
  static vic::sprite::canvas<CLOCK_SIZE * 2U, 1> h(head, 0);
  static vic::sprite::canvas<CLOCK_SIZE * 2U, 1> m(head, 1);
  static vic::sprite::canvas<CLOCK_SIZE * 2U, 1> s(head, 2);

  // a shared two pixel solid brush
  static vic::shader::brush br;
  vic::shader::brush::shape_type shape;
  br.get_stock_shape(shape, vic::shader::brush::style_solid, 2U);
  br.set_shape(shape);

  h.shader_register(&br);
  h.cls();                                                       // clear the canvas
  h.set_color(vic::color::blue);
  h.line({ 0, 0 }, get_hand(radius / 2U, static_cast<std::uint16_t>(360U * hour / 12U + 30U * min / 60U)));
  h.render(center);                                              // restores the face of the old hand
  h.shader_remove(&br);

  m.shader_register(&br);
  m.cls();
  m.set_color(vic::color::red);
  m.line({ 0, 0 }, get_hand(radius * 3U / 4U, static_cast<std::uint16_t>(360U * min / 60U + 6U * sec / 60U)));
  m.render(center);
  m.shader_remove(&br);

  s.shader_register(&br);
  s.cls();
  s.set_color(vic::color::green);
  s.line({ 0, 0 }, get_hand(radius * 20U / 21U, static_cast<std::uint16_t>(360U * sec / 60U)));
  s.render(center);
  s.shader_remove(&br);
}
```

## The loop

```cpp
extern vic::drv& get_singlehead(void);                           // demo/windows/main.cpp or demo/linux/main.cpp
void get_time(std::uint8_t& h, std::uint8_t& m, std::uint8_t& s);  // demo helper, local time

void demo()
{
  vic::drv& head = get_singlehead();
  head.init();
  const vic::vertex_type center = { static_cast<std::int16_t>(head.screen_width()  / 2U),
                                    static_cast<std::int16_t>(head.screen_height() / 2U) };
  render_face(head, center);
  for (;;) {
    std::uint8_t h, m, s;
    get_time(h, m, s);
    render_time(head, center, CLOCK_SIZE / 2U, h, m, s);
    vic::io::delay(100U);
  }
}
```

## Key points

- `dc::fill()` with a gradient shader fills the face up to the gray circle. The fill reads the pixels back, so it
  needs a head with `pixel_get()`.
- A canvas sprite is a drawing context: every primitive, the pen and the shaders work on it. The drawing only changes
  the sprite pattern, `render()` puts it on the head and restores the background of the previous pattern.
- Coordinates on the canvas are relative to the sprite origin, so `line({ 0, 0 }, ...)` starts at the position given
  to `render()`.
- The canvas size (`CLOCK_SIZE * 2` pixels) must hold all pixels of the hand including the brush width.
- The brush shader renders its pen shape for every pixel, stock shapes are 1 to 4 pixel wide in the styles solid, dash,
  dot and dashdot. `set_pen_width()` is the alternative for plain wide lines, see the
  [shaders example](/vic/examples/shaders/).

See [Sprites](/vic/reference/sprite/) and [Shaders](/vic/reference/shader/).
