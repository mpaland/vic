---
title: Driver base
description: vic::drv, the base class of every head with the public screen, viewport and display API and the virtual hooks a head overrides.
sidebar:
  order: 3
---

`vic::drv` (`drv.h`) is the base class of every display driver (head). It holds the screen and viewport geometry
and the orientation, and it defines the interface between the drawing and text contexts and the hardware: a set of
public functions for the application (init, screen size, viewport, framebuffer planes, display control) and a set
of protected virtual hooks which a head overrides to render natively. All colors arrive at the head in ARGB8888, a
head with fewer colors reduces them itself.

```cpp
#include "head/SSD1306.h"

vic::head::SSD1306<128U, 64U> oled(i2c_handle);   // see the head page for the template parameters
oled.init();
oled.display_contrast(200U);

vic::dc dc(oled);                                   // draw through a context, not on the head directly
if (oled.screen_is_inside({ 127, 63 })) {
  dc.plot({ 127, 63 });
}
```

## Driver lifecycle

### `init()`

```cpp
virtual void init(void) = 0;
```

Initializes the driver and the display controller. Call it once before drawing.

### `shutdown()`

```cpp
virtual void shutdown(void) = 0;
```

Shuts the driver and the display down.

### `version()`

```cpp
virtual const char* version(void) const = 0;
```

Returns the driver version and name.

### `is_graphic()`

```cpp
virtual bool is_graphic(void) const = 0;
```

Returns `true` for a graphic display, `false` for an alpha numeric (character) display.

## Screen and viewport

The **screen** is the drawing area (the buffer), the **viewport** is the visible part of it on the display. Most
heads have a viewport of the screen size at offset 0/0. A head with a bigger buffer than the display can scroll
the viewport over the screen. Graphic heads count in pixel, alpha numeric heads in chars.

### `screen_width()`

```cpp
std::uint16_t screen_width() const;
```

Returns the screen (buffer) width in pixel or chars.

### `screen_height()`

```cpp
std::uint16_t screen_height() const;
```

Returns the screen (buffer) height in pixel or chars.

### `screen_is_inside()`

```cpp
bool screen_is_inside(vertex_type v) const;
```

Returns `true` if the vertex is within the screen area.

### `viewport_width()`

```cpp
std::uint16_t viewport_width() const;
```

Returns the viewport (display) width in pixel or chars.

### `viewport_height()`

```cpp
std::uint16_t viewport_height() const;
```

Returns the viewport (display) height in pixel or chars.

### `viewport_is_inside()`

```cpp
bool viewport_is_inside(vertex_type v) const;
```

Returns `true` if the vertex (in screen coordinates) is within the visible viewport.

### `viewport_set()`

```cpp
virtual void viewport_set(vertex_type v);
```

Sets the viewport origin (top/left corner) in screen coordinates and calls `present()` to refresh the display.

### `viewport_get()`

```cpp
virtual vertex_type viewport_get() const;
```

Returns the actual viewport origin in screen coordinates.

## Framebuffer planes

Heads with several hardware framebuffers (planes) can draw into one plane while another is displayed. The base
implementation has exactly one buffer.

### `framebuffer_get_count()`

```cpp
virtual std::size_t framebuffer_get_count() const;
```

Returns the number of available framebuffers/planes, 1 if the head has no plane support.

### `framebuffer_set_access()`

```cpp
virtual bool framebuffer_set_access(std::size_t plane);
```

Uses the given plane as read/write buffer for all following drawing. Returns `true` on success, the base
implementation returns `false`.

| Parameter | Description |
|---|---|
| `plane` | Index of the plane, 0 for the first |

### `framebuffer_set_display()`

```cpp
virtual bool framebuffer_set_display(std::size_t plane, std::uint8_t alpha = 0U);
```

Sets the given plane as active display. Returns `true` on success, the base implementation returns `false`.

| Parameter | Description |
|---|---|
| `plane` | Index of the plane to display, 0 for the first |
| `alpha` | Alpha level, 0 = opaque/active, 255 = completely transparent/disabled |

## Display control

These functions do nothing unless the head supports them.

### `display_enable()`

```cpp
virtual void display_enable(bool enable = true);
```

Switches the display on (`true`) or off (`false`: standby, power save).

### `display_brightness()`

```cpp
virtual void display_brightness(std::uint8_t level);
```

Sets the display or backlight brightness, 0: dark, backlight off; 255: maximum brightness.

### `display_contrast()`

```cpp
virtual void display_contrast(std::uint8_t level);
```

Sets the display contrast, 0: minimum, 255: maximum.

## Orientation

```cpp
typedef enum tag_orientation_type {
  orientation_0 = 0,    //   0° one to one
  orientation_90,       //  90° clockwise
  orientation_180,      // 180° clockwise
  orientation_270,      // 270° clockwise
  orientation_0m,       //   0°            vertical screen mirror
  orientation_90m,      //  90° clockwise, vertical screen mirror
  orientation_180m,     // 180° clockwise, vertical screen mirror
  orientation_270m      // 270° clockwise, vertical screen mirror
} orientation_type;
```

`drv::orientation_type` is the hardware orientation of the display. The head takes care of the rotation, the
orientation is chosen so that (0, 0) is always the top/left pixel of the screen as the user sees it.

## Head implementation

This section is for writing a head. Start with the [skeleton head](/vic/heads/skeleton/) and read
[Architecture](/vic/design/architecture/).

The hooks below are `protected`. Only the classes which are friends of `drv` call them: `dc`, `tc`,
`shader::output`, `sprite::base`, `head::multihead` and `head::framebuffer`. The application draws through a
drawing or text context.

