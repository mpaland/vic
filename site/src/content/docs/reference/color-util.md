---
title: Colors and helpers
description: The vic::color functions, formats and named colors and the vertex, pixel, rect types and integer helpers of util.h.
sidebar:
  order: 7
---

`color.h` (namespace `vic::color`) defines the internal color type, the color formats of heads and bitmaps, color
assembly, blending and conversion functions and the named stock colors. `util.h` defines the basic geometry types
`vertex_type`, `pixel_type` and `rect_type` (namespace `vic`) and fast integer helpers (namespace `vic::util`).
No function uses floating point math.

```cpp
#include "color.h"

vic::color::value_type c = vic::color::argb(0x20, 0x80, 0xFF);    // opaque light blue
vic::color::value_type t = vic::color::set_alpha(c, 128U);          // half transparent
vic::color::value_type h = vic::color::hsv(120U, 255U, 200U);       // green from HSV
vic::color::value_type m = vic::color::mix(c, vic::color::white, 64U);   // 25 % c, 75 % white
std::uint16_t rgb565     = vic::color::color_to_RGB565(h);           // native head value

vic::vertex_type p = vic::util::vertex_rotate({ 60, 10 }, { 50, 50 }, 30);   // 30 degree clockwise
std::int16_t x = static_cast<std::int16_t>(50 + 40 * vic::util::cos(45) / 16384);
```

## Color type

### `value_type`

```cpp
typedef std::uint32_t value_type;
```

The internal color of vic: 32 bit ARGB8888, `0xAARRGGBB`. Alpha 255 is opaque, 0 is transparent.

```
|   A   |   R   |   G   |   B   |
 31   24 23   16 15    8 7     0
```

### `format_type`

```cpp
typedef enum tag_format_type : std::uint8_t { ... } format_type;
```

The color formats of heads, bitmaps and sprite sheets. The upper nibble of the value is the number of bytes per
pixel.

| Format | Value | Description |
|---|---|---|
| `format_L1` | `0x10` | 1 bit luminance (monochrome), like XBM |
| `format_L2` | `0x11` | 2 bit grayscale |
| `format_L4` | `0x12` | 4 bit grayscale |
| `format_L8` | `0x13` | 8 bit, 256 gray levels (0 = black, 255 = white) |
| `format_C8` | `0x14` | 8 bit color lookup table, reserved, not supported yet |
| `format_C16` | `0x20` | 16 bit color lookup table, reserved, not supported yet |
| `format_RGB332` | `0x15` | 8 bit RGB |
| `format_RGB444` | `0x21` | 12 bit RGB |
| `format_RGB555` | `0x22` | 15 bit RGB |
| `format_RGB565` | `0x23` | 16 bit RGB |
| `format_RGB666` | `0x30` | 18 bit RGB |
| `format_RGB888` | `0x31` | 24 bit RGB, no alpha |
| `format_ARGB4444` | `0x24` | 16 bit ARGB, 4 bit alpha |
| `format_ARGB6666` | `0x32` | 18 bit ARGB, 6 bit alpha |
| `format_ARGB8888` | `0x40` | 32 bit ARGB, 8 bit alpha, the internal format of vic |
| `format_RGBA8888` | `0x41` | 32 bit RGBA, 8 bit alpha, alpha in the least significant byte |

## Color assembly

### `argb()`

```cpp
value_type argb(std::uint8_t _red, std::uint8_t _green, std::uint8_t _blue, std::uint8_t _alpha = 255U);
```

Returns the ARGB color of the red, green, blue and alpha components. Alpha 0 is transparent, 255 (default) opaque.

### `rgb_static`

```cpp
template <std::uint8_t Red, std::uint8_t Green, std::uint8_t Blue, std::uint8_t Alpha = 255U>
struct rgb_static {
  static const value_type value;
};
```

Compile time color assembly: `vic::color::rgb_static<0x20, 0x80, 0xFF>::value`.

### `hsv()`

```cpp
value_type hsv(std::uint16_t hue, std::uint8_t saturation, std::uint8_t value, std::uint8_t alpha = 255U);
```

Returns the ARGB color of HSV components.

