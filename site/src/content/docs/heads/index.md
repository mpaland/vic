---
title: Heads
description: Display drivers in vic, how they are instantiated and initialized, their native hooks, orientation, viewport and I/O layer.
sidebar:
  label: Overview
  order: 0
---

A display together with its controller is called a **head**. Every head is a class in the namespace `vic::head`,
derived from the driver base class `vic::drv` (`src/drv.h`). A head is platform independent: it talks to the
hardware only through the I/O functions declared in `src/io.h`, which your platform implements once.

## Instantiate and initialize

Everything that is fixed by the hardware (screen size, orientation, bus mode, controller variant) is a template
parameter, so unused code is optimized away. The constructor takes the logical device handle of the bus (and
nothing else for most heads). Heads are normally static objects:

```cpp
#include "head/SSD1306.h"
#include "dc.h"

static vic::head::SSD1306<128, 64, vic::drv::orientation_180> oled(reinterpret_cast<vic::io::handle_type>(0x3C));

int main()
{
  oled.init();                  // io::init(), controller init sequence, clear screen
  vic::dc dc(oled);             // drawing context on the head
  dc.set_color(vic::color::white);
  dc.disc({ 64, 32 }, 20U);
  oled.display_contrast(0x40U); // display control is public
  return 0;
}
```

The public interface of every head is small:

| Function | Meaning |
|---|---|
| `init()` | init the interface (`io::init()`), send the controller init sequence, clear the screen |
| `shutdown()` | display off / sleep / power down, also called by the destructor |
| `version()` | head name and version string |
| `is_graphic()` | `true` for graphic heads, `false` for alpha numeric (text) heads |
| `screen_width()`, `screen_height()` | screen size in pixel (graphic) or characters (alpha numeric) |
| `viewport_width()`, `viewport_height()`, `viewport_set()`, `viewport_get()` | viewport, see below |
| `display_enable()`, `display_brightness()`, `display_contrast()` | display control, no-op if the head doesn't support it |
| `framebuffer_set_display()`, `framebuffer_set_access()`, `framebuffer_get_count()` | framebuffer planes, see [`framebuffer`](/vic/heads/framebuffer/) |

Some heads add their own public functions (`display_inverse()`, `cursor_set()`, `char_define()`, ...), listed on
their pages. All drawing functions are protected: you draw with a drawing context `vic::dc` on graphic heads or a
text context `vic::tc` on alpha numeric heads, see [dc](/vic/reference/dc/) and [drv](/vic/reference/drv/).

## Native hooks and the gpr fallback

The graphic primitive renderer (gpr) decomposes every primitive (lines, circles, discs, triangles, text, ...) into
a few basic operations. `drv` declares them as virtual functions with a slow generic fallback, so a head only
has to implement `pixel_set()` and `pixel_get()`. A head overrides the others when its controller can do them
faster, typically with one address window setup and a pixel stream or a native command:

| Hook | Fallback in `drv` | Overridden by heads with |
|---|---|---|
| `pixel_set(vertex, color)` | none (no-op), mandatory for graphic heads | every graphic head |
| `pixel_get(vertex)` | returns `color::none` | heads which can read back (RAM buffer or bus read) |
| `line_horz(v0, v1, color)` | `pixel_set()` per pixel | an auto incrementing GRAM window or a line command |
| `line_vert(v0, v1, color)` | `pixel_set()` per pixel | a one column window or a line command |
| `box(rect, color)` | one `line_horz()` per row | a window fill or a rectangle command |
| `move(source, destination)` | `pixel_get()` / `pixel_set()` per pixel | a copy command or a RAM buffer |
| `blit(rect, pixels, stride)` | clipped `pixel_set()` per pixel | an auto incrementing GRAM window |
| `cls(bg_color)` | no-op | every head |
| `present()` | no-op | heads with a RAM buffer (sends it to the display) |

The drawing context hands `line_horz()`, `line_vert()`, `box()` and `blit()` directly to the head when no shader
is active and the color is opaque, otherwise every pixel goes through the shader pipe and `pixel_set()`. A disc is
therefore a handful of `line_horz()` calls, which a TFT head sends as a few window streams. See
[Architecture](/vic/design/architecture/).

### pixel_get and read back

`pixel_get()` returns `color::none` when the head can't read its pixels back (serial buses without read, write only
controllers). Alpha blending and anti aliasing then blend against the background color of the drawing context;
put a [`framebuffer`](/vic/heads/framebuffer/) head in front of the display if the real pixels are needed. See
[Write only displays](/vic/design/write-only-displays/) and [Anti aliasing](/vic/design/anti-aliasing/).

