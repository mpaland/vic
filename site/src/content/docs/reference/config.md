---
title: Configuration
description: The global compile time options of vic in vic_cfg.h.
sidebar:
  order: 9
---

The global options of vic are plain preprocessor defines in `src/vic_cfg.h`. They size stack buffers and select
font data at compile time, there is no runtime configuration and no heap. Change an option by editing
`vic_cfg.h` in your copy of vic, the defines are not guarded with `#ifndef`, so defining them on the compiler
command line as well leads to a redefinition.

```cpp
// vic_cfg.h
#define VIC_GPR_FILL_STACK_SIZE     128   // more fill segments for complex regions
#define VIC_TXR_VECTOR_MAX_WIDTH    128
#define VIC_FONT_VECTOR_ASCII_ONLY  1     // vector fonts with ASCII chars only, less than half the flash
```

## Options

| Option | Default | Used by | Meaning |
|---|---|---|---|
| `VIC_GPR_FILL_STACK_SIZE` | `64` | [`dc::fill()`](/vic/reference/dc/#fill) | Size in byte of the segment stack of the flood fill. The stack is allocated on the CPU stack for the runtime of `fill()`, one segment takes 8 byte, so the default holds 8 segments. If the fill of complex regions is incomplete, increase this value. |
| `VIC_TXR_VECTOR_MAX_WIDTH` | `128` | [`tc`](/vic/reference/tc/) with a vector font | Maximum width in pixel of a rendered vector font glyph, wider glyphs are clipped. The text renderer allocates a row buffer of `(VIC_TXR_VECTOR_MAX_WIDTH + 1) * 2` byte on the CPU stack. |
| `VIC_FONT_VECTOR_ASCII_ONLY` | `0` | [Vector fonts](/vic/reference/fonts/#vector-fonts) `src/font/*_vec.h` | `1` keeps only the ASCII chars (`0x20` - `0x7E`) of the vector fonts, without the chars beyond ASCII (Latin-1: umlauts, accents, symbols). That is less than half of the font data, e.g. 11 KB instead of 27 KB for `Montserrat_vec`. |

## Widget limits

Some widgets have their own size limits, defined in their headers and documented with the
[widgets](/vic/reference/widgets/). `VIC_WIDGET_LABEL_MAX_LEN` (`widget/label.h`) and `VIC_WIDGET_TEXTBOX_LINE_LEN`
(`widget/textbox.h`) are guarded with `#ifndef` and can be defined before the include or on the command line.