| Parameter | Description |
|---|---|
| `hue` | Hue in degree, 0 red, 120 green, 240 blue, taken modulo 360 |
| `saturation` | Saturation, 0 (gray) to 255 (full color) |
| `value` | Value (brightness), 0 to 255 |
| `alpha` | Alpha, 0 transparent to 255 opaque (default) |

## Channel access

### `get_alpha()`, `get_red()`, `get_green()`, `get_blue()`

```cpp
std::uint8_t get_alpha(value_type color);
std::uint8_t get_red  (value_type color);
std::uint8_t get_green(value_type color);
std::uint8_t get_blue (value_type color);
```

Return a single channel of an ARGB color.

### `set_alpha()`, `set_red()`, `set_green()`, `set_blue()`

```cpp
value_type set_alpha(value_type color, std::uint8_t _alpha);
value_type set_red  (value_type color, std::uint8_t _red);
value_type set_green(value_type color, std::uint8_t _green);
value_type set_blue (value_type color, std::uint8_t _blue);
```

Return the color with one channel replaced.

### `is_opaque()`

```cpp
bool is_opaque(value_type color);
```

Returns `true` if the alpha of the color is 255.

## Dimming, mixing and blending

### `dim()`

```cpp
value_type dim(value_type color, std::uint8_t lum);
value_type dim_25(value_type color);
value_type dim_50(value_type color);
value_type dim_75(value_type color);
```

Dims the RGB part of a color, the alpha channel is unaffected. `lum` 0 is dark, 255 leaves the color unchanged.
`dim_25()`, `dim_50()` and `dim_75()` are fast shift variants which dim to 25 %, 50 % and 75 %.

### `mix()`

```cpp
value_type mix(value_type foreground, value_type background, std::uint8_t lum);
value_type mix_25(value_type foreground, value_type background);
value_type mix_50(value_type foreground, value_type background);
value_type mix_75(value_type foreground, value_type background);
```

Mixes the RGB parts of two colors, the alpha of the foreground color is taken. `lum` 255 is 100 % foreground,
0 is 100 % background. `mix_25()`, `mix_50()` and `mix_75()` mix 25 %, 50 % and 75 % foreground.

### `alpha_blend()`

```cpp
value_type alpha_blend(value_type front_color, value_type back_color);
```

Combines two colors by their alpha channels with Porter-Duff composition (front over back):
`C = (aF * F + (255 - aF) * aB * B) / (aF + (255 - aF) * aB)`. An opaque front color or a transparent back color
returns the front color, a transparent front color returns the back color. The result alpha is the combined alpha.

| Parameter | Description |
|---|---|
| `front_color` | Color over `back_color` |
| `back_color` | Color below `front_color` |

### `argb_to_rgb()`

```cpp
value_type argb_to_rgb(value_type front_color, value_type opaque_background);
```

Converts an ARGB color to an opaque color over an opaque background color (the alpha of the background doesn't
matter). Used to reduce the internal ARGB color to a discrete display color. The result alpha is 255.

## Format conversion

### `color_to_*()`

```cpp
std::uint8_t  color_to_L1    (value_type color);
std::uint8_t  color_to_L2    (value_type color);
std::uint8_t  color_to_L4    (value_type color);
std::uint8_t  color_to_L8    (value_type color);
std::uint8_t  color_to_RGB332(value_type color);
std::uint16_t color_to_RGB444(value_type color);
std::uint16_t color_to_RGB555(value_type color);
std::uint16_t color_to_RGB565(value_type color);
std::uint32_t color_to_RGB666(value_type color);
std::uint32_t color_to_RGB888(value_type color);
```

Convert the internal ARGB color to a native head or bitmap value, the alpha channel is dropped. `L1` is 1 for any
color which is not black, `L2`, `L4` and `L8` are the average of R, G and B. The bit layouts are `RRRGGGBB`,
`RRRRGGGGBBBB`, `0RRRRRGGGGGBBBBB`, `RRRRRGGGGGGBBBBB`, `RRRRRRGGGGGGBBBBBB` and `RRRRRRRRGGGGGGGGBBBBBBBB`.

