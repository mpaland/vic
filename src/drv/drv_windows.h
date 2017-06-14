///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2017, PALANDesign Hannover, Germany
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
// \brief This is a Windows graphics head driver, setup the desired screen and
// viewport size as template parameter
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_DRV_WINDOWS_H_
#define _VGX_DRV_WINDOWS_H_

#include "../drv.h"

#include <Windows.h>
#include <process.h>


// defines the driver name and version
#define VGX_DRV_WINDOWS_VERSION   "Windows driver 3.00"

namespace vgx {
namespace head {


/**
 * Windows head
 * \param Screen_Size_X Screen (buffer) width
 * \param Screen_Size_Y Screen (buffer) height
 * \param Viewport_Size_X Viewport (window) width
 * \param Viewport_Size_Y Viewport (window) height
 */
template<std::uint16_t Screen_Size_X, std::uint16_t Screen_Size_Y,
         std::uint16_t Viewport_Size_X = Screen_Size_X, std::uint16_t Viewport_Size_Y = Screen_Size_Y>
class windows final : public drv
{
public:
  /**
   * ctor
   * \param viewport_x Viewport x position (left) on the screen, relative to top/left screen corner
   * \param viewport_y Viewport y position (top)  on the screen, relative to top/left screen corner
   * \param window_x Window x position on Windows desktop
   * \param window_y Window y position on Windows desktop
   * \param zoom_x X zoom factor
   * \param zoom_y Y zoom factor
   */
  windows(std::uint16_t viewport_x = 0U, std::uint16_t viewport_y = 0U,
          std::uint16_t window_x = 0U, std::uint16_t window_y = 0U,
          std::uint8_t zoom_x = 1U, std::uint8_t zoom_y = 1U,
          const LPCTSTR caption = L"vgx screen")
    : drv(Screen_Size_X, Screen_Size_Y,
          Viewport_Size_X, Viewport_Size_Y,
          viewport_x, viewport_y,
          orientation_0)
    , window_x_(window_x)
    , window_y_(window_y)
    , zoom_x_(zoom_x)
    , zoom_y_(zoom_y)
    , caption_(caption)
  { }


  /**
   * dtor
   * Shutdown the driver
   */
  ~windows()
  { drv_shutdown(); }


protected:

  virtual void drv_init()
  {
    // create init event
    wnd_init_ev_ = ::CreateEvent(NULL, TRUE, FALSE, NULL);

    // create window thread
    thread_handle_ = reinterpret_cast<HANDLE>(::_beginthreadex(
      NULL,
      0U,
      reinterpret_cast<unsigned(__stdcall *)(void*)>(&windows::worker_thread),
      reinterpret_cast<void*>(this),
      0U,
      NULL
    ));

    // wait until screen is created
    ::WaitForSingleObject(wnd_init_ev_, INFINITE);
  }


  virtual void drv_shutdown()
  {
    (void)::CloseWindow(hwnd_);
    (void)::DeleteObject(hbmp_);
    return;
  }


  virtual inline const char* drv_version() const
  {
    return (const char*)VGX_DRV_WINDOWS_VERSION;
  }


  virtual inline bool drv_is_graphic() const
  {
    // This Windows driver is a graphic display
    return true;
  }


