---
title: framebuffer
description: RAM framebuffer head in front of any other head, with one or more planes, selectable storage format and dirty rect transfer on present().
sidebar:
  order: 50
---

The `framebuffer` head is a RAM framebuffer (with one or more planes) in front of any other head, the bound head. All
drawing goes into the buffer, `present()` transfers only the changed area (the dirty rect) with `blit()` to the bound
head. The device heads keep no buffers themselves, so the framebuffer is your decision per display. Use it

- in front of a display which can't read its pixels back (`pixel_get()` returns `color::none`), to get alpha
  blending, anti aliasing on images and gradients, sprites and `fill()`, see
  [Write only displays](/vic/design/write-only-displays/),
- in front of a display with an auto incrementing GRAM window (MIPI DCS TFTs, SSD1331, SSD1351, ILI9325) for the
  "render in RAM, send the changed window" model,
- with two planes for double buffering.

```cpp
#include "head/framebuffer.h"
```

## Template parameters and constructor

```cpp
template<std::uint16_t Screen_Size_X, std::uint16_t Screen_Size_Y,
         std::size_t Plane_Count = 1U,
         color::format_type Format = color::format_ARGB8888>
class framebuffer;

framebuffer(drv& head);
```

| Parameter | Type | Default | Meaning |
|---|---|---|---|
| `Screen_Size_X` | `std::uint16_t` | | buffer width, must match the bound head |
| `Screen_Size_Y` | `std::uint16_t` | | buffer height, must match the bound head |
| `Plane_Count` | `std::size_t` | 1 | number of planes, drawing and display plane can differ (double buffering) |
| `Format` | `color::format_type` | `format_ARGB8888` | storage format of the buffer |
| `head` | `drv&` | | the bound head |

| `Format` | RAM per pixel | 240 x 320 | 128 x 64 | Notes |
|---|---|---|---|---|
| `color::format_ARGB8888` | 4 byte | 300 KB | 32 KB | lossless, keeps the alpha channel |
| `color::format_RGB565` | 2 byte | 150 KB | 16 KB | matches the TFT and RGB OLED heads |
| `color::format_RGB332` | 1 byte | 75 KB | 8 KB | |
| `color::format_L1` | 1 bit | 9.4 KB | 1 KB | monochrome, any non black color is on |

Other formats are rejected with a `static_assert`. `plane_size()` returns the bytes of one plane.

## How it works

- `pixel_set()`, `line_horz()`, `line_vert()`, `box()`, `move()` and `blit()` write the active (access) plane and
  extend its dirty rect. `pixel_get()` reads the active plane, so read back always works.
- `present()` transfers the dirty rect of the displayed plane to the bound head, row by row in chunks of up to 64
  pixels with `head.blit()`, clears the dirty rect and calls `head.present()`. A head with a GRAM window streams
  every chunk with one address setup.
- `cls()` clears the active plane; if it is the displayed plane, the bound head clears itself with its own (fast)
  `cls()` and nothing has to be transferred.
- `init()`, `shutdown()`, `is_graphic()` and the display controls (`display_enable()`, `display_brightness()`,
  `display_contrast()`) are forwarded to the bound head. `init()` of the framebuffer initializes the display.

Since `present()` is called after every primitive, collect a frame with `dc.present_lock()` to get one transfer of
the combined dirty rect.

## Planes

| Function | Meaning |
|---|---|
| `framebuffer_set_access(plane)` | use the plane as drawing (read / write) buffer, `false` for an invalid plane |
| `framebuffer_set_display(plane, alpha)` | show the plane: if it changes, the whole plane is transferred to the head; `alpha` is unused |
| `framebuffer_get_count()` | number of planes |
| `framebuffer_get_access()`, `framebuffer_get_display()` | index of the drawing / displayed plane |
| `dirty()` | dirty rect of the active plane, empty if nothing changed |
| `plane_size()` | static, bytes per plane |

Double buffering: draw into plane 1 while plane 0 is shown, then `framebuffer_set_display(1)` and
`framebuffer_set_access(0)`.

## Example

From `demo/heads/framebuffer.cpp`: the SSD1331 can't read its pixels back on SPI; with the framebuffer in front the
drawing context can read pixels (anti aliasing, fill) and the display gets the dirty rect on `present()`.

```cpp
#include "head/SSD1331.h"
#include "head/framebuffer.h"
#include "scene.h"          // demo_scene(): frame, line, circle, disc and text

static vic::head::SSD1331<96, 64, vic::drv::orientation_180>  oled(reinterpret_cast<vic::io::handle_type>(0));
static vic::head::framebuffer<96, 64>                          fb(oled);

int main()
{
  fb.init();                                // also inits the display
  demo_scene(fb);
  vic::dc d(fb);                            // draw on the framebuffer, not on oled
  d.anti_aliasing_enable();                 // needs pixel read back, provided by the framebuffer
  d.set_color(vic::color::green);
  d.line({ 0, 63 }, { 95, 0 });
  return 0;
}
```

A 240 x 320 TFT with an RGB565 copy:

```cpp
vic::head::MIPI_DCS<vic::head::MIPI_DCS_ST7789, 240, 320> lcd(spi_handle);   // the write only display
vic::head::framebuffer<240, 320, 1, vic::color::format_RGB565> fb(lcd);       // 150 KB RAM copy
vic::dc dc(fb);
fb.init();
```

:::note
The drawing context must be created on the framebuffer, not on the bound head. Drawing on the bound head directly
bypasses the buffer and is overwritten by the next transfer of that area.
:::
