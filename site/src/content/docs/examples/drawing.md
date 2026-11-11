---
title: Drawing primitives
description: Lines, boxes, rounded boxes, circles, discs, sectors, triangles, pen width and flood fill with the drawing context.
sidebar:
  order: 1
---

All drawing is done with a drawing context (`vic::dc`) on a head. It holds the pen state (color, pen width, anti
aliasing) and renders the primitives through its shader pipe to the head.

## Primitives

```cpp
#include "dc.h"

vic::dc _dc(_head);
_dc.cls(vic::color::black);
_dc.present_lock();                                    // collect everything, present once at the end

// lines, both end points are included
_dc.set_color(vic::color::white);
_dc.line({ 10, 10 }, { 100, 40 });
_dc.line_horz({ 10, 50 }, { 100, 50 });
_dc.line_vert({ 110, 10 }, { 110, 50 });
const vic::vertex_type zigzag[] = { { 10, 60 }, { 40, 80 }, { 70, 60 }, { 100, 80 } };
_dc.polyline(zigzag, 4U);

// boxes and rectangles, right/bottom are exclusive
_dc.set_color(vic::color::orange);
_dc.box({ 120, 10, 160, 40 });                         // filled
_dc.box({ 170, 10, 210, 40 }, 6U);                     // filled with rounded corners
_dc.rectangle({ 220, 10, 260, 40 });                   // outline
_dc.rectangle({ 270, 10, 310, 40 }, 6U);               // outline with rounded corners

// circles, discs and sectors, angles: 0 is 3 o'clock, counter clockwise
_dc.set_color(vic::color::cyan);
_dc.circle({ 40, 130 }, 25U);
_dc.circle({ 100, 130 }, 25U, 0U, 180U);              // upper half
_dc.disc({ 160, 130 }, 25U);
_dc.disc_sector({ 220, 130 }, 25U, 0U);               // quadrant 0: top right
_dc.sector({ 280, 130 }, 12U, 25U, 30U, 150U);        // ring piece from 30 to 150 degree, inner radius 0 is a pie

// triangles
_dc.set_color(vic::color::magenta);
_dc.triangle({ 10, 195 }, { 40, 165 }, { 70, 195 });
_dc.triangle_solid({ 80, 195 }, { 110, 165 }, { 140, 195 });

// wide pen: line, polyline, triangle, rectangle and circle
_dc.set_color(vic::color::yellow);
_dc.set_pen_width(3U);
_dc.line({ 160, 195 }, { 220, 165 });
_dc.rectangle({ 230, 165, 270, 195 }, 5U);
_dc.set_pen_width(1U);

_dc.present_lock(false);                               // last unlock calls present()
```

Key points:

- Coordinates are `vertex_type { x, y }`, rects are `rect_type { left, top, right, bottom }` with exclusive right and
  bottom.
- `set_color()` selects the pen color for all following primitives, `pixel_set(v, color)` takes its own color.
- `present_lock()` / `present_lock(false)` are nested. Every primitive presents the head by itself; inside a lock the
  head is presented once by the last unlock, which matters on heads with a framebuffer.
- The pen width affects line, polyline, triangle, rectangle and circle. A wide rectangle grows inwards, a wide circle
  is the ring around its radius.
- `anti_aliasing_enable()` smooths sloped lines, circles, discs, sectors, solid triangles and rounded corners, see the
  [anti aliasing example](/vic/examples/anti-aliasing/).

## Flood fill

`dc::fill()` fills a region with the pen color. It works on heads which support `pixel_get()`, the region is drawn as
horizontal spans with `line_horz()`.

```cpp
void fill(vertex_type start, color::value_type color, bool use_color_as_bounding = true);
```

- **Bounded mode** (`use_color_as_bounding = true`): fill from `start` up to pixels of `color`, the outline.
- **Background mode** (`false`): replace all pixels of `color` which are connected to `start`.

`demo_fill.cpp` draws on a RGB565 [framebuffer](/vic/heads/framebuffer/) in front of the head and fills the regions of
a Venn diagram and a square spiral:

```cpp
#include "dc.h"
#include "head/framebuffer.h"

const vic::color::value_type outline = vic::color::argb(0xF0U, 0xF0U, 0xF0U);

// all drawing goes to a RGB565 framebuffer, present() transfers the changed rect to the head
static vic::head::framebuffer<320, 200, 1U, vic::color::format_RGB565> fb(_head);
vic::dc _dc(fb);
_dc.cls(vic::color::argb(0x1DU, 0x2BU, 0x53U));

// outlines: three overlapping circles
_dc.set_color(outline);
_dc.circle({  56,  76 }, 34U);
_dc.circle({ 104,  76 }, 34U);
_dc.circle({  80, 118 }, 34U);

// bounded mode: fill the region "A and B" up to the outline color
_dc.set_color(vic::color::argb(0xE6U, 0x39U, 0x46U));
_dc.fill({ 80, 62 }, outline, true);

// background mode: replace the background color around the shapes
_dc.set_color(vic::color::argb(0x2BU, 0x1DU, 0x3FU));
_dc.fill({ 2, 18 }, vic::color::argb(0x1DU, 0x2BU, 0x53U), false);
```

Key points:

- The colors are compared as given and as stored by the head, e.g. RGB565, which is probed at the start pixel. So a
  fill works on heads which store fewer color bits.
- `line_horz()` is virtual. `demo_fill.cpp` derives a `slowmo_dc` which overrides it to count the spans and to present
  the head every few spans, so the fill can be watched while it works.

See [Drawing context](/vic/reference/dc/) for all functions.
