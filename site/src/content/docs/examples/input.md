---
title: Input
description: A settings page with input widgets and a compass, operated by a simulated touch through the widget event routing.
sidebar:
  order: 5
---

`demo_input.cpp` is a settings page: checkboxes, a switch, a slider, a spinbox and two buttons on the left, a heading up
compass with its readout and a soft key bar of image buttons on the right. A simulated touch cursor walks through a
script of taps and long taps. Every event goes through `widget::base::event()` exactly as a touch driver would send
it, the widgets react and their event handlers report into a status line.

![Input demo](/vic/media/demo_input.gif)

## The widgets

```cpp
#include "dc.h"
#include "sprite.h"
#include "widget/label.h"
#include "widget/groupbox.h"
#include "widget/checkbox.h"
#include "widget/switch.h"
#include "widget/slider.h"
#include "widget/spinbox.h"
#include "widget/button.h"
#include "widget/compass.h"
#include "font/LCD_6x10.h"

const vic::color::value_type COL_BG    = vic::color::black;
const vic::color::value_type COL_TEXT  = vic::color::white;
const vic::color::value_type COL_FRAME = vic::color::gray08;
const vic::color::value_type COL_CTRL  = vic::color::gray04;
const vic::color::value_type COL_GREEN = vic::color::argb(0x20U, 0xD0U, 0x20U);
const vic::color::value_type COL_CYAN  = vic::color::argb(0x40U, 0xE0U, 0xFFU);

vic::font::LCD_6x10 font_lcd;

vic::drv& head = get_singlehead();   // from demo/windows/main.cpp or demo/linux/main.cpp
head.init();
vic::dc _dc(head);
_dc.cls(COL_BG);

// group box first: widgets created later are on top for the event routing
const vic::widget::groupbox::config_type grp_cfg = { { 2, 10, 154, 166 }, 4U, &font_lcd, "settings", COL_FRAME, COL_TEXT, COL_BG };
vic::widget::groupbox grp(head, grp_cfg);

const vic::widget::checkbox::config_type cb_cfg = { { 12, 22 }, 12U, &font_lcd, "Sound", COL_TEXT, COL_GREEN, COL_CTRL, COL_FRAME };
vic::widget::checkbox cb_sound(head, cb_cfg);

const vic::widget::toggle::config_type sw_cfg = { { 100, 58 }, 30U, 14U, COL_GREEN, COL_CTRL, COL_TEXT, COL_FRAME };
vic::widget::toggle sw_auto(head, sw_cfg);

const vic::widget::slider::config_type sl_cfg = { { 10, 90 }, 136U, 16U, vic::widget::slider::horizontal, 0, 100, 4U, 6U,
                                                  COL_CTRL, COL_CYAN, COL_TEXT, COL_BG };
vic::widget::slider sl_bright(head, sl_cfg);

const vic::widget::spinbox::config_type sb_cfg = { { 70, 108, 146, 126 }, 3U, &font_lcd, 0, 20000, 100, 0U,
                                                   COL_TEXT, COL_BG, COL_CTRL, COL_FRAME };
vic::widget::spinbox sb_alt(head, sb_cfg);

const vic::widget::button::config_type ok_cfg = { { 12, 136, 76, 158 }, 4U, &font_lcd, "OK", COL_TEXT, COL_CTRL, COL_FRAME, COL_FRAME };
vic::widget::button btn_ok(head, ok_cfg);

const vic::widget::compass::config_type cp_cfg = { { 239, 76 }, 52U, &font_lcd, COL_BG, COL_FRAME, COL_TEXT, COL_TEXT, vic::color::magenta };
vic::widget::compass cp(head, cp_cfg);

const vic::widget::label::config_type status_cfg = { { 4, 172 }, 156U, 12U, &font_lcd, vic::widget::label::align_left,
                                                     COL_CYAN, COL_BG, vic::color::none };
vic::widget::label status(head, status_cfg);

grp.init();
cb_sound.init(); sw_auto.init(); sl_bright.init(); sb_alt.init(); btn_ok.init();
cp.anti_aliasing_enable();
cp.init();
status.init();
cb_sound.set_checked(true);
sl_bright.set_value(60);
sb_alt.set_value(3500);
```

## Event handlers

One generic handler reports taps and long taps of every widget. The handler argument carries the widget name:

