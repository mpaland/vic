---
title: seven_segment
description: Generic alpha numeric head for one row of 7 segment digits on any serial segment driver.
sidebar:
  order: 17
---

An alpha numeric head with one row of 7 segment digits. The segment patterns are kept in a buffer and written to
the device with `present()`, one byte per digit in the common `dp g f e d c b a` bit order (bit 0 = segment a, bit 7 =
decimal point). Use it with any serial segment driver or shift register chain which accepts raw segment data.

```cpp
#include "head/seven_segment.h"
```

## Template parameters and constructor

```cpp
template<std::uint16_t Columns> class seven_segment;
seven_segment(io::handle_type device_handle);
```

| Parameter | Type | Default | Meaning |
|---|---|---|---|
| `Columns` | `std::uint16_t` | | number of digits, the screen is `Columns` x 1 characters |
| `device_handle` | `io::handle_type` | | logical device handle of the segment driver interface |

## Interface

`present()` sends all `Columns` segment bytes with one `io::write()`, option 0. There is no read.

## Characters

| Character | Segments |
|---|---|
| `0`..`9`, `A`..`F`, `a`..`f` | hex digits |
| space, `-`, `_` | blank, segment g, segment d |
| `o`, `r`, `H`, `L`, `P`, `U` | the usual 7 segment shapes |
| `.` | sets the decimal point of the previous digit, the cursor doesn't move |

Other characters are ignored and don't move the cursor. Characters below 20h are ignored.

## Text functions

`cls()`, `text_out()`, `text_set_pos()`, `text_clear_eol()`, `text_clear_sol()`, `text_clear_line()`. The string
output of the text context ends with `present()`, so a `t.out(...)` updates the digits in one transfer. No graphics,
no orientation.

## Head specific functions

| Function | Meaning |
|---|---|
| `segments(column)` | segment pattern of a digit, 0 outside the screen |
| `char_to_segments(ch, valid)` | static, segment pattern of a character, `valid` is false if it is not displayable |
| `SEG_A` .. `SEG_G`, `SEG_DP` | public segment bit constants |

## Init sequence

`init()` calls `io::init()`, clears the buffer and presents it. `shutdown()` does the same (all segments off).

## Example

From `demo/heads/seven_segment.cpp` (4 digits driven by shift registers):

```cpp
#include "head/seven_segment.h"
#include "tc.h"

static vic::head::seven_segment<4> digits(reinterpret_cast<vic::io::handle_type>(0));

int main()
{
  digits.init();
  vic::tc t(digits);
  t.cls();
  t.out({ 0, 0 }, reinterpret_cast<const std::uint8_t*>("12.34"));   // 4 digits, dp at the second one
  return 0;
}
```

## Controller notes

The head is not tied to a controller, so there is no datasheet summary. For the MAX7219 with LED matrices use the
[MAX7219](/vic/heads/max7219/) head.
