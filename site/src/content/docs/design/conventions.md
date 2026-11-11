---
title: Conventions
description: Coordinates, rects, angles, colors and pen width used throughout vic.
sidebar:
  order: 2
---

## Coordinates and rects

- A point is a `vertex_type { x, y }` with signed 16 bit coordinates. `{ 0, 0 }` is the top left pixel of the
  screen.
- A rect is a `rect_type { left, top, right, bottom }`. Right and bottom are **exclusive**: `{ left, top, right,
  bottom }` covers the pixels `left <= x < right` and `top <= y < bottom`. So `{ 0, 0, 10, 10 }` is a 10 x 10 box.
- Lines include **both** end points.

## Angles

Angles are given in degrees, 0° is at 3 o'clock and angles run **counter clockwise**. Angle intervals like in
`circle()` and `sector()` are half open (`[start, end)`), so adjacent sectors tile without gaps or double drawn
pixels.

`util::vertex_rotate()` and the canvas sprite rotation turn clockwise on screen.

## Colors

All colors are `color::value_type`, 32 bit ARGB8888. `color::argb(r, g, b, a = 255)` builds one,
`color::none` means "no color" (for example a pixel that can't be read back). The head converts to its own format
(RGB565, 4 bit gray, monochrome, ...). See [Colors and helpers](/vic/reference/color-util/).

An alpha below 255 is only blended when alpha blending is enabled at the drawing context
(`alpha_blending_enable()`) and the background can be read, see [Write only displays](/vic/design/write-only-displays/).

## Pen width

Outlines are one pixel wide by default, `set_pen_width()` makes `line()`, `polyline()`, `triangle()`, `rectangle()`
and `circle()` wider:

- A line is the rectangle around its segment with butt caps, an axis aligned line is a box (an even width has its
  extra pixel at the bottom / right). Every pixel is written once, so translucent pens work.
- Polylines and triangles get round joints. The joints overlap the lines, a translucent pen is blended twice there.
- A rectangle grows inwards, its outer size stays the rect. The pen width of a rounded rectangle is limited to its
  border radius.
- A circle is the ring around its radius, like `sector()` with the radii `radius - (width - 1) / 2` and
  `radius + width / 2`.
- `line_horz()` and `line_vert()` always stay one pixel wide, filled primitives and text are not affected.

## No floating point

vic uses no `float` or `double` anywhere. Trigonometry comes from integer tables: `util::sin()` and `util::cos()`
return the value scaled by 16384. Anti aliased primitives use 32 bit sub pixel math, so coordinates have to stay
within +/- 4000 pixels there.
