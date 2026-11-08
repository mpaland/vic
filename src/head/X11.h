///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2025-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// This file is part of the vic library.
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// \brief X11 head, emulates a display in an X11 window on Linux (and any other
//        X11 system, Wayland desktops run it with XWayland).
//        The only dependency is Xlib, link with -lX11 (CMake: X11::X11).
//        Setup the desired screen and viewport size as template parameter.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_X11_H_
#define _VIC_HEAD_X11_H_

// vic first: the Xlib macros (None, Bool, True, Complex ...) must not reach the vic headers
#include "../drv.h"

#include <cstdio>
#include <cstdlib>
#include <X11/Xlib.h>
#include <X11/Xutil.h>


// defines the head name and version
#define VIC_HEAD_X11_VERSION   "X11 head 1.00"


namespace vic {
namespace head {


/**
 * X11 head
 * \param Screen_Size_X Screen (buffer) width
 * \param Screen_Size_Y Screen (buffer) height
 * \param Viewport_Size_X Viewport (window) width
 * \param Viewport_Size_Y Viewport (window) height
 */
template<std::uint16_t Screen_Size_X, std::uint16_t Screen_Size_Y,
         std::uint16_t Viewport_Size_X = Screen_Size_X, std::uint16_t Viewport_Size_Y = Screen_Size_Y>
class x11 final : public drv
{
public:

  /**
   * ctor
   * \param viewport_x Viewport x position (left) on the screen, relative to top/left screen corner
   * \param viewport_y Viewport y position (top)  on the screen, relative to top/left screen corner
   * \param window_x Window x position on the desktop
   * \param window_y Window y position on the desktop
   * \param zoom_x X zoom factor
   * \param zoom_y Y zoom factor
   * \param caption Window caption, must be valid until init() is called
   */
  x11(std::uint16_t viewport_x = 0U, std::uint16_t viewport_y = 0U,
      std::int16_t  window_x   = 0,  std::int16_t  window_y   = 0,
      std::uint8_t  zoom_x     = 1U, std::uint8_t  zoom_y     = 1U,
      const char*   caption    = "vic screen")
    : drv(Screen_Size_X, Screen_Size_Y,
          Viewport_Size_X, Viewport_Size_Y,
          static_cast<std::int16_t>(viewport_x), static_cast<std::int16_t>(viewport_y),
          orientation_0)
    , caption_(caption)
    , window_x_(window_x)
    , window_y_(window_y)
    , zoom_x_(zoom_x)
    , zoom_y_(zoom_y)
    , stride_(static_cast<std::size_t>(Screen_Size_X) * zoom_x)
    , display_(nullptr)
    , window_(0)
    , image_(nullptr)
    , wm_delete_(0)
  {
    // create the (zoomed) frame buffer
    frame_buffer_ = new std::uint32_t[stride_ * Screen_Size_Y * zoom_y];
  }


  /**
   * dtor
   * Shutdown the driver
   */
  ~x11()
  {
    shutdown();
    delete [] frame_buffer_;
  }


  // the head owns the window and the frame buffer
  x11(const x11&) = delete;
  x11& operator=(const x11&) = delete;


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   D R I V E R   F U N C T I O N S
  //

  virtual void init()
  {
    display_ = ::XOpenDisplay(nullptr);
    if (!display_) {
      std::fprintf(stderr, "vic X11 head: can't open the display, is DISPLAY set?\n");
      return;
    }
    const int screen = DefaultScreen(display_);
    const unsigned int width  = static_cast<unsigned int>(viewport_width()  * zoom_x_);
    const unsigned int height = static_cast<unsigned int>(viewport_height() * zoom_y_);
    window_ = ::XCreateSimpleWindow(display_, RootWindow(display_, screen), window_x_, window_y_, width, height, 0U,
                                    BlackPixel(display_, screen), BlackPixel(display_, screen));

    // fixed window size at the given position
    XSizeHints hints = {};
    hints.flags      = USPosition | PMinSize | PMaxSize;
    hints.x          = window_x_;
    hints.y          = window_y_;
    hints.min_width  = hints.max_width  = static_cast<int>(width);
    hints.min_height = hints.max_height = static_cast<int>(height);
    ::XSetWMNormalHints(display_, window_, &hints);
    ::XStoreName(display_, window_, caption_);

    // closing the window sends a message instead of killing the connection
    wm_delete_ = ::XInternAtom(display_, "WM_DELETE_WINDOW", False);
    ::XSetWMProtocols(display_, window_, &wm_delete_, 1);

    // show the window and wait until it is visible
    ::XSelectInput(display_, window_, ExposureMask);
    ::XMapWindow(display_, window_);
    XEvent ev;
    ::XWindowEvent(display_, window_, ExposureMask, &ev);

    // the frame buffer is the image: 32 bit 0RGB pixels in host byte order, Xlib converts them for the server
    image_ = ::XCreateImage(display_, DefaultVisual(display_, screen), static_cast<unsigned int>(DefaultDepth(display_, screen)),
                            ZPixmap, 0, reinterpret_cast<char*>(frame_buffer_),
                            static_cast<unsigned int>(stride_), static_cast<unsigned int>(Screen_Size_Y * zoom_y_), 32, 0);
    if (!image_ || (image_->bits_per_pixel != 32)) {
      std::fprintf(stderr, "vic X11 head: a 24 bit true color display is needed\n");
      shutdown();
      return;
    }
    const std::uint32_t one = 1U;
    image_->byte_order = (*reinterpret_cast<const std::uint8_t*>(&one) == 1U) ? LSBFirst : MSBFirst;

    cls();
    present();
  }


