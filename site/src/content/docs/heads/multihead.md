---
title: multihead
description: Combines several heads to one big screen or video wall, graphic or alpha numeric.
sidebar:
  order: 52
---

The `multihead` combines several single displays (heads) to one big screen, e.g. a row of LED matrices, two TFTs on
top of each other or a video wall of character displays. Every head gets a position in the multihead screen, the
drawing context draws on the multihead as on one display.

```cpp
#include "head/multihead.h"
```

## Template parameters and constructor

```cpp
template<std::uint16_t Screen_Size_X, std::uint16_t Screen_Size_Y, std::size_t Head_Count>
class multihead;

multihead(const std::initializer_list<head_type>& il);
```

| Parameter | Type | Default | Meaning |
|---|---|---|---|
| `Screen_Size_X` | `std::uint16_t` | | (virtual) screen width |
| `Screen_Size_Y` | `std::uint16_t` | | (virtual) screen height |
| `Head_Count` | `std::size_t` | | number of heads, all heads must be given in the constructor initializer list |
| `il` | `std::initializer_list<head_type>` | | `{ head, { x, y } }` pairs: the head and its top left position in the multihead screen; heads beyond `Head_Count` are ignored |

`head_count()` returns the number of registered heads.

## How it works

| Function | Behaviour |
|---|---|
| `init()`, `shutdown()`, `cls()`, `present()` | called on every head |
| `pixel_set()` | every head gets the vertex in its own coordinates (vertex - position) and clips it itself |
| `pixel_get()` | read from the first head which covers the vertex, `color::none` if no head covers it |
| text functions | `text_set_pos()` is translated into the coordinates of every head, `text_out()` and the clear functions are called on every head, so every head renders its own part |
| `display_enable()`, `display_brightness()`, `display_contrast()` | forwarded to every head |
| `is_graphic()` | `true` if at least one head is graphic |

:::note
The multihead implements only `pixel_set()` and `pixel_get()`: lines, boxes, `move()` and `blit()` use the `drv`
fallbacks pixel by pixel, the native hooks of the heads (window streams, accelerator commands) are not used.
:::

Whether `pixel_get()` really reads back depends on the heads; a write only head returns `color::none` for its area.

## Example

From `demo/heads/multihead.cpp`, four MAX7219 8 x 8 modules side by side as one 32 x 8 screen. Every module is its
own head with its own chip select handle:

```cpp
#include "head/MAX7219.h"
#include "head/multihead.h"
#include "scene.h"          // demo_scene(): frame, line, circle, disc and text

static vic::head::MAX7219<8, 8> m0(reinterpret_cast<vic::io::handle_type>(0));
static vic::head::MAX7219<8, 8> m1(reinterpret_cast<vic::io::handle_type>(1));
static vic::head::MAX7219<8, 8> m2(reinterpret_cast<vic::io::handle_type>(2));
static vic::head::MAX7219<8, 8> m3(reinterpret_cast<vic::io::handle_type>(3));
static vic::head::multihead<32, 8, 4> wall = { { m0, { 0, 0 } }, { m1, { 8, 0 } }, { m2, { 16, 0 } }, { m3, { 24, 0 } } };

int main()
{
  wall.init();              // inits all four modules
  demo_scene(wall);
  return 0;
}
```

The Windows demo `demo/windows/main.cpp` builds a multihead out of two emulated windows (`get_multihead()`) and a
text multihead out of two `windows_text` heads (`get_text_multihead()`), see [windows](/vic/heads/windows/) and the
[multihead example](/vic/examples/multihead/).
