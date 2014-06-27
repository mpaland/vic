# VGX (Video or Visual GraphiX) library

## What is the vgx library?
A lightweight but very powerful graphic and text rendering C++ library for graphic and alpha numeric displays.

## What is vgx NOT?
vgx is not meant to be a full bloated windowmanager, widgets, dialogs etc. rendering framework.
If you need an advanced windowed GUI, there are many other cool libraries around, like µC/GUI, StWin etc.

So, what are the highlights?
- Universal library for graphic and alpha numeric displays
- High performance primitive rendering of lines, circles, boxes, text etc.
- Support of advanced controls like gauges, bars, buttons, checkboxes etc.
- Very easy to use and fast implemention of own/new display drivers
- Platform and CPU independent code, NO dependencies, NO STL, NO new/delete, just clean C++
- VERY clean, high quality and stable C++ code, LINT and L4 warning free
- Multi head support, as many displays as you like in one system
- Support of various color formats from 1 to 32 bpp displays
- Support of different font formats (proportional, monospace), ASCII/UTF-8 support
- Anti aliasing support for smooth primitive and text/font rendering
- Framebuffer support (if the controller has native support)
- Clipping region support
- MIT license

To be done (and implemented):
- Improve/implement anti aliasing support for primitives
- Implement fast fill algorithm
- Implement PNG and BMP rendering support
- Add more high quality and standard fonts


## Design of vgx
vgx usage should be as easy as possible and implementing drivers for new controllers as fast as possible.
A graphic driver only needs to implement `pixel_set` and `pixel_get` operations. But most modern
display controllers provide sophisticated functions like line, box rendering, block moving etc.
If a controller can provide such special functions, the according function is implemented in the driver by 
simply overriding the virtual function of the `gpr` base class.
All graphic functions which the controller/driver can't provide natively are handled by the `gpr`.
Of cource, native rendering on a specialized video controller is always faster and should be prefered.


## Class design
![](doc/vgxlib.svg?raw=true)

### gpr
The Graphics Primitive Renderer is the base class of the whole rendering stack.
It provides fast rendering functions for graphic primitives and text fonts.
All color calculations are done in ARGB 32 bit format. If a display has less colors like 15 bpp, the `drv_head`
class is responsible for color reduction.
The `gpr` doesn't use ANY floting point math, only fast integer operations.
Furthermore the `gpr` should be a collection of reference implementations of the most modern, fast and reliable primitive rendering algorithms.
So, if you find any errors, have speed improvements or even know a better algorithm for a primitive - PLEASE share and
contribure!

### drv
Base class of the head to add some mandatory routines.
Further this class is used for pointing to the head, cause `drv_head` is a templated class which can't be used without template parameters.

### drv_head
The base class of every specific driver. `drv_head` is a templated class which takes the color type and format of the 
display/controller as paramters.

### drv_xxxxx
The specific driver for the according controller or platform. All virtual functions of the `gpr` which the the controller 
can natively provide are overridden here.
The low level access interface is defined in the vgx_cfg.h config file.

### ctrl
The base class for advanced controls like gauges, bars, checkboxes, buttons etc.


## Usage
Using vgx is pretty easy.
In a single head design (just one display in the system) you create your head first by instancing the according driver.
On a Windows (emulation/test) platform this would be:
```c++
vgx::drv_windows head(240, 120, 0, 0, 50, 50, 2, 2);
head.init();  // explicit head init
```

Now you can perform any operations on the head:
```c++
head.color_set(VGX_COLOR_BRIGHTBLUE);
head.line(10, 10, 40, 40);

// write some text
head.color_set(VGX_COLOR_YELLOW);
head.text_set_font(vgx::font_Arial_num_16x24_aa4);
head.text_string_pos(10, 50, (const std::uint8_t*)"12,34.56-7890");
```

If you need multiple displays in your design, we are talking about a "multi head" scenario.
Let's create two heads:
```c++
vgx::drv_windows head0(240, 120, 0, 0, 50,  50, 2, 2);
vgx::drv_windows head1(240, 120, 0, 0, 00, 200, 2, 2);
head0.init();  // explicit head 0 init
head1.init();  // explicit head 1 init
```

Using head 0:
```c++
head0.color_set(VGX_COLOR_BRIGHTBLUE);
head0.line(10, 10, 40, 40);
```

And using head 1 (eg. for system debug, status. etc):
```c++
head1.color_set(VGX_COLOR_RED);
head1.box(100, 100, 120, 120);
```


## License
vgx is written under the MIT licence.
