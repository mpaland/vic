---
title: Write your own head
description: Step by step guide to a new display driver based on src/head/skeleton.h, and the platform I/O functions of src/io.h.
sidebar:
  order: 99
---

`src/head/skeleton.h` is the template for a new head. Copy it, rename the class and implement the functions your
display supports. This page walks through it and describes the platform I/O layer every head talks to.

## 1. Derive from drv

A head is a class in `vic::head` derived from `vic::drv`. Everything that is fixed by the hardware is a template
parameter, the constructor takes the device handle and passes the sizes, the viewport offset and the orientation to
`drv`:

```cpp
#include "../drv.h"

#define VIC_HEAD_MYLCD_VERSION   "MyLCD head 1.00"

namespace vic {
namespace head {

template<std::uint16_t Screen_Size_X,   std::uint16_t Screen_Size_Y,    // screen (buffer) size, must match the orientation
         std::uint16_t Viewport_Size_X = Screen_Size_X,                 // viewport (physical) size
         std::uint16_t Viewport_Size_Y = Screen_Size_Y,
         drv::orientation_type Orientation = drv::orientation_0>
class mylcd : public drv
{
  const io::handle_type device_handle_;   // logical device handle of the display interface

public:
  mylcd(io::handle_type device_handle, std::int16_t viewport_x = 0, std::int16_t viewport_y = 0)
    : drv(Screen_Size_X,   Screen_Size_Y,
          Viewport_Size_X, Viewport_Size_Y,
          viewport_x,      viewport_y,
          Orientation)
    , device_handle_(device_handle)
  { }

  ~mylcd()
  {
    shutdown();
  }
  ...
};

} // namespace head
} // namespace vic
```

The constructor only initializes memory, no I/O: heads are usually static objects which are constructed before the
platform is ready. Most device heads have no viewport and pass the screen size twice and `0, 0` as offset.

## 2. The mandatory functions

These are pure virtual in `drv`, every head must implement them, even if unsupported. They are the only public
functions besides the display controls.

| Function | Implement |
|---|---|
| `void init()` | `io::init(device_handle_)`, the controller init sequence (with `io::delay()` waits), clear the screen |
| `void shutdown()` | display off, sleep or power down; also called by the destructor |
| `const char* version() const` | return the version define |
| `bool is_graphic() const` | `true` for a graphic display, `false` for an alpha numeric one |

## 3. pixel_set and pixel_get

For a graphic display `pixel_set()` and `pixel_get()` are mandatory. Check the limits first, the head may get
vertices outside the screen:

```cpp
protected:
  virtual void pixel_set(vertex_type vertex, color::value_type color)
  {
    if (!screen_is_inside(vertex)) {
      return;                                     // out of bounds
    }
    // set the pixel in the display (buffer), converted to the native format
  }

  virtual color::value_type pixel_get(vertex_type vertex)
  {
    if (!screen_is_inside(vertex)) {
      return color::none;
    }
    // return the pixel in ARGB format with opaque alpha,
    // color::none if the display can't be read back
  }
```

Return `color::none` from `pixel_get()` if the display can't read its pixels back (or the I/O layer returns `false`
from `io::read()`). The drawing context then blends alpha and anti aliasing pixels against its background color, see
[Write only displays](/vic/design/write-only-displays/). All drawing functions are `protected`: the drawing context,
the shaders, the sprites and the `framebuffer` / `multihead` heads are friends of `drv`.

## 4. Override hooks for speed

Everything else has a fallback in `drv` that ends in `pixel_set()`. Override what the controller does faster, in this
order of benefit:

| Hook | Override when |
|---|---|
| `cls(bg_color)` | always: one window stream, a clear command or a buffer fill |
| `line_horz(v0, v1, color)` | the controller has an auto incrementing address window or a line command; the gpr fills discs, triangles and boxes with horizontal lines |
| `line_vert(v0, v1, color)` | a one column window or a line command |
| `box(rect, color)` | a window fill or a rectangle command; right and bottom of `rect` are outside |
| `blit(rect, pixels, stride)` | an address window: clip the rect to the screen, set the window once and stream the converted pixels; used by the `framebuffer` head and for bitmaps |
| `move(source, destination)` | a copy command; the fallback needs `pixel_get()` |
| `present()` | the head renders into a RAM buffer: send the buffer (or its changed part) here |

