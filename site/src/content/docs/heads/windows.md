---
title: windows
description: Emulates a graphic or text display in a Win32 window, with zoom, for developing and running the demos on Windows.
sidebar:
  order: 54
---

The `windows` head emulates a graphic display in a Win32 window, `windows_text` an alpha numeric display. Both are
used to develop the application on the PC and run the vic demos on Windows; the pixels are zoomed so that small
displays are readable. They need no `io::init()`, `io::write()` or `io::read()`, only `io::delay()`.

```cpp
#include "head/Windows.h"
```

## windows: graphic display

```cpp
template<std::uint16_t Screen_Size_X, std::uint16_t Screen_Size_Y,
         std::uint16_t Viewport_Size_X = Screen_Size_X, std::uint16_t Viewport_Size_Y = Screen_Size_Y>
class windows;

windows(std::uint16_t viewport_x = 0U, std::uint16_t viewport_y = 0U,
        std::uint16_t window_x   = 0U, std::uint16_t window_y   = 0U,
        std::uint8_t zoom_x      = 1U, std::uint8_t zoom_y      = 1U,
        std::string caption = "VIC screen");
```

| Parameter | Type | Default | Meaning |
|---|---|---|---|
| `Screen_Size_X` | `std::uint16_t` | | screen (buffer) width |
| `Screen_Size_Y` | `std::uint16_t` | | screen (buffer) height |
| `Viewport_Size_X` | `std::uint16_t` | `Screen_Size_X` | viewport (window) width |
| `Viewport_Size_Y` | `std::uint16_t` | `Screen_Size_Y` | viewport (window) height |
| `viewport_x` | `std::uint16_t` | 0 | viewport x position (left) on the screen |
| `viewport_y` | `std::uint16_t` | 0 | viewport y position (top) on the screen |
| `window_x` | `std::uint16_t` | 0 | window x position on the Windows desktop |
| `window_y` | `std::uint16_t` | 0 | window y position on the Windows desktop |
| `zoom_x` | `std::uint8_t` | 1 | x zoom factor, every pixel is `zoom_x` window pixels wide |
| `zoom_y` | `std::uint8_t` | 1 | y zoom factor |
| `caption` | `std::string` | `"VIC screen"` | window caption |

- `init()` starts a worker thread which creates the window (viewport size times zoom) and runs the message loop,
  then clears and presents. `shutdown()` deletes the memory DC and calls `CloseWindow()`.
- The pixels are kept in a zoomed RGB888 frame buffer (`color::color_to_RGB888()`), `pixel_set()` writes
  `zoom_x * zoom_y` buffer pixels.
- `pixel_get()` reads the frame buffer, so alpha blending and anti aliasing work against the real pixels.
- `present()` copies the frame buffer to the window with `BitBlt()`. The drawing context calls it after every
  primitive, so you see the image being built up; use `dc.present_lock()` for whole frames.
- No orientation, the native hooks are the `drv` fallbacks.

## windows_text: alpha numeric display

```cpp
template<std::uint16_t Columns, std::uint16_t Rows,
         std::uint16_t Viewport_Size_X = Columns, std::uint16_t Viewport_Size_Y = Rows>
class windows_text;

windows_text(std::uint16_t viewport_x = 0U, std::uint16_t viewport_y = 0U,
             std::uint16_t window_x   = 0U, std::uint16_t window_y   = 0U,
             std::string caption = "vic text screen");
```

| Parameter | Type | Default | Meaning |
|---|---|---|---|
| `Columns` | `std::uint16_t` | | screen width in characters |
| `Rows` | `std::uint16_t` | | screen height in characters |
| `Viewport_Size_X`, `Viewport_Size_Y` | `std::uint16_t` | `Columns`, `Rows` | viewport size in characters |
| `viewport_x`, `viewport_y` | `std::uint16_t` | 0 | viewport offset |
| `window_x`, `window_y` | `std::uint16_t` | 0 | window position on the desktop |
| `caption` | `std::string` | `"vic text screen"` | window caption |

Every character is drawn in its own box with the "Enhanced LED Board-7" font (loaded from
`enhanced_led_board-7.ttf`), which looks like a dot matrix character display. `is_graphic()` returns `false`, write
to it with a text context `vic::tc`.

## The demo main.cpp

The graphic demos (`demo/demo_*.cpp`) implement `demo()` and get their head from `get_singlehead()`.
`demo/windows/main.cpp` provides the heads, the I/O layer and `main()`:

```cpp
#include <Windows.h>
#include "head/windows.h"
#include "head/multihead.h"

// create a single head: 320 x 200 pixel window with zoom factor 2
vic::drv& get_singlehead()
{
  static vic::head::windows<320, 200> _head( 0, 0, 0, 0, 2U, 2U );
  return _head;
}

// two windows as multihead
template<std::uint16_t Screen_Size_X, std::uint16_t Screen_Size_Y>
vic::drv& get_multihead()
{
  static vic::head::windows<250, 100> _head0( 0, 0, 0,   0, 4U, 4U );
  static vic::head::windows<250, 100> _head1( 0, 0, 0, 440, 4U, 4U );
  static vic::head::multihead<250U, 400U, 2U> _multihead = { { _head0, { 0, 0 }}, { _head1, { 0, 100 }} };
  return _multihead;
}

// a text multihead out of two 12 x 4 text windows
vic::drv& get_text_multihead()
{
  static vic::head::windows_text<12, 4> _head0( 0, 0,   0, 0 );
  static vic::head::windows_text<12, 4> _head1( 0, 0, 450, 0 );
  static vic::head::multihead<24U, 4U, 2U> _multihead = { { _head0, { 0U, 0U }}, { _head1, { 12U, 0U }} };
  return _multihead;
}

// the only io function the emulation needs
void vic::io::delay(std::uint32_t time_to_delay)
{
  ::Sleep(time_to_delay);
}

extern void demo(void);

int main()
{
  demo();
  vic::io::delay(100000U);
  return 0;
}
```

(`get_text_singlehead()`, a 16 x 4 `windows_text`, is left out above.) CMake builds every demo as `vic_<demo>`
together with `windows/main.cpp` and links `user32` and `gdi32`; `build.cmd` locates Visual Studio, see
[Installation](/vic/getting-started/installation/). The Linux counterpart is the [`x11`](/vic/heads/x11/) head.
