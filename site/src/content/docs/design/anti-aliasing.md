---
title: Anti aliasing
description: Pixel coverage anti aliasing with Wu lines and 4 x 4 sub pixel sampling, integer math only.
sidebar:
  order: 4
---

Anti aliasing is switched on per drawing context with `dc.anti_aliasing_enable(true)` (and off again with `false`).
It is off by default and the normal rendering is not affected by it at all.

![Anti aliasing demo: left plain, right anti aliased](/vic/media/demo_aa.gif)

## Principle: pixel coverage

Every pixel which is only partly covered by the primitive gets the pen color with an alpha channel of
`pen alpha * coverage / 255`. This alpha pixel takes the normal alpha path of vic, so the output shader blends it with
the pixel the head reads back, and a canvas sprite blends it into its pattern. No floating point math is used
anywhere.

- **Lines** (and therefore polylines and triangle outlines) use the Xiaolin Wu algorithm: for every step along the
  major axis the two nearest pixels are drawn, their coverages always sum up to 255. The end points are drawn
  exactly, axis aligned lines are drawn as plain lines.
- **Circles, discs, disc sectors, sectors and solid triangles** test their edge pixels against the exact shape with a
  4 x 4 sub pixel grid, so an edge pixel has 17 coverage levels. The exact shapes are:
  - a disc is a circle with radius `r + 0.5`,
  - a circle outline is the one pixel wide ring from `r - 0.5` to `r + 0.5`,
  - a sector is the ring from `inner - 0.5` to `outer + 0.5`: its outer edge is the edge of a disc with the outer
    radius, so an anti aliased pie equals a disc and a ring sits exactly on a disc with radius `inner - 1`. It keeps
    its half open angle interval `[start, end)`,
  - a triangle covers everything on or inside its edges.

  The result has the same visual size as the non anti aliased primitive. Fully covered pixels are drawn as solid
  horizontal runs, so the interior is still rendered with the native line functions of the head and only the edge
  pixels are single alpha pixels.
- **Wide lines** (pen width > 1) are sampled like the filled primitives against the rectangle of their stroke, their
  round joints against a disc, wide circles are ring sectors.
- Rounded boxes and rectangles get anti aliased corners, their straight parts stay as they are.
- **Not affected:** pixels, axis aligned lines, boxes, plain rectangles and text. Fonts bring their own anti aliasing
  with 2, 4 or 8 bit per pixel glyphs, the text renderer maps the pixel intensity to the alpha channel in the same way.

```cpp
vic::dc dc(head);
dc.anti_aliasing_enable();                   // from now on edges are smooth
dc.set_color(vic::color::white);
dc.line({ 10, 10 }, { 100, 40 });            // Wu line
dc.disc({ 60, 60 }, 25U);                    // disc with coverage sampled edge
dc.sector({ 60, 60 }, 10U, 25U, 30U, 150U);  // ring piece, radial and angular edges are smooth
dc.anti_aliasing_enable(false);              // back to plain pixels
```

## Costs

- The edge pixels cost CPU time: 16 shape tests per edge pixel, 9 per interior pixel inside the bounding box.
- Every edge pixel needs the background pixel for the alpha blending. On a write only serial display (no `pixel_get`
  support) use a background color or the `framebuffer` head, see [Write only displays](/vic/design/write-only-displays/).
  Heads that keep their own framebuffer (like the SSD1306 head) and canvas sprites can blend anyway.
- Coordinates have to stay within +/- 4000 pixels for the 32 bit sub pixel math.

The [anti aliasing example](/vic/examples/anti-aliasing/) walks through `demo/demo_aa.cpp`, which renders the same
scene twice, left without and right with anti aliasing, with a 3x magnifier of the moving clock hand.
