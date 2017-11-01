# Video Interface Controller library
#### A universal, lightweight, extremly fast and powerful C++ library for driving graphic, dot, alpha numeric and seven-segment displays in embedded systems.
If you need a robust, universal and embedded library for rendering text and graphics on various displays - VIC is your library!

**The VIC lib is beta and still under heavy development. If you have specific questions, do not hesitate to contact me!**

## Highlights
- Universal library for graphic and alpha numeric displays
- HEADER ONLY implementation, no module compilation
- Platform and CPU independent code, NO dependencies, NO STL, NO new/delete, NO `float` - just clean and pure C++11
- Platform independent driver design, the very same low level display driver runs everywhere
- High performance primitive rendering of lines, circles, triangles, boxes, text etc.
- Support of advanced controls like gauges, bars, (radio) buttons, checkboxes etc.
- Sprite support for moving objects
- Multiple heads support, as many displays as you may like in one system
- Multihead driver, combine any number of single displays to one big display
- Support of various color formats from 1 to 32 bpp displays
- Support for different font formats (proportional, monospace), ASCII/UTF-8 support
- Pen shape support for line drawing
- Antialiasing support for smooth primitive and text/font rendering
- Framebuffer and viewport support
- Clipping region support
- Gradient color rendering support
- NO floating point math, only fast integer operations
- VERY clean and stable C++ code, LINT and L4 warning free, automotive ready
- Very easy to use and fast implemention of own/new display drivers
- Doxygen commented code
- MIT license

## What is VIC NOT?
vic is not meant to be a full bloated window manager, widgets, dialogs, theme rendering framework.
If you need an advanced windowed GUI, there are many other cool libraries around, like µC/GUI, StWin etc.

## To be done (and implemented)
- Improve anti aliasing support
- Sprite support
- Add more standard and high quality fonts


## Design of VIC
Basicly every display needs a driver. But unlike as in many other designs, the driver is the highest class of which the head
(display) is instanciated.
A graphic driver only needs to implement `drv_pixel_set_color` and `drv_pixel_get` functions. But most modern
display controllers can provide more sophisticated functions for native line/box rendering, block moving etc.
If a controller can provide such special functions, the according function is implemented in the driver by simply overriding the virtual function of the `gpr` base class.
All graphic functions which the controller/driver can't provide natively are handled by the `gpr`.
Of cource, native rendering on a specialized LCD controller is always faster and should be prefered.


## Class design
![](https://cdn.rawgit.com/mpaland/vic/v.0.3.0/doc/viclib.svg)

### gpr
The Graphic Primitive Renderer provides fast rendering functions for graphic primitives.
The `gpr` doesn't use any floating point math, only fast integer operations.
Furthermore the `gpr` should be a collection of reference implementations of the most modern, fast and reliable primitive rendering algorithms.
So, if you find any errors, have speed improvements or even a better algorithm for some primitives - **please** share and
contribure!

### txr
The Text Renderer is responsible for rendering text fonts on graphic displays. It supports monospaced and proportional fonts.

### dc
Drawing context

### tc
Text context

### drv
The base class of the head driver which adds some mandatory driver routines.
All color calculations are done in ARGB 32 bit format. If a display has less colors, like 15 bit RGB, the specific driver
class is responsible for color reduction. Conversion routines provides the `drv` class.

### head
The specific display driver. A display together with its controller is referenced here as `head`. And `head` is the namespace of all specific drivers.
All virtual functions of the `gpr` and `drv` are overridden here.

### sprite


### ctrl
The base class for advanced controls like gauges, progress bars, checkboxes, buttons etc.


## Usage
Using VIC is really easy and fun.
In a single head design (just one display in the system) you create your head first by instancing the according driver.
E.g. on a Windows (emulation/test) platform this would be:
```c++
// Create a screen of 240 * 120 pixel with a viewport of the same size (240 * 120)
// The viewport has an offset of 0,0 and the window is placed at 100,100 on the windows desktop
// For development convinience a horizontal and vertical zoom factor of 4 is selected
víc::head::windows<240,120,240,120> _head(0, 0, 100, 100, 4, 4);
_head.init();  // explicit head init
```

Now you can perform any operations on the head:
```c++
_head.pen_set_color(vic::color::brightblue);  // select bright blue drawing color
_head.line(10, 10, 40, 40);                   // draw a line from 10,10 to 40,40

// write some text
_head.pen_set_color(vic::color::yellow);
_head.text_set_font(vic::font::arial_num_16x24_aa4);
_head.text_string_pos(10, 50, (const std::uint8_t*)"12,34.56-7890");
```

If you need multiple displays in your design, we are talking about a "multiple heads" scenario. All heads are independent.
Let's create two independent heads:
```c++
vic::head::windows<240,120,240,120> _head0(0, 0, 0,  00, 2, 2);
vic::head::windows<240,120,240,120> _head1(0, 0, 0, 200, 2, 2);
_head0.init();  // explicit head 0 init
_head1.init();  // explicit head 1 init
```
Using head 0:
```c++
_head0.pen_set_color(vic::color::brightblue);
_head0.line(10, 10, 40, 40);
```
And using head 1 (eg. for system debug, status. etc):
```c++
head1.pen_set_color(vic::color::red);
head1.box(100, 100, 120, 120);
```

Or you can combine multiple single displays to one big screen. It's a common way to create big screens out of single modules.
This is done by using the `multihead` driver.
```c++
// Create 3 MAX7219 heads with a rotation of 90° on SPI device ID 1,2 and 3
// The single heads have a resolution of 8 * 8 pixels
vic::head::MAX7219<8U, 8U> _head0(drv::orientation_90, 1U);
vic::head::MAX7219<8U, 8U> _head1(drv::orientation_90, 2U);
vic::head::MAX7219<8U, 8U> _head2(drv::orientation_90, 3U);
// Create the multihead out of the three heads with offsets of 0, 8 and 16 pixels
// So the multihead has a srceen resolution of 24 * 8 pixels
vic::head::multihead<24U, 8U, 3U> _multihead = { {_head0, 0U, 0U}, {_head1, 8U, 0U}, {_head2, 16U, 0U} };

// Now, use the multihead like a normal head
_multihead.line(0, 0, 23, 7);
```


## License
vic is written under the MIT licence.
