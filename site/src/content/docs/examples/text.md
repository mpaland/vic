---
title: Text and fonts
description: Bitmap and vector fonts, inline color escapes, text measurement and scalable vector font sizes.
sidebar:
  order: 2
---

Text is written with a text context (`vic::tc`) on top of a drawing context. The fonts are classes in `src/font/`, one
header per font: monospace LCD fonts, anti aliased proportional fonts with 2 or 4 bit per pixel and scalable vector
fonts.

![Vector font demo](/vic/media/demo_vecfont.gif)

## Bitmap fonts

Condensed from `demo_font.cpp`, which cycles through all bitmap fonts:

```cpp
#include "dc.h"
#include "tc.h"
#include "font/LCD_6x8.h"
#include "font/Montserrat_12_aa2.h"

vic::font::LCD_6x8           lcd;
vic::font::Montserrat_12_aa2 montserrat;

vic::dc _dc(_head);
vic::tc _tc(_dc);                                   // text on a graphic head goes through a dc

_dc.present_lock();
_tc.set_font(montserrat);
_tc.set_color(vic::color::white);
_tc.out({ 8, 3 }, reinterpret_cast<const std::uint8_t*>("vic \aFFD040font\aFFFFFF demo"));

// right aligned text: measure first, then render
const char* counter = "01/12";
vic::rect_type r;
_tc.get_extend(r, reinterpret_cast<const std::uint8_t*>(counter));
_tc.out({ static_cast<std::int16_t>(320 - 8 - r.right), 3 }, reinterpret_cast<const std::uint8_t*>(counter));

// UTF-8 Latin-1 characters
_tc.set_font(lcd);
_tc.out({ 8, 20 }, reinterpret_cast<const std::uint8_t*>("\xC3\x84rger \xC3\x96l \xC2\xB0" "C"));   // Ärger Öl °C
_dc.present_lock(false);
```

Key points:

- `tc(dc&)` renders on a graphic head, `tc(drv&)` is for alpha numeric heads (character displays).
- `out(pos, utf8)` sets the position (left/top in pixel) and renders a zero terminated UTF-8 string. `\n` and `\r`
  are handled, the return value is the number of characters.
- `"\aRRGGBB"` switches the text color inline, e.g. `"\aFF6060red \a60FF60green"`. Split string literals where a hex
  digit follows a `\x` escape (`"\xC2\xB0" "C"`).
- `get_extend(rect, utf8)` returns the size of a line without rendering it, `char_height()` the line height of the
  selected font. `demo_font.cpp` uses both for its word wrap; the [textbox](/vic/reference/widgets/textbox/) widget does
  this for you.
- Anti aliased fonts blend their edge pixels with the background, so they need a head which can read back its pixels
  (or a framebuffer). The demo draws a gradient below the text to show the blending.

## Vector fonts

A vector font (`Montserrat_vec`, `Michroma_vec`, `Doto_vec`) is one header for every size. The size is the line height
in pixel, given as second parameter of `set_font()`. Condensed from `demo_vecfont.cpp`:

```cpp
#include "dc.h"
#include "tc.h"
#include "font/Montserrat_vec.h"

vic::font::Montserrat_vec montserrat;

vic::dc _dc(_head);
vic::tc _tc(_dc);
_tc.set_color(vic::color::white);

// waterfall: one font, every size
static const std::uint8_t sizes[] = { 9U, 12U, 16U, 22U, 30U, 44U };
std::int16_t y = 26;
_dc.present_lock();
for (std::size_t n = 0U; n < sizeof(sizes); ++n) {
  _tc.set_font(montserrat, sizes[n]);               // line height in pixel, 0 selects the default size
  _tc.out({ 8, y }, reinterpret_cast<const std::uint8_t*>("The quick brown fox jumps over the lazy dog"));
  y = static_cast<std::int16_t>(y + sizes[n] + 2);
}
_dc.present_lock(false);

// largest size which fits into 304 pixel
const std::uint8_t* word = reinterpret_cast<const std::uint8_t*>("Scal\aFFD040able");
std::uint8_t max_size = 8U;
for (std::uint8_t s = 8U; s < 160U; ++s) {
  vic::rect_type r;
  _tc.set_font(montserrat, s);
  _tc.get_extend(r, word);
  if (r.right > 304) {
    break;
  }
  max_size = s;
}
```

Key points:

- The size parameter is ignored by bitmap fonts. `set_font(font)` without size selects the default size of the font.
- Every size is rendered from the same outlines, the demo zooms a word smoothly from 8 pixel to `max_size`, one pixel
  size per frame.
- `VIC_FONT_VECTOR_ASCII_ONLY` in `vic_cfg.h` keeps only the ASCII glyphs of the vector fonts to save flash, see
  [Configuration](/vic/reference/config/).

See [Text context](/vic/reference/tc/) and [Fonts](/vic/reference/fonts/) for details.
