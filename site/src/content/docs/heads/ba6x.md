---
title: BA6x
description: Alpha numeric head for the Wincor Nixdorf BA60, BA63 and BA66 customer displays on USB or RS232.
sidebar:
  label: "BA6x (BA60, BA63, BA66)"
  order: 10
---

Head for the Wincor Nixdorf customer (pole) displays. The BA63 with two lines is the most common one, they are
connected via USB (modern version) or RS232. The head is alpha numeric (`is_graphic()` returns `false`), you write
to it with a text context `vic::tc`.

```cpp
#include "head/BA6x.h"
```

| Model | `BA6x_model_type` | Screen |
|---|---|---|
| BA60 | `BA60` | 1 line x 16 characters |
| BA63 | `BA63` | 2 lines x 20 characters |
| BA66 | `BA66` | 4 lines x 20 characters |

## Template parameters and constructor

```cpp
template<BA6x_model_type Model> class BA6x;
BA6x(io::handle_type device_handle);
```

| Parameter | Type | Default | Meaning |
|---|---|---|---|
| `Model` | `BA6x_model_type` | | display model, selects the screen size |
| `device_handle` | `io::handle_type` | | logical device ID (COM port number or USB ID) |

## Interface

Every command is sent as one framed message with `io::write()` option 0: `02h 00h <count> <data>`, at most 32 bytes
(29 data bytes), longer commands are split. After every message the head reads the 4 byte status answer with
`io::read()` option 0 (timeout 100 ms) and aborts if it is not `04h` with clean status bits.

| Call | Option | Meaning |
|---|---|---|
| `io::write()` | 0 | framed command `02h 00h count data` |
| `io::read()` | 0 | status answer `04h SB0 SB1 SB2`; `init()` also flushes the receive buffer with it |

The display commands inside the frames are ANSI like escape sequences: clear `ESC [ 2 J`, cursor
`ESC [ <row> ; <col> H` (1 based), clear to end of line `ESC [ 0 K`, character set `ESC R <code>`.

## Text functions

The head implements `cls()`, `text_out()` (char and string), `text_set_pos()` and `text_clear_eol()`. Characters
below 20h are ignored. Orientation, viewport and graphics are not supported, `pixel_get()` is not applicable.

:::caution
The string version of `text_out()` sends the bytes as they are, without UTF-8 decoding: the string must be in the
character set of the display (see `set_country_code()`).
:::

## Head specific functions

| Function | Meaning |
|---|---|
| `set_country_code(country_code_type)` | select the character set: country sets (`USA`, `Germany`, `France`, ...) or code pages (`CP437`, `CP850`, `CP852`, `CP857`, `CP858`, `CP866`, `CP737`, `CP862`, `IBM813`, `Katakana`, `UserPage`) |
| `self_test()` | starts the display self test (`00h 10h 00h`) and waits up to 30 s for the answer, returns `true` on success |
| `device_ready()` | status request (`00h 20h 00h`), returns `true` if the display is ready |

## Init sequence

`init()` calls `io::init()`, waits 750 ms (the display needs that time to get ready), flushes the receive buffer,
clears the display and sets the cursor home. `shutdown()` clears the display.

## Example

From `demo/heads/BA6x.cpp`:

```cpp
#include "head/BA6x.h"
#include "tc.h"

static vic::head::BA6x<vic::head::BA63> display(reinterpret_cast<vic::io::handle_type>(0));

int main()
{
  display.init();
  vic::tc t(display);
  t.cls();
  t.out({ 0, 0 }, reinterpret_cast<const std::uint8_t*>("vic head demo"));
  t.out({ 0, 1 }, reinterpret_cast<const std::uint8_t*>("Hello World"));
  return 0;
}
```

`demo/BA63-single.cpp` and `demo/BA63-dual.cpp` show a real USB HID I/O layer on Windows.

## Controller notes

There is no datasheet summary in `src/head/doc/` for the BA6x. The protocol used by the head is described above.