The drawing context hands `line_horz()`, `line_vert()` and `box()` to the head already clipped to the screen when no
shader is active and the color is opaque. `line_horz()` ignores the y of `v1`, `line_vert()` the x of `v1`, both
include their end points. A good pattern, used by the TFT heads, is to track the address pointer of the controller and
only send a new window when the next pixel is not where the controller writes anyway. Delete the overrides you don't
need, the skeleton calls the `drv` versions in them.

For an alpha numeric display delete the graphic section and implement `text_out(std::uint16_t ch)` (mandatory),
`text_set_pos()`, `text_set_inverse()`, `text_clear_eol()`, `text_clear_sol()`, `text_clear_line()`. `drv` decodes
UTF-8 strings and calls `text_out(ch)` per character, then `present()`.

## 5. Display control

Implement `display_enable()`, `display_brightness()` and `display_contrast()` if the display supports them (levels 0
.. 255, scale them to the controller), delete the rest. Head specific public functions (inverse video, cursor, user
characters) can be added freely.

## 6. Color conversion

vic works with 32 bit ARGB (`color::value_type`). The head converts to its native format with the helpers of
`src/color.h`, see [Colors and helpers](/vic/reference/color-util/):

| Native format | To the head | Back from the head |
|---|---|---|
| 1 bit | `color::color_to_L1()` (any non black color is 1) | `color::L1_to_color()` |
| 2 / 4 / 8 bit gray | `color::color_to_L2()`, `color_to_L4()`, `color_to_L8()` | `L2_to_color()`, `L4_to_color()`, `L8_to_color()` |
| RGB332, RGB444, RGB555 | `color::color_to_RGB332()`, `color_to_RGB444()`, `color_to_RGB555()` | `RGB332_to_color()`, ... |
| RGB565 | `color::color_to_RGB565()` | `color::RGB565_to_color()` |
| RGB666 | `color::color_to_RGB666()` | `color::RGB666_to_color()` |
| RGB888 | `color::color_to_RGB888()` | `color::RGB888_to_color()` |

`color::get_red()`, `get_green()`, `get_blue()` return the components, `color::argb()` builds a color. The alpha
channel of a pixel can be ignored, blending is done before the pixel reaches the head.

## 7. Orientation

The orientation is a template parameter and passed to `drv` (`orientation_` holds it). The screen size parameters are
the logical size and must match the orientation. Use the controller's address mode / remap bits if it has them (all
eight orientations at full speed, see [MIPI_DCS](/vic/heads/mipi-dcs/)); otherwise map the logical vertex to the
physical panel in software like the framebuffer heads do:

```cpp
static inline vertex_type to_phys(vertex_type v)   // PHYS_X / PHYS_Y: physical panel size
{
  const std::int16_t mx = static_cast<std::int16_t>(PHYS_X - 1U);
  const std::int16_t my = static_cast<std::int16_t>(PHYS_Y - 1U);
  switch (Orientation) {
    case orientation_0    : return { v.x, v.y };
    case orientation_0m   : return { v.x, static_cast<std::int16_t>(my - v.y) };
    case orientation_180  : return { static_cast<std::int16_t>(mx - v.x), static_cast<std::int16_t>(my - v.y) };
    case orientation_180m : return { static_cast<std::int16_t>(mx - v.x), v.y };
    case orientation_90   : return { static_cast<std::int16_t>(mx - v.y), v.x };
    case orientation_90m  : return { static_cast<std::int16_t>(mx - v.y), static_cast<std::int16_t>(my - v.x) };
    case orientation_270  : return { v.y, static_cast<std::int16_t>(my - v.x) };
    case orientation_270m :
    default               : return { v.y, v.x };
  }
}
```

The `m` orientations mirror the panel top / bottom after the rotation.

## 8. Talk to the hardware

Use only the `vic::io` functions below, never platform code: the head stays portable. Define an `option` convention
for `io::write()` / `io::read()` and document it in the header comment, e.g. 0 = command, 1 = data (the D/C line) as
most heads do. Collect bytes into blocks (the TFT heads stream 32 pixels per `io::write()`), every call costs bus
setup time.

## 9. Test it