  virtual void shutdown()
  {
    if (!display_) {
      return;
    }
    if (image_) {
      image_->data = nullptr;   // the frame buffer belongs to the head
      XDestroyImage(image_);
      image_ = nullptr;
    }
    ::XDestroyWindow(display_, window_);
    ::XCloseDisplay(display_);
    display_ = nullptr;
  }


  inline virtual const char* version() const
  {
    return (const char*)VIC_HEAD_X11_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    // This X11 driver is a graphic display
    return true;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

protected:

  /**
   * Clear screen, set all pixels off, delete all characters or fill screen with background/blank color
   * \param bg_color Backgound/erase color, defines normally the default color of the display
   */
  virtual void cls(color::value_type bg_color = color::none)
  {
    const std::uint32_t c = color::color_to_RGB888(bg_color);
    for (std::size_t i = 0U, n = stride_ * Screen_Size_Y * zoom_y_; i < n; ++i) {
      frame_buffer_[i] = c;
    }
  }


  /**
   * Rendering is done (copy the frame buffer to the window)
   */
  virtual void present()
  {
    if (!display_) {
      return;
    }

    // service the window events, an exposed window is restored by the image below
    while (::XPending(display_) > 0) {
      XEvent ev;
      ::XNextEvent(display_, &ev);
      if ((ev.type == ClientMessage) && (static_cast<Atom>(ev.xclient.data.l[0]) == wm_delete_)) {
        // window closed by the user
        std::exit(0);
      }
    }

    // copy the viewport to the window
    const vertex_type vp = viewport_get();
    ::XPutImage(display_, window_, DefaultGC(display_, DefaultScreen(display_)), image_,
                vp.x * zoom_x_, vp.y * zoom_y_, 0, 0,
                static_cast<unsigned int>(viewport_width() * zoom_x_), static_cast<unsigned int>(viewport_height() * zoom_y_));
    ::XFlush(display_);
  }


  ///////////////////////////////////////////////////////////////////////////////
  // G R A P H I C   F U N C T I O N S
  //

  /**
   * Set pixel in given color, the color doesn't change the actual drawing color
   * \param vertex Pixel coordinates
   * \param color Color of pixel in ARGB format
   */
  virtual inline void pixel_set(vertex_type vertex, color::value_type color)
  {
    // check limits
    if (!screen_is_inside(vertex)) {
      // out of bounds
      return;
    }

    const std::uint32_t c = color::color_to_RGB888(color);
    std::uint32_t* row = &frame_buffer_[static_cast<std::size_t>(vertex.y) * zoom_y_ * stride_ + static_cast<std::size_t>(vertex.x) * zoom_x_];
    for (std::uint_fast8_t y = 0U; y < zoom_y_; ++y, row += stride_) {
      for (std::uint_fast8_t x = 0U; x < zoom_x_; ++x) {
        row[x] = c;
      }
    }
  }


  /**
   * Get pixel color
   * \param vertex Vertex of the pixel
   * \return Color of pixel in ARGB format
   */
  virtual inline color::value_type pixel_get(vertex_type vertex)
  {
    // check limits
    if (!screen_is_inside(vertex)) {
      // out of bounds
      return color::none;
    }

    // return the color
    return color::RGB888_to_color(frame_buffer_[static_cast<std::size_t>(vertex.y) * zoom_y_ * stride_ + static_cast<std::size_t>(vertex.x) * zoom_x_]);
  }


private:
  const char*         caption_;         // window caption
  const std::int16_t  window_x_;        // x coordinate of the window on the desktop
  const std::int16_t  window_y_;        // y coordinate of the window on the desktop
  const std::uint8_t  zoom_x_;          // x zoom factor of the window
  const std::uint8_t  zoom_y_;          // y zoom factor of the window
  const std::size_t   stride_;          // zoomed frame buffer width in pixel
  std::uint32_t*      frame_buffer_;    // zoomed frame buffer in 0RGB format, the image data
  Display*            display_;         // X server connection
  Window              window_;          // output window
  XImage*             image_;           // image of the frame buffer
  Atom                wm_delete_;       // WM_DELETE_WINDOW protocol atom
};

} // namespace head
} // namespace vic

#endif  // _VIC_HEAD_X11_H_
