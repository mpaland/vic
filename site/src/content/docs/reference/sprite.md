---
title: Sprites
description: vic::sprite, canvas and sheet sprites with background restore, z-order, rotation and collision detection.
sidebar:
  order: 4
---

Sprites (`sprite.h`, namespace `vic::sprite`) are moving objects which restore their background when they move or
change their frame. `sheet` renders frames directly out of a constant sprite sheet in flash and needs no RAM for the
pattern, `canvas` is a sprite which is drawn on with the normal [`dc`](/vic/reference/dc/) functions, and
`canvas_sheet` and `canvas_sheet_xy` load sheet frames into a canvas, so they can be rotated and tinted. Overlapping
sprites are rendered in z-order with alpha blending, pixel exact collision detection is available per sprite. See
[Sprite types](#sprite-types) for the differences.

![Sprite demo](/vic/media/demo_sprite.gif)

```cpp
#include "sprite.h"
#include "sprite_sheets/coin.h"                  // sprite_sheet_coin, see "Sprite sheets" below

vic::sprite::canvas<400U, 1U> ball(head, 5);     // 400 pixel per frame, 1 frame, z-index 5
ball.set_color(vic::color::green);
ball.disc({ 10, 10 }, 10U);                      // draw into the canvas, nothing appears yet

vic::sprite::canvas_sheet_xy<16U, 16U> coin(head, &sprite_sheet_coin, 10);   // 16 x 16 pixel frames, in front of the ball

for (std::int16_t x = 0; x < 200; ++x) {
  ball.render({ x, 40 });                        // move: restores the old background, draws the new position
  coin.render({ 100, 40 }, static_cast<std::uint16_t>(x % 8), static_cast<std::int16_t>(x * 5));   // frame and rotation
}
```

## Sprite types

The four sprite types differ in where the pattern (the pixels of the sprite) comes from, what can be done with it
and how much RAM it takes:

- **`sheet`** renders its frames directly out of a constant sprite sheet in flash. It needs no RAM for the pattern
  and is the smallest and fastest sprite, but the frames can't be rotated, changed by shaders or drawn on. Take it
  for every sprite which just plays the frames of a sheet.
- **`canvas`** is a drawing context. Its frames are not taken out of a sheet, they are drawn with the `dc`
  functions (lines, discs, text) and can be changed at any time. It can be rotated. Take it for sprites which are
  created at run time.
- **`canvas_sheet`** loads a sheet frame into a canvas on every `render()`. The frame passes the shader pipe of the
  canvas, so it can be recolored (`tint`), zoomed or flipped, and the canvas can be rotated. The pattern may have
  any shape and position, only its pixel count is limited. It needs the most RAM.
- **`canvas_sheet_xy`** does the same as a `canvas_sheet` with a canvas of a fixed size. Its buffers are plain
  arrays instead of trees, so it needs about half of the RAM and renders faster, and less again if it is never
  rotated (`Rotate = false`). Pixels which a shader moves out of the canvas are dropped. Take it instead of a
  `canvas_sheet` whenever the size of the frame is known.

| | `sheet<Frame_Width, Frame_Height>` | `canvas<Pixel, Frames>` | `canvas_sheet<Pixel>` | `canvas_sheet_xy<Width, Height, Rotate>` |
|---|---|---|---|---|
| Pattern | Sheet frame, read from flash | Drawn with the `dc` functions | Sheet frame, loaded into a canvas | Sheet frame, loaded into a canvas |
| Rotation | No | Yes | Yes | Yes, no with `Rotate = false` |
| Shaders | No | While drawing | While the frame is loaded | While the frame is loaded, inside the canvas |
| Pattern buffer | None | AVL tree, 13 byte per pixel | AVL tree, 13 byte per pixel | Array, 4 byte per pixel |
| Restore buffer | Array, 4 byte and 1 bit per pixel | AVL tree, 15 byte per pixel | AVL tree, 15 byte per pixel | Array, 4 byte and 1 bit per pixel, about twice the pixels with rotation |
| RAM of a 24 x 24 pixel sprite | 2488 byte | 16300 byte | 16308 byte | 8104 byte, 4820 byte with `Rotate = false` |

The RAM is the `sizeof` of `sheet<24, 24>`, `canvas<576, 1>`, `canvas_sheet<576>` and `canvas_sheet_xy<24, 24>` on
a 32 bit build. The AVL trees are [avl_array](https://github.com/mpaland/avl_array) buffers of vertex and color,
the arrays are plain two dimensional arrays with x and y as keys. No heap is used.

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
of `Frame_Width * Frame_Height` entries (a two dimensional array of 4 byte and one bit per pixel). Pixels with the
background color of the sheet are transparent. A sheet sprite can't be rotated or drawn on.

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

## Canvas sheet sprite with array buffers

```cpp
template <std::uint16_t Width, std::uint16_t Height, bool Rotate = true>
class canvas_sheet_xy : public base;
```

A canvas sheet sprite of a fixed size. It is used like a `canvas_sheet` (`frame_count()`, `set_rotation()`,
`get_rotation()`, `get_rotation_center()`, `render()`), but stores the pattern and the background in plain two
dimensional arrays with x and y as keys instead of AVL trees. It needs about half of the RAM and renders faster.
The price is a pattern which is limited to the canvas of `Width x Height`: pixels which a shader moves out of the
canvas are dropped. So a frame which is zoomed by 2 needs twice the width and height, shaders like `tint` or `flip`
need nothing extra. Rotation with `set_rotation()` or `render()` needs nothing extra either, the restore array is
as big as the diagonal of the canvas. A sprite which is never rotated gets `Rotate = false`: its restore array has
the size of the canvas then, and rotating it is a compile error.

| Template parameter | Description |
|---|---|
| `Width` | Width of the canvas, at least `sprite_width` of the sheet |
| `Height` | Height of the canvas, at least `sprite_height` of the sheet |
| `Rotate` | `true` (default) if the sprite can be rotated, `false` for a restore array of the canvas size |

The saving is biggest for square frames, `sizeof` on a 32 bit build:

| Frame | `canvas_sheet` | `canvas_sheet_xy` | `canvas_sheet_xy`, `Rotate = false` |
|---|---|---|---|
| 16 x 16 | `canvas_sheet<320>`: 9140 byte | `canvas_sheet_xy<16, 16>`: 3972 byte | `canvas_sheet_xy<16, 16, false>`: 2220 byte |
| 24 x 24 | `canvas_sheet<576>`: 16308 byte | `canvas_sheet_xy<24, 24>`: 8104 byte | `canvas_sheet_xy<24, 24, false>`: 4820 byte |
| 32 x 32 | `canvas_sheet<1280>`: 36020 byte | `canvas_sheet_xy<32, 32>`: 14184 byte | `canvas_sheet_xy<32, 32, false>`: 8460 byte |

### `canvas_sheet_xy()`

```cpp
canvas_sheet_xy(drv& head, const sheet_info_type* sheet_info, std::int16_t z_index = 0);
```

Creates a canvas sheet sprite with array buffers.

| Parameter | Description |
|---|---|
| `head` | Head to render on |
| `sheet_info` | The sprite sheet |
| `z_index` | Z-index, a bigger z-index is in front |

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