### present

The drawing context calls `present()` after every primitive. Heads which write directly into the display RAM
ignore it, heads with a RAM buffer (SSD1306, ST7565, PCD8544, e-paper, ...) transfer the buffer. Use
`dc.present_lock()` / `dc.present_lock(false)` to collect a whole frame and present it once.

## Orientation

`drv::orientation_type` has eight values: `orientation_0`, `orientation_90`, `orientation_180`, `orientation_270`
(clockwise) and the vertically mirrored `orientation_0m`, `orientation_90m`, `orientation_180m`,
`orientation_270m`. The head takes care of the rotation so that `(0, 0)` is always the top left pixel. The screen
size template parameters are the **logical** size and must match the orientation, e.g. a 240 x 320 panel is
`320, 240` in `orientation_90`.

How a head rotates depends on the controller:

- TFT and RGB OLED controllers (MIPI DCS, ILI9325, SSD1331, SSD1351) use their address mode / remap registers, all
  eight orientations at full speed.
- Heads with a RAM framebuffer (PCD8544, SSD1322, SSD1680, ST7565, ST7920) map the pixels in software, all eight
  orientations.
- The SSD1306 head supports `orientation_0`, `0m`, `180` and `180m` with the controller mirror bits.
- Alpha numeric heads and the virtual heads have no orientation.

The datasheet default is `orientation_0`; many modules are upright in another orientation, the head pages and the
demos name it.

## Screen and viewport

`drv` distinguishes the screen (the drawing buffer) from the viewport (the visible window of the physical
display). `viewport_set()` moves the viewport over a larger screen and calls `present()`. Only heads with their
own buffer support a viewport larger than the display: [`memory`](/vic/heads/memory/),
[`dummy`](/vic/heads/dummy/), [`windows`](/vic/heads/windows/), [`x11`](/vic/heads/x11/) and the
[skeleton](/vic/heads/skeleton/) template take viewport sizes and offsets. The device heads have screen = viewport.

## I/O abstraction

The heads access the hardware through `src/io.h`:

```cpp
namespace vic { namespace io {
  void delay(std::uint32_t time_to_delay);                                  // ms
  typedef const void* handle_type;                                          // logical device handle
  void init(handle_type device_handle);                                     // bus setup, display reset
  bool write(handle_type device_handle, std::uint32_t option,
             const std::uint8_t* data_out, std::size_t data_out_length,
             std::uint8_t* data_in, std::size_t data_in_length,
             std::uint32_t timeout = 0U);
  bool read(handle_type device_handle, std::uint32_t option,
            std::uint8_t* data_in, std::size_t& data_in_length,
            std::uint32_t timeout = 0U);
  namespace mem {                                                           // memory mapped buses
    template <typename VALUE_TYPE> void write(volatile void* const address, VALUE_TYPE value);
    template <typename VALUE_TYPE> VALUE_TYPE read(volatile void* const address);
  }
} }
```

The `option` parameter tells the I/O layer what the bytes are. Most heads use `0` = command and `1` = data, i.e.
the level of the D/C (A0, RS) line. A few heads add more values:

| Head | `io::write()` option | `io::read()` option |
|---|---|---|
| `MIPI_DCS`, `SSD1331`, `SSD1351` | 0 command, 1 parameters / pixels | 1 GRAM pixel bytes after the read command |
| `ILI9325` (SPI mode) | 0 index register, 1 data | 1 GRAM pixel |
| `SSD1306` | 0 command, 1 data (I²C: the control byte is the first byte of the block) | 1 one GDDRAM byte |
| `SSD1322`, `PCD8544`, `ST7565`, `ST7920` | 0 command / instruction, 1 data | not used |
| `SSD1680` | 0 command, 1 data | 2 BUSY pin in bit 0 |
| `HD44780` | low byte: 0 instruction, 1 data, 2 init nibble; bits 8..23: execution time in µs | 0 busy flag and address counter |
| `MAX7219`, `STPxxP05`, `seven_segment`, `digole` | 0 | not used |
| `BA6x` | 0 framed command | 0 status answer |

