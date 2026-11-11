---
title: Write only displays
description: Alpha blending and anti aliasing on displays that can't read their pixels back, and when to use a framebuffer head.
sidebar:
  order: 3
---

Alpha blending and anti aliasing need the pixel below the new one. Most serial displays can't read their GRAM back:
their head returns `color::none` from `pixel_get()`. vic has two answers for that.

## A background color, no RAM

The drawing context blends alpha and anti aliasing pixels against its background color once one is set with
`dc::set_bg_color()`. This is exact on uniform backgrounds and needs no RAM at all:

```cpp
vic::dc dc(lcd);
dc.set_bg_color(vic::color::black);   // the screen background
dc.anti_aliasing_enable();
dc.set_color(vic::color::blue);
dc.disc({ 120, 160 }, 50);            // edge pixels blended against black
```

## A framebuffer head in front of the display

If the real pixels are needed (sprites, flood fill, anti aliasing on images and gradients, translucent boxes), put
the [`framebuffer`](/vic/heads/framebuffer/) head between the display driver and the drawing context. It keeps a RAM
copy in the format you choose (ARGB8888, RGB565, RGB332 or 1 bit L1), remembers the dirty rect and transfers only the
changed window with `blit()` on `present()`.

This is also the "render in RAM, stream the changed window" model modern TFT controllers with an auto incrementing
GRAM window are made for. It is your decision per display, the device drivers themselves keep no buffers.

A 240 x 320 ST7789 over SPI can't read its GRAM back, so sprites and anti aliasing on a gradient won't blend. A
`framebuffer` head in front of the display driver fixes that, the drawing code stays the same:

```cpp
vic::head::MIPI_DCS<vic::head::MIPI_DCS_ST7789, 240, 320> lcd(spi_handle);   // the write only display
vic::head::framebuffer<240, 320, 1, vic::color::format_RGB565> fb(lcd);       // 150 KB RAM copy, blits the dirty rect
vic::dc dc(fb);                                                               // draw into the framebuffer, not into lcd
fb.init();                                                                    // initializes the lcd as well

dc.anti_aliasing_enable();
dc.alpha_blending_enable();
dc.present_lock();                                   // collect a whole frame
dc.set_color(vic::color::blue);
dc.disc({ 120, 160 }, 50);                           // edge pixels blended against the RAM copy
dc.set_color(vic::color::argb(0xFF, 0xFF, 0xFF, 0x80));   // white, alpha 128
dc.box({ 20, 20, 220, 60 });                         // half transparent box over the disc
dc.present_lock(false);                              // transfers the dirty rect (one GRAM window)
```

Without the `framebuffer` head and with a uniform background, `dc.set_bg_color(vic::color::black)` gives the same
anti aliasing result for the disc without any RAM, only the transparent box can't be blended then.

## Other options

- Heads that keep their own framebuffer anyway (like the SSD1306 head) can read back.
- [Canvas sprites](/vic/reference/sprite/) keep their own pixels and blend into their pattern.
- The [`memory`](/vic/heads/memory/) head is a pure RAM head for off screen rendering.
