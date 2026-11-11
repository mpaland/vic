---
title: Sprites
description: vic::sprite, canvas and sheet sprites with background restore, z-order, rotation and collision detection.
sidebar:
  order: 4
---

Sprites (`sprite.h`, namespace `vic::sprite`) are moving objects which restore their background when they move or
change their frame. `sheet` renders frames directly out of a constant sprite sheet in flash and needs no RAM for the
pattern, `canvas` is a sprite which is drawn on with the normal [`dc`](/vic/reference/dc/) functions, and
`canvas_sheet` loads sheet frames into a canvas, so they can be rotated and tinted. Overlapping sprites are rendered
in z-order with alpha blending, pixel exact collision detection is available per sprite.

![Sprite demo](/vic/media/demo_sprite.gif)

```cpp
#include "sprite.h"
#include "sprite_sheets/coin.h"                  // sprite_sheet_coin, see "Sprite sheets" below

vic::sprite::canvas<400U, 1U> ball(head, 5);     // 400 pixel per frame, 1 frame, z-index 5
ball.set_color(vic::color::green);
ball.disc({ 10, 10 }, 10U);                      // draw into the canvas, nothing appears yet

vic::sprite::canvas_sheet<320U> coin(head, &sprite_sheet_coin, 10);   // in front of the ball

for (std::int16_t x = 0; x < 200; ++x) {
  ball.render({ x, 40 });                        // move: restores the old background, draws the new position
  coin.render({ 100, 40 }, static_cast<std::uint16_t>(x % 8), static_cast<std::int16_t>(x * 5));   // frame and rotation
}
```

## Sprite types

| Class | Pattern | RAM | Rotation |
|---|---|---|---|
| `canvas<Pixel, Frames>` | Drawn with the `dc` functions, `Frames` frames | Pattern and restore buffer, `Pixel` entries each | Yes |
| `sheet<Frame_Width, Frame_Height>` | Frames of a constant sprite sheet | Restore buffer only | No |
| `canvas_sheet<Pixel>` | A sheet frame loaded into a one frame canvas on every `render()` | Like a canvas | Yes |

