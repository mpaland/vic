---
title: Anti aliasing
description: The same scene without and with anti aliasing side by side, with a 3x magnifier of the moving clock hand.
sidebar:
  order: 6
---

`demo_aa.cpp` splits the screen in two halves which show exactly the same scene: on the left anti aliasing is off, on
the right it is on. Both halves are rendered by the very same code, the only difference is one call to
`dc::anti_aliasing_enable()`. A 3x magnifier at the bottom shows the pixels of the moving clock hand.

![Anti aliasing demo](/vic/media/demo_aa.gif)

## Two drawing contexts on one head

```cpp
#include "dc.h"
#include "shader/gradient.h"

const std::int16_t PANEL_W  = 160;   // width of one half
const std::int16_t ANIM_TOP = 82;    // animated zone, redrawn every frame
const std::int16_t ANIM_BOT = 150;

vic::dc dc_plain(_head);
vic::dc dc_aa(_head);
dc_plain.anti_aliasing_enable(false);
dc_aa.anti_aliasing_enable(true);

vic::dc* const     dcs[2] = { &dc_plain, &dc_aa };
const std::int16_t xos[2] = { 0, PANEL_W };            // x origin of each half
```

## One scene, rendered twice

The scene functions take the drawing context and the x origin of the panel:

```cpp
void render_animated(vic::dc& _dc, std::int16_t xo, std::uint32_t t)
{
  const std::int16_t hand_angle = static_cast<std::int16_t>((t * 3U) % 360U);
  const std::int16_t spin_angle = static_cast<std::int16_t>((t * 6U) % 360U);

  // clock: ring and a rotating hand with pen width 3
  const vic::vertex_type c = { static_cast<std::int16_t>(xo + 32), 116 };
  _dc.set_color(vic::color::argb(0x60U, 0xC0U, 0xFFU));
  _dc.circle(c, 28U);
  _dc.set_color(vic::color::argb(0xFFU, 0xE0U, 0x40U));
  _dc.set_pen_width(3U);
  _dc.line(c, vic::util::vertex_rotate({ c.x, static_cast<std::int16_t>(c.y - 25) }, c, hand_angle));
  _dc.set_pen_width(1U);
  _dc.disc(c, 2U);

  // pie spinner: three ring sectors which rotate
  const vic::vertex_type p = { static_cast<std::int16_t>(xo + 136), 116 };
  const std::uint16_t a0 = static_cast<std::uint16_t>(spin_angle);
  _dc.set_color(vic::color::argb(0xE0U, 0x40U, 0x50U));
  _dc.sector(p, 14U, 24U, a0, static_cast<std::uint16_t>(a0 + 150U));
  _dc.set_color(vic::color::argb(0xF0U, 0xC0U, 0x30U));
  _dc.sector(p, 14U, 24U, static_cast<std::uint16_t>(a0 + 150U), static_cast<std::uint16_t>(a0 + 250U));
  _dc.set_color(vic::color::argb(0x40U, 0xA0U, 0xE0U));
  _dc.sector(p, 14U, 24U, static_cast<std::uint16_t>(a0 + 250U), static_cast<std::uint16_t>(a0 + 360U));
}
```

The static part works the same way: a fan of lines with all slopes, a circle with a translucent disc on top
(`vic::color::argb(0xFFU, 0x90U, 0x30U, 0xA0U)`, anti aliasing and alpha blending work together), a rounded box and a
rounded rectangle with pen width 2.

## The frame loop

```cpp
// the panel background: a vertical gradient, also used to clear the animated zone
vic::shader::gradient_vertical<3> background = { { 0, 18,           vic::color::argb(0x18U, 0x20U, 0x38U) },
                                                 { 0, ANIM_TOP,     vic::color::argb(0x30U, 0x28U, 0x50U) },
                                                 { 0, ANIM_BOT - 1, vic::color::argb(0x60U, 0x30U, 0x40U) } };

void render_magnifier(vic::dc& _dc, std::int16_t xo);   // see below

for (std::uint32_t t = 0U; ; ++t) {
  dc_plain.present_lock();                             // both contexts locked: one present per frame
  dc_aa.present_lock();
  for (std::size_t p = 0U; p < 2U; ++p) {
    // clear the animated zone with the gradient
    dcs[p]->shader_register(&background);
    dcs[p]->set_color(vic::color::white);
    dcs[p]->box({ xos[p], ANIM_TOP, static_cast<std::int16_t>(xos[p] + PANEL_W), ANIM_BOT });
    dcs[p]->shader_remove(&background);

    render_animated(*dcs[p], xos[p], t);
    render_magnifier(dc_plain, xos[p]);
  }
  dc_aa.present_lock(false);
  dc_plain.present_lock(false);
  vic::io::delay(40U);
}
```

## The magnifier

The magnifier reads the pixels around the clock hand back with `pixel_get()` and draws every pixel as a 3 x 3 box. It
uses the plain context, so the magnified boxes themselves are not smoothed:

```cpp
void render_magnifier(vic::dc& _dc, std::int16_t xo)
{
  const std::int16_t src_x = static_cast<std::int16_t>(xo + 32 - 20);
  const std::int16_t src_y = 116 - 27;
  const std::int16_t dst_x = static_cast<std::int16_t>(xo + (160 - 40 * 3) / 2);
  for (std::int16_t y = 0; y < 12; ++y) {
    for (std::int16_t x = 0; x < 40; ++x) {
      _dc.set_color(_dc.pixel_get({ static_cast<std::int16_t>(src_x + x), static_cast<std::int16_t>(src_y + y) }));
      _dc.box({ static_cast<std::int16_t>(dst_x + x * 3),       static_cast<std::int16_t>(152 + y * 3),
                static_cast<std::int16_t>(dst_x + (x + 1) * 3), static_cast<std::int16_t>(152 + (y + 1) * 3) });
    }
  }
}
```

## Key points

- Anti aliasing is a state of the drawing context. Several contexts with different settings can draw on one head.
- Affected are sloped lines (Wu lines), wide lines, circles, discs, sectors, solid triangles and rounded corners. Axis
  aligned lines, boxes, plain rectangles and text are not affected, fonts bring their own anti aliasing.
- The edge pixels are alpha blended with the background, so the head must support `pixel_get()` (the Windows and X11
  heads do). On a write only display use a [framebuffer](/vic/heads/framebuffer/), a head with its own framebuffer or
  canvas sprites, or set a background color with `set_bg_color()` (exact on uniform backgrounds).
- The gradient background makes the edge pixels blend against changing colors, which shows that the blending uses
  the real background.

See [Anti aliasing](/vic/design/anti-aliasing/) for the algorithms and their costs.