A read which the bus can't do returns `false`, the head then falls back (`pixel_get()` returns `color::none`, the
HD44780 and SSD1680 heads wait a fixed time). `demo/heads/io_trace.cpp` implements the four functions as a hex
dump to stdout and is the template for a real I/O layer. Every function is described in
[Write your own head: Platform I/O](/vic/heads/skeleton/#platform-io), installation in
[Installation](/vic/getting-started/installation/).

## Available heads

| Head | Displays / controllers | Color format | Interface | |
|---|---|---|---|---|
| `BA6x` | Wincor Nixdorf BA60 (1 x 16), BA63 (2 x 20), BA66 (4 x 20) customer displays | alpha numeric | USB, RS232 | [BA6x](/vic/heads/ba6x/) |
| `digole` | Digole serial graphic modules, up to 256 x 256 | RGB888 (`ESC` color command) | UART, SPI, I²C | [digole](/vic/heads/digole/) |
| `HD44780` | HD44780, ST7066, KS0066U, SED1278, SPLC780 character LCDs | alpha numeric | 4 or 8 bit parallel, I²C expander | [HD44780](/vic/heads/hd44780/) |
| `ILI9325` | ILI9325 240 x 320 TFT | RGB565 / RGB666 | SPI, 8 / 9 / 16 / 18 bit memory mapped | [ILI9325](/vic/heads/ili9325/) |
| `MAX7219` | MAX7219 / MAX7221 8 x 8 LED matrix | 1 bit | SPI | [MAX7219](/vic/heads/max7219/) |
| `MIPI_DCS` | ILI9341, ILI9488, ST7735, ST7789, ST7796, GC9A01, JD9853 TFTs | RGB565 / RGB666 | SPI, parallel | [MIPI_DCS](/vic/heads/mipi-dcs/) |
| `PCD8544` | Nokia 5110 / 3310 84 x 48 LCD | 1 bit | SPI | [PCD8544](/vic/heads/pcd8544/) |
| `seven_segment` | 7 segment digits on any raw segment driver | segments | SPI, shift register | [seven_segment](/vic/heads/seven-segment/) |
| `SSD1306` | SSD1306, SSD1309, SH1106 monochrome OLEDs | 1 bit | I²C, 3 / 4 wire SPI, 8 bit parallel | [SSD1306](/vic/heads/ssd1306/) |
| `SSD1322` | SSD1322 256 x 64 gray scale OLEDs | 4 bit gray | SPI, parallel | [SSD1322](/vic/heads/ssd1322/) |
| `SSD1331` | SSD1331 96 x 64 RGB OLED | RGB565 | SPI, 8 / 16 bit parallel | [SSD1331](/vic/heads/ssd1331/) |
| `SSD1351` | SSD1351 128 x 128 RGB OLED | RGB565 | SPI, parallel | [SSD1351](/vic/heads/ssd1351/) |
| `SSD1680` | SSD1680, SSD1681 black / white (/ red) e-paper | 1 bit (+ red plane) | SPI + BUSY | [SSD1680](/vic/heads/ssd1680/) |
| `ST7565` | ST7565, ST7567, UC1701 128 x 64 COG LCDs | 1 bit | SPI, parallel | [ST7565](/vic/heads/st7565/) |
| `ST7920` | ST7920 128 x 64 graphic LCD | 1 bit | serial, 4 / 8 bit parallel | [ST7920](/vic/heads/st7920/) |
| `STPxxP05` | STP08xP05, STP16xP05, STP24DP05 LED drivers | 1 bit | SPI | [STPxxP05](/vic/heads/stpxxp05/) |
| `framebuffer` | RAM framebuffer with dirty rect transfer in front of any head | ARGB8888, RGB565, RGB332, L1 | none (bound head) | [framebuffer](/vic/heads/framebuffer/) |
| `memory` | pure RAM head with statistics, unit tests and off screen rendering | ARGB8888 | none | [memory](/vic/heads/memory/) |
| `multihead` | combines several heads to one screen | of the heads | none (bound heads) | [multihead](/vic/heads/multihead/) |
| `dummy` | null head, swallows everything | none | none | [dummy](/vic/heads/dummy/) |
| `windows`, `windows_text` | display emulation in a Win32 window | RGB888 | Win32 GDI | [windows](/vic/heads/windows/) |
| `x11` | display emulation in an X11 window | RGB888 | Xlib | [x11](/vic/heads/x11/) |
| `skeleton` | template for a new head | | | [Write your own head](/vic/heads/skeleton/) |

Every device head has a small demo program in `demo/heads/<head>.cpp` (CMake target `vic_head_<head>`), linked
with `io_trace.cpp`, which prints the command and data stream the head sends. The programming summaries of the
controller datasheets are in
[`src/head/doc/`](https://github.com/mpaland/vic/tree/master/src/head/doc).
