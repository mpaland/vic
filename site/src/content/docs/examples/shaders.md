---
title: Shaders
description: Clipping, gradient, tint, rotation and brush shaders in the pixel pipeline of a drawing context.
sidebar:
  order: 9
---

Shaders are registered at a drawing context and transform every pixel on its way to the head: they clip, move,
recolor or multiply pixels. A shader is an object you own, `shader_register()` puts it into the pipe of the context,
`shader_remove()` takes it out again.

## Clipping and gradient

```cpp
#include "dc.h"
#include "shader/base.h"
#include "shader/gradient.h"

vic::dc _dc(_head);

vic::shader::clipping _clip({ 0, 0, 100, 50 });                          // draw only inside this rect
vic::shader::gradient<2U> _gradient = { { { 10, 25 }, vic::color::red },   // color points, the pixel color is
                                        { { 90, 25 }, vic::color::blue } }; // interpolated by distance
_dc.shader_register(&_clip);
_dc.shader_register(&_gradient);
_dc.disc({ 50, 25 }, 40U);                                                 // clipped and gradient filled
_dc.shader_remove(&_gradient);
_dc.shader_remove(&_clip);
```

- `shader_register()` inserts the shader at the front of the pipe, so the shader registered last processes a pixel
  first. Here the gradient colors the pixel, then the clipping decides whether it is drawn.
- `clipping(rect, inside = true)` draws only the pixels inside the rect, `inside = false` suppresses them. `set()`
  changes the region later.
- The gradient shaders take the color of their points and only the alpha of the pen color. `gradient<N>` interpolates
  by inverse squared distance, `gradient_horizontal<N>` ignores y, `gradient_vertical<N>` ignores x,
  `gradient_solid<N>` takes the color of the nearest point. The points are `pixel_type { vertex, color }`, both
  `{ { x, y }, color }` and `{ x, y, color }` initialize them.

A vertical gradient as background, as the font and sprite demos use it:

```cpp
vic::shader::gradient_vertical<3> sky = { { 0,   0, vic::color::argb(0x10U, 0x10U, 0x40U) },
                                          { 0, 100, vic::color::argb(0x50U, 0x30U, 0x70U) },
                                          { 0, 159, vic::color::argb(0xE0U, 0x70U, 0x30U) } };
_dc.shader_register(&sky);
_dc.set_color(vic::color::white);   // opaque pen, the gradient supplies the color
_dc.box({ 0, 0, 320, 160 });
_dc.shader_remove(&sky);
```

:::note
With an active shader every primitive is rendered pixel by pixel through the pipe. The native line and box functions
of the head (and its hardware acceleration) are only used when no shader is registered.
:::

## Tint

`tint(color, weight = 128)` mixes the RGB part of every pixel towards the tint color, the alpha is kept (0 leaves the
pixel unchanged, 255 replaces the RGB part). `demo_sprite.cpp` registers it on a sprite for a single
render, so a ghost turns red while it collides:

```cpp
vic::shader::tint red(vic::color::red, 200U);
ghost.shader_register(&red);
ghost.render(pos, frame);
ghost.shader_remove(&red);
```

## Rotation

`rotate(center, angle)` (`shader/rotate.h`) rotates every pixel around a center, the angle is in degree, clockwise on
screen:

```cpp
#include "shader/rotate.h"

vic::shader::rotate rot({ 160, 100 }, 30);
_dc.shader_register(&rot);
_dc.set_color(vic::color::yellow);
_dc.box({ 120, 80, 200, 120 });     // drawn as a rotated box
_dc.shader_remove(&rot);
rot.set({ 160, 100 }, 60);          // new center and angle
```

The rotation shader maps every pixel, which is fine for single primitives. For moving objects the demos use other
ways: canvas sprites rotate their pattern at render time with `set_rotation()` (the rolling ball and the spinning coins
in the [sprites example](/vic/examples/sprites/)), and vertices are rotated before drawing with
`vic::util::vertex_rotate(point, center, degree)` (the clock hand in the [anti aliasing example](/vic/examples/anti-aliasing/)).

## Brush

The brush shader renders a pen shape for every pixel: stock shapes are 1 to 4 pixel wide, in the styles
`style_solid`, `style_dash`, `style_dot` and `style_dashdot`. `demo_clock.cpp` draws its hands with it:

```cpp
#include "shader/brush.h"

vic::shader::brush br;
vic::shader::brush::shape_type shape;
br.get_stock_shape(shape, vic::shader::brush::style_dash, 2U);   // false if the width is not 1 - 4
br.set_shape(shape);

_dc.shader_register(&br);
_dc.set_color(vic::color::green);
_dc.line({ 10, 10 }, { 150, 60 });  // two pixel dashed line
_dc.shader_remove(&br);
br.reset();                          // restart the dash pattern
```

- `shape_type` is `{ alpha, width, height, style }`, `alpha` is an array of `width * height` alpha values, so custom
  pens are possible.
- For plain wide lines `set_pen_width()` of the drawing context is the faster way, it renders the stroke geometry
  (with anti aliasing if enabled) instead of stamping a shape per pixel.

See [Shaders](/vic/reference/shader/) for all shaders.
