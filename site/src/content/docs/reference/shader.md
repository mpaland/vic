---
title: Shaders
description: vic::shader, the shader pipe and every stock shader with its constructor and functions.
sidebar:
  order: 5
---

Shaders (namespace `vic::shader`) transform every pixel on its way from the primitive renderer to the head. A
**pixel shader** changes the color (gradient, tint, alpha blend), a **vertex shader** changes the position (offset,
zoom, flip, rotate) or drops the pixel (clipping), the brush shader turns one pixel into a pen shape. Shaders are
registered at a [drawing context](/vic/reference/dc/#shader-pipe) and can be combined in any number and order.
They are plain objects without heap, the context only links them.

```cpp
#include "dc.h"
#include "shader/gradient.h"

vic::shader::clipping clip({ 0, 0, 100, 50 });                 // draw only inside this rect
vic::shader::gradient_horizontal<2U> grad = { { { 0, 0 },   vic::color::red },
                                              { { 100, 0 }, vic::color::blue } };
dc.shader_register(&clip);
dc.shader_register(&grad);                                     // runs first: grad -> clip -> output
dc.disc({ 50, 25 }, 40U);                                      // red to blue disc, clipped
dc.shader_remove(&grad);
dc.shader_init();                                              // remove all shaders
```

## The shader pipe

The pipe is a singly linked list of shaders. Every shader has a `next_` pointer, its `pixel_set()` does its work
and calls `next_->pixel_set()`. The last shader is always the **output shader** of the context, which blends and
sends the pixel to the head. `dc::shader_register()` inserts a new shader at the **front**, so the shader registered
last is rendered first.

`pixel_get()` runs the same way: a vertex shader maps the position before it asks the next shader, so reading a
pixel back (alpha blending, `fill()`) sees the same coordinates as writing it.

:::caution
With an active shader the drawing context renders every pixel through the pipe, the native `line_horz()`,
`line_vert()`, `box()` and `blit()` commands of the head are not used. Register shaders only while they are needed.
:::

A custom shader derives from `shader::base`, implements `pixel_set()` and overrides `pixel_get()` if it moves
pixels:

```cpp
// draws every second row only (scan line effect)
class scanline : public vic::shader::base
{
public:
  void pixel_set(vic::vertex_type vertex, vic::color::value_type color) override
  {
    if (!(vertex.y & 1)) {
      next_->pixel_set(vertex, color);
    }
  }
};
```

## Base classes

### `base`

```cpp
class base;                       // shader/base.h
```

Abstract shader base class, the default constructor is protected.

| Member | Description |
|---|---|
| `base* next_` | Next shader in the pipe, set by `dc::shader_register()` |
| `virtual void pixel_set(vertex_type vertex, color::value_type color) = 0` | Processes a pixel and passes it to `next_` |
| `virtual color::value_type pixel_get(vertex_type vertex)` | Returns the color of a pixel, the base implementation asks `next_` |

### `output`

```cpp
class output : public base;       // shader/base.h
```

The output shader is always the last shader of the pipe and sends the pixels to the head. Every `dc` owns one, its
settings are reached through the context (`dc::alpha_blending_enable()`, `dc::set_bg_color()`).

| Function | Description |
|---|---|
| `output(drv& head)` | Creates the output shader of a head, alpha blending enabled, background color `color::none` |
| `void alpha_blending_enable(bool enable = true)` | Enables or disables alpha blending |
| `bool alpha_blending_is_enabled() const` | Returns `true` if alpha blending is enabled |
| `void set_bg_color(color::value_type bg_color)` | Sets the background color for heads which can't read back, `color::none` disables it |
| `color::value_type get_bg_color() const` | Returns the background color |
| `virtual void pixel_set(vertex_type vertex, color::value_type color) final` | Sets the pixel on the head. A translucent color is blended with the head pixel (or the background color if the head returns `color::none`) if blending is enabled, opaque colors never read the background |
| `virtual color::value_type pixel_get(vertex_type vertex) final` | Returns the head pixel, `color::none` if the head can't read |

## Pixel shaders

### `alpha_blend`

```cpp
class alpha_blend : public base;  // shader/base.h
```

Alpha blends every pixel with the pixel read back from the next shader. Use it for dynamic blending effects while
alpha blending is disabled in the output shader. Needs a pixel read back. No constructor parameters.

| Function | Description |
|---|---|
| `virtual void pixel_set(vertex_type vertex, color::value_type color) final` | Blends `color` over the pixel of `next_` and passes the result |

### `tint`

```cpp
class tint final : public base;   // shader/base.h
```

Mixes the RGB part of every pixel towards the tint color, the alpha of the pixel is kept. Useful to colorize sprites
or bitmaps, e.g. a hit flash or a disabled (gray) look. Register it on a canvas sprite to tint the frames while
they are drawn or loaded.

| Function | Description |
|---|---|
| `tint(color::value_type color, std::uint8_t weight = 128U)` | Creates a tint shader |
| `void set(color::value_type color, std::uint8_t weight = 128U)` | Sets the tint color and weight |
| `virtual void pixel_set(vertex_type vertex, color::value_type color)` | Passes `mix(tint, color, weight)` with the alpha of `color` |

| Parameter | Description |
|---|---|
| `color` | Tint color, the RGB part is used |
| `weight` | 0 leaves the pixel unchanged, 255 replaces the RGB part by the tint color |

### Gradients

```cpp
template<std::size_t Max_Pixel> class gradient;              // shader/gradient.h
template<std::size_t Max_Pixel> class gradient_horizontal;
template<std::size_t Max_Pixel> class gradient_vertical;
template<std::size_t Max_Pixel> class gradient_solid;
```

The gradient shaders replace the pixel color by a color interpolated from up to `Max_Pixel` color points
(`pixel_type { vertex, color }`). The alpha of the drawing color is multiplied with the alpha of the gradient
color, so anti aliased edges and translucent pens keep working. The points may lie anywhere, also outside the
drawn shape.

| Shader | Color at a pixel |
|---|---|
| `gradient<N>` | Weighted by the inverse squared distance to every point |
| `gradient_horizontal<N>` | Like `gradient`, only the x distance counts (y is ignored): vertical color bands |
| `gradient_vertical<N>` | Like `gradient`, only the y distance counts (x is ignored): horizontal color bands |
| `gradient_solid<N>` | The color of the nearest point, the first point wins on equal distance (Voronoi cells) |

All four have the same functions (from `gradient_base<Max_Pixel>`):

| Function | Description |
|---|---|
| `gradient(const std::initializer_list<pixel_type>& il)` | Creates the gradient from a list of points, at most `Max_Pixel` are taken |
| `gradient(const pixel_type* gradient_pixels)` | Creates the gradient from an array of exactly `Max_Pixel` points |
| `void set(const pixel_type* gradient_pixels, std::size_t pixel_count)` | Sets new points, at most `Max_Pixel` are taken |
| `virtual color::value_type mix(vertex_type pos)` | Returns the gradient color at a position |
| `virtual void pixel_set(vertex_type vertex, color::value_type color) final` | Passes the gradient color with the multiplied alpha |

```cpp
vic::shader::gradient<4U> gr = { { { 0, 0 },     vic::color::darkred },
                                 { { 100, 20 },  vic::color::darkorange },
                                 { { 250, 50 },  vic::color::brown },
                                 { { 300, 80 },  vic::color::green } };
dc.shader_register(&gr);
```

## Vertex shaders

### `clipping`

```cpp
class clipping : public base;     // shader/base.h
```

Draws only the pixels inside (or outside) a clipping rect.

| Function | Description |
|---|---|
| `clipping()` | Creates a disabled clipping region (an empty rect, everything is drawn) |
| `clipping(const rect_type& region, bool inside = true)` | Creates an enabled clipping region |
| `void set(const rect_type& region, bool inside = true)` | Sets the clipping region |
| `rect_type get() const` | Returns the clipping region |
| `bool is_inside(const vertex_type& vertex) const` | Returns `true` if the vertex is drawn |
| `virtual void pixel_set(vertex_type vertex, color::value_type color) final` | Passes the pixel if it is drawn |

| Parameter | Description |
|---|---|
| `region` | Clipping rect, right/bottom exclusive |
| `inside` | `true`: the pixels inside the rect are drawn (default); `false`: the pixels inside are suppressed |

### `offset`

```cpp
class offset : public base;       // shader/base.h
```

Adds an offset to every vertex, e.g. to draw a widget at a different position or to scroll.

| Function | Description |
|---|---|
| `offset(vertex_type _offset)` | Creates the shader with an offset |
| `void set(vertex_type _offset)` | Sets the offset |
| `vertex_type get() const` | Returns the offset |
| `virtual void pixel_set(vertex_type vertex, color::value_type color) final` | Passes the pixel at `vertex + offset` |
| `virtual color::value_type pixel_get(vertex_type vertex) final` | Reads the pixel at `vertex + offset` |

### `zoom`

```cpp
class zoom : public base;         // shader/base.h
```

Integer zoom: every pixel becomes a block of `x_level * y_level` pixels at `(x * x_level, y * y_level)`.

| Function | Description |
|---|---|
| `zoom(std::uint16_t x_level, std::uint16_t y_level)` | Creates the shader with the zoom factors of the x and y axis |
| `virtual void pixel_set(vertex_type vertex, color::value_type color) final` | Sets the zoomed pixel block |
| `virtual color::value_type pixel_get(vertex_type vertex) final` | Reads the top/left pixel of the block |

### `flip`

```cpp
class flip final : public base;   // shader/base.h
```

Mirrors the vertices horizontally and/or vertically. Both directions are disabled after construction.

| Function | Description |
|---|---|
| `flip()` | Creates the shader, no flip |
| `void set_horizontal(bool enable, const vertex_type& axis)` | Enables/disables the horizontal flip, x is mirrored at `axis.x`: `x' = 2 * axis.x - x` |
| `void set_vertical(bool enable, const vertex_type& axis)` | Enables/disables the vertical flip, y is mirrored at `axis.y`: `y' = 2 * axis.y - y` |
| `virtual void pixel_set(vertex_type vertex, color::value_type color)` | Passes the pixel at the mirrored vertex |
| `virtual color::value_type pixel_get(vertex_type vertex)` | Reads the pixel at the mirrored vertex |

### `rotate`

```cpp
class rotate final : public base; // shader/rotate.h
```

Rotates every vertex around a center. To avoid holes in the destination, every pixel is set at all destination
positions around the rotated vertex which map back to the source vertex.

| Function | Description |
|---|---|
| `rotate(vertex_type center, std::int16_t angle)` | Creates the shader |
| `void set(vertex_type center, std::int16_t angle)` | Sets the rotation center and angle |
| `virtual void pixel_set(vertex_type vertex, color::value_type color)` | Sets the rotated pixel |
| `virtual color::value_type pixel_get(vertex_type vertex)` | Reads the pixel at the rotated vertex |

| Parameter | Description |
|---|---|
| `center` | Rotation center |
| `angle` | Rotation angle in degree, math positive (clockwise on the screen) |

## Brush shader

```cpp
class brush : public base;        // shader/brush.h
```

Renders a pen shape for every pixel: each pixel is replaced by an alpha mask of `width x height` pixels centered at
the vertex, the alpha of the mask is multiplied with the alpha of the color. The style (dash, dot, dashdot) skips
pixels by counting the `pixel_set()` calls, so it follows the drawing order of the primitive. The default shape is
a one pixel solid pen.

```cpp
typedef enum tag_style_type {
  style_solid = 0,    // ***********
  style_dash,         // *** *** ***
  style_dot,          // * * * * * *
  style_dashdot       // *** * *** * ***
} style_type;

typedef struct tag_shape_type {
  const std::uint8_t* alpha;    // brush shape as alpha value array (width * height values)
  std::uint8_t        width;    // width of shape
  std::uint8_t        height;   // height of shape
  style_type          style;    // style
} shape_type;
```

```cpp
#include "shader/brush.h"

vic::shader::brush pen;
vic::shader::brush::shape_type shape;
pen.get_stock_shape(shape, vic::shader::brush::style_dash, 3U);
pen.set_shape(shape);
dc.shader_register(&pen);
dc.circle({ 60, 60 }, 40U);       // dashed 3 pixel circle
dc.shader_remove(&pen);
```

### `brush()`

```cpp
brush();
```

Creates a brush with a one pixel solid pen.

### `set_shape()`

```cpp
void set_shape(const shape_type& shape);
```

Selects the pen shape and resets the style counter. The alpha array must live as long as the shape is used.

### `get_shape()`

```cpp
const shape_type& get_shape() const;
```

Returns the actual shape.

### `get_stock_shape()`

```cpp
bool get_stock_shape(shape_type& shape, style_type style, std::uint8_t width) const;
```

Fills `shape` with a stock pen of 1 to 4 pixel width (width 3 has half transparent corners, width 4 has open
corners). Returns `false` and leaves `shape` unchanged if the width is not available.

| Parameter | Description |
|---|---|
| `shape` | Receives the shape |
| `style` | Brush style |
| `width` | Pen width, 1 to 4 |

### `reset()`

```cpp
void reset();
```

Resets the style counter, the next pixel starts a new dash / dot sequence.

### `pixel_set()`

```cpp
virtual void pixel_set(vertex_type vertex, color::value_type color) final;
```

Renders the shape centered at the vertex, depending on the style.