- Render the same scene on your head and on the [`memory`](/vic/heads/memory/) head and compare with `pixel_get()`,
  or let a test count the calls with the memory head statistics. The unit tests in `test/` show the pattern.
- Link your head with `demo/heads/io_trace.cpp` and look at the command and data stream it sends, like the head
  demos in `demo/heads/` do. Compare it with the datasheet init sequence before you connect a display.
- Put a [`framebuffer`](/vic/heads/framebuffer/) in front to check `blit()`.

## Platform I/O

`src/io.h` declares the hardware dependent functions of vic. You implement `delay()`, `init()`, `write()` and
`read()` once for your platform, in a `.cpp` file or in a copy of `io.h` with inline definitions (better inlining).
The `io::mem` functions are already implemented in the header. A function that is never called (for example `read()`
for write only heads) still has to be defined if a head references it.

### io::delay

```cpp
void delay(std::uint32_t time_to_delay);
```

| Parameter | Meaning |
|---|---|
| `time_to_delay` | time to wait in milliseconds |

Used by the init sequences (reset and power up waits), the HD44780 head (`HD44780_WAIT_DELAY`), the SSD1331 head
(`GAC_Delay`) and the SSD1680 head (BUSY poll interval, `Refresh_Delay`). A busy wait or an RTOS sleep, both are fine.

### io::handle_type

```cpp
typedef const void* handle_type;
```

The logical device handle. The heads store it and pass it unchanged to every `io::` call, its meaning is up to the
I/O layer: an index into a table of bus devices, a pointer to a device descriptor, a chip select number or an I²C
address (the demos use `reinterpret_cast<vic::io::handle_type>(0x3C)` for an SSD1306 on I²C). The `digole` head in
I²C mode is the only head which interprets it (as the I²C address).

### io::init

```cpp
void init(handle_type device_handle);
```

| Parameter | Meaning |
|---|---|
| `device_handle` | logical device handle of the display |

Called by `init()` of the head before the first command. Configure the bus and the control pins (D/C, CS, BUSY) and
pulse the reset line of the display: several heads rely on it (ILI9341, GC9A01, ST7565R / ST7567, SSD1680). When
several heads share one bus, it is called once per head.

### io::write

```cpp
bool write(handle_type device_handle,
           std::uint32_t option,
           const std::uint8_t* data_out, std::size_t data_out_length,
           std::uint8_t* data_in, std::size_t data_in_length,
           std::uint32_t timeout = 0U);
```

| Parameter | Meaning |
|---|---|
| `device_handle` | logical device handle |
| `option` | optional data for the device like the register selection, defined by the head (see below) |
| `data_out` | data transmit buffer |
| `data_out_length` | number of bytes to send |
| `data_in` | data receive buffer, may be used with bidirectional devices like SPI; the heads pass `nullptr` |
| `data_in_length` | additional input length; the heads pass 0 |
| `timeout` | time in ms to wait for sending the data, 0 = no waiting |
| return | `true` if successful |

The bytes of one call belong together: one SPI transfer with CS active, one I²C transaction, one framed UART
message. The option values of the heads:

| Head | Option |
|---|---|
| `MIPI_DCS`, `SSD1322`, `SSD1331`, `SSD1351`, `SSD1680`, `PCD8544`, `ST7565` | 0 = command (D/C low), 1 = parameters / data (D/C high) |
| `SSD1306` | 0 = command, 1 = data; on I²C the control byte (00h / 40h) is already the first byte, on 3-wire SPI build the 9 bit frames from the option |
| `ILI9325` (SPI mode) | 0 = index register, 1 = data; the I/O layer builds the SPI start byte |
| `ST7920` | 0 = instruction (RS = 0), 1 = data (RS = 1); serial: sync byte F8h / FAh and nibble split by the I/O layer, 72 µs pacing |
| `HD44780` | low byte: 0 = instruction, 1 = data, 2 = single high nibble of the 4 bit init; bits 8..23: execution time in µs to wait after the byte (`HD44780_WAIT_IO`); one byte per call |
| `MAX7219`, `STPxxP05`, `seven_segment`, `digole`, `BA6x` | 0, the bytes are the complete frame |

### io::read

```cpp
bool read(handle_type device_handle,
          std::uint32_t option,
          std::uint8_t* data_in, std::size_t& data_in_length,
          std::uint32_t timeout = 0U);
```

