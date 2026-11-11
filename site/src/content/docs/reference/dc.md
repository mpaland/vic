---
title: Drawing context
description: vic::dc, the drawing context with pen state, primitives, screen functions and the shader pipe.
sidebar:
  order: 1
---

`vic::dc` (`dc.h`) is the drawing context. It stores the pen color, the pen width and the anti aliasing setting,
owns a pipe of shaders and renders all primitives on the head it was created on. The primitives themselves come
from its base class `vic::gpr` (`gpr.h`), the graphic primitive renderer, which decomposes every shape into the
native hooks of the head (`line_horz`, `line_vert`, `box`, `blit`, `move`) and single pixels. Several drawing
contexts can share one head, each with its own color, pen and shaders.

```cpp
#include "dc.h"

vic::dc dc(head);
dc.cls(vic::color::black);
dc.anti_aliasing_enable();
dc.set_bg_color(vic::color::black);           // blend AA edges against black on a write only head

dc.present_lock();                            // collect the frame, one transfer at the end
dc.set_color(vic::color::orange);
dc.box({ 10, 10, 110, 50 }, 8U);              // rounded box, right/bottom exclusive
dc.set_pen_width(3U);
dc.set_color(vic::color::white);
dc.circle({ 160, 60 }, 30U);                  // 3 pixel wide ring
dc.sector({ 160, 60 }, 10U, 25U, 30U, 150U);  // ring piece from 30 to 150 degree
dc.present_lock(false);                       // unlock and present
```

## Construction

### `dc()`

```cpp
dc(drv& head);
```

Creates a drawing context on a head. The shader pipe is empty (only the output shader), the pen color is
`color::gray`, the pen width 1, anti aliasing is off and alpha blending is on.

| Parameter | Description |
|---|---|
| `head` | The head (display driver) to draw on |

## State

### `set_color()`

```cpp
void set_color(color::value_type color);
```