  virtual void drv_cls()
  {
    HGDIOBJ org = ::SelectObject(hmemdc_, ::GetStockObject(DC_PEN));
    ::SelectObject(hmemdc_, ::GetStockObject(DC_BRUSH));
    ::SetDCPenColor(hmemdc_,   RGB(color::get_red(color_bg_get()), color::get_green(color_bg_get()), color::get_blue(color_bg_get())));
    ::SetDCBrushColor(hmemdc_, RGB(color::get_red(color_bg_get()), color::get_green(color_bg_get()), color::get_blue(color_bg_get())));
    ::Rectangle(hmemdc_, 0, 0, screen_width() * zoom_x_, screen_height() * zoom_y_);  // needs to be 1 pixel bigger for Windows API
    ::SelectObject(hmemdc_, org);
  }

 
  /**
   * Set pixel in given color, the color doesn't change the actual drawing color
   * \param x X value
   * \param y Y value
   * \param color Color of pixel in ARGB format
   */
  virtual inline void drv_pixel_set_color(vertex_type point, color::value_type color)
  {
    // check limits and clipping
    if (!screen_is_inside(point) || !clipping_.is_inside(point)) {
      // out of bounds or outside clipping region
      return;
    }

    color = !color::get_alpha(color) ? color : color::mix_alpha(color, drv_pixel_get(point));
    for (int c = 0; c < zoom_x_; ++c) {
      for (int r = 0; r < zoom_y_; ++r) {
        (void)::SetPixel(hmemdc_, point.x * zoom_x_ + c, point.y * zoom_y_ + r, RGB(color::get_red(color), color::get_green(color), color::get_blue(color)));
      }
    }
  }


  /**
   * Get pixel color
   * \param x X value
   * \param y Y value
   * \return Color of pixel in ARGB format
   */
  virtual inline color::value_type drv_pixel_get(vertex_type point)
  {
    // check limits and clipping
    if (!screen_is_inside(point)) {
      // out of bounds or outside clipping region
      return vgx::color::black;
    }

    COLORREF clr = ::GetPixel(hmemdc_, point.x * zoom_x_, point.y * zoom_y_);
    return color::rgb(GetRValue(clr), GetGValue(clr), GetBValue(clr));
  }


  /**
   * Rendering is done (copy RAM / frame buffer to screen)
   */
  virtual void drv_present()
  {
    // copy memory bitmap to viewport
    HDC hDC = ::GetDC(hwnd_);
    ::BitBlt(hDC, viewport_get().x * zoom_x_, viewport_get().y * zoom_y_, viewport_width() * zoom_x_,viewport_height() * zoom_y_, hmemdc_, 0, 0, SRCCOPY);
    ::ReleaseDC(hwnd_, hDC);
  }


protected:
  static unsigned worker_thread(void* arg)
  {
    windows* d = static_cast<windows*>(arg);

    const HINSTANCE hInstance = ::GetModuleHandle(NULL);
    const LPCTSTR className = L"vgx_screen";
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = ::DefWindowProc; 
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;  // no background brush, suppress WM_ERASEBKGND
    wc.lpszMenuName = NULL;
    wc.lpszClassName = className;
    wc.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = ::LoadIcon(NULL, IDI_APPLICATION);
    (void)::RegisterClassEx(&wc);

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
      d->caption_,
      WS_POPUP | WS_CAPTION,
      CW_USEDEFAULT, CW_USEDEFAULT,
      ::abs(rect.left) + ::abs(rect.right),
      ::abs(rect.top)  + ::abs(rect.bottom),
      NULL,
      NULL,
      hInstance,
      NULL
    );
    if (!d->hwnd_) {
      ::MessageBox(NULL, L"Error creating window", L"Error", MB_OK | MB_ICONERROR);
      ::SetEvent(d->wnd_init_ev_);
      return 0U;
    }
    ::SetWindowPos(d->hwnd_, HWND_TOP, d->window_x_, d->window_y_, 0, 0, SWP_NOSIZE |SWP_NOZORDER | SWP_SHOWWINDOW);

    // create a bitmap in zoomed screen size for all drawing
    HDC hDC = ::GetDC(d->hwnd_);
    d->hmemdc_ = ::CreateCompatibleDC(hDC);
    d->hbmp_   = ::CreateCompatibleBitmap(hDC, d->screen_width() * d->zoom_x_, d->screen_height() * d->zoom_y_);
    ::SelectObject(d->hmemdc_, d->hbmp_);
    ::ReleaseDC(d->hwnd_, hDC);
    ::SetEvent(d->wnd_init_ev_);

