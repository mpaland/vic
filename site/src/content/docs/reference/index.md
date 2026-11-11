---
title: Command reference
description: Conventions, class overview and headers of the vic public API.
sidebar:
  label: Overview
  order: 0
---

The command reference documents every public function of the vic classes, taken from the headers in `src/`.
This page lists the conventions all classes share, the classes themselves and the header each one needs.

```cpp
#include "head/Windows.h"   // a head (display driver)
#include "dc.h"             // drawing context (pulls in gpr.h, drv.h, color.h, util.h, shader/base.h)
#include "tc.h"             // text context (pulls in dc.h and txr.h)
#include "font/LCD_6x8.h"   // a font

vic::head::windows<240, 120, 240, 120> head(0U, 0U, 100U, 100U, 4U, 4U);
head.init();

vic::dc dc(head);
dc.set_color(vic::color::blue);
dc.box({ 10, 10, 50, 30 });             // pixels 10..49 x 10..29

vic::tc tc(dc);
vic::font::LCD_6x8 font;
tc.set_font(font);
tc.out({ 10, 40 }, (const std::uint8_t*)"Hello vic");
```

## Conventions

### Coordinates

All coordinates are `vic::vertex_type { x, y }` with `std::int16_t` members. (0, 0) is the top/left pixel of the
screen, x grows to the right, y grows downwards. Coordinates outside the screen are allowed in the primitive
calls, the drawing context clips the native line and box calls to the screen.

### Rectangles

A `vic::rect_type { left, top, right, bottom }` covers the pixels `left <= x < right` and `top <= y < bottom`:
**right and bottom are exclusive**. `{ 10, 10, 50, 30 }` is 40 x 20 pixels, `rect.width()` is `right - left`.
The functions which take two corner vertices instead (`box(v0, v1)`, `rectangle(v0, v1)`) include both vertices.
See [`rect_type`](/vic/reference/color-util/#rect_type) for the helpers.

### Colors

All colors are `vic::color::value_type`, a 32 bit ARGB8888 value `0xAARRGGBB`. Alpha 255 is opaque, alpha 0 is
transparent. `color::none` (`0x00000000`) means "no color", e.g. a pixel a head can't read back. Heads with fewer
colors reduce the ARGB value themselves. See [Colors and helpers](/vic/reference/color-util/).

### Angles

Angles are given in degree. In `circle()` and `sector()` 0° is at 3 o'clock and the angles run **counter
clockwise** (90° is at 12 o'clock). Angle intervals are half open `[start, end)`: the start ray belongs to the
shape, the end ray doesn't, so adjacent sectors tile without gaps or double drawn pixels.

Rotations (`util::vertex_rotate()`, the `rotate` shader, `sprite::canvas::set_rotation()`) are math positive in
screen coordinates, which is **clockwise** on the screen.

### Present

Every drawing function of the `dc` calls `present()` after the primitive is done, so the head can transfer its
buffer. `present_lock()` / `present_lock(false)` collect several primitives into one transfer. Functions which
don't present are marked in their description.

## Classes

| Class | Header | Purpose |
|---|---|---|
| [`vic::dc`](/vic/reference/dc/) | `dc.h` | Drawing context: pen color and width, anti aliasing, primitives, shader pipe. Derived from `vic::gpr` (`gpr.h`), the graphic primitive renderer. |
| [`vic::tc`](/vic/reference/tc/) | `tc.h` | Text context: renders UTF-8 strings with bitmap and vector fonts on a `dc`, or forwards text to an alpha numeric head. Derived from `vic::txr` (`txr.h`), the text renderer. |
| [`vic::drv`](/vic/reference/drv/) | `drv.h` | Driver base class of every head: screen, viewport, framebuffer planes, display control and the virtual hooks a head overrides. |
| [`vic::sprite`](/vic/reference/sprite/) | `sprite.h` | Sprite engine: `canvas`, `sheet` and `canvas_sheet` sprites with z-order, rotation and collision detection. |
| [`vic::shader`](/vic/reference/shader/) | `shader/base.h`, `shader/brush.h`, `shader/gradient.h`, `shader/rotate.h` | Pixel and vertex shaders of the shader pipe: clipping, zoom, offset, flip, tint, alpha blend, brush, gradients, rotate. |
| [`vic::widget`](/vic/reference/widgets/) | `widget/*.h` | Widgets: bitmap, progress, label, textbox, gauges, LED, input widgets and more. |
| [`vic::color`, `vic::util`](/vic/reference/color-util/) | `color.h`, `util.h` | Color formats, assembly, blending, conversion and named colors; vertex, pixel and rect types, integer math helpers. |
| [`vic::font`](/vic/reference/fonts/) | `font.h`, `font/*.h` | Font structures and the available bitmap and vector fonts, font generators. |
| [Configuration](/vic/reference/config/) | `vic_cfg.h` | Global compile time options. |

The specific display drivers are in the namespace `vic::head` (`head/*.h`), see [Heads](/vic/heads/).

## Which header to include

vic is header only, put `src/` on the include path (see [Installation](/vic/getting-started/installation/)) and
include what you use:

| You need | Include |
|---|---|
| A display | `head/<Head>.h`, e.g. `head/MIPI_DCS.h`, `head/framebuffer.h` |
| Drawing primitives | `dc.h` |
| Text | `tc.h` and one or more `font/<Font>.h` |
| Sprites | `sprite.h` |
| Clipping, zoom, offset, flip, tint, alpha blend shaders | `shader/base.h` (already included by `dc.h`) |
| Brush shader | `shader/brush.h` |
| Gradient shaders | `shader/gradient.h` |
| Rotate shader | `shader/rotate.h` |
| Widgets | `widget/<widget>.h` |
| Colors, vertex/rect types, math helpers | `color.h`, `util.h` (already included by `dc.h`) |

Your platform provides the I/O functions declared in `io.h` (bus access, delay), which every head uses.
