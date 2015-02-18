///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2015, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// This file is part of the vgx library.
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
// \brief Windows head driver
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_DRV_WINDOWS_H_
#define _VGX_DRV_WINDOWS_H_

#include "vgx_drv.h"

#include <Windows.h>
#include <process.h>

// defines the driver name and version
#define VGX_DRV_WINDOWS_VERSION   "Windows driver 2.01"


namespace vgx {
namespace head {

/**
 * Windows head
 * \param Screen_Size_X Screen (buffer) width
 * \param Screen_Size_Y Screen (buffer) height
 * \param Viewport_Size_X Viewport (window) width
 * \param Viewport_Size_Y Viewport (window) height
 */
template<std::uint16_t Screen_Size_X,   std::uint16_t Screen_Size_Y,
         std::uint16_t Viewport_Size_X, std::uint16_t Viewport_Size_Y>
class windows : public drv
{
public:
  /**
   * ctor
   * \param viewport_x Viewport x position (left) on the screen, relative to top/left corner
   * \param viewport_y Viewport y position (top)  on the screen, relative to top/left corner
   * \param window_x Window x position on desktop
   * \param window_y Window y position on desktop
   * \param zoom_x X zoom factor
   * \param zoom_y Y zoom factor
   */
  windows(std::uint16_t viewport_x = 0U, std::uint16_t viewport_y = 0U,
          std::uint16_t window_x = 0U, std::uint16_t window_y = 0U,
          std::uint8_t zoom_x = 1U, std::uint8_t zoom_y = 1U)
    : drv(Screen_Size_X, Screen_Size_Y,
          Viewport_Size_X, Viewport_Size_Y,
          viewport_x, viewport_y)
    , window_x_(window_x)
    , window_y_(window_y)
    , zoom_x_(zoom_x)
    , zoom_y_(zoom_y)
    , wnd_state_(create)
  { }


  /**
   * dtor
   * Deinit the driver
   */
  ~windows()
  { deinit(); }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   D R I V E R   F U N C T I O N S

  virtual void init()
  {
    // create window thread
    thread_handle_ = reinterpret_cast<HANDLE>(::_beginthreadex(
      NULL,
      0U,
      reinterpret_cast<unsigned(__stdcall *)(void*)>(&windows::worker_thread),
      reinterpret_cast<void*>(this),
      0U,
      NULL
    ));
    while (wnd_state_ == create) {
      ::Sleep(10U);
    }
  }


  virtual void deinit()
  {
    (void)::CloseWindow(hwnd_);
    (void)::DeleteObject(hbmp_);
    return;
  }


  virtual inline const char* version() const
  {
    return (const char*)VGX_DRV_WINDOWS_VERSION;
  }


  virtual bool is_graphic() const
  {
    // This Windows driver is a graphic display
    return true;
  }


