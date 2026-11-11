---
title: Text context
description: vic::tc, the text context for UTF-8 text with bitmap and vector fonts on graphic and alpha numeric heads.
sidebar:
  order: 2
---

`vic::tc` (`tc.h`) is the text context. On a graphic display it is created on a drawing context and renders UTF-8
strings with the selected font through the [`dc`](/vic/reference/dc/), so text takes the shader pipe and alpha
blending like any other pixel. On an alpha numeric display (character LCD, VFD, seven segment) it is created on the
head and forwards the text to the head, which renders it with its own character set. The glyph rendering comes from
the base class `vic::txr` (`txr.h`), the text renderer, which supports monospaced and proportional bitmap fonts with
1, 2, 4 or 8 bit per pixel and scalable vector fonts.

```cpp
#include "tc.h"
#include "font/Montserrat_16_aa4.h"
#include "font/Michroma_vec.h"

vic::dc dc(head);
vic::tc tc(dc);                              // text on a graphic head through the dc

vic::font::Montserrat_16_aa4 font16;
tc.set_font(font16);
tc.set_color(vic::color::white);
tc.out({ 10, 10 }, (const std::uint8_t*)"Temperature\n\aFFA500" "23.5 \xC2\xB0" "C");   // 2 lines, orange value

vic::font::Michroma_vec vec;
tc.set_font(vec, 40U);                       // vector font, 40 pixel line height
vic::rect_type ext;
tc.get_extend(ext, (const std::uint8_t*)"88:88");
tc.out({ static_cast<std::int16_t>((tc.screen_width() - ext.width()) / 2), 60 }, (const std::uint8_t*)"12:34");
```

## Construction

### `tc()`

```cpp
tc(drv& head);
tc(dc& _dc);
```

Creates a text context. `tc(dc&)` is the text context for a graphic head, the text is rendered with the selected
font through the drawing context. `tc(drv&)` is the text context for an alpha numeric head, the output, position,
inverse mode and line clearing are forwarded to the text functions of the head, the font and the color of the
context are not used.

| Parameter | Description |
|---|---|
| `head` | Alpha numeric head |
| `_dc` | Drawing context of a graphic head |

## Font and color

### `set_font()`

```cpp
void set_font(const font::font_type& font, std::uint8_t size = 0U);
```

Selects the font. For a vector font `size` is the line height in pixel, 0 selects the default size of the font
(16 for the stock vector fonts). Bitmap fonts ignore `size`. The font object must live as long as it is selected.
See [Fonts](/vic/reference/fonts/).

| Parameter | Description |
|---|---|
| `font` | Font to use |
| `size` | Line height in pixel of a vector font, 0 for the default size, ignored by bitmap fonts |

### `set_color()`

```cpp
void set_color(color::value_type color);
```

