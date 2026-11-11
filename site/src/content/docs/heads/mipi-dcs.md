---
title: MIPI_DCS
description: Generic head for the MIPI DCS TFT controllers ILI9341, ILI9488, ST7735, ST7789, ST7796, GC9A01 and JD9853.
sidebar:
  label: "MIPI DCS (ILI9341, ST7789, ...)"
  order: 15
---

One head for the SPI / parallel TFT controllers which share the MIPI Display Command Set. They differ only in
their vendor init sequence, the panel size, the GRAM offset of the panel and the pixel format on SPI, so the
controller is a template parameter. The head writes directly into the GRAM through the column / row address window
(2Ah / 2Bh / 2Ch) with native auto increment, there is no framebuffer.

```cpp
#include "head/MIPI_DCS.h"
```

| Controller | `MIPI_DCS_controller_type` | GRAM | Typical panels |
|---|---|---|---|
| ILI9341 | `MIPI_DCS_ILI9341` | 240 x 320 | 2.2" .. 3.2" 240 x 320 |
| ILI9488 | `MIPI_DCS_ILI9488` | 320 x 480 | 3.5" .. 4" 320 x 480, 18 bit pixels on SPI |
| ST7735 | `MIPI_DCS_ST7735` | 132 x 162 | 1.8" 128 x 160, 1.44" 128 x 128, 0.96" 80 x 160 (with offsets) |
| ST7789 | `MIPI_DCS_ST7789` | 240 x 320 | 2" 240 x 320, 1.3" / 1.54" 240 x 240, 1.14" 135 x 240 (with offsets) |
| ST7796 | `MIPI_DCS_ST7796` | 320 x 480 | 3.5" / 4" 320 x 480 |
| GC9A01 | `MIPI_DCS_GC9A01` | 240 x 240 | 1.28" round 240 x 240 |
| JD9853 | `MIPI_DCS_JD9853` | 240 x 320 | 2" 240 x 320, 1.47" 172 x 320 (offset 34), vendor registers from the OTP |

## Template parameters

```cpp
template<MIPI_DCS_controller_type Controller,
         std::uint16_t Screen_Size_X, std::uint16_t Screen_Size_Y,
         drv::orientation_type Orientation = drv::orientation_0,
         bool RGB_to_BGR  = (Controller != MIPI_DCS_ST7789),
         bool Invert      = (Controller == MIPI_DCS_ST7789) || (Controller == MIPI_DCS_GC9A01),
         bool Color_18bit = (Controller == MIPI_DCS_ILI9488),
         std::uint16_t Offset_X = 0U, std::uint16_t Offset_Y = 0U>
class MIPI_DCS;
```

| Parameter | Type | Default | Meaning |
|---|---|---|---|
| `Controller` | `MIPI_DCS_controller_type` | | controller, selects the init table and the defaults below |
| `Screen_Size_X` | `std::uint16_t` | | logical screen width in pixel, must match the orientation |
| `Screen_Size_Y` | `std::uint16_t` | | logical screen height in pixel (ILI9341: 240 x 320 for 0 / 180, 320 x 240 for 90 / 270) |
| `Orientation` | `drv::orientation_type` | `orientation_0` | hardware orientation, set with MADCTL (36h) |
| `RGB_to_BGR` | `bool` | true, false for the ST7789 | MADCTL BGR bit, most modules need it with RGB565 data |
| `Invert` | `bool` | true for ST7789 and GC9A01 | display inversion (21h), these modules show correct colors with it |
| `Color_18bit` | `bool` | true for the ILI9488 | 18 bit pixels (3 bytes, 3Ah = 66h) instead of RGB565 (3Ah = 55h), mandatory for the ILI9488 on SPI |
| `Offset_X` | `std::uint16_t` | 0 | column offset of the panel in the GRAM, in the orientation the window commands see (after MADCTL) |
| `Offset_Y` | `std::uint16_t` | 0 | row offset of the panel in the GRAM |

Offset examples from the header: ST7789 240 x 240: 0 / 0 for `orientation_0` and `orientation_90`, 0 / 80 for
`orientation_0m` and `orientation_180`, 80 / 0 for `orientation_270` and `orientation_90m` (MV puts the row offset
into X); ST7735 128 x 160 "green tab": 2 / 1, 128 x 128: 2 / 3 (swapped for MV); JD9853 1.47" 172 x 320: 34 / 0.