The buffers are AVL trees ([avl_array](https://github.com/mpaland/avl_array)) of vertex and color, no heap is used.

### Rendering model

All sprites are linked in one chain, sorted by z-index (a bigger z-index is in front, sprites with the same
z-index are placed behind the existing ones). The constructor inserts the sprite, the destructor removes it.

`render()` first restores the background of all pixels the sprite doesn't cover any more, then renders its pattern:
every pixel is blended with the sprites in front of and behind it and with the background. The background of a
pixel is read with the `pixel_get()` of the head when the pixel is covered for the first time, or taken from
[`dc::set_bg_color()`](/vic/reference/dc/#set_bg_color) of the sprite if the head can't read back. The sprite pixels
are written to the head directly, not through the shader pipe of the sprite, and the head is presented once at the
end. Pixels a sprite hides from another sprite keep their background info, so sprites can cross each other in any
order.

:::caution
On a write only head without `set_bg_color()` the sprites can't restore a non uniform background. Use the
[framebuffer head](/vic/heads/framebuffer/), see [Write only displays](/vic/design/write-only-displays/).
:::

## Sprite base

`vic::sprite::base` is derived from `vic::dc` and is the common base of all sprites.

### `render()`

```cpp
void render(vertex_type position, std::uint16_t frame = 0U);
```

Renders the sprite on the head: restores the background of the old position and renders the frame at the new
position. A frame which doesn't exist (e.g. `0xFFFF`) renders nothing, so the sprite is hidden and its background
is restored. A new sprite has no frame rendered until the first `render()`. Runs the collision detection if a
handler is set. Presents the head.

| Parameter | Description |
|---|---|
| `position` | Screen vertex where the origin (0, 0) of the sprite is rendered |
| `frame` | Frame to render |

### `get_position()`

```cpp
vertex_type get_position() const;
```

Returns the position of the sprite origin of the last `render()`.

### `get_z_index()`

```cpp
std::int16_t get_z_index() const;
```

Returns the z-index of the sprite.

### `set_collision_handler()`

```cpp
void set_collision_handler(collision_handler_type handler, void* arg = nullptr);
```

Sets the collision handler and enables the pixel exact collision detection for this sprite. In `render()` the
handler is called once for every other sprite which has at least one (non background) pixel at the same screen
position as this sprite, after the pixels are rendered and before the head is presented. Translucent pixels count,
the real (rotated) shape of both sprites is used. Only the rendered sprite reports: when sprite A is rendered and
overlaps sprite B, the handler of A is called with `other = B`, B reports when B is rendered. The handler may change
colors, frames or its own state, it must not destroy sprites or render the reporting sprite. Without a handler the
detection is off and costs nothing.

```cpp
typedef void (*collision_handler_type)(base& self, base& other, void* arg);
```

| Parameter | Description |
|---|---|
| `handler` | Collision handler, `nullptr` disables the detection |
| `arg` | Argument passed to the handler |

### `get_collision()`

```cpp
base* get_collision() const;
```

Returns the first sprite (the one with the highest z-index) this sprite collided with in the last `render()`,
`nullptr` if there was no collision or no collision handler is set. This is the polling alternative to the handler
(set a handler which does nothing to enable the detection).

```cpp
void on_hit(vic::sprite::base& self, vic::sprite::base& other, void* arg)
{
  (void)self; (void)other;
  *static_cast<bool*>(arg) = true;
}

bool hit = false;
ship.set_collision_handler(on_hit, &hit);
ship.render({ x, y });
if (hit) { /* ... */ }
```

### `present()`

```cpp
virtual void present();
```

Does nothing. Drawing on a sprite only modifies its pattern buffer, so the primitives don't present the head. The
head is presented once by `render()`.

### Buffer interface

```cpp
virtual bool pattern_it_get(std::uint16_t frame, pixel_type& pixel) = 0;
virtual void pattern_it_next(std::uint16_t frame, bool begin) = 0;
virtual bool pattern_find(std::uint16_t frame, pixel_type& pixel) = 0;
virtual bool restore_it_get(pixel_type& pixel) = 0;
virtual void restore_it_next(bool begin) = 0;
virtual void restore_it_delete(void) = 0;
virtual bool restore_find(pixel_type& pixel) = 0;
virtual void restore_set(const pixel_type& pixel) = 0;
```

The pattern (sprite pixels) and restore (background) buffer access which `render()` uses across all sprites. The
sprite classes implement it, applications don't call it. `pattern_find()` takes a vertex relative to the sprite
origin and returns `true` and the color if the frame has a pixel there (rotation applied).

## Canvas sprite

```cpp
template <std::uint16_t Pixel, std::uint16_t Frames>
class canvas : public base;
```

A canvas is a drawing context whose output shader stores every pixel in the pattern buffer of the edit frame. All
`dc` functions work on it: colors, pen width, anti aliasing, shaders, text with a `tc` on the canvas. The canvas
coordinates are relative to the sprite origin. Pixels drawn over existing pattern pixels are alpha blended with
them, a pixel set to `color::none` is removed from the pattern (punch out).

| Template parameter | Description |
|---|---|
| `Pixel` | Maximum number of pixels per frame, also the size of the restore buffer |
| `Frames` | Number of frames |

### `canvas()`

```cpp
canvas(drv& head, std::int16_t z_index = 0);
```

Creates a canvas sprite on a head.

| Parameter | Description |
|---|---|
| `head` | Head to render on |
| `z_index` | Z-index, a bigger z-index is in front |

### `select_frame()`

```cpp
void select_frame(std::uint16_t frame);
```

Selects the frame which the drawing functions edit, invalid frames are ignored. The displayed frame is the one
given to `render()`.

### `get_frame()`

```cpp
std::uint16_t get_frame() const;
```

Returns the actual edit frame.

### `cls()`

```cpp
void cls();
```

Clears the edit frame (removes all its pixels). The screen is not touched until the next `render()`.

### `pixel_count()`

```cpp
std::uint16_t pixel_count() const;
```

Returns the number of pixels in the edit frame.

### `set_rotation()`

```cpp
void set_rotation(std::int16_t angle, vertex_type center = { 0, 0 });
```

Sets the rotation of the sprite. The pattern is not modified, it is rotated when the sprite is rendered (and when
other sprites test against it), every destination pixel is mapped back to the pattern. Without rotation (angle 0)
rendering costs nothing extra. A rotated pattern covers about as many pixels as the pattern has (+/- 20 % by
rasterization), so size `Pixel` with some headroom when rotating.

| Parameter | Description |
|---|---|
| `angle` | Rotation angle in degree, clockwise on the screen, 0 disables the rotation |
| `center` | Rotation center in canvas coordinates, the sprite stays anchored at its origin |

### `get_rotation()`

```cpp
std::int16_t get_rotation() const;
```

Returns the rotation angle in degree (reduced to -359 - 359), 0 if the sprite is not rotated.

### `get_rotation_center()`

```cpp
vertex_type get_rotation_center() const;
```

Returns the rotation center in canvas coordinates.

## Sheet sprite

```cpp
template <std::uint16_t Frame_Width, std::uint16_t Frame_Height>
class sheet : public base;
```

A sheet sprite takes its frames directly out of a sprite sheet, the pattern needs no RAM, only the restore buffer
of `Frame_Width * Frame_Height` entries. Pixels with the background color of the sheet are transparent. A sheet
sprite can't be rotated or drawn on.

### `sheet()`

```cpp
sheet(drv& head, const sheet_info_type* sheet_info, std::int16_t z_index = 0);
```

Creates a sheet sprite. The frame size of the template should match `sprite_width` and `sprite_height` of the
sheet.

| Parameter | Description |
|---|---|
| `head` | Head to render on |
| `sheet_info` | The sprite sheet |
| `z_index` | Z-index, a bigger z-index is in front |

### `frame_count()`

```cpp
std::uint16_t frame_count() const;
```

Returns the number of frames of the sheet.

## Canvas sheet sprite

```cpp
template <std::uint16_t Pixel>
class canvas_sheet : public canvas<Pixel, 1U>;
```

Combines a one frame canvas with a sprite sheet: every `render()` clears the canvas and loads the sheet frame into
it through the shader pipe of the canvas. So the frames can be rotated, and shaders like `tint` colorize them while
they are loaded. `Pixel` must be at least `sprite_width * sprite_height` of the sheet, more for rotation.

### `canvas_sheet()`

```cpp
canvas_sheet(drv& head, const sheet_info_type* sheet_info, std::int16_t z_index = 0);
```

Creates a canvas sheet sprite.

| Parameter | Description |
|---|---|
| `head` | Head to render on |
| `sheet_info` | The sprite sheet |
| `z_index` | Z-index, a bigger z-index is in front |

### `frame_count()`

```cpp
std::uint16_t frame_count() const;
```

Returns the number of frames of the sheet.

### `set_rotation()`

```cpp
void set_rotation(std::int16_t angle, vertex_type center);
void set_rotation(std::int16_t angle);
```

Sets the rotation around an explicit center (in frame coordinates) or around the center of the sheet frame. See
[`canvas::set_rotation()`](#set_rotation).

### `render()`

```cpp
void render(vertex_type position, std::uint16_t frame, std::int16_t angle);
void render(vertex_type position, std::uint16_t frame = 0U);
```

Loads the sheet frame into the canvas and renders it. The variant with `angle` rotates the frame around its center
first. An invalid frame renders an empty sprite (hides it). Presents the head.

| Parameter | Description |
|---|---|
| `position` | Screen vertex where the origin of the sprite is rendered |
| `frame` | Frame of the sheet |
| `angle` | Rotation angle in degree, clockwise on the screen |

## Sprite sheets

### `sheet_info_type`

```cpp
typedef struct tag_sheet_info_type {
  std::uint16_t         width;          // width of sprite sheet
  std::uint16_t         height;         // height of sprite sheet
  std::uint16_t         sprite_width;   // width of sprite
  std::uint16_t         sprite_height;  // height of sprite
  color::value_type     bg_color;       // background color (transparent), compared against the DECODED pixel color
  color::format_type    format;         // supported formats are RGB332, RBG565, RGB888, ARGB8888 and RGBA8888
  const std::uint8_t*   data;           // sprite sheet data
} sheet_info_type;
```

A sprite sheet is one image with all frames of the same size in a grid. The frames are numbered row by row:
`width / sprite_width` frames per row, `frame_count = (width / sprite_width) * (height / sprite_height)`.
The pixels are stored row major over the whole sheet, every pixel takes the bytes of its format (the upper nibble
of `color::format_type`), the least significant byte comes first, so there are no alignment or endianness
requirements. `bg_color` is compared against the decoded ARGB color: `color::black` for an RGB565 sheet with a
black background, `color::none` for an ARGB8888 sheet with transparent pixels.

| Format | Bytes per pixel | Byte order |
|---|---|---|
| `color::format_RGB332` | 1 | RRRGGGBB |
| `color::format_RGB565` | 2 | low byte, high byte of the RGB565 value |
| `color::format_RGB888` | 3 | B, G, R |
| `color::format_ARGB8888` | 4 | B, G, R, A (a little endian `0xAARRGGBB`) |
| `color::format_RGBA8888` | 4 | A, B, G, R |

### `sheet_pixel_color()`

```cpp
color::value_type sheet_pixel_color(const sheet_info_type& info, const std::uint8_t* p);
```

Decodes one pixel of a sprite sheet and returns its ARGB color. The pixel is read byte wise.

| Parameter | Description |
|---|---|
| `info` | The sheet info (for the format) |
| `p` | Pointer to the pixel data |

### Generating sheets

The sheets of the sprite demo are in `demo/sprite_sheets/`. `coin.h`, `ghost.h`, `ship.h` and `star.h` are
generated procedurally by `demo/sprite_sheets/gen_sheets.py` (Python 3, no packages needed), run from the repo
root:

```
python demo/sprite_sheets/gen_sheets.py
```

The script draws every frame into an RGBA pixel array with the frames side by side in one row and writes a header
with the byte array `sprite_sheet_<name>_data` and the `vic::sprite::sheet_info_type sprite_sheet_<name>`. RGB565
sheets get black as background color (transparent pixels are written black), ARGB8888 sheets get `color::none`
(transparent pixels have alpha 0, translucent pixels like the ghost body keep their alpha). The generated header
looks like this:

```cpp
static const std::uint8_t sprite_sheet_coin_data[4096] = {
  0x00, 0x00, /* ... 8 frames of 16 x 16 pixel, RGB565, LSB first */
};

static const vic::sprite::sheet_info_type sprite_sheet_coin = {
  128,                         // sheet width
  16,                          // sheet height
  16,                          // sprite width
  16,                          // sprite height
  vic::color::black,           // sprite background color (transparent)
  vic::color::format_RGB565,   // data format
  sprite_sheet_coin_data
};
```

Sheets from an image editor are converted the same way: export the sheet as raw pixels in one of the formats above
(row major, LSB first) and wrap the bytes in a `sheet_info_type`. `demo/sprite_sheets/bina.h` is such a sheet,
a 128 x 192 RGB565 sheet of 32 x 48 pixel frames.
