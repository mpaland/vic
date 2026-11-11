---
title: memory
description: Pure RAM head with full ARGB pixels and call statistics, for unit tests and off screen rendering.
sidebar:
  order: 51
---

The `memory` head stores every pixel as a full 32 bit ARGB value (the alpha channel is kept) and counts the
`present()`, `cls()`, `pixel_set()` and `blit()` calls. It needs no I/O layer. Use it

- as reference head in unit tests: the vic tests render on the memory head and compare the result with ASCII art
  rasters,
- as off screen rendering buffer,
- to check how many pixels a primitive really writes.

It deliberately implements only `pixel_set()`, `pixel_get()` and `blit()`, so the `drv` fallbacks (`line_horz()`,
`line_vert()`, `box()`, `move()`) are exercised.

```cpp
#include "head/memory.h"
```

## Template parameters and constructor

```cpp
template<std::uint16_t Screen_Size_X, std::uint16_t Screen_Size_Y,
         std::uint16_t Viewport_Size_X = Screen_Size_X, std::uint16_t Viewport_Size_Y = Screen_Size_Y>
class memory;

memory(std::int16_t viewport_x = 0, std::int16_t viewport_y = 0);
```

| Parameter | Type | Default | Meaning |
|---|---|---|---|
| `Screen_Size_X` | `std::uint16_t` | | screen (buffer) width |
| `Screen_Size_Y` | `std::uint16_t` | | screen (buffer) height |
| `Viewport_Size_X` | `std::uint16_t` | `Screen_Size_X` | viewport width |
| `Viewport_Size_Y` | `std::uint16_t` | `Screen_Size_Y` | viewport height |
| `viewport_x` | `std::int16_t` | 0 | viewport x position (left) on the screen |
| `viewport_y` | `std::int16_t` | 0 | viewport y position (top) on the screen |

The buffer is `Screen_Size_X * Screen_Size_Y * 4` bytes, initialized to `color::none`. `init()` clears it,
`shutdown()` does nothing.

## Inspection interface

All functions are public and const (except `reset_counters()`), they don't modify any counter.

| Function | Returns |
|---|---|
| `buffer()` | pointer to the raw ARGB buffer, row major |
| `pixel(vertex)` | color of a pixel, `color::none` outside the screen |
| `present_count()` | number of `present()` calls |
| `cls_count()` | number of `cls()` calls |
| `pixel_set_count()` | number of accepted (inside the screen) `pixel_set()` calls |
| `oob_count()` | number of rejected (outside the screen) `pixel_set()` calls |
| `blit_count()` | number of `blit()` calls |
| `blit_pixels()` | number of pixels written by `blit()` (inside the screen) |
| `cls_color()` | background color of the last `cls()` |
| `reset_counters()` | resets all counters |

## Example

From `demo/heads/memory.cpp`, the scene is rendered into RAM and printed as ASCII art:

```cpp
#include <cstdio>
#include "head/memory.h"
#include "scene.h"          // demo_scene(): frame, line, circle, disc and text

static vic::head::memory<48, 16> mem;

int main()
{
  mem.init();
  demo_scene(mem);
  for (std::int16_t y = 0; y < 16; ++y) {
    for (std::int16_t x = 0; x < 48; ++x) {
      std::putchar(mem.pixel({ x, y }) != vic::color::none ? '#' : '.');
    }
    std::putchar('\n');
  }
  return 0;
}
```

A unit test checks the pixels and the statistics, condensed from `test/test_dc.cpp`:

```cpp
static const vic::color::value_type block[6] = { vic::color::red, vic::color::green, vic::color::blue,
                                                 vic::color::white, vic::color::red, vic::color::green };
vic::head::memory<8, 6> h;
vic::dc dc(h);
dc.cls(vic::color::black);

// opaque block, no shader: one native blit
dc.blit({ 2, 2, 5, 4 }, block, 3U);
// h.blit_count() == 1, h.pixel_set_count() == 0, h.present_count() == 1
// h.pixel({ 2, 2 }) == vic::color::red, h.pixel({ 4, 3 }) == vic::color::green
```