## Constructor

```cpp
MIPI_DCS(io::handle_type device_handle);
```

| Parameter | Type | Meaning |
|---|---|---|
| `device_handle` | `io::handle_type` | logical device handle of the bus, passed to every `io::` call |

## Color format

RGB565 big endian (two bytes per pixel, `color::color_to_RGB565()`), or with `Color_18bit` three bytes with 6 bit
R, G, B left aligned in each byte. The GRAM read returns 18 bit pixels in the same three byte format.

## Interface

| Call | Option | Meaning |
|---|---|---|
| `io::write()` | 0 | command byte (D/C low) |
| `io::write()` | 1 | parameters and pixel data (D/C high), pixels in blocks of 32 |
| `io::read()` | 1 | three bytes of one GRAM pixel after RAMRD (2Eh); the I/O layer sends the dummy clock / byte of the controller |

SPI mode 0, mode 3 on most ST7789 modules. A write only I/O layer returns `false` from `io::read()`.

## Orientation

All eight orientations, set in hardware with the MADCTL bits. `orientation_0` is the datasheet default (no MX / MY /
MV); the reference drivers (TFT_eSPI, Adafruit) use MX as their rotation 0, which is `orientation_180m` here.

| Orientation | MADCTL bits |
|---|---|
| `orientation_0` | none |
| `orientation_0m` | MY |
| `orientation_180` | MX, MY |
| `orientation_180m` | MX |
| `orientation_90` | MX, MV |
| `orientation_90m` | MX, MY, MV |
| `orientation_270` | MY, MV |
| `orientation_270m` | MV |

## Native hooks and read back

| Hook | Implementation |
|---|---|
| `pixel_set()` | sets the window only if the tracked write pointer is not already at the pixel, so pixels along x are streamed |
| `line_horz()` | one window and a pixel stream |
| `line_vert()` | a one column wide window |
| `box()` | window over the clipped rect and a pixel stream |
| `blit()` | window over the clipped block, the pixels are streamed |
| `cls()` | one stream over the whole screen |
| `move()` | `drv` fallback (needs `pixel_get()`) |
| `present()` | nothing, the data is written directly |

`pixel_get()` reads one pixel with RAMRD. The ST7735 and ST7789 read the GRAM in the 18 bit format only, so an RGB565
head switches 3Ah to 66h for the read and back. If the I/O layer can't read (SPI modules without MISO),
`pixel_get()` returns `color::none`: alpha blending and anti aliasing then use the background color of the
drawing context, see [Write only displays](/vic/design/write-only-displays/).

## Head specific functions

| Function | Meaning |
|---|---|
| `display_enable(bool)` | 29h / 28h, the GRAM is retained |
| `display_brightness(level)` | 53h 24h and 51h level, only effective with the backlight on the controller's PWM pin; nothing on the ST7735 and JD9853 (no brightness commands); lost in sleep in |
| `display_inverse(bool)` | 21h / 20h relative to the `Invert` default |

## Init sequence

`init()` calls `io::init()`, runs the vendor init table of the controller (command, argument count, arguments, an
optional delay), then sets MADCTL from the orientation, COLMOD 55h / 66h, INVON / INVOFF and clears the screen.
The tables follow the reference drivers: ILI9341 and ILI9488 have no software reset, the hardware reset is the job
of `io::init()`; ST7735, ST7789, ST7796 and JD9853 start with SWRESET. `shutdown()` sends 28h, 10h and waits
120 ms until sleep is reached.

:::caution
Not covered: partial and scroll modes, the tearing effect line, the RGB interface, 24 bit formats and the 3 wire
(9 bit) serial interface (an I/O layer may build the 9 bit frames from the option).
:::

## Example