### `*_to_color()`

```cpp
value_type L1_to_color    (std::uint8_t  head_color);
value_type L2_to_color    (std::uint8_t  head_color);
value_type L4_to_color    (std::uint8_t  head_color);
value_type L8_to_color    (std::uint8_t  head_color);
value_type RGB332_to_color(std::uint8_t  head_color);
value_type RGB444_to_color(std::uint16_t head_color);
value_type RGB555_to_color(std::uint16_t head_color);
value_type RGB565_to_color(std::uint16_t head_color);
value_type RGB666_to_color(std::uint32_t head_color);
value_type RGB888_to_color(std::uint32_t head_color);
```

Convert a native head or bitmap value to an opaque ARGB color. `L1_to_color()` returns `color::white` for any
value other than 0, else `color::black`. The gray levels become gray tones from black to white.

## Named colors

All named colors are opaque except `none`.

| Name | Value | Name | Value |
|---|---|---|---|
| `none`, `transparent` | `0x00000000` | `red` | `0xFFFF0000` |
| `black` | `0xFF000000` | `green` | `0xFF00FF00` |
| `gray01` | `0xFF101010` | `blue` | `0xFF0000FF` |
| `gray02` | `0xFF202020` | `cyan` | `0xFF00FFFF` |
| `gray03` | `0xFF303030` | `magenta` | `0xFFFF00FF` |
| `gray04` | `0xFF404040` | `yellow` | `0xFFFFFF00` |
| `gray05` | `0xFF505050` | `orange` | `0xFFFFA500` |
| `gray06` | `0xFF606060` | `brown` | `0xFFA52A2A` |
| `gray07` | `0xFF707070` | `pink` | `0xFFFFC0CB` |
| `gray`, `gray08` | `0xFF808080` | `salmon` | `0xFFFA8072` |
| `gray09` | `0xFF909090` | `darkred` | `0xFF800000` |
| `gray10` | `0xFFA0A0A0` | `darkgreen` | `0xFF008000` |
| `gray11` | `0xFFB0B0B0` | `darkblue`, `navy` | `0xFF000080` |
| `gray12` | `0xFFC0C0C0` | `darkcyan` | `0xFF008080` |
| `gray13` | `0xFFD0D0D0` | `darkmagenta` | `0xFF800080` |
| `gray14` | `0xFFE0E0E0` | `darkyellow` | `0xFF808000` |
| `gray15` | `0xFFF0F0F0` | `darkorange` | `0xFFD2691E` |
| `white` | `0xFFFFFFFF` | `darkpink` | `0xFFFF1493` |
| | | `darksalmon` | `0xFFE9967A` |
| | | `darkviolet` | `0xFF9400D3` |
| | | `darkgray` | `0xFFA9A9A9` |

## Geometry types

### `vertex_type`

```cpp
typedef struct tag_vertex_type {
  std::int16_t x;
  std::int16_t y;
} vertex_type;
```

A point (x, y coordinates). Operators: `==`, `!=`, `<` (row major: y first, then x), `+`, `-`, `+=`, `-=`.

| Function | Description |
|---|---|
| `void swap(tag_vertex_type& v2)` | Swaps this vertex and `v2` |
| `void min_x(tag_vertex_type& max_x)` | Swaps the two vertices if necessary, so that this one has the smaller x |
| `void min_y(tag_vertex_type& max_y)` | Swaps the two vertices if necessary, so that this one has the smaller y |

### `pixel_type`

```cpp
typedef struct tag_pixel_type {
  vertex_type   vertex;
  std::uint32_t color;    // ARGB 8-8-8-8 color
} pixel_type;
```

A vertex with a color, e.g. the color points of the gradient shaders. `==` and `!=` compare the vertex only,
`+`, `-` and `+=` move the vertex by a `vertex_type`.

### `rect_type`

```cpp
typedef struct tag_rect_type {
  std::int16_t left;
  std::int16_t top;
  std::int16_t right;
  std::int16_t bottom;
} rect_type;
```

A rectangle. Left and top are inside, **right and bottom are outside** the rect. The rect must be normalized:
`left <= right` and `top <= bottom`. Operators: `==`, `!=`.