  virtual void cls()
  {
    ::HGDIOBJ org = ::SelectObject(hmemdc_, ::GetStockObject(DC_PEN));
    ::SelectObject(hmemdc_, ::GetStockObject(DC_BRUSH));
    ::SetDCPenColor(hmemdc_,   RGB(color::get_red(color_bg_), color::get_green(color_bg_), color::get_blue(color_bg_)));
    ::SetDCBrushColor(hmemdc_, RGB(color::get_red(color_bg_), color::get_green(color_bg_), color::get_blue(color_bg_)));
    ::Rectangle(hmemdc_, 0, 0, screen_width() * zoom_x_, screen_height() * zoom_y_);  // needs to be 1 pixel bigger for Windows API
    ::SelectObject(hmemdc_, org);
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S   F O R   G R A P H I C   H E A D S

  /**
   * Set pixel in given color, the color doesn't change the actual drawing color
   * \param x X value
   * \param y Y value
   * \param color Color of pixel in ARGB format
   */
  virtual void drv_pixel_set_color(int16_t x, int16_t y, color::value_type color)
  {
    // check limits and clipping
    if (x >= screen_width() || y >= screen_height() || (clipping_ && !clipping_->is_clipping(x, y))) {
      // out of bounds or outside clipping region
      return;
    }

    for (int c = 0; c < zoom_x_; ++c) {
      for (int r = 0; r < zoom_y_; ++r) {
        (void)::SetPixel(hmemdc_, x * zoom_x_ + c, y * zoom_y_ + r, RGB(color::get_red(color), color::get_green(color), color::get_blue(color)));
      }
    }
  }


  /**
   * Get pixel color
   * \param x X value
   * \param y Y value
   * \return Color of pixel in ARGB format
   */
  virtual inline color::value_type drv_pixel_get(int16_t x, int16_t y) const
  {
    // check limits and clipping
    if (x >= screen_width() || y >= screen_height()) {
      // out of bounds or outside clipping region
      return vgx::color::black;
    }

    COLORREF clr = ::GetPixel(hmemdc_, x * zoom_x_, y * zoom_y_);
    return color::rgb(GetRValue(clr), GetGValue(clr), GetBValue(clr));
  }

  /////////////////////////////////////////////////////////////////////////////

  /**
   * Rendering is done (copy RAM / frame buffer to screen)
   */
  virtual void drv_primitive_done()
  {
    if (primitive_lock_) {
      // do nothing if lock is engaged
      return;
    }

    // copy memory bitmap to viewport
    ::HDC hDC = ::GetDC(hwnd_);
    ::BitBlt(hDC, 0, 0, viewport_width() * zoom_x_,viewport_height() * zoom_y_, hmemdc_, 0, 0, SRCCOPY);
    ::ReleaseDC(hwnd_, hDC);
  }


protected:
  static unsigned int worker_thread(void* arg)
  {
    windows* d = static_cast<windows*>(arg);

    // create the window class - ONLY ONCE
    static WNDCLASSEX wc = { };
    HINSTANCE hInstance = ::GetModuleHandle(NULL);
    LPCTSTR className = L"vgx screen";
    if (!wc.cbSize) {
      wc.cbSize = sizeof(WNDCLASSEX);
      wc.style = CS_HREDRAW | CS_VREDRAW;
      wc.lpfnWndProc = DefWindowProc;
      wc.cbClsExtra = 0;
      wc.cbWndExtra = 0;
      wc.hInstance = hInstance;
      wc.hCursor = LoadCursor(NULL, IDC_ARROW);
      wc.hbrBackground = NULL;  // no background brush, suppress WM_ERASEBKGND
      wc.lpszMenuName = NULL;
      wc.lpszClassName = className;
      wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
      wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
      if (!RegisterClassEx(&wc)) {
        ::MessageBox(NULL, L"Error registering class", L"Error", MB_OK | MB_ICONERROR);
        d->wnd_state_ = windows::error;
        return 0U;
      }
    }

    // adjust windows size to viewport width and height
    RECT rect;
    rect.left   = 0;
    rect.top    = 0;
    rect.right  = d->viewport_width()  * d->zoom_x_;
    rect.bottom = d->viewport_height() * d->zoom_y_;
    ::AdjustWindowRect(&rect, WS_POPUP | WS_CAPTION, FALSE);

    // finally create and display the window
    d->hwnd_ = CreateWindowEx(
      0,
      className,
      L"vgx - Screen",
      WS_POPUP | WS_CAPTION,
      CW_USEDEFAULT, CW_USEDEFAULT,
      abs(rect.left) + abs(rect.right),
      abs(rect.top)  + abs(rect.bottom),
      NULL,
      NULL,
      hInstance,
      NULL
    );
    if (!d->hwnd_) {
      ::MessageBox(NULL, L"Error creating window", L"Error", MB_OK | MB_ICONERROR);
      d->wnd_state_ = windows::error;
      return 0U;
    }
    ::SetWindowPos(d->hwnd_, HWND_TOP, d->window_x_, d->window_y_, 0, 0, SWP_NOSIZE |SWP_NOZORDER | SWP_SHOWWINDOW);

    // create a bitmap in zoomed screen size for all drawing
    HDC hDC = ::GetDC(d->hwnd_);
    d->hmemdc_ = ::CreateCompatibleDC(hDC);
    d->hbmp_   = ::CreateCompatibleBitmap(hDC, d->screen_width() * d->zoom_x_, d->screen_height() * d->zoom_y_);
    ::SelectObject(d->hmemdc_, d->hbmp_);
    ::ReleaseDC(d->hwnd_, hDC);
    d->wnd_state_ = windows::ready;

    // process upon WM_QUIT or error
    ::MSG Msg;
    while (::GetMessage(&Msg, NULL, 0, 0) > 0)
    {
      ::TranslateMessage(&Msg);
      ::DispatchMessage(&Msg); 
    }
    return 0U;
  }

public:
  // must be public for thread accessibility
  typedef enum enum_window_state_type {
    create = 0,
    ready,
    error,
    end
  } window_state_type;

  ::HANDLE thread_handle_;                // worker thread handle
  volatile window_state_type wnd_state_;  // window state

  std::int16_t  window_x_;                // x coordinate of output window
  std::int16_t  window_y_;                // y coordinate of output window
  std::uint8_t  zoom_x_;                  // x zoom factor of output window
  std::uint8_t  zoom_y_;                  // y zoom factor of output window
  ::HWND        hwnd_;
  ::HDC         hmemdc_;
  ::HBITMAP     hbmp_;
};

} // namespace head
} // namespace vgx

#endif  // _VGX_DRV_WINDOWS_H_
