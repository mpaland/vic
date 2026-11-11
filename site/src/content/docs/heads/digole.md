---
title: digole
description: Head for the Digole serial graphic display adapters on UART, SPI or I²C, with native line, box and move commands.
sidebar:
  label: "digole (Digole serial modules)"
  order: 11
---

Head for the Digole serial graphic display adapters. The module firmware has native line, rectangle and move
commands, so most primitives are rendered by the display itself and only a few bytes go over the bus. The class
is named `digole`.

```cpp
#include "head/Digole.h"
```

## Template parameters

```cpp
template<std::uint16_t Screen_Size_X = 8U, std::uint16_t Screen_Size_Y = 8U,
         std::uint16_t Viewport_Size_X = Screen_Size_X, std::uint16_t Viewport_Size_Y = Screen_Size_Y,
         drv::orientation_type Orientation = drv::orientation_0,
         digole_interface_type Interface_Mode = DIGOLE_UART>
class digole;
```

| Parameter | Type | Default | Meaning |
|---|---|---|---|
| `Screen_Size_X` | `std::uint16_t` | 8 | screen width in pixel |
| `Screen_Size_Y` | `std::uint16_t` | 8 | screen height in pixel |
| `Viewport_Size_X` | `std::uint16_t` | `Screen_Size_X` | viewport width |
| `Viewport_Size_Y` | `std::uint16_t` | `Screen_Size_Y` | viewport height |
| `Orientation` | `drv::orientation_type` | `orientation_0` | sent to the module in `init()` |
| `Interface_Mode` | `digole_interface_type` | `DIGOLE_UART` | `DIGOLE_SPI`, `DIGOLE_I2C` or `DIGOLE_UART` |

## Constructor

```cpp
digole(io::handle_type device_handle, std::uint32_t uart_baudrate = 9600U);
```

| Parameter | Type | Default | Meaning |
|---|---|---|---|
| `device_handle` | `io::handle_type` | | SPI: device id, I²C: address, UART: port id |
| `uart_baudrate` | `std::uint32_t` | 9600 | baud rate of the UART interface, unused for SPI or I²C |

## Color format and interface

Colors are sent with the enhanced color command `ESC r g b` (the three letters `E`, `S`, `C` and 8 bit red, green,
blue), only when the color changed since the last
command. Every command string is one `io::write()` with option 0, there is no read.

## Orientation

`init()` sends `SD` with the value of the orientation enum (0 = `orientation_0` .. 3 = `orientation_270`), the
module rotates itself.

## Native hooks and read back

| Hook | Command |
|---|---|
| `pixel_set()` | `DP x y` |
| `line_horz()`, `line_vert()` | `LN x0 y0 x1 y1` |
| `box()` | `FR left top right bottom` (inclusive corners) |
| `move()` | `MA left top width height dx dy` |
| `cls()` | `CL`, the background color is ignored |
| `blit()` | `drv` fallback, pixel by pixel |

`pixel_get()` is not implemented and returns `color::none`, alpha blending and anti aliasing blend against the
background color of the drawing context, see [Write only displays](/vic/design/write-only-displays/).

:::caution
Coordinates are sent as 8 bit values, the display must not be larger than 256 x 256 pixel.
:::

## Head specific functions

| Function | Meaning |
|---|---|
| `display_enable(bool)` | `SOO 1` / `SOO 0` |
| `display_brightness(level)` | backlight `BL 1` for any level > 0, `BL 0` for 0 (on / off only) |

## Init sequence

`io::init()`; in I²C mode the module is first addressed at its default address 27h and gets the address from the
device handle (`SI2CA addr`), the head then uses the new address. Then orientation (`SD`), cursor off (`CS 0`),
clear, display and backlight on. In UART mode the last command sets the module baud rate with `SB` (300 .. 115200,
other values send nothing). `shutdown()` clears the screen and switches display and backlight off.

## Example

From `demo/heads/Digole.cpp` (128 x 64 module on UART, 9600 baud):

```cpp
#include "head/Digole.h"
#include "scene.h"          // demo_scene(): frame, line, circle, disc and text

static vic::head::digole<128, 64> lcd(reinterpret_cast<vic::io::handle_type>(0), 9600U);

int main()
{
  lcd.init();
  demo_scene(lcd);
  return 0;
}
```

## Controller notes

There is no datasheet summary in `src/head/doc/` for the Digole modules; the command strings the head uses are
listed above.
