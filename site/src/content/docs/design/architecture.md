---
title: Architecture
description: How vic renders directly to the display with as few commands as possible, and how the classes fit together.
sidebar:
  order: 1
---

Most display libraries render into a RAM framebuffer and transfer the complete buffer to the display afterwards.
This works fine for memory mapped displays, but embedded displays are often connected via SPI, I²C or UART with a
few MBit/s or even kBit/s. Transferring a complete framebuffer over such a link takes far too long for fast updates,
and small microcontrollers don't have the RAM for a big framebuffer anyway.

vic therefore renders **directly to the display** and sends as few commands as possible:

- Every primitive is decomposed into the most powerful commands the display controller understands. If the
  controller can fill a box or draw a horizontal line natively, a disc is sent as a handful of line commands instead
  of thousands of pixels. A head simply overrides `line_horz`, `line_vert`, `box` or `move` to use such commands.
- Only what changes is drawn. Sprites restore just their background, the progress bar widget draws only the delta.
- Alpha blending and anti aliasing read the background pixel back from the display (or use a framebuffer head or a
  canvas sprite when the display is write only), everything else is write only traffic.
- If a display or an application really needs a framebuffer, the [`framebuffer`](/vic/heads/framebuffer/) and
  [`memory`](/vic/heads/memory/) heads provide one in RAM and the display is updated with `present()`.

## The rendering pipeline

```
your application
      │  dc / tc            drawing and text context: pen color, pen width, anti aliasing, font
      ▼
   gpr / txr              graphic primitive renderer, text renderer: integer math only
      │
      ▼
  shader pipe             clipping, zoom, offset, flip, rotate, tint, brush, gradient, alpha blend
      │
      ▼
     drv                  driver base: native hooks pixel_set, line_horz, line_vert, box, move, blit, cls
      │
      ▼
    head                  the display driver: SSD1306, ST7789, MAX7219, e-paper, framebuffer, multihead, ...
```

### gpr - Graphic Primitive Renderer

The `gpr` renders all basic primitives: lines, polylines, circles, discs, sectors, triangles, boxes, rounded boxes
and flood fill. It uses no floating point math, only fast integer operations, and should be a collection of the
fastest modern algorithms for each primitive. Everything the head can't do natively is decomposed here.

### txr - Text Renderer

The `txr` renders bitmap fonts (monospaced and proportional, 1, 2, 4 or 8 bit anti aliased) and scaling vector fonts
on graphic displays, and characters on alpha numeric displays. It decodes UTF-8 and handles the control chars `\n`,
`\r` and the inline color escape `\aRRGGBB`.

### dc - Drawing context

At least one drawing context is needed to draw anything. The `dc` stores the pen color, the pen width, the anti
aliasing setting and its own pipe of shaders. Several drawing contexts can share one head. See
[`dc` reference](/vic/reference/dc/).

### tc - Text context

The text context is the counterpart of the `dc` for text, on graphic heads through a `dc` and on alpha numeric heads
directly. See [`tc` reference](/vic/reference/tc/).

### Shaders

Shaders are registered at a drawing context and modify every rendered pixel (or its position) before it is sent to
the head: alpha blender, clipping, zoom, offset, flip, tint, brush, gradients (distance weighted, horizontal, vertical, solid) and rotate. See
[Shaders](/vic/reference/shader/).

### drv - Driver base

The base class of every head. All color calculations are done in ARGB 32 bit format, a head with fewer colors
reduces them itself with the conversion routines of `color`. See [`drv` reference](/vic/reference/drv/).

### head - Display driver

A display together with its controller is called a **head**, and `vic::head` is the namespace of all drivers. A head
only needs `pixel_set` and `pixel_get`. Most modern controllers can do more, like native lines, boxes or block moves:
then the head overrides the according virtual function of `drv` and every primitive above uses it automatically.
Native rendering on a specialized controller is always faster and is preferred. A head is platform independent: it
talks to the hardware only through the four functions of `io.h`. See [Heads](/vic/heads/).

## Native hooks

The native hooks are deliberately limited to primitives which have exactly one correct pixel result: `line_horz`,
`line_vert`, `box`, `move`, `cls` and `blit` (a block of pixels). Everything else (diagonal lines, circles, discs,
sectors, triangles, text) is decomposed by the `gpr` into these hooks, so **every head renders exactly the same
pixels** and the anti aliasing stays deterministic: the interior of an anti aliased disc is still sent as native
horizontal runs, only the edge pixels are single alpha pixels. There are no hooks for native diagonal lines or circles
on purpose, a controller's own algorithm would make the result head dependent.

## Sprites and widgets

On top of the drawing context sit the [sprite engine](/vic/reference/sprite/), moving objects that restore their
background, with z-order, rotation and pixel exact collision detection, and the [widgets](/vic/reference/widgets/):
gauges, labels, progress bars, bitmaps and input widgets, each redrawn only where it changes.

## Further reading

- [Conventions](/vic/design/conventions/): coordinates, rects, angles and pen width.
- [Write only displays](/vic/design/write-only-displays/): blending without reading pixels back.
- [Anti aliasing](/vic/design/anti-aliasing/): how smooth edges are made with integer math.