| Function | Description |
|---|---|
| `void clear()` | Sets the rect to `{ 0, 0, 0, 0 }` |
| `void normalize(const vertex_type& v1, const vertex_type& v2)` | Sets the rect so that both vertices are inside, right and bottom are limited to 32767 |
| `vertex_type& top_left()` | The left/top corner as vertex (also `const`) |
| `vertex_type& bottom_right()` | The right/bottom corner as vertex, outside the rect (also `const`) |
| `std::int16_t width() const` | `right - left` |
| `std::int16_t height() const` | `bottom - top` |
| `bool empty() const` | `true` if the width or the height is 0 |
| `bool contain(const vertex_type& vertex) const` | `true` if the vertex is inside the rect |
| `void inflate(const vertex_type& vertex)` | Grows the rect so that the vertex is inside |

## Integer helpers

The functions of the namespace `vic::util`.

### `min2()`, `max2()`, `min3()`, `max3()`

```cpp
template <typename T> const T& min2(const T& a, const T& b);
template <typename T> T& max2(T& a, T& b);
template <typename T> const T& min3(const T& a, const T& b, const T& c);
template <typename T> const T& max3(const T& a, const T& b, const T& c);
```

Return the minimum or maximum of two or three values.

### `min_max_swap()`

```cpp
template<typename T> void min_max_swap(T& minimum, T& maximum);
```

Swaps the two values if necessary, so that `minimum` holds the smaller one.

### `abs()`

```cpp
template<typename T> T abs(T val);
```

Returns the absolute value of a signed integer (branch free).

### `isqrt()`

```cpp
std::uint32_t isqrt(std::uint32_t value);
```

Integer square root, returns `floor(sqrt(value))`.

### `sin()`, `cos()`

```cpp
std::int16_t sin(std::int16_t angle);
std::int16_t cos(std::int16_t angle);
```

Fast table lookup of the sine and cosine of an angle in degree (any value, also negative), normalized to 16384:
the result is `sin(angle) * 16384`, so `r * util::sin(a) / 16384` is the scaled value.

### `vertex_rotate()`

```cpp
vertex_type vertex_rotate(const vertex_type& point, const vertex_type& center, std::int16_t angle);
vertex_type vertex_rotate(const vertex_type& point, const vertex_type& center, std::int32_t sin_value, std::int32_t cos_value);
```

Rotates a vertex around a center and returns the rotated vertex, rounded to the closest pixel. The angle is in
degree, math positive, which is **clockwise** on the screen. Use the variant with precomputed `util::sin()` and
`util::cos()` values to rotate many vertices by the same angle.

### `distance_squared()`

```cpp
std::uint32_t distance_squared(const vertex_type& v0, const vertex_type& v1);
```

Returns the squared distance between two vertices, use `isqrt()` for the distance.

### `orient_2d()`

```cpp
std::int32_t orient_2d(const vertex_type& p, const vertex_type& v0, const vertex_type& v1);
```

Returns the relation of the vertex `p` to the line `v0`-`v1`: > 0 if `p` is on the "right" side, 0 if `p` is on the
line, < 0 if `p` is on the "left" side.

### `div_round_closest()`

```cpp
std::int32_t div_round_closest(std::int32_t data, std::int16_t divisor);
```

Integer division rounded to the closest value, e.g. `8 / 3 = 3`.

### `div255()`

```cpp
std::uint16_t div255(std::uint16_t data);
```

Fast division by 255.

### `byte_reverse()`

```cpp
std::uint8_t byte_reverse(std::uint8_t data);
```

Reverses the bit order of a byte (`0x80` becomes `0x01`).

### `utf8_decode()`

```cpp
std::uint16_t utf8_decode(const std::uint8_t*& string);
```

Decodes one UTF-8 sequence (1 to 3 bytes, Basic Multilingual Plane) and advances the string pointer past it.
Returns the code point, 0 at the end of the string (the pointer stays on the terminating NUL). The function never
reads past the NUL, invalid lead bytes, stray continuation bytes and 4 byte sequences are skipped byte wise.
