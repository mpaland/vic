# MIPI DCS TFT controllers programming summary (ILI9341, ILI9488, ST7735, ST7789, ST7796, GC9A01, JD9853)

Sources in this folder: `ILI9341.pdf` (ILI Technology, 240RGBx320, 262K color), `ILI9488.pdf` (ILI Technology,
320RGBx480, 16.7M color, V100), `ST7735.pdf` (Sitronix ST7735R), `ST7789.pdf` (Sitronix ST7789VW V1.0),
`ST7796.pdf` (Sitronix ST7796S V1.0), `GC9A01.pdf` (Galaxycore GC9A01A), `JD9853.pdf` (Jadard JD9853, see
also `JD9853.md`). The seven controllers implement the
MIPI DCS (Display Command Set) user commands, so one head drives them all; they differ in their vendor
("command 2") registers, the panel size, the GRAM offset of the panel and the pixel formats of the serial
interface. The last section describes the vic head `src/head/MIPI_DCS.h`.

## 1 Overview

| Controller | GRAM | Typical panels | Interfaces | Notes |
|---|---|---|---|---|
| ILI9341 | 240 x 320 x 18 bit | 2.2" .. 3.2" 240 x 320 | 8/9/16/18 bit 8080, 3/4 wire SPI, RGB | the classic Arduino TFT |
| ILI9488 | 320 x 480 x 18 bit | 3.5" .. 4" 320 x 480 | 8/9/16/18/24 bit 8080, 3/4 wire SPI, RGB, MIPI DSI | serial interface takes 18 / 24 bit pixels only |
| ST7735 | 132 x 162 x 18 bit | 1.8" 128 x 160, 1.44" 128 x 128, 0.96" 80 x 160 | 8 bit 8080, 3/4 wire SPI, RGB | panel offsets differ per module ("tab color") |
| ST7789 | 240 x 320 x 18 bit | 1.3" / 1.54" 240 x 240, 1.14" 135 x 240, 2" 240 x 320 | 8/9/16/18 bit 8080, 3/4 wire SPI, RGB | SPI mode 3 on most modules, display inversion on |
| ST7796 | 320 x 480 x 18 bit | 3.5" / 4" 320 x 480 | 8/9/16/18 bit 8080, 3/4 wire SPI, RGB, DSI | vendor registers behind CSCON (F0h) |
| GC9A01 | 240 x 240 x 18 bit | 1.28" round 240 x 240 | 8/9/16/18 bit 8080, 3/4 wire SPI, RGB | inter register enable (FEh / EFh) before the vendor registers, inversion on |
| JD9853 | 240 x 320 x 18 bit | 1.47" 172 x 320 (column offset 34), 2" 240 x 320 | 8/9/16/18 bit 8080, 3/4 wire SPI, 2 lane SPI, QSPI, RGB | vendor registers in the OTP, init is SLPOUT + DISPON |

All run on 2.8 .. 3.3 V logic (ILI9341 1.65 .. 3.3 V I/O), the 4 wire SPI is `CS`, `SCL`, `SDA`, `D/C`
(D/C = 0 command, 1 parameter / pixel data), typical clock 20 .. 80 MHz for writes, much slower for reads.

## 2 Address window and memory write

The frame memory is accessed through a window; the window is defined in the coordinate system that
MADCTL produces, so the host always works in its own (rotated) coordinates:

| Hex | Name | Parameters | Function |
|---|---|---|---|
| 2A | CASET | SC[15:0], EC[15:0] | column start / end (inclusive) |
| 2B | RASET / PASET | SP[15:0], EP[15:0] | row (page) start / end (inclusive) |
| 2C | RAMWR | pixels | memory write from (SC, SP); the pointer increments along the columns, wraps to SC at EC and increments the row, wraps to (SC, SP) after (EC, EP) |
| 3C | RAMWRC | pixels | continue the memory write without resetting the pointer |
| 2E | RAMRD | dummy byte, pixels | memory read, 18 bit per pixel on the serial interface (R, G, B in bits 7:2 of three bytes) |
| 3E | RAMRDC | pixels | continue the memory read |

Pixel formats (3Ah COLMOD, DBI[2:0] = MCU interface, DPI[2:0] = RGB interface):

| 3Ah | Format | Bytes on SPI | Supported by |
|---|---|---|---|
| 55h (DBI = 101) | 16 bit RGB565 | 2 | ILI9341, ST7735, ST7789, ST7796, GC9A01, JD9853; not on the ILI9488 serial interface |
| 66h (DBI = 110) | 18 bit RGB666 | 3 (6 bits left aligned per byte) | all |
| 77h (DBI = 111) | 24 bit RGB888 | 3 | ILI9488, ST7796 |

## 3 MADCTL (36h): orientation and color order

| Bit | Name | Meaning |
|---|---|---|
| D7 MY | row address order | 1: rows are written bottom to top (vertical mirror) |
| D6 MX | column address order | 1: columns are written right to left (horizontal mirror) |
| D5 MV | row / column exchange | 1: the window and the write pointer are transposed (rotation) |
| D4 ML | vertical refresh order | panel refresh direction, no effect on the image |
| D3 BGR | color order | 1: BGR color filter panel (most modules need 1 with RGB565 data) |
| D2 MH | horizontal refresh order | panel refresh direction, no effect on the image |

