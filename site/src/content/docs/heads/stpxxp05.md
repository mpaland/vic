---
title: STPxxP05
description: Head for the STP08xP05, STP16xP05 and STP24DP05 LED array drivers on SPI.
sidebar:
  label: "STPxxP05 LED driver"
  order: 25
---

Head for the ST LED array drivers STP08xP05 (8 channels), STP16xP05 (16) and STP24DP05 (24), e.g. LED bars
or LED arrays. The LEDs are organized as a bit array, the pixel (x, y) is LED number `y * Screen_Size_X + x`. The LED
states are kept in a buffer (the chip can't be read back) and shifted out with `present()`.

```cpp
#include "head/STPxxP05.h"
```

## Template parameters and constructor

```cpp
template<std::uint16_t Screen_Size_X = 8U, std::uint16_t Screen_Size_Y = 1U,
         std::uint_fast8_t Byte_Count = 1U>
class STPxxP05;

STPxxP05(io::handle_type device_handle);
```

| Parameter | Type | Default | Meaning |
|---|---|---|---|
| `Screen_Size_X` | `std::uint16_t` | 8 | screen width, an 8 LED bar is 8 x 1 |
| `Screen_Size_Y` | `std::uint16_t` | 1 | screen height |
| `Byte_Count` | `std::uint_fast8_t` | 1 | available channels in bytes: 1 for STP08xP05, 2 for STP16xP05, 3 for STP24DP05, more for chained chips |
| `device_handle` | `io::handle_type` | | logical SPI device handle |

LED `n` is bit `n & 7` of buffer byte `n >> 3`; pixels without a channel (`n >= Byte_Count * 8`) are ignored.

## Color format and interface

1 bit per pixel, any color which is not black switches the LED on (`color::color_to_L1()`). `present()` writes the
`Byte_Count` buffer bytes with one `io::write()`, option 0; the I/O layer latches them with the LE pulse after the
transfer. SPI mode 3, MSB first, 10 MHz.

## Orientation and hooks

No orientation and no viewport. Only `pixel_set()`, `pixel_get()`, `cls()` and `present()`; `pixel_get()` reads the
buffer.

## Head specific functions

| Function | Meaning |
|---|---|
| `buffer()` | pointer to the raw LED buffer, `Byte_Count` bytes |

## Init sequence

`init()` calls `io::init()`, clears the buffer and presents it (all LEDs off). `shutdown()` does the same.

## Example

From `demo/heads/STPxxP05.cpp` (two chained 16 bit drivers as a 32 x 1 bar):

```cpp
#include "head/STPxxP05.h"
#include "scene.h"          // demo_scene(): a checkerboard on tiny screens

static vic::head::STPxxP05<32, 1, 4U> bar(reinterpret_cast<vic::io::handle_type>(0));

int main()
{
  bar.init();
  demo_scene(bar);
  return 0;
}
```

## Controller notes

There is no datasheet summary in `src/head/doc/` for the STPxxP05 drivers. The head only shifts the LED bits out.
