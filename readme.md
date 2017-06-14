# Video GraphiX library for driving displays and LCDs

## What is the vgx library?
A lightweight and extremly fast and powerful primitive and text rendering C++ library for driving graphic and alpha numeric displays.
If you need a robust and universal library for rendering text and graphics on various displays - VGX is your library!

**vgx is beta and still under heavy development. If you have specific questions, do not hesitate to contact me!**

## What are the highlights?
- Universal library for graphic and alpha numeric displays
- High performance primitive rendering of lines, circles, boxes, text etc.
- Support of advanced controls like gauges, bars, (radio) buttons, checkboxes etc.
- Platform and CPU independent code, NO dependencies, NO STL, NO new/delete, just clean and pure C++
- Platform independant driver design, the very same display driver runs everywhere
- Very easy to use and fast implemention of own/new display drivers
- HEADER ONLY implementation, no module compilation
- NO floating point math, only fast integer operations
- VERY clean and stable C++ code, LINT and L4 warning free, automotive ready
- Multiple heads support, as many displays as you may like in one system
- Multihead driver, combine single displays to one big display
- Support of various color formats from 1 to 32 bpp displays
- Support for different font formats (proportional, monospace), ASCII/UTF-8 support
- Pen support for line drawing
- Antialiasing support for primitive and text/font rendering
- Framebuffer and viewport support
- Clipping region support
- Gradient color rendering support
- Doxygen commented code
- MIT license

## What is vgx NOT?
VGX is not meant to be a full bloated window manager, widgets, dialogs, theme rendering framework.
If you need an advanced windowed GUI, there are many other cool libraries around, like µC/GUI, StWin etc.

## To be done (and implemented)
- Improve anti aliasing support for primitives
- Implement fast fill algorithm
- Add more standard and high quality fonts


## Design of vgx
Basicly every display needs a driver. But unlike as in many other designs, the driver is the highest class of which the head
(display) is instanciated.
A graphic driver only needs to implement `drv_pixel_set_color` and `drv_pixel_get` operations. But most modern
display controllers can provide more sophisticated functions like line, box rendering, block moving etc.
If a controller can provide such special functions, the according function is implemented in the driver by simply overriding the virtual function of the `gpr` base class.
All graphic functions which the controller/driver can't provide natively are handled by the `gpr`.
Of cource, native rendering on a specialized LCD controller is always faster and should be prefered.


## Class design
![](https://cdn.rawgit.com/mpaland/vgx/master/docs/vgxlib.svg)

### base
The base class provides common functions.

### gpr
The Graphic Primitive Renderer provides fast rendering functions for graphic primitives.
The `gpr` doesn't use any floating point math, only fast integer operations.
Furthermore the `gpr` should be a collection of reference implementations of the most modern, fast and reliable primitive rendering algorithms.
So, if you find any errors, have speed improvements or even a better algorithm for some primitives - **please** share and
contribure!

### txr
The Text Renderer is responsible for rendering text fonts on graphic displays. It supports monospaced and proportional fonts.

### drv
The base class of the head driver which adds some mandatory driver routines.
All color calculations are done in ARGB 32 bit format. If a display has less colors, like 15 bit RGB, the specific driver
class is responsible for color reduction. Conversion routines provides the `drv` class.

### head
The specific display driver. A display together with its controller is referenced here as `head`. And `head` is the namespace of all specific drivers.
All virtual functions of the `gpr` and `drv` are overridden here.

### drv_io
This class has four routines for direct hardware access. You have to implement these four routines according to your hardware.

### ctrl
The base class for advanced controls like gauges, progress bars, checkboxes, buttons etc.


## Usage
Using vgx is pretty easy.
In a single head design (just one display in the system) you create your head first by instancing the according driver.
E.g. on a Windows (emulation/test) platform this would be:
```c++
// Create a screen of 240 * 120 pixel with a viewport of the same size (240 * 120)
// The viewport has an offset of 0,0 and the window is placed at 100,100 on the windows desktop
// For development convinience a horizontal and vertical zoom factor of 4 is selected
vgx::head::windows<240,120,240,120> _head(0, 0, 100, 100, 4, 4);
_head.init();  // explicit head init
```

Now you can perform any operations on the head:
```c++
_head.color_set(vgx::color::brightblue);  // select bright blue drawing color
_head.line(10, 10, 40, 40);               // draw a line from 10,10 to 40,40

// write some text
_head.color_set(vgx::color::yellow);
_head.text_set_font(vgx::font::arial_num_16x24_aa4);
_head.text_string_pos(10, 50, (const std::uint8_t*)"12,34.56-7890");
```

If you need multiple displays in your design, we are talking about a "multiple heads" scenario. The heads are independent.
Let's create two heads:
```c++
vgx::head::windows<240,120,240,120> _head0(0, 0, 0,  00, 2, 2);
vgx::head::windows<240,120,240,120> _head1(0, 0, 0, 200, 2, 2);
_head0.init();  // explicit head 0 init
_head1.init();  // explicit head 1 init
```
Using head 0:
```c++
_head0.color_set(vgx::color::brightblue);
_head0.line(10, 10, 40, 40);
```
And using head 1 (eg. for system debug, status. etc):
```c++
head1.color_set(vgx::color::red);
head1.box(100, 100, 120, 120);
```

Or you can combine multiple single displays to a bigger one. This is a common way to create big screens out of single modules.
This is done by using the `multihead` driver.
```c++
// Create 3 MAX7219 heads with a rotation of 90° on SPI device ID 1,2 and 3
// The single heads have a resolution of 8 * 8 pixels
vgx::head::MAX7219<8U, 8U> _head0(drv::orientation_90, 1U);
vgx::head::MAX7219<8U, 8U> _head1(drv::orientation_90, 2U);
vgx::head::MAX7219<8U, 8U> _head2(drv::orientation_90, 3U);
// Create the multihead out of the three heads with offsets of 0, 8 and 16 pixels
// So the multihead has a srceen resolution of 24 * 8 pixels
vgx::head::multihead<24U, 8U, 3U> _multihead = { {_head0, 0U, 0U}, {_head1, 8U, 0U}, {_head2, 16U, 0U} };

// Now, use the multihead like a normal head
_multihead.line(0, 0, 23, 7);
```


## License
vgx is written under the MIT licence.