Sets the text color, default is `color::white`. Anti aliased glyph pixels get the color with an alpha of
`color alpha * pixel intensity / 255`, so they need alpha blending (a head which reads back or
[`dc::set_bg_color()`](/vic/reference/dc/#set_bg_color)). A string can change the color inline with `\aRRGGBB`.

### `char_width()`

```cpp
std::uint8_t char_width() const;
```

Returns the average char width of the selected font in pixel: the advance of `'A'` (or of the first char if the
font has no `'A'`), the char width of a monospaced font. For a vector font the advance is scaled to the selected
size. Returns 0 if no font is selected.

### `char_height()`

```cpp
std::uint8_t char_height() const;
```

Returns the line height of the selected font in pixel (the selected size of a vector font), 0 if no font is
selected.

## Output

### `set_pos()`

```cpp
void set_pos(vertex_type pos);
```

Sets the text cursor. On a graphic head `pos` is the left/top pixel of the line box, on an alpha numeric head it is
the character cell (0/0 is left/top). The x position is also the start of every new line after `\n` and `\r`.

### `out()`

```cpp
void out(std::uint16_t ch);
std::uint16_t out(const std::uint8_t* string);
std::uint16_t out(vertex_type pos, const std::uint8_t* string);
```

`out(ch)` renders a single char at the cursor and moves the cursor by its advance. `ch` is a 16 bit
ASCII/UNICODE code point (not UTF-8), chars below `0x20` and chars the font doesn't contain are skipped. It doesn't
present.

`out(string)` renders a zero terminated UTF-8 (or ASCII) string at the cursor and presents. Control chars:

| Char | Effect |
|---|---|
| `\n` (`0x0A`) | New line: x back to the `set_pos()` x, y down by the line height |
| `\r` (`0x0D`) | x back to the `set_pos()` x |
| `\aRRGGBB` (`0x07` and six hex digits) | Sets the text color to the opaque color `0xRRGGBB`, only if all six hex digits are present |

`out(pos, string)` combines `set_pos()` and `out(string)`. The string functions return the number of chars, not
bytes, the control chars are counted.

On an alpha numeric head the chars and strings go to the head, which handles the control chars itself.

| Parameter | Description |
|---|---|
| `ch` | Character in 16 bit ASCII/UNICODE format |
| `string` | Zero terminated string in UTF-8/ASCII format |
| `pos` | Position, pixel on graphic heads, character cell on alpha numeric heads |

:::tip
UTF-8 chars written as hex escapes must end their string literal if a hex digit follows:
`"23.5 \xC2\xB0" "C"`, otherwise the compiler reads `\xB0C` as one escape sequence.
:::

### `get_extend()`

```cpp
std::uint16_t get_extend(rect_type& rect, const std::uint8_t* string) const;
```

Returns the size the string would take, without rendering it. `rect` is set to `{ 0, 0, width, height }`, so
`rect.width()` is the advance of the string and `rect.height()` the line height. Color escapes, `\n` and `\r`
have no extent, the string is measured as one line. Returns the number of rendered chars. On an alpha numeric head
every char takes one cell, the height is 1.

| Parameter | Description |
|---|---|
| `rect` | Receives the extent of the string |
| `string` | Zero terminated string in UTF-8 format |

### `set_inverse()`

```cpp
void set_inverse(bool inverse = true);
```

Sets inverse video mode. Only supported on alpha numeric heads.

### `clear_eol()`

```cpp
void clear_eol();
```

Clears the actual line from the cursor position to the end of the line. Only supported on alpha numeric heads.

### `clear_sol()`

```cpp
void clear_sol();
```

Clears the actual line from the start to the cursor position. Only supported on alpha numeric heads.

### `clear_line()`

```cpp
void clear_line();
```

Clears the actual line. Only supported on alpha numeric heads.

## Screen

### `cls()`

```cpp
void cls(color::value_type bg_color = color::none);
```

Clears the screen: sets all pixels off, deletes all characters or fills the screen with the background color.

### `present()`

```cpp
void present();
```

Transfers the rendered text to the screen (`present()` of the dc or the head). Does nothing while the
`present_lock()` of the text context is engaged.

### `present_lock()`

```cpp
void present_lock(bool lock = true);
```

Prevents the `present()` after every string. Call `present_lock(false)` after all output, it releases the lock and
presents. The lock is nested and independent of the lock of the drawing context.

### `screen_width()`

```cpp
std::uint16_t screen_width() const;
```

Returns the screen width, in pixel on graphic heads, in chars on alpha numeric heads.

### `screen_height()`

```cpp
std::uint16_t screen_height() const;
```

Returns the screen height, in pixel on graphic heads, in chars on alpha numeric heads.

## Vector font rendering

Vector fonts are rasterized at runtime: the quadratic outlines are scaled to 1/64 pixel, every pixel row gets the
exact covered area (nonzero winding) and the coverage becomes the alpha of the text color. No float, no heap, one
row buffer of `(VIC_TXR_VECTOR_MAX_WIDTH + 1) * 2` byte on the stack, wider glyphs are clipped. Horizontal and
vertical stems are hinted to whole pixels and the cursor keeps the sub pixel part of the advance, so no rounding
error adds up along a line. See [Fonts](/vic/reference/fonts/) and [Configuration](/vic/reference/config/).