A head must implement the four pure virtual functions (`init()`, `shutdown()`, `version()`, `is_graphic()`).
A graphic head overrides at least `pixel_set()`, everything else has a working fallback built on single pixels.
Every native hook a controller can do faster (a filled window, a line command, a GRAM window stream) should be
overridden. The hooks are deliberately limited to primitives with exactly one correct pixel result, diagonal lines,
circles and text are always decomposed by the `gpr`, so every head renders identical pixels.

The colors passed to the hooks are already final: the drawing context calls the native line, box and blit hooks
only without an active shader and with opaque colors, and it clips them to the screen.

### Constructor

```cpp
drv(std::uint16_t screen_size_x,   std::uint16_t screen_size_y,
    std::uint16_t viewport_size_x, std::uint16_t viewport_size_y,
    std::int16_t  viewport_x = 0U, std::int16_t  viewport_y = 0U,
    orientation_type orientation = orientation_0);
```

Protected constructor, called by the head.

| Parameter | Description |
|---|---|
| `screen_size_x`, `screen_size_y` | Screen (buffer) size in pixel (graphic) or chars (alpha numeric) |
| `viewport_size_x`, `viewport_size_y` | Viewport (physical display) size in pixel or chars |
| `viewport_x`, `viewport_y` | Viewport offset within the screen, relative to the top/left corner |
| `orientation` | Orientation of the display |

The values are stored in the protected members `screen_size_x_`, `screen_size_y_`, `viewport_size_x_`,
`viewport_size_y_`, `orientation_` (all `const`) and `viewport_`.

### `cls()`

```cpp
virtual void cls(color::value_type bg_color = color::none);
```

Clears the screen: all pixels off, all characters deleted, or the screen filled with the background color.
`color::none` means the default (off) color of the display. The base implementation does nothing.

### `present()`

```cpp
virtual void present();
```

Called after every primitive (unless the context is present locked). A head with a RAM buffer transfers it (or its
dirty part) to the display here. The base implementation does nothing.

### `pixel_set()`

```cpp
virtual void pixel_set(vertex_type vertex, color::value_type color);
```

Sets a pixel in the given color. The alpha channel may be ignored, blending is done before. The base implementation
does nothing, every graphic head overrides it.

### `pixel_get()`

```cpp
virtual color::value_type pixel_get(vertex_type vertex);
```

Returns the color of a pixel in ARGB format with opaque alpha. A head which can't read its pixels back returns
`color::none` (the base implementation), the drawing context then blends against its background color. See
[Write only displays](/vic/design/write-only-displays/) and the [framebuffer head](/vic/heads/framebuffer/).

### `blit()`

```cpp
virtual void blit(const rect_type& rect, const color::value_type* pixels, std::size_t stride);
```

Writes a rectangular block of pixels in one shot, e.g. the flush of a framebuffer or a bitmap. Heads with an
autoincrementing GRAM window should override it and stream the block with one address setup. The fallback clips
the block to the screen and calls `pixel_set()` for every pixel.

| Parameter | Description |
|---|---|
| `rect` | Destination rect on the screen, right/bottom exclusive |
| `pixels` | Pixel block in ARGB format, row major, the first pixel is the left/top pixel of `rect` |
| `stride` | Distance between two rows in pixels |

### `line_horz()`

```cpp
virtual void line_horz(vertex_type v0, vertex_type v1, color::value_type color);
```

Draws a horizontal line, one pixel wide, both end points included, the y of `v1` is ignored. The fallback sets
every pixel.

### `line_vert()`

```cpp
virtual void line_vert(vertex_type v0, vertex_type v1, color::value_type color);
```

Draws a vertical line, one pixel wide, both end points included, the x of `v1` is ignored. The fallback sets every
pixel.

### `box()`

```cpp
virtual void box(const rect_type& rect, color::value_type color);
```

Draws a filled box, right/bottom exclusive. The fallback draws it with `line_horz()`, an empty rect draws nothing.

### `move()`

```cpp
virtual void move(const rect_type& source, vertex_type destination);
```

Moves (copies) a display area to the destination top/left vertex, overlapping areas are handled. The fallback reads
every pixel with `pixel_get()`, so it needs a head which can read back.

### Text hooks

```cpp
virtual void text_out(std::uint16_t ch);
virtual std::uint16_t text_out(const std::uint8_t* string);
virtual void text_set_pos(vertex_type pos);
virtual void text_set_inverse(bool inverse);
virtual void text_clear_eol();
virtual void text_clear_sol();
virtual void text_clear_line();
```

The text functions of an alpha numeric head, called by a [`tc`](/vic/reference/tc/) created on the head.

| Hook | Description |
|---|---|
| `text_out(ch)` | Outputs a 16 bit ASCII/UNICODE char at the cursor and moves the cursor. Base: nothing |
| `text_out(string)` | Outputs a zero terminated UTF-8 string, returns the number of chars. Base: decodes the string, calls `text_out(ch)` for every char except `\n` and `\r`, then `present()` |
| `text_set_pos(pos)` | Sets the cursor in chars, 0/0 is left/top. Base: nothing |
| `text_set_inverse(inverse)` | Sets normal or inverse video. Base: nothing |
| `text_clear_eol()` | Clears the line from the cursor (included) to the end. Base: nothing |
| `text_clear_sol()` | Clears the line from the start to the cursor (included). Base: nothing |
| `text_clear_line()` | Clears the actual line. Base: nothing |
