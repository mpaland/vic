# VGX (Video or Visual GraphiX) library for LCDs

## What is the vgx library?
A lightweight but very powerful graphic and text rendering C++ library for graphic and alpha numeric displays.

## What is vgx NOT?
VGX is not meant to be a full bloated windowmanager, widgets, dialogs, theme rendering framework.
If you need an advanced windowed GUI, there are many other cool libraries around, like µC/GUI, StWin etc.

So, what are the highlights?
- Universal library for graphic and alpha numeric displays
- High performance primitive rendering of lines, circles, boxes, text etc.
- Support of advanced controls like gauges, bars, buttons, checkboxes etc.
- Very easy to use and fast implemention of own/new display drivers
- Platform and CPU independent code, NO dependencies, NO STL, NO new/delete, just clean C++
- VERY clean and stable C++ code, LINT and L4 warning free
- Multi head support, as many displays as you may like in one system
- Support of various color formats from 1 to 32 bpp displays
- Support for different font formats (proportional, monospace), ASCII/UTF-8 support
- Antialiasing support for primitive and text/font rendering
- Framebuffer support
- Anti aliasing support
- Clipping support
- MIT license

To be done (and implemented):
- Improve anti aliasing support for primitives
- Implement fast fill algorithm
- Implement PNG and BMP rendering support
- Add more high class fonts


## Design of VGX:
Basicly every display needs a driver. But unlike as in other designs, the driver is the highest class of which the head
display is instanciated.
A graphic driver only needs to implement pixel set and pixel get operations. But most modern
display controllers provide sophisticated functions like line, box rendering, block moving etc.
If a controller can provide such special functions, the according function is implemented in the driver by 
simply overriding the virtual function of the GPR base class.
All graphic functions which the controller/driver can't provide natively are handled by the GPR.
Of cource, native rendering on a specialized LCD controller is always faster and should be used.


## Class design
![](doc/vgxlib.svg?raw=true)

### GPR
The Graphics Primitive Renderer is the base class of the whole stack.
It provides fast rendering functions for graphic primitives and text fonts.
All color calculations are done in ARGB 32 bit format. If a display has less colors like 15 bit RGB, the head
class is responsible for color reduction.
The GPR doesn't use any floting point math, only fast integer operations.
Furthermore it should be a collection and reference implementation of the most modern, fast and reliable rendering algorithms.
So, if you find any errors, have speed improvements or even a better algorithm for some primitives - PLEASE share and
contribure!

### DRV
Base class of the head to add some mandatory routines.
Further this class is used for pointing to the head, cause drv_head is a template class which can't be used without template parameters.

### HEAD
The base class of every specific driver. drv_head is a templated class which takes the color type and format of the 
display/controller as paramters.

### CTRL
The base class for advanced controls like gauges, bars, checkboxes etc.


## Usage


## License
vgx is under MIT
