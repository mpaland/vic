---
title: Widgets
description: The widget base class, the config_type pattern, redraw behaviour and the input event routing of the vic widgets.
sidebar:
  label: Overview
  order: 0
---

Widgets are ready made controls like gauges, labels, buttons and sliders. They live in `src/widget/`, one header per
widget, all in the namespace `vic::widget`. Every widget draws through its own drawing context (and text context) on
the head it was created for, so a widget works on every graphic head, on a `framebuffer` and on a `multihead`.

![Widget demo](/vic/media/demo_widget.gif)

## Concept

All widgets derive from `vic::widget::base` (`widget/widget.h`) and follow the same pattern:

1. Fill a `config_type` struct. It is a plain aggregate without defaults, every member is set in declaration order.
2. Construct the widget with the head and the config: `widget(drv& head, const config_type& config)`. The config is
   copied, pointers inside it (fonts, captions, bands, markers, XPM images) must stay valid.
3. Call `init()` to draw the widget the first time.
4. Change it with its setters (`set_value()`, `set_text()`, `set_state()`, ...). A setter draws only if something
   changed, so it can be called every frame.

```cpp
#include "widget/progress.h"

vic::widget::progress::config_type cfg = { { 10, 10 }, 100U, 13U, vic::widget::progress::left_to_right,
                                           0, 100, vic::color::red, vic::color::darkgray, vic::color::white };
vic::widget::progress prog(_head, cfg);   // _head is any graphic head (vic::drv&)
prog.init();                              // initial draw
prog.set_pos(50);                         // delta draw: only the changed part of the bar
```

### Redraw behaviour

- Every widget (except `bitmap`) has a public `redraw()` which draws it completely. `init()` calls it.
- A widget draws within a `present_lock()` of its drawing context, so the head is presented once per change.
- Setters draw as little as possible: `progress`, `arc`, `spinner` and the fill of `bargauge` are delta drawn,
  `label`, `checkbox`, `led`, `toggle`, `slider`, `spinbox` and `compass` redraw only on a changed value.
- Colors with `color::none` (for example `color_bg` or `color_border`) mean "don't draw this part", the background
  below is left untouched.
- Widgets with `anti_aliasing_enable()` need a head which can read its pixels back (or a background color set in the
  drawing context), see [Anti aliasing](/vic/design/anti-aliasing/).

### Canvas sprites for needles and markers

Moving parts are [canvas sprites](/vic/reference/sprite/): the needle of `gauge` and the markers of `bargauge`. A
sprite stores the background below its pixels and restores it when it moves, so moving a needle costs only the needle
pixels and the scale is never redrawn. The template parameter sizes the sprite buffer: `gauge<Needle_Pixels = 256>`,
`bargauge<Marker_Pixels = 32>`.

### Helpers in widget.h

