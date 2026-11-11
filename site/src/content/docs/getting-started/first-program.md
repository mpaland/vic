---
title: Your first program
description: Create a head, put a drawing and a text context on it and draw the first primitives.
sidebar:
  order: 3
---

Every vic program has the same three parts: a **head** (the display driver), a **drawing context** (`dc`) or
**text context** (`tc`) on that head, and the drawing calls.

## 1. Create the head

A head is the driver of one display with its controller. On the desktop the `windows` and `x11` heads emulate a
display in a window, which is the fastest way to develop:

```cpp
#include "head/Windows.h"

// screen of 240 x 120 pixel with a viewport of the same size,
// window at 100,100 on the desktop, zoomed 4x in both directions
vic::head::windows<240, 120, 240, 120> head(0U, 0U, 100U, 100U, 4U, 4U);
head.init();
```

On Linux the `x11` head takes the same parameters and only needs Xlib (link with `-lX11`):

```cpp
#include "head/X11.h"

vic::head::x11<240, 120, 240, 120> head(0U, 0U, 100U, 100U, 4U, 4U);
head.init();
```

On the target the head is the real display, its constructor gets the I/O handle of the bus, for example an ST7789
TFT on SPI:

```cpp
#include "head/MIPI_DCS.h"

vic::head::MIPI_DCS<vic::head::MIPI_DCS_ST7789, 240, 320> head(spi_handle);
head.init();
```

See [Heads](/vic/heads/) for all drivers and their parameters.

## 2. Draw

```cpp
#include "dc.h"

vic::dc dc(head);
dc.set_color(vic::color::blue);          // pen color
dc.line({ 10, 10 }, { 40, 40 });         // both end points included
dc.box({ 50, 10, 90, 30 }, 4U);          // filled box with rounded corners, right/bottom exclusive
dc.disc({ 120, 60 }, 20U);               // filled circle

dc.anti_aliasing_enable();               // smooth edges from now on
dc.set_color(vic::color::orange);
dc.sector({ 180, 60 }, 20U, 30U, 30U, 150U);   // ring segment from 30° to 150°
```

Coordinates are `vertex_type { x, y }`, rects are `rect_type { left, top, right, bottom }` with exclusive
right / bottom. Angles are degrees, 0° is at 3 o'clock and they run counter clockwise.

## 3. Write text

```cpp
#include "tc.h"
#include "font/Montserrat_vec.h"

vic::tc tc(dc);                          // text on a graphic head through the dc
vic::font::Montserrat_vec font;
tc.set_font(font, 20U);                  // vector font, 20 px line height
tc.set_color(vic::color::yellow);
tc.out({ 10, 80 }, (const std::uint8_t*)"Hello vic");
```

Text contexts work on alpha numeric heads (HD44780, BA6x, ...) too, then positions are character cells:
`vic::tc tc(head);`.

## 4. Present

Heads that buffer (the `framebuffer` head, e-paper, the emulation heads) transfer their content with `present()`.
Drawing functions present automatically. To collect a whole frame and transfer it once, lock the presentation:

```cpp
dc.present_lock();                       // nested lock
// ... draw a frame ...
dc.present_lock(false);                  // the last unlock calls present()
```

## Platform I/O

On the target the head talks to the bus through the functions declared in `src/io.h`, which your platform provides
once (`io::init`, `io::write`, `io::read`, `io::delay`). The desktop heads only need `io::delay`. See
[Write your own head](/vic/heads/skeleton/#platform-io).

## Next

- [Command reference](/vic/reference/) for every function of `dc`, `tc`, sprites, shaders and widgets.
- [Examples](/vic/examples/) walk through the demos.
- [Architecture](/vic/design/architecture/) explains how primitives reach the display.