| Parameter | Meaning |
|---|---|
| `device_handle` | logical device handle |
| `option` | optional data for the device like the register selection, defined by the head |
| `data_in` | data receive buffer |
| `data_in_length` | on input the maximum buffer size, on output the number of received bytes |
| `timeout` | time in ms to wait for receiving data, 0 = no waiting (FIFO check) |
| return | `true` if successful, `false` if nothing could be read |

Return `false` if the bus can't read (SPI without MISO, write only modules): the heads fall back instead of failing.

| Head | Option | Read |
|---|---|---|
| `MIPI_DCS` | 1 | 3 bytes of one GRAM pixel after RAMRD; the I/O layer produces the dummy clock / byte; `false`: `pixel_get()` returns `color::none` |
| `ILI9325` (SPI mode) | 1 | 2 bytes of one GRAM pixel, including the dummy bytes of the controller |
| `SSD1331`, `SSD1351` | 1 | 2 bytes of one pixel (parallel bus), with the dummy read |
| `SSD1306` | 1 | 1 GDDRAM byte (parallel bus, without framebuffer), with the dummy read |
| `HD44780` | 0 | 1 byte busy flag (bit 7) and address counter (`HD44780_WAIT_BUSY`); `false`: the head waits with `io::delay()` |
| `SSD1680` | 2 | 1 byte, bit 0 = level of the BUSY pin; `false`: the head waits `Refresh_Delay` |
| `BA6x` | 0 | 4 byte status answer with timeout (100 ms, 30 s for the self test); `init()` flushes the receive buffer with timeout 0 |

### io::mem::write and io::mem::read

```cpp
namespace mem {
  template <typename VALUE_TYPE>
  inline void write(volatile void* const address, VALUE_TYPE value);

  template <typename VALUE_TYPE>
  inline VALUE_TYPE read(volatile void* const address);
}
```

| Parameter | Meaning |
|---|---|
| `VALUE_TYPE` | access width: `std::uint8_t`, `std::uint16_t` or `std::uint32_t` |
| `address` | the address to write to / read from |
| `value` | the value written to the address |
| return (`read`) | the value read from the address |

Direct volatile memory access for displays on a memory mapped (external) bus, implemented in `io.h`. The
[ILI9325](/vic/heads/ili9325/) head uses them in its 8 / 9 / 16 / 18 bit modes with the index and data register
addresses from its constructor.

### io_trace.cpp

`demo/heads/io_trace.cpp` is the I/O layer of the head demos. There is no hardware behind it, it is the template for a
real one:

```cpp
void vic::io::delay(std::uint32_t time_to_delay)
{
  // real io layer: sleep / busy wait for time_to_delay milliseconds
  std::printf("  delay %u ms\n", static_cast<unsigned>(time_to_delay));
}

void vic::io::init(handle_type device_handle)
{
  // real io layer: configure the bus, the control pins and pulse the reset line of the display
  std::printf("  init  handle %p (bus setup and display reset go here)\n", device_handle);
}

bool vic::io::write(handle_type device_handle, std::uint32_t option,
                    const std::uint8_t* data_out, std::size_t data_out_length,
                    std::uint8_t* data_in, std::size_t data_in_length, std::uint32_t timeout)
{
  // real io layer: set the register select / D/C line from option, then transfer data_out
  // (and receive data_in on a bidirectional bus like SPI)
  // here: print option, length and the first 16 bytes, return zeros in data_in
  return true;
}

bool vic::io::read(handle_type device_handle, std::uint32_t option,
                   std::uint8_t* data_in, std::size_t& data_in_length, std::uint32_t timeout)
{
  // real io layer: read data_in_length bytes with the register select from option (include the dummy
  // read / dummy clocks the controller needs), return false if the bus can't read
  // here: 1 byte reads return 00h (busy flags: not busy), 4 byte reads the positive BA6x status,
  // everything else returns false with data_in_length = 0 (no pixel read back)
  ...
}
```

`write()` prints every call as a hex dump (option, length, first bytes), `read()` simulates the answers the heads
wait for and `delay()` only prints. To bring up a display, copy a head demo, replace these four functions with your
bus code and keep the option convention of the head.
