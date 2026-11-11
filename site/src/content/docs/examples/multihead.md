---
title: Multiple heads
description: Several independent displays with their own drawing contexts, and the multihead driver which combines displays to one screen.
sidebar:
  order: 8
---

A system can drive several displays. vic knows two scenarios: independent heads, each with its own drawing context, and
the `multihead` driver which combines several displays to one big screen.

## Independent heads

Every head is an independent driver instance, every drawing context draws on exactly one head:

```cpp
#include "head/Windows.h"
#include "dc.h"

// two 240 x 120 windows with zoom 2, the second one placed 200 pixel lower on the desktop
vic::head::windows<240, 120> _head0(0U, 0U, 0U,   0U, 2U, 2U);
vic::head::windows<240, 120> _head1(0U, 0U, 0U, 200U, 2U, 2U);
_head0.init();
_head1.init();

vic::dc _dc0(_head0);
vic::dc _dc1(_head1);
_dc0.set_color(vic::color::blue);
_dc0.line({ 10, 10 }, { 40, 40 });
_dc1.set_color(vic::color::red);       // e.g. for system debug, status etc.
_dc1.box({ 100, 100, 120, 120 });
```

The heads can be of different types, for example a color TFT as main display and a small monochrome OLED for a status
line.

## The multihead driver

`vic::head::multihead` (`head/multihead.h`) is a head itself. It owns no pixels, it maps the coordinates of its virtual
screen onto the registered heads. This is the common way to build big screens out of single modules:

```cpp
#include "head/MAX7219.h"
#include "head/multihead.h"
#include "dc.h"

// three MAX7219 8 x 8 LED matrix heads on the SPI device handles spi1, spi2 and spi3
vic::head::MAX7219<8U, 8U> _head0(spi1);
vic::head::MAX7219<8U, 8U> _head1(spi2);
vic::head::MAX7219<8U, 8U> _head2(spi3);

// the multihead: a 24 x 8 screen out of three heads at the offsets 0, 8 and 16
vic::head::multihead<24U, 8U, 3U> _multihead = { { _head0, { 0, 0 } }, { _head1, { 8, 0 } }, { _head2, { 16, 0 } } };
_multihead.init();                     // initializes all registered heads

// use the multihead like a normal head
vic::dc _dc(_multihead);
_dc.line({ 0, 0 }, { 23, 7 });
```

- Template parameters: `multihead<Screen_Size_X, Screen_Size_Y, Head_Count>`. All heads must be given in the
  initializer list, heads beyond `Head_Count` are ignored.
- Every entry is `{ head, viewport }`: the head and the position of its top left pixel in the multihead screen.
- `init()` and `shutdown()` are forwarded to all heads. The multihead is graphic if at least one head is graphic.
- All rendering is done by vic, only `pixel_set()` and `pixel_get()` of the heads are used, the hardware acceleration
  of a head is not used.

`demo/heads/multihead.cpp` builds a 32 x 8 screen out of four MAX7219 modules and dumps the resulting command stream.

## Multihead on the desktop

`demo/windows/main.cpp` provides a multihead out of two windows next to the single head of the demos. Two 250 x 100
windows with zoom 4 are stacked: the first one shows the lines 0 - 99 of the multihead screen, the second one the
lines 100 - 199 (condensed, in the file `get_multihead()` is a template):

```cpp
#include "head/Windows.h"
#include "head/multihead.h"

vic::drv& get_multihead()
{
  static vic::head::windows<250, 100> _head0(0, 0, 0,   0, 4U, 4U);
  static vic::head::windows<250, 100> _head1(0, 0, 0, 440, 4U, 4U);
  static vic::head::multihead<250U, 400U, 2U> _multihead = { { _head0, { 0, 0 } }, { _head1, { 0, 100 } } };
  return _multihead;
}
```

The same file has a text variant: two `windows_text<12, 4>` alpha numeric heads combined to a 24 x 4 character
multihead.

See [Multihead](/vic/heads/multihead/) for the driver reference and [Heads](/vic/heads/) for all display drivers.
