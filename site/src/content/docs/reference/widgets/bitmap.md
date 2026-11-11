---
title: bitmap
description: Renders raw bitmaps in several color formats and XPM3 images.
sidebar:
  order: 3
---

`bitmap` renders raw bitmaps (L1, RGB332, RGB555, RGB565, RGB888 and ARGB8888) and XPM3 images. The raw data is read
byte wise with the least significant byte first, so there are no alignment requirements. `imagebutton` uses it to
render its icon.

![Widget demo](/vic/media/demo_widget.gif)

## Header

```cpp
#include "widget/bitmap.h"
```

## Example

```cpp
#include "widget/bitmap.h"

// 8 x 8 XPM3 icon: "width height colors chars_per_pixel", the color table, the rows
static const char* xpm_arrow[] = {
  "8 8 2 1", "  c None", "x c #40E0FF",
  "   xx   ", "  xxxx  ", " xxxxxx ", "xxxxxxxx",
  "   xx   ", "   xx   ", "   xx   ", "   xx   "
};

// 8 x 2 monochrome L1 pattern, LSB first, every row starts with a new byte
static const std::uint8_t l1_pattern[] = { 0x55U, 0xAAU };

vic::widget::bitmap bmp(_head);
bmp.init();                                                       // unused, does nothing
bmp.render_xpm({ 10, 10 }, xpm_arrow);
bmp.set_color(vic::color::yellow);                                // foreground color of L1 bitmaps
bmp.render_raw({ 30, 10 }, 8U, 2U, vic::color::format_L1, l1_pattern);
```

## Configuration

`bitmap` has no `config_type`, the position and the data are given with every render call. The XPM decoder is
configured in `widget/bitmap.h`:

| Define | Default | Meaning |
|---|---|---|
| `VIC_WIDGET_BITMAP_XPM_MAX_CHAR_ON_PIXEL` | `4U` | Maximum chars per XPM pixel stored in the color lookup table, one CLUT entry takes 8 byte. Images with more chars per pixel are decoded without the CLUT |
| `VIC_WIDGET_BITMAP_XPM_CLUT_SIZE` | `16U` | Entries of the XPM color lookup table, 16 entries take 128 byte |

## Member functions

### Constructor

```cpp
bitmap(drv& head);
```

| Parameter | Description |
|---|---|
| `head` | Reference to the head driver |

### init

```cpp
virtual void init() final;
```

Unused, does nothing. The bitmap is drawn by the render functions.

### set_color

```cpp
void set_color(color::value_type color);
```

Set the foreground color for monochrome (L1) bitmaps, default is gray.

| Parameter | Description |
|---|---|
| `color` | Foreground color |

### render_raw

```cpp
void render_raw(vertex_type origin, std::uint16_t width, std::uint16_t height, color::format_type format, const std::uint8_t* data_ptr);
```

Render the bitmap from the given raw color format.

| Parameter | Description |
|---|---|
| `origin` | Origin (left/top) where the bitmap is rendered |
| `width` | Width of the bitmap |
| `height` | Height of the bitmap |
| `format` | Data format, see below |
| `data_ptr` | Bitmap data in the given raw format |

| Format | Data |
|---|---|
| `color::format_L1` | Monochrome, XBM like, LSB first, rows are byte aligned. Set bits are drawn in the `set_color()` color, clear bits are transparent |
| `color::format_RGB332` | One byte per pixel |
| `color::format_RGB555` | Two bytes per pixel, LSB first |
| `color::format_RGB565` | Two bytes per pixel, LSB first |
| `color::format_RGB888` | Three bytes per pixel in the order B G R |
| `color::format_ARGB8888` | Four bytes per pixel in the order B G R A, transferred with `blit()` in row chunks of up to 32 pixel |

Other formats are not supported, rendering stops at the first pixel.

### render_xpm

```cpp
void render_xpm(vertex_type origin, const char* xpm3_image[]);
```

Render the bitmap from the XPM3 format. Colors are given as `#RRGGBB`, `None` is transparent. Named colors are not
supported and render transparent.

| Parameter | Description |
|---|---|
| `origin` | Origin (left/top) where the bitmap is rendered |
| `xpm3_image` | Image data in XPM3 format, like `{ "4 2 2 1", "a c #FF0000", "  c None", "aa  ", " aa " }` |

## Events

The hit area is the rect of the last rendered bitmap. `bitmap` has no own event reaction, an event handler set with
`set_event_handler()` is called. See [input routing](/vic/reference/widgets/#input-routing).