The rotation sequence of the reference drivers is `MX` (rotation 0 of the modules), `MV`, `MY`, `MX | MY | MV`,
i.e. relative to the datasheet default: 90° = `MX | MV`, 180° = `MX | MY`, 270° = `MY | MV`. Panels that are
smaller than the GRAM (ST7789 240 x 240 on 240 x 320, ST7735 128 x 128 / 80 x 160 on 132 x 162) need a
column / row offset that depends on the mirror bits, e.g. ST7789 240 x 240: offset (0, 0) for `0`, (0, 80) for
`MY`, (80, 0) for `MX | MV`.

## 4 Common DCS commands used by the head

| Hex | Name | Function |
|---|---|---|
| 01 | SWRESET | software reset, wait 5 ms (120 ms before SLPOUT) |
| 10 / 11 | SLPIN / SLPOUT | sleep in / out, 120 ms before the next sleep command, 5 ms before other commands |
| 13 | NORON | normal display mode (leave partial mode) |
| 20 / 21 | INVOFF / INVON | display inversion, the ST7789 and GC9A01 modules show correct colors with INVON |
| 28 / 29 | DISPOFF / DISPON | display off (blank) / on |
| 35 | TEON | tearing effect line on (GC9A01 init) |
| 36 | MADCTL | see section 3 |
| 3A | COLMOD | pixel format, see section 2 |
| 51 / 53 | WRDISBV / WRCTRLD | display brightness (backlight PWM output, if the module uses it) and brightness control enable (24h) |

## 5 Vendor init sequences

The vendor registers (power, VCOM, frame rate, gamma, driver timing) have no cross vendor meaning; the head
carries the sequences of the reference drivers (TFT_eSPI / Adafruit) as tables: `ILI9341` (power control A/B,
timing A/B, PWCTR1/2, VMCTR1/2, FRMCTR1, DFUNCTR, gamma, SLPOUT 120 ms, DISPON), `ILI9488` (gamma, PWCTR1/2,
VMCTR, interface mode B0h, frame rate B1h, inversion B4h, DFUNCTR B6h, E9h, adjust control F7h, SLPOUT, DISPON),
`ST7735` (SWRESET, SLPOUT 500 ms, FRMCTR1..3, INVCTR, PWCTR1..5, VMCTR1, gamma, NORON, DISPON), `ST7789`
(SWRESET, SLPOUT, NORON, DISPON, the module defaults are fine), `ST7796` (SWRESET, SLPOUT, CSCON C3h / 96h to
unlock, inversion B4h, DFUNCTR B6h, E8h timing, PWCTR2/3, VCOM C5h, gamma, CSCON 3Ch / 69h to lock, DISPON),
`GC9A01` (inter register enable FEh / EFh and the long vendor register list, TEON, SLPOUT, DISPON), `JD9853`
(SWRESET, SLPOUT, NORON, DISPON, the vendor registers come from the OTP). MADCTL, COLMOD and the inversion are
set by the head after the table.

## 6 The vic head `src/head/MIPI_DCS.h`

```cpp
vic::head::MIPI_DCS<Controller,                       // MIPI_DCS_ILI9341 .. MIPI_DCS_GC9A01
                    Screen_Size_X, Screen_Size_Y,     // logical size, swapped for orientation_90 / 270
                    Orientation  = drv::orientation_0,
                    RGB_to_BGR   = (Controller != MIPI_DCS_ST7789),                 // MADCTL BGR
                    Invert       = (Controller == ST7789) || (Controller == GC9A01), // INVON
                    Color_18bit  = (Controller == MIPI_DCS_ILI9488),                // 3Ah = 66h
                    Offset_X = 0, Offset_Y = 0>                                     // panel offset in the GRAM
MIPI_DCS(io::handle_type device);
```

- io layer: `io::write()` option 0 = command byte, 1 = parameters / pixel data (D/C line); `io::read()`
  option 1 reads three bytes per pixel after RAMRD (the io layer sends the dummy clock / byte). SPI mode 0, mode
  3 for most ST7789 modules.
- init: vendor table of the controller (with the delays), then MADCTL from the orientation table (datasheet
  default = `orientation_0`, the module's "rotation 0" with MX is `orientation_180m`), COLMOD 55h / 66h,
  INVON / INVOFF, clear screen.
- Drawing: every primitive sets the window (2Ah / 2Bh / 2Ch) in logical coordinates plus the offsets and
  streams RGB565 (or RGB666) pixels in blocks of 32; the head tracks the write pointer so that consecutive
  pixels along x need no new window. `blit()` streams the clipped block, `box()` is window + stream, vertical
  lines use a one column window.
- `pixel_get()` reads 18 bit pixels through RAMRD, `color::none` when the io layer can't read (SPI modules
  without MISO).
- `display_enable()` = 29h / 28h, `display_brightness()` = 53h 24h, 51h level, `display_inverse()` relative to
  the panel default, `shutdown()` = 28h, 10h.
- Not covered: partial / scroll modes, tearing effect synchronisation, RGB interface, 24 bit formats, the
  3 wire (9 bit) serial interface (an io layer may build the 9 bit frames from the option).