```cpp
// handler argument: the status label and the name of the widget
struct report_type {
  vic::widget::label* line;
  const char*         name;
};

void on_widget(vic::widget::base& w, vic::vertex_type v, vic::widget::event_type ev, void* arg)
{
  (void)w; (void)v;
  const report_type* r = static_cast<const report_type*>(arg);
  if (ev == vic::widget::event_tap)      { r->line->set_text(r->name); }
  if (ev == vic::widget::event_long_tap) { r->line->set_text("long tap"); }
}

report_type rep_sound = { &status, "Sound" }, rep_auto = { &status, "Autopilot" }, rep_ok = { &status, "OK" };
cb_sound.set_event_handler(on_widget, &rep_sound);
sw_auto.set_event_handler(on_widget, &rep_auto);
btn_ok.set_event_handler(on_widget, &rep_ok);
```

The soft keys are [image buttons](/vic/reference/widgets/imagebutton/), their handler makes the tapped key the active
one with `set_active()`.

## The simulated touch driver

The touch script is a list of positions and events. A crosshair canvas sprite moves to the position in 30 frames, at
frame 40 the finger goes down, at frame 48 (56 for a long tap) it goes up and the tap is sent:

```cpp
struct step_type {
  vic::vertex_type        pos;
  vic::widget::event_type ev;       // event_tap or event_long_tap
};

const step_type script[] = {
  { {  18,  28 }, vic::widget::event_tap },        // Sound off
  { { 115,  65 }, vic::widget::event_tap },        // Autopilot on
  { {  40,  98 }, vic::widget::event_tap },        // Brightness low
  { { 137, 117 }, vic::widget::event_tap },        // Altitude + 100
  { { 137, 117 }, vic::widget::event_long_tap },   // Altitude + 1000
  { {  44, 147 }, vic::widget::event_tap },        // OK
};
const std::uint32_t steps = sizeof(script) / sizeof(script[0]);

// the touch cursor: a crosshair canvas sprite with a high z-index
vic::sprite::canvas<64, 1> cursor(head, 10);
cursor.set_color(COL_TEXT);
cursor.line({ -6, 0 }, { 6, 0 });
cursor.line({ 0, -6 }, { 0, 6 });

vic::vertex_type cursor_pos = { 160, 100 };
vic::vertex_type from       = cursor_pos;
for (std::uint32_t t = 0U; ; ++t) {
  const std::uint32_t step  = (t / 60U) % steps;
  const std::uint32_t phase = t % 60U;
  const vic::vertex_type to = script[step].pos;
  if (phase == 0U) {
    from = cursor_pos;
  }
  cursor_pos = phase < 30U ? vic::vertex_type{ static_cast<std::int16_t>(from.x + (to.x - from.x) * static_cast<std::int32_t>(phase) / 30),
                                               static_cast<std::int16_t>(from.y + (to.y - from.y) * static_cast<std::int32_t>(phase) / 30) }
                           : to;

  // the touch driver part: down, then up and the tap
  const bool long_tap = script[step].ev == vic::widget::event_long_tap;
  if ((phase == 40U) || (phase == (long_tap ? 56U : 48U))) {
    cursor.render(cursor_pos, 0xFFFFU);              // hide the cursor while the widget below redraws
    if (phase == 40U) {
      vic::widget::base::event(cursor_pos, vic::widget::event_down);
    }
    else {
      vic::widget::base::event(cursor_pos, vic::widget::event_up);
      vic::widget::base::event(cursor_pos, script[step].ev);
    }
  }
  cursor.render(cursor_pos, 0U);

  // the compass turns
  cp.set_heading(static_cast<std::int16_t>((t / 3U) % 360U));
  vic::io::delay(40U);
}
```

## Key points

- `base::event(pos, ev)` finds the widget at `pos` (the last created one on overlaps), lets it react and then calls its
  handler. A touch, mouse or encoder driver needs nothing else.
- The widgets react by themselves: the button shows its pressed look between `event_down` and `event_up`, checkbox and
  toggle flip on `event_tap`, the slider jumps to the position, the spinbox steps by `step` or `10 * step` on a long
  tap. The handler runs afterwards and sees the new state.
- The cursor sprite is hidden (rendered with frame `0xFFFF`) while a widget below it redraws, so the sprite doesn't
  restore a stale background.
- A tap beside the widgets but inside the group box goes to the group box, a tap outside of all widgets returns
  `nullptr` from `event()`.

See [Widgets](/vic/reference/widgets/#input-routing) for the routing and the reference pages of the input widgets:
[button](/vic/reference/widgets/button/), [checkbox](/vic/reference/widgets/checkbox/),
[toggle](/vic/reference/widgets/switch/), [slider](/vic/reference/widgets/slider/),
[spinbox](/vic/reference/widgets/spinbox/), [imagebutton](/vic/reference/widgets/imagebutton/) and
[compass](/vic/reference/widgets/compass/).
