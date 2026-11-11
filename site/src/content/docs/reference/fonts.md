---
title: Fonts
description: The vic font format, the available bitmap and vector fonts and how to generate new fonts from TrueType files.
sidebar:
  order: 8
---

Fonts are header files in `src/font/` (namespace `vic::font`). A font is a class derived from `font::font_type`
with one `info()` method which returns the glyph data, the structures are defined in `font.h`. A font is selected
with [`tc::set_font()`](/vic/reference/tc/#set_font) and rendered by the text renderer. There are two kinds:
**bitmap fonts** with a fixed size and 1, 2, 4 or 8 bit per pixel, and **vector fonts** with quadratic outlines
which scale to any size, one header serves all sizes.

![Vector font demo](/vic/media/demo_vecfont.gif)

```cpp
#include "tc.h"
#include "font/LCD_6x8.h"
#include "font/Montserrat_12_aa2.h"
#include "font/Montserrat_vec.h"

vic::tc tc(dc);

vic::font::LCD_6x8 lcd;                 // 1 bit monospaced
tc.set_font(lcd);
tc.out({ 0, 0 }, (const std::uint8_t*)"STATUS OK");

vic::font::Montserrat_12_aa2 small;     // 2 bit anti aliased, proportional
tc.set_font(small);
tc.out({ 0, 12 }, (const std::uint8_t*)"Gr\xC3\xBC\xC3\x9F" "e");        // UTF-8 umlauts

vic::font::Montserrat_vec vec;          // one header, any size
tc.set_font(vec, 32U);                  // 32 pixel line height
tc.out({ 0, 30 }, (const std::uint8_t*)"23.5");
```

## Available fonts

### Bitmap fonts

| Class | Header | Type | Height | Bit per pixel | Chars |
|---|---|---|---|---|---|
| `LCD_6x8` | `font/LCD_6x8.h` | Monospaced, 6 x 8 | 8 | 1 | `0x20` - `0xFE` |
| `LCD_6x10` | `font/LCD_6x10.h` | Monospaced, 6 x 10 | 10 | 1 | `0x20` - `0xFE` |
| `LCD_8x8` | `font/LCD_8x8.h` | Monospaced, 8 x 8 | 8 | 1 | `0x20` - `0xFE` |
| `Montserrat_10_aa2` | `font/Montserrat_10_aa2.h` | Proportional, UNICODE | 10 | 2 | ASCII + Latin-1 |
| `Montserrat_12_aa2` | `font/Montserrat_12_aa2.h` | Proportional, UNICODE | 12 | 2 | ASCII + Latin-1 |
| `Montserrat_16_aa4` | `font/Montserrat_16_aa4.h` | Proportional, UNICODE | 16 | 4 | ASCII + Latin-1 |
| `Montserrat_24_aa4` | `font/Montserrat_24_aa4.h` | Proportional, UNICODE | 24 | 4 | ASCII + Latin-1 |
| `Michroma_10_aa2` | `font/Michroma_10_aa2.h` | Proportional, UNICODE | 10 | 2 | ASCII + Latin-1 |
| `Michroma_12_aa2` | `font/Michroma_12_aa2.h` | Proportional, UNICODE | 12 | 2 | ASCII + Latin-1 |
| `Michroma_16_aa4` | `font/Michroma_16_aa4.h` | Proportional, UNICODE | 16 | 4 | ASCII + Latin-1 |
| `Michroma_24_aa4` | `font/Michroma_24_aa4.h` | Proportional, UNICODE | 24 | 4 | ASCII + Latin-1 |
| `Arial_std_12_aa2` | `font/Arial_std_12_aa2.h` | Proportional, UNICODE (legacy) | 12 | 2 | `0x20` - `0x7E`, `0xA3` - `0xAA`, `0xB0` - `0xFF` |
| `Arial_num_16x24_aa4` | `font/Arial_num_16x24_AA4.h` | Proportional, digits 16 x 24 (legacy) | 24 | 4 | `!`, `+` - `:`, `=`, `°`, `±` |

The Montserrat and Michroma bitmap fonts are generated with `tools/fontgen.py` from the TTF files in
`src/font/ttf/` (SIL Open Font License), ASCII + Latin-1 are the chars `0x20` - `0x7E` and `0xA0` - `0xFF` without
the soft hyphen `0xAD`.

### Vector fonts

| Class | Header | Description | Data | ASCII only |
|---|---|---|---|---|
| `Montserrat_vec` | `font/Montserrat_vec.h` | Proportional, ASCII + Latin-1 | 26776 byte | 11308 byte |
| `Michroma_vec` | `font/Michroma_vec.h` | Proportional, ASCII + Latin-1 | 31574 byte | 13464 byte |
| `Doto_vec` | `font/Doto_vec.h` | Dot matrix with round dots, the look of an LED display, ASCII + Latin-1 | 26246 byte | 13364 byte |

All three are generated with `tools/vecfontgen.py` from the OFL fonts in `src/font/ttf/`, `Doto_vec` from the
variable font with `--axes ROND=100,wght=700`. The default size is 16 pixel. The dots of `Doto_vec` are stored as
moves of one dot contour, its dot grid is exactly regular at line heights of multiples of 39 pixel (39 px: 3 px
dots every 4 px).

## Using vector fonts

```cpp
tc.set_font(vec, size);
```

`size` is the line height in pixel (1 to 255), 0 selects the default size of the font. A vector font at size N
has the glyph size of the N pixel bitmap font generated from the same TTF. The glyphs are rasterized at runtime
with exact area coverage, so they are always anti aliased: the coverage becomes the alpha of the text color, which
needs alpha blending (a head which reads back, `dc::set_bg_color()` or a canvas sprite).

**Hinting**: the font data contains hints for the horizontal edges and stems (the bar of a `T`, the crossbar of an
`e`, the baseline, x-height and cap height zones) and for the straight vertical stems (the stems of an `H`). The
renderer snaps them to whole pixel rows and columns and rounds the stem thickness to whole pixels (at least 1), the
points in between are interpolated. So stems, bars and dot rows stay crisp at every size, round sides stay anti
aliased. The cursor keeps the sub pixel part of the advance and every glyph is hinted at its sub pixel position, so
no rounding error adds up along a line.

**Limits**: a glyph is at most `VIC_TXR_VECTOR_MAX_WIDTH` pixels wide (default 128), wider glyphs are clipped.

**ASCII only**: `#define VIC_FONT_VECTOR_ASCII_ONLY 1` in `vic_cfg.h` keeps the ASCII chars `0x20` - `0x7E` of the
vector fonts only, that is less than half of the data (see the table above). See
[Configuration](/vic/reference/config/).

## Font structures

### `font_type`

```cpp
class font_type
{
public:
  virtual const info_type& info(void) const = 0;
};
```

Abstract font base class. A font returns its `info_type`. The font classes have no data members, the glyph data
are `static const` tables in `info()`, so they stay in flash.

### `info_type`

```cpp
typedef struct tag_info_type {
  std::uint8_t attr;              // font attributes
  std::uint8_t ysize;             // y-size to render in pixel
  std::uint8_t ydist;             // y-distance (cursor movement for \n)
  std::uint8_t baseline;          // baseline (ascent) in pixel from the top of the line
  union tag_family {
    const mono_type*     mono;
    const prop_type*     prop;
    const prop_ext_type* prop_ext;
    const vector_type*   vector;
  } family;
} info_type;
```

`ysize` is the line height, for a vector font the default size. `family` points to the glyph data of the font
type selected by `attr`.

### Attributes

| Constant | Value | Meaning |
|---|---|---|
| `AA_MASK` | `0x0F` | Mask of the bit per pixel |
| `AA_NONE` | `0x01` | 1 bpp, no anti aliasing |
| `AA_2` | `0x02` | 2 bpp anti aliased |
| `AA_4` | `0x04` | 4 bpp anti aliased |
| `AA_8` | `0x08` | 8 bpp anti aliased |
| `ENCODING_MASK` | `0x10` | Mask of the encoding |
| `ENCODING_ASCII` | `0x00` | ASCII + ISO 8859, `prop_type` / `mono_type` |
| `ENCODING_UNICODE` | `0x10` | Unicode, `prop_ext_type` |
| `TYPE_MASK` | `0x20` | Mask of the type |
| `TYPE_MONO` | `0x00` | Monospaced font, `mono_type` |
| `TYPE_PROP` | `0x20` | Proportional font |
| `FORMAT_MASK` | `0x40` | Mask of the format |
| `FORMAT_BITMAP` | `0x00` | Bitmap font, fixed size |
| `FORMAT_VECTOR` | `0x40` | Vector (outline) font, scalable, anti aliased, the AA bits are unused |

### Glyph structures

| Structure | Used by | Content |
|---|---|---|
| `mono_type` | Monospaced fonts | `first`, `last` char code, `xsize`, `bytes_per_line`, the glyph bitmaps of all chars |
| `prop_type` | Proportional ASCII fonts | A range `first` - `last` with a `charinfo_type` per char, `next` range |
| `charinfo_type` | `prop_type` | `xsize`, `xdist` (advance), `bytes_per_line`, `data`, the glyph has the font height |
| `prop_ext_type` | Proportional UNICODE fonts | A 16 bit range `first` - `last` with a `charinfo_ext_type` per char, `next` range |
| `charinfo_ext_type` | `prop_ext_type` | `xsize`, `ysize`, `xpos`, `ypos` (glyph box), `xdist` (advance), `data` |
| `vector_type` | Vector fonts | `units` (line height in font units), `ascent` (top of the line box above the baseline), first `block` |
| `vector_block_type` | `vector_type` | A 16 bit range `first` - `last` with a `vector_charinfo_type` per char, `next` block |
| `vector_charinfo_type` | `vector_block_type` | `xdist` (advance in font units), `size` (words of data), `data`: the y hints, the x hints, then the contours |

The bitmap rows start at the most significant bit, a pixel of an anti aliased font is its intensity (0 =
transparent, max = full color). The vector contours are TrueType quadratic outlines in font units, y up, the LSB of
x is the on-curve flag (`x = (x << 1) | on_curve`), a negative point count `-n` means n moved copies of the last
contour.

`font.h` also defines the 1 bpp row macros `________` to `XXXXXXXX` (`0x00` to `0xFF`) for hand drawn fonts.

## Generating fonts

Both generators need Python 3 with `freetype-py` (`pip install freetype-py`). `tools/gen_fonts.cmd` regenerates all
Montserrat, Michroma and vector fonts of the repository.

### Bitmap fonts: `tools/fontgen.py`

Rasterizes a TrueType/OpenType font with FreeType into a proportional UNICODE font header (`prop_ext_type`, 1, 2
or 4 bit per pixel). The glyphs are hinted with the FreeType auto hinter, so a one pixel stem is one solid pixel
column and not two gray ones, stems thinner than a pixel are darkened. The largest FreeType pixel size whose
ascent + descent fits into the line height is used, glyphs the font doesn't contain are skipped.

```
python tools/fontgen.py src/font/ttf/Montserrat-Regular.ttf --size 12 --bpp 2
python tools/fontgen.py font.ttf --size 24 --bpp 4 --ranges 20-7E,A0-FF --name MyFont_24 --out src/font/MyFont_24.h
python tools/fontgen.py font.ttf --size 16 --bpp 4 --preview font16.png
```

| Option | Default | Description |
|---|---|---|
| `ttf` | | TrueType/OpenType font file |
| `--size N` | | Line height in pixel (the `ysize` of the font) |
| `--bpp N` | `2` | Bit per pixel: 1 (mono), 2 or 4 (anti aliased) |
| `--ranges R` | `20-7E,A0-FF` | Hex code point ranges, default ASCII + Latin-1 |
| `--name NAME` | `<Family>_<size>_aa<bpp>` | Class name |
| `--out FILE` | `src/font/<name>.h` | Output header |
| `--darken F` | `auto` | Stem darkening: `auto` (stems become solid pixels), `1` = off, or a factor like `1.5` |
| `--preview FILE` | | Writes a PNG preview sheet of the generated glyphs (needs Pillow) |
| `--selftest` | | Runs the pack/unpack self test |

### Vector fonts: `tools/vecfontgen.py`

Extracts the quadratic TrueType outlines (unscaled font units) and the autohinting edges and stems into a vector
font header. Composite glyphs (accented chars) are stored decomposed. CFF (cubic) OpenType fonts are not supported,
use the TrueType version. The line box is the ink box of the whole charset, like `fontgen.py` does.

```
python tools/vecfontgen.py src/font/ttf/Montserrat-Regular.ttf
python tools/vecfontgen.py font.ttf --ranges 20-7E --name MyFont_vec --out src/font/MyFont_vec.h
python tools/vecfontgen.py src/font/ttf/Doto-Variable.ttf --axes ROND=100,wght=700
```

| Option | Default | Description |
|---|---|---|
| `ttf` | | TrueType font file (quadratic outlines) |
| `--ranges R` | `20-7E,A0-FF` | Hex code point ranges, default ASCII + Latin-1 |
| `--size N` | `16` | Default line height in pixel, used by `set_font()` without a size |
| `--name NAME` | `<Family>_vec` | Class name |
| `--out FILE` | `src/font/<name>.h` | Output header |
| `--axes AXES` | | Instance of a variable font in design coordinates, e.g. `ROND=100,wght=700` |
| `--no-hints` | | Doesn't generate the vertical hints |
| `--selftest` | | Runs the encode/decode self test |

## Flash size

Rule of thumb for a bitmap font: about `glyphs * (height * width * bpp / 8 + 8)` byte. 190 glyphs of a 12 pixel
2 bit font take about 5 KB, a 24 pixel 4 bit font about 30 KB. Restrict the ranges (`--ranges 30-39` for the
digits) if a display only shows numbers. A vector font takes 26 to 32 KB for ASCII + Latin-1 and 11 to 14 KB for
ASCII only, for all sizes together, about as much as one 24 pixel 4 bit bitmap font.
