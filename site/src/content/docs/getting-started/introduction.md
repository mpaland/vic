---
title: Introduction
description: What vic is, what it is made for and what it is not.
sidebar:
  order: 1
---

**vic** (Video Interface Controller) is a universal, lightweight and fast C++ library for driving graphic, dot
matrix, alpha numeric and seven-segment displays in embedded systems. It is header only, written in plain C++11 and
uses no heap, no STL and no floating point.

![The vic sprite demo](/vic/media/demo_sprite.gif)

## Highlights

- Designed for displays on **(slow) serial buses** like SPI, I²C or UART: primitives are rendered with a minimum of
  commands sent to the display, no framebuffer transfer is needed (but supported).
- Fast primitive rendering of lines, circles, triangles, boxes, text and more, with integer math only.
- [Sprite engine](/vic/reference/sprite/) (sheet and dynamic canvas) for moving objects with rotation, zoom and
  collision detection.
- Vertex / pixel [shader pipe](/vic/reference/shader/) with clipping, gradient, brush, zoom, rotate and more.
- All color formats from monochrome up to 32 bit alpha blending color displays, full transparency and alpha channel
  blending.
- [Anti aliasing](/vic/design/anti-aliasing/) for smooth edges of lines, circles, discs, sectors and triangles.
- Header only, no module compilation, no dependencies, no `new` / `delete`, no `float`.
- Multi display support, and a [multihead](/vic/heads/multihead/) driver that combines any number of (different)
  displays into one big screen.
- Platform independent [head drivers](/vic/heads/) with an I/O abstraction: the very same display driver runs
  everywhere.
- [Widgets](/vic/reference/widgets/) like bitmaps (raw and XPM), gauges, labels, checkboxes, progress bars, sliders.
- Bitmap and scaling [vector fonts](/vic/reference/fonts/), proportional and monospaced, ASCII / UTF-8.
- Framebuffer, viewport and blitter support.
- Clean C++, lint and `/W4` warning free, unit tested with MSVC, GCC, Clang and sanitizers on every commit.
- MIT license.

## What vic is not

vic is not a fully loaded window manager with dialogs and a theme engine. If you need an advanced windowed GUI, there
are libraries like LVGL, µC/GUI or StWin for that. Such a library can use vic as its rendering and display driver
layer: a GUI framework's flush callback is exactly a `blit()`, so a vic head can serve as its display driver.

**vic or a GUI framework?** vic is a graphic and widget rendering and display driver layer. If your display is a TFT,
your controller has RAM for a framebuffer and you need advanced controls and themes, a GUI framework is the right
tool. vic is made for the other cases:

- write only or serial displays,
- controllers with their own GRAM and native fill commands,
- monochrome and odd devices (OLEDs, LED matrices, VFD text displays, seven segment),
- heterogeneous multihead setups,
- MCUs with a few KB of RAM.

## Next steps

- [Installation](/vic/getting-started/installation/): add vic to your CMake or PlatformIO project.
- [Your first program](/vic/getting-started/first-program/): a head, a drawing context and the first pixels.
- [Architecture](/vic/design/architecture/): how vic renders with as few display commands as possible.
