---
title: Examples
description: How the vic demos are built and run, and an overview of all examples.
sidebar:
  label: Overview
  order: 0
---

The examples are condensed from the demo programs in `demo/`. Every demo is a single `.cpp` file which implements
`demo()`, the head and `main()` come from a small platform file, so the very same demo code runs on Windows and on
Linux.

## Building the demos

The demos are part of the CMake project and are built if vic is the top level project:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVIC_BUILD_DEMOS=ON
cmake --build build
```

`VIC_BUILD_DEMOS` is on by default on Windows. On Linux the demos need the Xlib headers (`sudo apt install libx11-dev`),
without X11 they are skipped. Every demo becomes a target `vic_<demo>`, e.g. `vic_demo_gauges`, built as C++14 with
`/W4` (MSVC) resp. `-Wall -Wextra`. See [Installation](/vic/getting-started/installation/) for the tests.

## How a demo runs

`demo/windows/main.cpp` (linked on Windows, with the [Windows head](/vic/heads/windows/)) and `demo/linux/main.cpp`
(linked on Linux, with the [X11 head](/vic/heads/x11/)) provide three things:

```cpp
// a 320 x 200 pixel window with zoom factor 2
vic::drv& get_singlehead()
{
  static vic::head::windows<320, 200> _head(0, 0, 0, 0, 2U, 2U);   // vic::head::x11<320, 200> on Linux
  return _head;
}

// the platform delay used by the demos
void vic::io::delay(std::uint32_t time_to_delay)
{
  ::Sleep(time_to_delay);                                          // nanosleep() on Linux
}

extern void demo(void);

int main()
{
  demo();                                                          // most demos loop forever
  vic::io::delay(100000U);
  return 0;
}
```

A demo gets the head with `get_singlehead()`, calls `init()` on it and draws. Animations are paced with
`vic::io::delay()`, typically 40 ms per frame (25 frames per second):

```cpp
#include "dc.h"

extern vic::drv& get_singlehead(void);

void demo()
{
  vic::drv& head = get_singlehead();
  head.init();
  vic::dc _dc(head);
  _dc.cls(vic::color::black);
  for (std::uint32_t t = 0U; ; ++t) {
    // draw frame t
    vic::io::delay(40U);
  }
}
```

The Windows `main.cpp` contains a multihead variant too, see [Multiple heads](/vic/examples/multihead/).

## The demos

| Demo | Shows |
|---|---|
| `demo_fill.cpp` | `dc::fill()` in slow motion on a RGB565 framebuffer, bounded and background mode |
| `demo_font.cpp` | All bitmap fonts with a pangram, digits, Latin-1 and color escapes |
| `demo_vecfont.cpp` | The vector fonts in every size, a word zooming from 8 pixel to full width |
| `demo_sprite.cpp` | Sheet and canvas sprites, rotation, z-order, alpha blending, collision detection |
| `demo_gauges.cpp` | An aircraft engine page with round gauges, bar gauges, labels and group boxes |
| `demo_input.cpp` | A settings page with input widgets and a compass, operated by a simulated touch |
| `demo_aa.cpp` | The same scene without and with anti aliasing side by side, with a magnifier |
| `demo_clock.cpp` | An analog clock with gradient face and sprite hands drawn with a brush |
| `demo_widget.cpp` | Every widget with its features on three alternating pages |

`demo/heads/` holds one small portable program per head (`vic_head_<name>`) which draws a test scene and dumps the
command / data stream of the head to stdout, see [Heads](/vic/heads/).

## Examples

- [Drawing primitives](/vic/examples/drawing/): lines, boxes, circles, sectors, triangles, pen width and flood fill
- [Text and fonts](/vic/examples/text/): bitmap and vector fonts, color escapes, text measurement
- [Sprites](/vic/examples/sprites/): a night scene with sheet and canvas sprites
- [Gauges](/vic/examples/gauges/): an engine page built from gauge widgets
- [Input](/vic/examples/input/): a touch operated settings page and the event routing
- [Anti aliasing](/vic/examples/anti-aliasing/): split screen comparison with magnifier
- [Clock](/vic/examples/clock/): gradient face and rotating hands as canvas sprites
- [Multiple heads](/vic/examples/multihead/): independent heads and the multihead driver
- [Shaders](/vic/examples/shaders/): clipping, gradients, rotation and brushes