Sets the drawing (pen) color in ARGB format. Colors with an alpha below 255 are blended with the background, see
[`alpha_blending_enable()`](#alpha_blending_enable).

### `get_color()`

```cpp
color::value_type get_color() const;
```

Returns the actual drawing color in ARGB format.

### `set_pen_width()`

```cpp
void set_pen_width(std::uint16_t width);
```

Sets the pen width in pixel, 1 is default, 0 is taken as 1. Affected primitives: `line()`, `polyline()`,
`triangle()`, `rectangle()` and `circle()`:

- A sloped line is the rectangle around its segment with butt caps, an axis aligned line is a box (an even width
  has its extra pixel at the bottom / right). Every pixel is written once.
- Polylines and triangles get round joints. With a translucent pen the joints are blended twice.
- A wide rectangle grows inwards, its outer size stays the rect. The pen width of a rounded rectangle is limited
  to its border radius.
- A wide circle is the ring around its radius, like `sector()` with the radii `radius - (width - 1) / 2` and
  `radius + width / 2`.

`line_horz()`, `line_vert()`, the filled primitives and text are not affected.

### `get_pen_width()`

```cpp
std::uint16_t get_pen_width() const;
```

Returns the actual pen width in pixel.

### `anti_aliasing_enable()`

```cpp
void anti_aliasing_enable(bool enable = true);
```

Enables or disables anti aliasing, it is off by default. Anti aliased primitives render their edge pixels with a
coverage dependent alpha, so alpha blending must be available: the head reads its pixels back, a background color is
set with `set_bg_color()`, or the context is a canvas sprite. Affected primitives: `line()`, `polyline()`,
`triangle()`, `triangle_solid()`, `circle()`, `disc()`, `disc_sector()`, `sector()` and the corners of rounded
boxes and rectangles. Pixels, axis aligned lines, plain boxes and plain rectangles are not affected.
See [Anti aliasing](/vic/design/anti-aliasing/).

| Parameter | Description |
|---|---|
| `enable` | `true` to enable, `false` to disable anti aliasing |

### `alpha_blending_enable()`

```cpp
void alpha_blending_enable(bool enable = true);
```

Enables or disables alpha blending in the output shader, it is on by default. With blending enabled, a pixel which
is not opaque is blended with the background pixel, which is read back from the head (`pixel_get()`) or taken from
`set_bg_color()`. Opaque pixels never read the background. With blending disabled the color is passed to the head as
it is.

### `set_bg_color()`

```cpp
void set_bg_color(color::value_type bg_color);
```

Sets the background color for heads which can't read their pixels back. Such heads return `color::none` from
`pixel_get()`. With a background color set, alpha blending and anti aliasing blend against this color instead,
which is exact on uniform backgrounds and needs no RAM. Sprites use it to restore their background. Default is
`color::none`, which disables the fallback. See [Write only displays](/vic/design/write-only-displays/).

| Parameter | Description |
|---|---|
| `bg_color` | Background color, `color::none` disables the fallback |

### `get_bg_color()`

```cpp
color::value_type get_bg_color() const;
```

Returns the background color of the context.

## Pixels and lines

### `pixel_set()`

```cpp
void pixel_set(vertex_type vertex);
void pixel_set(vertex_type vertex, color::value_type color);
```

Sets a pixel in the pen color or in the given color. The pixel takes the shader pipe. `present()` is **not**
called, use it for bulk pixel output and call `present()` afterwards.

| Parameter | Description |
|---|---|
| `vertex` | Pixel to set |
| `color` | Color of the pixel in ARGB format |

### `pixel_get()`

```cpp
color::value_type pixel_get(vertex_type vertex);
```

Returns the color of a pixel in ARGB format, read through the shader pipe (vertex shaders like offset or zoom map
the position). Returns `color::none` if the head can't read its pixels back.

### `plot()`

```cpp
void plot(vertex_type vertex);
void plot(vertex_type vertex, color::value_type color);
```

Plots a point (one pixel) in the pen color or in the given color. This is `pixel_set()` with an immediate
`present()`.

### `line()`

```cpp
void line(vertex_type v0, vertex_type v1);
```

Draws a line with the actual pen, both end points are included. Horizontal and vertical lines are drawn with
`line_horz()` / `line_vert()` (native head commands if possible). Sloped lines use Bresenham with run length
emission, every run is one native `line_horz()` / `line_vert()` call. Affected by anti aliasing (Xiaolin Wu
algorithm for sloped lines) and by the pen width.

| Parameter | Description |
|---|---|
| `v0` | Start vertex, included in the line |
| `v1` | End vertex, included in the line |

:::note
Wide and anti aliased lines use 32 bit sub pixel math, keep them shorter than 4000 pixels.
:::

### `line_horz()`

```cpp
virtual void line_horz(vertex_type v0, vertex_type v1) override;
```

Draws a horizontal line, one pixel wide, both end points included. The y component of `v1` is ignored. Without
an active shader and with an opaque pen color the line is clipped to the screen and drawn with the native
`line_horz()` of the head, otherwise every pixel takes the shader pipe. Not affected by pen width or anti aliasing.
Presents.

### `line_vert()`

```cpp
virtual void line_vert(vertex_type v0, vertex_type v1) override;
```

Draws a vertical line, one pixel wide, both end points included. The x component of `v1` is ignored. Native head
command under the same conditions as `line_horz()`. Not affected by pen width or anti aliasing. Presents.

### `polyline()`

```cpp
void polyline(const vertex_type* vertices, std::size_t vertex_count);
```

Draws connected lines through all vertices. With a pen width > 1 the inner vertices get round joints. Nothing is
drawn with less than 2 vertices. Affected by anti aliasing and pen width. Presents once at the end.

| Parameter | Description |
|---|---|
| `vertices` | Pointer to an array of vertices |
| `vertex_count` | Number of vertices in the array, at least 2 |

## Shapes

### `box()`

```cpp
virtual void box(const rect_type& rect) override;
void box(const rect_type& rect, std::uint16_t border_radius);
void box(vertex_type v0, vertex_type v1);
void box(vertex_type v0, vertex_type v1, std::uint16_t border_radius);
```

Draws a box (filled rectangle). Left/top are inside, right/bottom are outside the rect. The vertex variants
normalize the two corners, both vertices are inside the box. Without an active shader and with an opaque pen color
the plain box is clipped to the screen and drawn with the native `box()` of the head.

With a border radius the corners are rounded. The radius is limited so that the four corner discs fit into the
rect, 0 draws a plain box. The rounded box is decomposed into three bands and four quarter discs without overlap,
so every pixel is written exactly once (translucent colors work). The rounded corners are affected by anti
aliasing, the plain box is not. Presents.

| Parameter | Description |
|---|---|
| `rect` | Box bounding, right/bottom exclusive |
| `v0`, `v1` | Two corner vertices, both included |
| `border_radius` | Radius of the corners, 0 for angular |

### `rectangle()`

```cpp
void rectangle(const rect_type& rect);
void rectangle(vertex_type v0, vertex_type v1);
void rectangle(const rect_type& rect, std::uint16_t border_radius);
void rectangle(vertex_type v0, vertex_type v1, std::uint16_t border_radius);
```

Draws a rectangle (frame) with the actual pen. Every frame pixel is written exactly once. An empty rect draws
nothing. With a pen width > 1 the frame grows inwards (four boxes), a rect which is not wider and higher than two
pen widths becomes a filled box.

With a border radius the corners are quarter circles, the radius is limited like in `box()` and 0 draws a plain
rectangle. The pen width of a rounded rectangle is limited to its border radius. The rounded corners are affected
by anti aliasing. Presents.

| Parameter | Description |
|---|---|
| `rect` | Rectangle, right/bottom exclusive |
| `v0`, `v1` | Two corner vertices, both included |
| `border_radius` | Radius of the corners, 0 for angular |

### `triangle()`

```cpp
void triangle(vertex_type v0, vertex_type v1, vertex_type v2);
```

Draws the outline of a triangle with the actual pen, the three vertices are included. With a pen width > 1 the
corners get round joints. Affected by anti aliasing and pen width. Presents.

### `triangle_solid()`

```cpp
void triangle_solid(vertex_type v0, vertex_type v1, vertex_type v2);
```

Draws a solid (filled) triangle, the vertices may be given in any order and are included. Every row is one
`line_horz()` call. A degenerated triangle (all vertices on one line) is drawn as lines. Affected by anti aliasing
(coverage sampling of the edges), not by the pen width. Presents.

### `circle()`

```cpp
void circle(vertex_type center, std::uint16_t radius, std::uint16_t start_angle = 0U, std::uint16_t end_angle = 360U);
```

Draws a circle or a circle piece (arc) with the actual pen. The arc runs counter clockwise from `start_angle` to
`end_angle`, 0° is at 3 o'clock, the interval is half open. `start_angle` is reduced to 0 - 359, `end_angle` is
limited to 360. Nothing is drawn if `start_angle >= end_angle`, so an arc over 0° needs two calls. Affected by anti
aliasing (a one pixel wide ring from `r - 0.5` to `r + 0.5`) and pen width (a ring sector). Presents.

| Parameter | Description |
|---|---|
| `center` | Center vertex |
| `radius` | Circle radius in pixel |
| `start_angle` | Start angle in degree, included |
| `end_angle` | End angle in degree, excluded |

### `disc()`

```cpp
void disc(vertex_type center, std::uint16_t radius);
```

Draws a disc (filled circle). The disc uses the same midpoint algorithm as `circle()`, so disc and circle have
identical outlines. Every row is one `line_horz()` call. Affected by anti aliasing (a circle with radius
`r + 0.5`). Presents.

### `disc_sector()`

```cpp
void disc_sector(vertex_type center, std::uint16_t radius, std::uint8_t quadrant);
```

Draws a filled quarter circle. The center row and column belong to every quadrant. Quadrants > 3 are ignored.
Affected by anti aliasing. Presents.

| Parameter | Description |
|---|---|
| `center` | Center vertex |
| `radius` | Disc radius |
| `quadrant` | 0: top/right, 1: top/left, 2: bottom/left, 3: bottom/right |

### `sector()`

```cpp
void sector(vertex_type center, std::uint16_t inner_radius, std::uint16_t outer_radius, std::uint16_t start_angle, std::uint16_t end_angle);
```

Draws a sector: a pie (`inner_radius` 0) or a ring segment. The inner radius is inside, the outer radius is
outside the sector. The angle interval `[start_angle, end_angle)` is half open, so adjacent sectors tile without
gaps or double drawn pixels. An `end_angle <= start_angle` wraps around 0°, the span is limited to 360°. Nothing
is drawn if `inner_radius >= outer_radius` or the span is 0. Affected by anti aliasing (the ring from
`inner - 0.5` to `outer + 0.5`, so an anti aliased pie equals a disc). Presents.

| Parameter | Description |
|---|---|
| `center` | Center vertex |
| `inner_radius` | Inner radius, 0 for a pie |
| `outer_radius` | Outer radius |
| `start_angle` | Start angle in degree, 0 is at 3 o'clock, counter clockwise, included |
| `end_angle` | End angle in degree, excluded |

### `fill()`

```cpp
void fill(vertex_type start, color::value_type color, bool use_color_as_bounding = true);
```

Flood fills a region with the pen color. The region is drawn as horizontal spans with `line_horz()` (native head
commands without shaders). Filled pixels are detected by **reading them back**, so the head must support
`pixel_get()` (or use the [framebuffer head](/vic/heads/framebuffer/)). This is reliable with an opaque pen color and
shaders which only move pixels (offset, zoom, flip, clipping), with color changing shaders (gradient, tint) it
depends on the region. The colors are compared as given and as stored by the head (e.g. RGB565). The span stack
lives on the CPU stack, its size is `VIC_GPR_FILL_STACK_SIZE`, see [Configuration](/vic/reference/config/).
Presents once at the end.

| Parameter | Description |
|---|---|
| `start` | Start vertex inside the region |
| `color` | Bounding color or background color, depending on `use_color_as_bounding` |
| `use_color_as_bounding` | `true`: fill up to pixels of `color`; `false`: fill all connected pixels of `color` |

## Screen

### `cls()`

```cpp
void cls(color::value_type bg_color = color::none);
```

Clears the screen: sets all pixels off, deletes all characters or fills the screen with the background color.
The head defines what `color::none` means, normally its default (off) color.

### `blit()`

```cpp
void blit(const rect_type& rect, const color::value_type* pixels, std::size_t stride);
```

Writes a block of pixels (bit block transfer), e.g. a bitmap or the flush of a GUI framework. Without an active
shader and without translucent pixels (or with alpha blending disabled) the block is handed to the native `blit()`
of the head in one piece, otherwise every pixel takes the shader pipe. Presents.

| Parameter | Description |
|---|---|
| `rect` | Destination rect, right/bottom exclusive |
| `pixels` | Pixel block in ARGB format, row major, the first pixel is the left/top pixel of `rect` |
| `stride` | Distance between two rows in pixels, normally `rect.width()` |

### `move()`

```cpp
void move(const rect_type& source, vertex_type destination);
void move(vertex_type source, vertex_type destination, std::uint16_t width, std::uint16_t height);
```

Moves (copies) a display area to a new top/left position with the `move()` of the head. The fallback
implementation of the head reads every pixel back with `pixel_get()`. Bypasses the shader pipe. Presents.

| Parameter | Description |
|---|---|
| `source` | Source rect, or the source top/left vertex |
| `destination` | Destination top/left vertex |
| `width`, `height` | Size of the area |

### `present()`

```cpp
virtual void present() override;
```

Primitive rendering is done, the head transfers its (frame) buffer to the real screen. Not all heads need this,
especially not the ones without a separate buffer. Does nothing while a `present_lock()` is engaged.

### `present_lock()`

```cpp
virtual void present_lock(bool lock = true) final;
```

Prevents the `present()` call after every single primitive. Use it to draw complex objects or whole frames, then
call `present_lock(false)`, which releases the lock and calls `present()`. The lock is nested, every lock call needs
an unlock call, the last unlock presents.

| Parameter | Description |
|---|---|
| `lock` | `true` engages the lock and increments the lock count, `false` decrements it |

### `screen_width()`

```cpp
virtual std::uint16_t screen_width() const final;
```

Returns the screen (buffer) width of the head in pixel.

### `screen_height()`

```cpp
virtual std::uint16_t screen_height() const final;
```

Returns the screen (buffer) height of the head in pixel.

## Shader pipe

Shaders modify every pixel (or its position) on the way to the head. See [Shaders](/vic/reference/shader/) for
the stock shaders.

:::caution
As soon as a shader is registered, `line_horz()`, `line_vert()`, `box()` and `blit()` can't use the native head
commands any more, every pixel takes the pipe.
:::

### `shader_register()`

```cpp
void shader_register(shader::base* _shader);
```

Registers a shader. The new shader is inserted at the **front** of the pipe, so it is rendered **first**. The
shader object must live as long as it is registered.

### `shader_remove()`

```cpp
bool shader_remove(const shader::base* _shader);
```

Removes a shader from the pipe. Returns `true` if the shader was removed, `false` if it was not found.

### `shader_init()`

```cpp
void shader_init();
```

Clears the shader pipe, only the output shader remains.

### `shader_is_active()`

```cpp
bool shader_is_active() const;
```

Returns `true` if the pipe contains one or more registered shaders.

### `shader_pipe()`

```cpp
shader::base* shader_pipe() const override;
```

Returns the first shader of the pipe (the output shader if no shader is registered). Writing to it renders a pixel
through the complete pipe.