    // process upon WM_QUIT or error
    MSG Msg;
    while (::GetMessage(&Msg, NULL, 0, 0) > 0)
    {
      ::TranslateMessage(&Msg);
      ::DispatchMessage(&Msg); 
    }
    return 0U;
  }


public:
  // must be public for thread accessibility
  HANDLE        thread_handle_;           // worker thread handle
  HANDLE        wnd_init_ev_;             // window init event
  const LPCTSTR caption_;                 // window caption

  std::int16_t  window_x_;                // x coordinate of output window
  std::int16_t  window_y_;                // y coordinate of output window
  std::uint8_t  zoom_x_;                  // x zoom factor of output window
  std::uint8_t  zoom_y_;                  // y zoom factor of output window
  HWND          hwnd_;
  HDC           hmemdc_;
  HBITMAP       hbmp_;
};



///////////////////////////////////////////////////////////////////////////////

/**
 * Windows text head
 * \param Columns Screen X size in chars
 * \param Rows Screen Y size in chars
 */
template<std::uint16_t COLUMNS, std::uint16_t ROWS,
         std::uint16_t Viewport_Size_X = COLUMNS, std::uint16_t Viewport_Size_Y = ROWS>
class windows_text final : public drv
{
public:
  /**
   * ctor
   * \param viewport_x X offset of the viewport, relative to top/left corner
   * \param viewport_y Y offset of the viewport, relative to top/left corner
   */
  windows_text(std::uint16_t viewport_x = 0U, std::uint16_t viewport_y = 0U,
               std::uint16_t window_x   = 0U, std::uint16_t window_y   = 0U,
               const LPCTSTR caption = L"vgx text screen")
    : drv(COLUMNS, ROWS,
          Viewport_Size_X, Viewport_Size_Y,
          viewport_x, viewport_y,
          orientation_0)
    , window_x_(window_x)
    , window_y_(window_y)
    , caption_(caption)
  {
    font_height     = 50U;    // font height in pixel
    font_width      = font_height * 40U / 75U;
    char_x_margin   = 2U;
    char_y_margin   = 2U;
    char_x_padding  = 2U;
    char_y_padding  = 1U;  
  }


  /**
   * dtor
   * Shutdown the driver
   */
  ~windows_text()
  { drv_shutdown(); }


protected:

  virtual void drv_init()
  {
    // create init event
    wnd_init_ev_ = ::CreateEvent(NULL, TRUE, FALSE, NULL);

    // create window thread
    thread_handle_ = reinterpret_cast<HANDLE>(::_beginthreadex(
      NULL,
      0U,
      reinterpret_cast<unsigned(__stdcall *)(void*)>(&windows_text::worker_thread),
      reinterpret_cast<void*>(this),
      0U,
      NULL
    ));

    // wait until screen is created
    ::WaitForSingleObject(wnd_init_ev_, INFINITE);

    // clear screen
    drv_cls();
  }


  virtual void drv_shutdown()
  {
    (void)::CloseWindow(hwnd_);
    (void)::DeleteObject(hbmp_);
  }


  virtual inline const char* drv_version() const
  {
    return (const char*)VGX_DRV_WINDOWS_VERSION;
  }


  virtual inline bool drv_is_graphic() const
  {
    // This Windows text driver is an alpha numeric (text only) display
    return false;
  }


  virtual void drv_cls()
  {
    for (std::int16_t y = 0; y < screen_height(); y++) {
      for (std::int16_t x = 0; x < screen_width(); x++) {
        frame_buffer_[x][y] = ' ';
      }
    }
    drv_present();
  }

 
  /**
   * Set pixel in given color, the color doesn't change the actual drawing color
   * Unused
   */
  virtual inline void drv_pixel_set_color(vertex_type, color::value_type)
  { }


  /**
   * Get pixel color - unsued
   * \return Dumy color (black)
   */
  virtual inline color::value_type drv_pixel_get(vertex_type)
  { return color::black; }


  /**
   * Text mode is ignored, BA6x has no inverse video mode
   */
  virtual void drv_text_mode(text_mode_type)
  { }


