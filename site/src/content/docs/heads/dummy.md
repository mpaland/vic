---
title: dummy
description: Null head that swallows every call, a placeholder for a display that is not there yet.
sidebar:
  order: 53
---

The `dummy` head is a non existent display replacement. All drawing, text and present functions are the no-op
fallbacks of `drv`, so every call compiles and runs and nothing is output. `pixel_get()` returns `color::none`. Use it
as placeholder while the hardware is not there yet, or to measure the rendering time without any I/O. If you need a
head that stores the pixels, use the [`memory`](/vic/heads/memory/) head.

```cpp
#include "head/dummy.h"
```

## Template parameters and constructor

```cpp
template<std::uint16_t Screen_Size_X = 0U, std::uint16_t Screen_Size_Y = 0U,
         std::uint16_t Viewport_Size_X = Screen_Size_X, std::uint16_t Viewport_Size_Y = Screen_Size_Y>
class dummy;

dummy(std::int16_t viewport_x = 0, std::int16_t viewport_y = 0, bool is_graphic = true);
```

| Parameter | Type | Default | Meaning |
|---|---|---|---|
| `Screen_Size_X` | `std::uint16_t` | 0 | screen width |
| `Screen_Size_Y` | `std::uint16_t` | 0 | screen height |
| `Viewport_Size_X` | `std::uint16_t` | `Screen_Size_X` | viewport width |
| `Viewport_Size_Y` | `std::uint16_t` | `Screen_Size_Y` | viewport height |
| `viewport_x` | `std::int16_t` | 0 | viewport x position (left) on the screen |
| `viewport_y` | `std::int16_t` | 0 | viewport y position (top) on the screen |
| `is_graphic` | `bool` | true | act as graphic head (true) or alpha numeric head (false) |

`init()` and `shutdown()` do nothing, no I/O function is called.

## Example

From `demo/heads/dummy.cpp`:

```cpp
#include "head/dummy.h"
#include "scene.h"          // demo_scene(): frame, line, circle, disc and text

static vic::head::dummy<128, 64> nothing;

int main()
{
  nothing.init();
  demo_scene(nothing);
  std::printf("nothing happened, as expected\n");
  return 0;
}
```