From `demo/heads/MIPI_DCS.cpp` (a 2.4" ILI9341 SPI module in landscape):

```cpp
#include "head/MIPI_DCS.h"
#include "scene.h"          // demo_scene(): frame, line, circle, disc and text

// other common modules:
//   head::MIPI_DCS<head::MIPI_DCS_ST7789, 240, 240, drv::orientation_0m, false, true, false, 0, 80> st7789_1_3;   // 1.3" square
//   head::MIPI_DCS<head::MIPI_DCS_ST7735, 128, 160, drv::orientation_0, true, false, false, 2, 1>  st7735_green;  // 1.8" green tab
//   head::MIPI_DCS<head::MIPI_DCS_ILI9488, 320, 480>                                                ili9488_3_5;   // 18 bit pixels on SPI
//   head::MIPI_DCS<head::MIPI_DCS_GC9A01, 240, 240>                                                 gc9a01_round;
//   head::MIPI_DCS<head::MIPI_DCS_JD9853, 172, 320, drv::orientation_0, false, false, false, 34, 0>  jd9853_1_47;   // 1.47" centred panel
static vic::head::MIPI_DCS<vic::head::MIPI_DCS_ILI9341, 320, 240, vic::drv::orientation_90> tft(reinterpret_cast<vic::io::handle_type>(0));

int main()
{
  tft.init();
  demo_scene(tft);
  tft.display_brightness(200U);
  return 0;
}
```

The reference libraries show these modules upright with `orientation_180m` (portrait) or `orientation_90`
(landscape). Build the demo target `vic_head_MIPI_DCS` to see the command stream.

## Controller notes

The common part is summarized in
[MIPI_DCS.md](https://github.com/mpaland/vic/blob/master/src/head/doc/MIPI_DCS.md):

- **Window and write:** CASET (2Ah) and RASET (2Bh) set the inclusive window in the coordinate system that MADCTL
  produces, RAMWR (2Ch) writes from the start; the pointer runs along the columns and wraps to the next row. RAMRD
  (2Eh) returns a dummy and 18 bit pixels.
- **Pixel format (3Ah):** 55h RGB565 (not on the ILI9488 serial interface), 66h RGB666 (all), 77h RGB888
  (parallel / DPI only).
- **MADCTL (36h):** MY row order, MX column order, MV row / column exchange, BGR color order; ML / MH only change
  the refresh direction. Panels smaller than the GRAM need offsets that depend on the mirror bits.
- **Common commands:** SWRESET 01h, SLPIN / SLPOUT 10h / 11h (120 ms), NORON 13h, INVOFF / INVON 20h / 21h,
  DISPOFF / DISPON 28h / 29h, TEON 35h, WRDISBV / WRCTRLD 51h / 53h.

Per controller:

- [ILI9341](https://github.com/mpaland/vic/blob/master/src/head/doc/ILI9341.md): 240 x 320 x 18 bit GRAM, panel
  covers the whole GRAM, no offsets. The init table (power control, VCOM, frame rate 79 Hz, gamma) contains no reset,
  so RESX plus 5 ms / 120 ms waits must come from the I/O layer. `display_brightness()` only drives the LEDPWM pin.
- [ILI9488](https://github.com/mpaland/vic/blob/master/src/head/doc/ILI9488.md): 320 x 480, the serial interface
  takes 18 bit (or 3 bit) pixels only, hence `Color_18bit` defaults to true. MADCTL bits can be stored in the OTP, the
  head writes 36h completely.
- [ST7735](https://github.com/mpaland/vic/blob/master/src/head/doc/ST7735.md): 132 x 162 GRAM (128 x 160 with
  GM = 11), offsets per module "tab color" (green tab 2 / 1, 80 x 160 26 / 1 or 24 / 0). No brightness commands. The
  GRAM read needs 3Ah = 66h.
- [ST7789](https://github.com/mpaland/vic/blob/master/src/head/doc/ST7789.md): 240 x 320, init is SWRESET, SLPOUT,
  NORON, DISPON; the module defaults need INVON and no BGR (the LCMCTRL XBGR default already inverts the order).
  240 x 240 panels need row offset 80 in the MY orientations.
- [ST7796](https://github.com/mpaland/vic/blob/master/src/head/doc/ST7796.md): 320 x 480, vendor registers behind
  CSCON (F0h C3h / 96h to unlock, 3Ch / 69h to lock); SPI writes up to 15 MHz, reads 6.6 MHz.
- [GC9A01](https://github.com/mpaland/vic/blob/master/src/head/doc/GC9A01.md): 240 x 240 round, inter register
  enable FEh / EFh before a long undocumented vendor register list, TEON, inversion on. No SWRESET in the table.
- [JD9853](https://github.com/mpaland/vic/blob/master/src/head/doc/JD9853.md): 240 x 320, the vendor registers are
  loaded from the OTP, so the init is SWRESET, SLPOUT, NORON, DISPON. The 1.47" 172 x 320 module is centred on the 240
  GRAM columns (offset 34, in Y for the MV orientations). No brightness commands.