  /**
   * Rendering is done (copy RAM / frame buffer to screen)
   */
  virtual void drv_present()
  {
    // render text

    HGDIOBJ org = ::SelectObject(hmemdc_, ::GetStockObject(DC_PEN));
    ::SelectObject(hmemdc_, ::GetStockObject(DC_BRUSH));
    ::SetDCPenColor(hmemdc_,   RGB(color::get_red(color_bg_get()), color::get_green(color_bg_get()), color::get_blue(color_bg_get())));
    ::SetDCBrushColor(hmemdc_, RGB(color::get_red(color_bg_get()), color::get_green(color_bg_get()), color::get_blue(color_bg_get())));
    ::Rectangle(hmemdc_, 0, 0, window_size_x_, window_size_y_);
    ::SelectObject(hmemdc_, org);

    ::SetTextColor(hmemdc_, RGB(0, 210, 195));
    ::SetBkColor(hmemdc_, RGB(0, 0, 0));

    HPEN pen_box = ::CreatePen(PS_DOT, 0U, RGB(0x00, 0x30, 0x30));
    ::SelectObject(hmemdc_, pen_box);

    vertex_type _viewport = viewport_get();
    for (std::int16_t col = _viewport.x; col < viewport_width() && col < screen_width(); col++) {
      for (std::int16_t row = _viewport.y; row < viewport_height() && row < screen_height(); row++) {

        std::int16_t x = 2 * char_x_margin + (col * (font_width  + 2 * char_x_padding + 2 * char_x_margin));
        std::int16_t y =     char_y_margin + (row * (font_height + 2 * char_y_padding + 2 * char_y_margin));

        RECT rect = { x + char_x_margin + char_x_padding,
                      y + char_y_margin + char_y_padding,
                      x + char_x_margin + char_x_padding + font_width,
                      y + char_y_margin + char_y_padding + font_height
                    };
        ::DrawText(hmemdc_, (LPCWSTR)&frame_buffer_[col][row], 1U, &rect, DT_CENTER | DT_TOP | DT_SINGLELINE);

        // draw a square around the char
        POINT box[] = { {x + char_x_margin, y + char_y_margin},
                        {x + char_x_margin + char_x_padding + font_width + char_x_padding, y + char_y_margin},
                        {x + char_x_margin + char_x_padding + font_width + char_x_padding, y + char_y_margin + char_y_padding + font_height + char_y_padding},
                        {x + char_x_margin, y + char_y_margin + char_y_padding + font_height + char_y_padding},
                        {x + char_x_margin, y + char_y_margin } };
        ::Polyline(hmemdc_, box, 5);
      }
    }

    HDC hDC = ::GetDC(hwnd_);
    ::BitBlt(hDC, 0, 0, window_size_x_, window_size_y_, hmemdc_, 0, 0, SRCCOPY);
    ::ReleaseDC(hwnd_, hDC);
  }


  /**
   * Output a single ASCII/UNICODE char at the actual cursor position
   * \param ch Output character in 16 bit ASCII/UNICODE (NOT UTF-8) format, 00-7F is compatible with ASCII
   */
  virtual void text_char(std::uint16_t ch)
  {
    if (ch < 0x20U) {
      // ignore non characters
      return;
    }

    // check limits
    if (screen_is_inside({ text_x_act_, text_y_act_ })) {
      frame_buffer_[text_x_act_][text_y_act_] = ch;
    }

    // always increment the text position
    text_x_act_++;
  }


protected:

