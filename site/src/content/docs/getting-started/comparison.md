---
title: Comparison
description: vic compared to LVGL, LovyanGFX, TFT_eSPI, Adafruit GFX and U8g2.
sidebar:
  order: 4
---

How vic compares to other popular embedded display libraries:

| | vic | LVGL | LovyanGFX | TFT_eSPI | Adafruit GFX | U8g2 |
|---|---|---|---|---|---|---|
| Kind | Rendering, display driver and widget layer | GUI framework | Graphics and sprite library | TFT graphics and sprite library | Graphics core, one driver library per display | Monochrome graphics library |
| Language | C++11 | C99 | C++ | C++ (Arduino) | C++ (Arduino) | C, C++ wrapper for Arduino |
| License | MIT | MIT | BSD | BSD, MIT parts | BSD | BSD, each font with its own license |
| Header only | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Dependencies | none | none | Arduino, ESP-IDF or Pico SDK | Arduino core | Arduino core, Adafruit BusIO | none |
| Heap | none | required (own pool or malloc) | for sprites | for sprites | for canvases | none |
| 8 bit MCUs | ✅ | ❌ (16 bit and up) | ❌ | ❌ (targets 32 bit) | ✅ | ✅ |
| Runs without tick source and periodic handler | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ |
| Draws without a framebuffer | ✅ (native display commands) | ❌ (draw buffer required) | ✅ | ✅ | ✅ (driver dependent) | page or full buffer, none in U8x8 text mode |
| Display drivers | included, platform independent | flush callback, some MIPI drivers included | included | included, one per build | separate library per display | included |
| Different displays in one system | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ |
| [Multihead](/vic/heads/multihead/) (several displays as one screen) | ✅ | manual, in the flush callback | ❌ | ❌ | ❌ | ❌ |
| Color depth | 1 bit up to ARGB8888 | 1 bit up to ARGB8888 | 1 bit up to ARGB8888 | RGB565 | 1, 8 and 16 bit | 1 bit |
| Alpha blending | ✅ | ✅ | ✅ | ✅ | ❌ | - |
| Character LCDs, VFDs, seven segment displays supported | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ |
| [Anti aliased](/vic/design/anti-aliasing/) primitives | ✅ | ✅ | ✅ | ✅ | ❌ | - |
| Arcs and sectors | ✅ | ✅ | ✅ | ✅ | ❌ | arcs |
| Gradients | ✅ | ✅ | ✅ | ✅ | ❌ | - |
| [Shader pipe](/vic/reference/shader/) (clipping, brush, zoom, free rotation) | ✅ | layers and transforms | clip rect, affine image transforms | viewport, sprite rotation | ❌ | clip window |
| [Sprites](/vic/reference/sprite/) | engine with rotation, zoom, z-order, collision detection, background restore | ❌ (animated images) | off screen buffers with rotation and zoom | off screen buffers with rotation | off screen canvases | ❌ |
| Anti aliased fonts | ✅ | ✅ | ✅ | ✅ | ❌ | - |
| Scalable [vector fonts](/vic/reference/fonts/) | ✅ | optional (Tiny TTF, FreeType) | ❌ | ❌ | ❌ | ❌ |
| UTF-8 | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ |
| [Widgets](/vic/reference/widgets/) | 19 (gauges, sliders, buttons ...) | 30+ with layouts, styles, themes | one button | one button | one button | menus and dialogs |
| Input | touch event routing | touch, keypad, encoder | touch drivers | touch driver (XPT2046) | ❌ | buttons, encoder |
| PC simulator | Windows, X11 | SDL, X11, Wayland, Windows | SDL, OpenCV, Linux framebuffer | ❌ | ❌ | SDL |
| Unit tests on every commit | ✅ | ✅ | build checks | ❌ | one host test | build tests |

Status November 2026: LVGL 9.6, LovyanGFX 1.3, TFT_eSPI 2.5.43, Adafruit GFX 1.13, U8g2 2.36.

vic is a rendering and display driver layer, not a GUI framework: see
[What vic is not](/vic/getting-started/introduction/#what-vic-is-not).