| Name | Description |
|---|---|
| `band_type` | `{ from, to, color }`, a colored value range of a scale, `to` is exclusive. Used by `gauge` and `bargauge`. |
| `event_type` | The input events, see below. |
| `sector_cw(dc, center, ri, ro, a0, a1)` | Draw a ring sector from compass angle `a0` to `a1` (0 is 12 o'clock, clockwise). |
| `erase_edges(dc, center, ri, ro, aa, bg)` | Erase a ring including two pixel of anti aliased halo with `bg`, does nothing if `aa` is false or `bg` is `color::none`. |
| `value_to_angle(value, lower, upper, a_start, a_end)` | Map a value of a range to an angle, rounded to the closest degree. |

## Input routing

The widgets are kept in a global chain in creation order (registered by the constructor, removed by the destructor).
The application feeds its input events (touch controller, mouse, encoder button) into the static
`base::event()`, which finds the widget at the position and routes the event to it.

```cpp
static base* event(vertex_type vertex, event_type ev);
```

Finds the widget which contains `vertex`. With overlapping widgets the one created last wins, as it is drawn on top
(for example a LED inside a group box). The widget reacts first (pressed look, toggle, new value), then its event
handler is called. Returns the widget which received the event, `nullptr` if there is no widget at the position.

| `event_type` | Meaning |
|---|---|
| `event_down` | Button / finger down |
| `event_up` | Button / finger up |
| `event_tap` | Short tap (down and up) |
| `event_long_tap` | Long tap |

A touch driver typically sends `event_down` on touch, `event_up` on release and then `event_tap` or
`event_long_tap`, depending on the press duration. Sending `event_down` on every move gives a dragging `slider` knob.

```cpp
void set_event_handler(event_handler_type handler, void* arg = nullptr);
```

Set the event handler of the widget, `nullptr` for none. `arg` is passed to the handler. The handler is called by
`event()` after the widget processed the event, so it sees the new state:

```cpp
typedef void (*event_handler_type)(base& widget, vertex_type vertex, event_type ev, void* arg);
```

```cpp
void event_enable(bool enable = true);
```

Enable or disable the event routing to this widget. A disabled widget is transparent for `event()`, the widget below
it receives the event (for example a decoration inside a button). `imagebutton` disables its internal `bitmap` this
way.

```cpp
base* find(vertex_type vertex);
```

Hit test: returns the widget at `vertex` (the last created one on overlaps), `nullptr` if none.

```cpp
#include "widget/checkbox.h"

void on_sound(vic::widget::base& w, vic::vertex_type pos, vic::widget::event_type ev, void* arg)
{
  (void)pos; (void)arg;
  if (ev == vic::widget::event_tap) {
    const bool on = static_cast<vic::widget::checkbox&>(w).is_checked();   // already toggled
    (void)on;   // switch the sound on or off
  }
}

cb_sound.set_event_handler(on_sound);   // cb_sound is a vic::widget::checkbox

// in the touch driver, touch_pos is the vic::vertex_type of the touch
vic::widget::base::event(touch_pos, vic::widget::event_down);
vic::widget::base::event(touch_pos, vic::widget::event_up);
vic::widget::base::event(touch_pos, vic::widget::event_tap);
```

Widgets react to these events by themselves:

| Widget | Reaction |
|---|---|
| `button`, `imagebutton` | `event_down` shows the pressed look, `event_up` releases it |
| `checkbox`, `toggle` | `event_tap` toggles the state |
| `slider` | `event_down` and `event_tap` set the value to the event position |
| `spinbox` | `event_tap` on the minus / plus cell changes the value by `step`, `event_long_tap` by `10 * step` |
| all others | no own reaction, the event handler is still called (a LED can act as a button) |

The [input example](/vic/examples/input/) operates a complete settings page with a simulated touch.

## Widget list

| Widget | Header | Description |
|---|---|---|
| [`arc`](/vic/reference/widgets/arc/) | `widget/arc.h` | Ring which fills with the value, a round progress bar |
| [`bargauge`](/vic/reference/widgets/bargauge/) | `widget/bargauge.h` | Linear scale with bands, ticks, fill bar and up to two markers |
| [`bitmap`](/vic/reference/widgets/bitmap/) | `widget/bitmap.h` | Raw bitmaps (L1, RGB332, RGB555, RGB565, RGB888, ARGB8888) and XPM3 images |
| [`button`](/vic/reference/widgets/button/) | `widget/button.h` | Push button with a caption |
| [`checkbox`](/vic/reference/widgets/checkbox/) | `widget/checkbox.h` | Box with a check mark and a caption |
| [`compass`](/vic/reference/widgets/compass/) | `widget/compass.h` | Heading up compass rose |
| [`gauge`](/vic/reference/widgets/gauge/) | `widget/gauge.h` | Round gauge with bands, ticks and a sprite needle |
| [`groupbox`](/vic/reference/widgets/groupbox/) | `widget/groupbox.h` | Frame with a title cut into the top border |
| [`imagebutton`](/vic/reference/widgets/imagebutton/) | `widget/imagebutton.h` | XPM icon with a caption, a soft key |
| [`label`](/vic/reference/widgets/label/) | `widget/label.h` | Text or numeric readout in a fixed cell |
| [`led`](/vic/reference/widgets/led/) | `widget/led.h` | Round or square indicator |
| [`progress`](/vic/reference/widgets/progress/) | `widget/progress.h` | Progress bar in four orientations |
| [`slider`](/vic/reference/widgets/slider/) | `widget/slider.h` | Track with a knob |
| [`spinbox`](/vic/reference/widgets/spinbox/) | `widget/spinbox.h` | Numeric value with minus and plus buttons |
| [`spinner`](/vic/reference/widgets/spinner/) | `widget/spinner.h` | Rotating ring segment, a busy indicator |
| [`toggle`](/vic/reference/widgets/switch/) | `widget/switch.h` | On / off switch with a sliding knob |
| [`textbox`](/vic/reference/widgets/textbox/) | `widget/textbox.h` | Multi line text with word wrap and scrolling |