  static unsigned worker_thread(void* arg)
  {
    windows_text* d = static_cast<windows_text*>(arg);

    const HINSTANCE hInstance = ::GetModuleHandle(NULL);
    const LPCTSTR className = L"vgx_text_screen";
    WNDCLASSEX wc = { };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = ::DefWindowProc; 
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;  // no background brush, suppress WM_ERASEBKGND
    wc.lpszMenuName = NULL;
    wc.lpszClassName = className;
    wc.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = ::LoadIcon(NULL, IDI_APPLICATION);
    (void)::RegisterClassEx(&wc);

    // finally create and display the window
    d->hwnd_ = CreateWindowEx(
      0,
      className,
      d->caption_,
      WS_POPUP | WS_CAPTION,
      CW_USEDEFAULT, CW_USEDEFAULT,
      1,
      1,
      NULL,
      NULL,
      hInstance,
      NULL
    );
    if (!d->hwnd_) {
      ::MessageBox(NULL, L"Error creating window", L"Error", MB_OK | MB_ICONERROR);
      ::SetEvent(d->wnd_init_ev_);
      return 0U;
    }

    d->window_size_x_ = (d->font_width  + 2 * d->char_x_margin + 2 * d->char_x_padding) * d->viewport_width()  + 4 * d->char_x_margin;
    d->window_size_y_ = (d->font_height + 2 * d->char_y_margin + 2 * d->char_y_padding) * d->viewport_height() + 2 * d->char_y_margin;

    // resize window
    // adjust windows size to viewport width and height
    RECT rect;
    rect.left   = 0;
    rect.top    = 0;
    rect.right  = d->window_size_x_;
    rect.bottom = d->window_size_y_;
    ::AdjustWindowRect(&rect, WS_POPUP | WS_CAPTION, FALSE);
    ::SetWindowPos(d->hwnd_, HWND_TOP, d->window_x_, d->window_y_, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);

    // create a bitmap in zoomed screen size for all drawing
    HDC hDC = ::GetDC(d->hwnd_);
    d->hmemdc_ = ::CreateCompatibleDC(hDC);
    d->hbmp_   = ::CreateCompatibleBitmap(hDC, d->window_size_x_, d->window_size_y_);
    ::SelectObject(d->hmemdc_, d->hbmp_);
    ::ReleaseDC(d->hwnd_, hDC);

    wchar_t szFontFile[] = L"enhanced_led_board-7.ttf";
    ::AddFontResourceEx(szFontFile, FR_PRIVATE, NULL);

    LOGFONT lf;
    memset(&lf, 0, sizeof(lf));
    lf.lfHeight = -MulDiv(d->font_height * 60U / 75U * 72U / GetDeviceCaps(d->hmemdc_, LOGPIXELSY), GetDeviceCaps(d->hmemdc_, LOGPIXELSY), 72);
    lf.lfWeight = FW_NORMAL;
    lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
    wcscpy_s(lf.lfFaceName, L"Enhanced LED Board-7");

    // create and select it
    HFONT font = ::CreateFontIndirect(&lf);
    ::SelectObject(d->hmemdc_, font);
  
    // copy memory bitmap to viewport
    hDC = ::GetDC(d->hwnd_);
    ::BitBlt(hDC, 0, 0, d->window_size_x_, d->window_size_y_, d->hmemdc_, 0, 0, SRCCOPY);
    ::ReleaseDC(d->hwnd_, hDC);

    ::SetEvent(d->wnd_init_ev_);

    // process upon WM_QUIT or error
    MSG Msg;
    while (::GetMessage(&Msg, NULL, 0, 0) > 0)
    {
      ::TranslateMessage(&Msg);
      ::DispatchMessage(&Msg); 
    }
    return 0U;
  }


public:
  // must be public for thread accessibility
  HANDLE        thread_handle_;           // worker thread handle
  HANDLE        wnd_init_ev_;             // window init event
  const LPCTSTR caption_;                 // window caption

  std::uint16_t frame_buffer_[COLUMNS][ROWS];

  std::uint16_t font_height   ;           // font height in pixel
  std::uint16_t font_width    ;
  std::uint16_t char_x_margin ;
  std::uint16_t char_y_margin ;
  std::uint16_t char_x_padding;
  std::uint16_t char_y_padding;

  std::int16_t  window_x_;                // x coordinate of output window
  std::int16_t  window_y_;                // y coordinate of output window
  std::int16_t  window_size_x_;           // window x size
  std::int16_t  window_size_y_;           // window y size
  HWND          hwnd_;
  HDC           hmemdc_;
  HBITMAP       hbmp_;
};

} // namespace head
} // namespace vgx

#endif  // _VGX_DRV_WINDOWS_H_
