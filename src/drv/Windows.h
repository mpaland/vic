///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2018, PALANDesign Hannover, Germany
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
// \brief This is a Windows graphics head driver, setup the desired screen and
// viewport size as template parameter
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_DRV_WINDOWS_H_
#define _VIC_DRV_WINDOWS_H_

#include <Windows.h>
#include <thread>
#include <string>

#include "../drv.h"
#include "../dc.h"
#include "../tc.h"


// defines the driver name and version
#define VIC_DRV_WINDOWS_VERSION   "Windows driver 4.11"


namespace vic {
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
          std::uint16_t window_x   = 0U, std::uint16_t window_y   = 0U,
          std::uint8_t zoom_x      = 1U, std::uint8_t zoom_y      = 1U,
          std::string caption = "VIC screen")
    : drv(Screen_Size_X, Screen_Size_Y,
          Viewport_Size_X, Viewport_Size_Y,
          viewport_x, viewport_y,
          orientation_0)
    , window_x_(window_x)
    , window_y_(window_y)
    , zoom_x_(zoom_x)
    , zoom_y_(zoom_y)
    , caption_(caption)
  {
    // create the frame buffer
    frame_buffer_ = static_cast<std::uint32_t*>(new std::uint32_t[Screen_Size_X * zoom_x * Screen_Size_Y * zoom_y]);
  }


  /**
   * dtor
   * Shutdown the driver
   */
  ~windows()
  {
    // driver shutdown
    shutdown();

    // delete frame buffer
    delete [] frame_buffer_;
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   D R I V E R   F U N C T I O N S
  //

  virtual void init()
  {
    // create init event to wait until the windows is created
    wnd_init_ev_ = ::CreateEvent(NULL, TRUE, FALSE, NULL);

    // create window thread
    thread_handle_ = new std::thread(worker_thread, (void*)this);

    // wait until the screen is created
    ::WaitForSingleObject(wnd_init_ev_, INFINITE);

    // close handle
    ::CloseHandle(wnd_init_ev_);

    cls();
  }


  virtual void shutdown()
  {
    (void)::CloseWindow(hwnd_);
  }


  inline virtual const char* version() const
  {
    return (const char*)VIC_DRV_WINDOWS_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    // This Windows driver is a graphic display
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
  virtual void cls( color::value_type bg_color = color::none)
  {
    for (std::int_fast16_t y = 0; y < Screen_Size_Y * zoom_y_; y++) {
      for (std::int_fast16_t x = 0; x < Screen_Size_X * zoom_x_; x++) {
        frame_buffer_[y * Screen_Size_X * zoom_x_ + x] = color::color_to_RGB888(bg_color);
      }
    }
  }


  /**
   * Rendering is done (copy RAM / framebuffer to screen)
   */
  virtual void present()
  {
    // copy memory bitmap to viewport
    HDC hDC = ::GetDC(hwnd_);
    HBITMAP hbmp = ::CreateBitmap(Screen_Size_X * zoom_x_, Screen_Size_Y * zoom_y_, 1U, 32U, (const void*)frame_buffer_);
    ::SelectObject(hmemdc_, hbmp);
    ::BitBlt(hDC, viewport_get().x * zoom_x_, viewport_get().y * zoom_y_, viewport_width() * zoom_x_,viewport_height() * zoom_y_, hmemdc_, 0, 0, SRCCOPY);
    ::ReleaseDC(hwnd_, hDC);
    ::DeleteObject(hbmp);
  }


  ///////////////////////////////////////////////////////////////////////////////
  // G R A P H I C   F U N C T I O N S
  //

  /**
   * Set pixel in given color, the color doesn't change the actual drawing color
   * \param x X value
   * \param y Y value
   * \param color Color of pixel in 0RGB format
   */
  virtual inline void pixel_set(vertex_type vertex, color::value_type color)
  {
    // check limits
    if (!screen_is_inside(vertex)) {
      // out of bounds
      return;
    }

    for (std::int_fast16_t y = vertex.y * zoom_y_; y < (vertex.y + 1) * zoom_y_; ++y) {
      for (std::int_fast16_t x = vertex.x * zoom_x_; x < (vertex.x + 1) * zoom_x_; ++x) {
        frame_buffer_[y * Screen_Size_X * zoom_x_ + x] = color::color_to_RGB888(color);
      }
    }
  }


  /**
   * Get pixel color
   * \param vertex Color of vertex to get
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
    return color::RGB888_to_color(frame_buffer_[vertex.y * zoom_y_ * Screen_Size_X * zoom_x_ + vertex.x * zoom_x_]);
  }


private:

   // worker thread
  static void worker_thread(void* arg)
  {
    windows* d = static_cast<windows*>(arg);

    const HINSTANCE hInstance = ::GetModuleHandle(NULL);
    const char* className = "vic_graphic_screen";
    WNDCLASSEXA wc = { };
    wc.cbSize = sizeof(WNDCLASSEXA);
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
   ::RegisterClassExA(&wc);

    // adjust windows size to viewport width and height
    RECT rect;
    rect.left   = 0;
    rect.top    = 0;
    rect.right  = d->viewport_width()  * d->zoom_x_;
    rect.bottom = d->viewport_height() * d->zoom_y_;
    ::AdjustWindowRect(&rect, WS_POPUP | WS_CAPTION, FALSE);

    // finally create and display the window
    d->hwnd_ = ::CreateWindowExA(
      0,
      className,
      d->caption_.c_str(),
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
      ::MessageBoxA(NULL, "Error creating window", "Error", MB_OK | MB_ICONERROR);
      ::SetEvent(d->wnd_init_ev_);
      return;
    }
    ::SetWindowPos(d->hwnd_, HWND_TOP, d->window_x_, d->window_y_, 0, 0, SWP_NOSIZE |SWP_NOZORDER | SWP_SHOWWINDOW);

    // create a compatible dc
    HDC hDC = ::GetDC(d->hwnd_);
    d->hmemdc_ = ::CreateCompatibleDC(hDC);
    ::ReleaseDC(d->hwnd_, hDC);

    // window is created
    ::SetEvent(d->wnd_init_ev_);

    // process upon WM_QUIT or error
    MSG Msg;
    while (::GetMessage(&Msg, nullptr, 0, 0) > 0)
    {
      ::TranslateMessage(&Msg);
      ::DispatchMessage(&Msg); 
    }
  }


public:
  // must be public for thread accessibility
  HANDLE        thread_handle_;     // worker thread handle
  HANDLE        wnd_init_ev_;       // window init event
  std::string   caption_;           // window caption

  HWND          hwnd_;              // window handle
  HDC           hmemdc_;            // mem dc

  std::int16_t  window_x_;          // x coordinate of output window
  std::int16_t  window_y_;          // y coordinate of output window
  std::uint8_t  zoom_x_;            // x zoom factor of output window
  std::uint8_t  zoom_y_;            // y zoom factor of output window

  std::uint32_t*  frame_buffer_;    // frame buffer for fast drawing
};



///////////////////////////////////////////////////////////////////////////////

/**
 * Windows text head
 * \param Columns Screen X size in chars
 * \param Rows Screen Y size in chars
 */
template<std::uint16_t Columns, std::uint16_t Rows,
         std::uint16_t Viewport_Size_X = Columns, std::uint16_t Viewport_Size_Y = Rows>
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
               std::string caption = "vic text screen")
    : drv(Columns, Rows,
          Viewport_Size_X, Viewport_Size_Y,
          viewport_x, viewport_y,
          orientation_0)
    , window_x_(window_x)
    , window_y_(window_y)
    , caption_(caption)
  {
    font_height_    = 50U;    // font height in pixel
    font_width_     = font_height_ * 40U / 75U;
    char_x_margin_  = 2U;
    char_y_margin_  = 2U;
    char_x_padding_ = 2U;
    char_y_padding_ = 1U;  
  }


  /**
   * dtor
   * Shutdown the driver
   */
  ~windows_text()
  {
    shutdown();
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   D R I V E R   F U N C T I O N S
  //

  virtual void init()
  {
    // create init event to wait until the windows is created
    wnd_init_ev_ = ::CreateEvent(NULL, TRUE, FALSE, NULL);

    // create window thread
    thread_handle_ = new std::thread(worker_thread, (void*)this);

    // wait until the screen is created
    ::WaitForSingleObject(wnd_init_ev_, INFINITE);

    // close handle
    ::CloseHandle(wnd_init_ev_);

    // clear screen
    cls();
  }


  virtual void shutdown()
  {
    (void)::CloseWindow(hwnd_);
    (void)::DeleteObject(hbmp_);
  }


  virtual inline const char* version() const
  {
    return (const char*)VIC_DRV_WINDOWS_VERSION;
  }


  virtual inline bool is_graphic() const
  {
    // This Windows text driver is an alpha numeric (text only) display
    return false;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

protected:

  virtual void cls(color::value_type bg_color = color::gray)
  {
    bg_color_ = bg_color;
    for (std::uint16_t y = 0U; y < screen_height(); y++) {
      for (std::uint16_t x = 0U; x < screen_width(); x++) {
        frame_buffer_[x][y] = ' ';
      }
    }
    present();
  }


  /**
   * Rendering is done (copy RAM / frame buffer to screen)
   */
  virtual void present()
  {
    HGDIOBJ org = ::SelectObject(hmemdc_, ::GetStockObject(DC_PEN));
    ::SelectObject(hmemdc_, ::GetStockObject(DC_BRUSH));
    ::Rectangle(hmemdc_, 0, 0, window_size_x_, window_size_y_);
    ::SelectObject(hmemdc_, org);

    ::SetTextColor(hmemdc_, RGB(0, 210, 195));
    ::SetBkColor(hmemdc_, RGB(color::get_red(bg_color_), color::get_green(bg_color_), color::get_blue(bg_color_)));

    HPEN pen_box = ::CreatePen(PS_DOT, 0U, RGB(0x00, 0x30, 0x30));
    ::SelectObject(hmemdc_, pen_box);

    vertex_type _viewport = viewport_get();
    for (std::int16_t col = _viewport.x; col < viewport_width() && col < screen_width(); col++) {
      for (std::int16_t row = _viewport.y; row < viewport_height() && row < screen_height(); row++) {

        std::int16_t x = 2 * char_x_margin_ + (col * (font_width_  + 2 * char_x_padding_ + 2 * char_x_margin_));
        std::int16_t y =     char_y_margin_ + (row * (font_height_ + 2 * char_y_padding_ + 2 * char_y_margin_));

        RECT rect = { x + char_x_margin_ + char_x_padding_,
                      y + char_y_margin_ + char_y_padding_,
                      x + char_x_margin_ + char_x_padding_ + font_width_,
                      y + char_y_margin_ + char_y_padding_ + font_height_
                    };
        ::DrawTextA(hmemdc_, (LPCSTR)&frame_buffer_[col][row], 1U, &rect, DT_CENTER | DT_TOP | DT_SINGLELINE);

        // draw a square around the char
        POINT box[] = {{ x + char_x_margin_, y + char_y_margin_ },
                       { x + char_x_margin_ + char_x_padding_ + font_width_ + char_x_padding_, y + char_y_margin_ },
                       { x + char_x_margin_ + char_x_padding_ + font_width_ + char_x_padding_, y + char_y_margin_ + char_y_padding_ + font_height_ + char_y_padding_ },
                       { x + char_x_margin_, y + char_y_margin_ + char_y_padding_ + font_height_ + char_y_padding_ },
                       { x + char_x_margin_, y + char_y_margin_ }};
        ::Polyline(hmemdc_, box, 5);
      }
    }

    HDC hDC = ::GetDC(hwnd_);
    ::BitBlt(hDC, 0, 0, window_size_x_, window_size_y_, hmemdc_, 0, 0, SRCCOPY);
    ::ReleaseDC(hwnd_, hDC);
  }


  ///////////////////////////////////////////////////////////////////////////////
  // A L P H A   T E X T   F U N C T I O N S
  //

  /**
   * Set the new text position
   * \param pos Position in chars on text displays (0/0 is left/top)
   */
  inline virtual void text_set_pos(vertex_type pos)
  {
    text_pos_ = pos;
  }


  /**
   * Output a single ASCII/UNICODE char at the actual cursor position
   * \param ch Output character in 16 bit ASCII/UNICODE (NOT UTF-8) format, 00-7F is compatible with ASCII
   */
  inline virtual void text_out(std::uint16_t ch)
  {
    if (ch < 0x20U) {
      // ignore non characters
      return;
    }

    // check limits
    if (screen_is_inside(text_pos_)) {
      frame_buffer_[text_pos_.x][text_pos_.y] = ch;
    }

    // always increment the text position
    text_pos_.x++;
  }


private:

  // worker thread
  static void worker_thread(void* arg)
  {
    windows_text* d = static_cast<windows_text*>(arg);

    const HINSTANCE hInstance = ::GetModuleHandle(NULL);
    const char* className = "vic_text_screen";
    WNDCLASSEXA wc = { };
    wc.cbSize = sizeof(WNDCLASSEXA);
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
    ::RegisterClassExA(&wc);

    // finally create and display the window
    d->hwnd_ = ::CreateWindowExA(
      0,
      className,
      d->caption_.c_str(),
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
      ::MessageBoxA(NULL, "Error creating window", "Error", MB_OK | MB_ICONERROR);
      ::SetEvent(d->wnd_init_ev_);
      return;
    }

    d->window_size_x_ = (d->font_width_  + 2U * d->char_x_margin_ + 2 * d->char_x_padding_) * d->viewport_width()  + 4U * d->char_x_margin_;
    d->window_size_y_ = (d->font_height_ + 2U * d->char_y_margin_ + 2 * d->char_y_padding_) * d->viewport_height() + 2U * d->char_y_margin_;

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

    const char* szFontFile = "enhanced_led_board-7.ttf";
    ::AddFontResourceExA(szFontFile, FR_PRIVATE, NULL);

    LOGFONTA lf;
    memset(&lf, 0, sizeof(lf));
    lf.lfHeight = -MulDiv(d->font_height_ * 60U / 75U * 72U / GetDeviceCaps(d->hmemdc_, LOGPIXELSY), GetDeviceCaps(d->hmemdc_, LOGPIXELSY), 72);
    lf.lfWeight = FW_NORMAL;
    lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
    strcpy_s(lf.lfFaceName, sizeof(lf.lfFaceName), "Enhanced LED Board-7");

    // create and select it
    HFONT font = ::CreateFontIndirectA(&lf);
    ::SelectObject(d->hmemdc_, font);
  
    // copy memory bitmap to viewport
    hDC = ::GetDC(d->hwnd_);
    ::BitBlt(hDC, 0, 0, d->window_size_x_, d->window_size_y_, d->hmemdc_, 0, 0, SRCCOPY);
    ::ReleaseDC(d->hwnd_, hDC);

    ::SetEvent(d->wnd_init_ev_);

    // process upon WM_QUIT or error
    MSG Msg;
    while (::GetMessage(&Msg, nullptr, 0, 0) > 0)
    {
      ::TranslateMessage(&Msg);
      ::DispatchMessage(&Msg); 
    }
  }


public:
  // must be public for thread accessibility
  HANDLE        thread_handle_;           // worker thread handle
  HANDLE        wnd_init_ev_;             // window init event
  std::string   caption_;                 // window caption

  std::uint16_t frame_buffer_[Columns][Rows];

  std::uint16_t font_height_;             // font height in pixel
  std::uint16_t font_width_;
  std::uint16_t char_x_margin_;
  std::uint16_t char_y_margin_;
  std::uint16_t char_x_padding_;
  std::uint16_t char_y_padding_;

  vertex_type   text_pos_;

  std::int16_t  window_x_;                // x coordinate of output window
  std::int16_t  window_y_;                // y coordinate of output window
  std::int16_t  window_size_x_;           // window x size
  std::int16_t  window_size_y_;           // window y size
  HWND          hwnd_;
  HDC           hmemdc_;
  HBITMAP       hbmp_;

  color::value_type bg_color_;            // background color
};

} // namespace head
} // namespace vic

#endif  // _VIC_DRV_WINDOWS_H_
