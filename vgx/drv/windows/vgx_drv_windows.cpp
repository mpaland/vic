///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2014, PALANDesign Hannover, Germany
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

#include "vgx_drv_windows.h"
#include <process.h>

// defines the driver name and version
#define VGX_DRV_VERSION   "Windows driver 1.00"


namespace vgx {
namespace head {

void windows::worker_thread(void* arg)
{
  windows* d = static_cast<windows*>(arg);

  // create a window with emulated screen size
  HINSTANCE hInstance = ::GetModuleHandle(NULL);
  LPCTSTR className = L"vgx screen";
  WNDCLASSEX wc;
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
    return;
  }

  // adjust screen size to display width and height
  RECT rect;
  rect.left   = 0;
  rect.top    = 0;
  rect.right  = d->xsize_ * d->xzoom_;
  rect.bottom = d->ysize_ * d->yzoom_;
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
    return;
  }
  ::SetWindowPos(d->hwnd_, HWND_TOP, d->xpos_, d->ypos_, 0, 0, SWP_NOSIZE |SWP_NOZORDER | SWP_SHOWWINDOW);

  // create a bitmap for all drawing
  HDC hDC = ::GetDC(d->hwnd_);
  d->hmemdc_ = ::CreateCompatibleDC(hDC);
  d->hbmp_   = ::CreateCompatibleBitmap(hDC, d->xsize_ * d->xzoom_, d->ysize_ * d->yzoom_);
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
}


void windows::init()
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


void windows::deinit()
{
  ::CloseWindow(hwnd_);
  ::DeleteObject(hbmp_);
  return;
}


void windows::brightness_set(std::uint8_t)
{
}


const char* windows::version() const
{
  return (const char*)VGX_DRV_VERSION;
}


void windows::drv_primitive_done()
{
  if (primitive_lock_) {
    return;
  }

  // copy memory bitmap to screen
  ::HDC hDC = ::GetDC(hwnd_);
  ::BitBlt(hDC, 0, 0, xsize_ * xzoom_, ysize_ * yzoom_, hmemdc_, 0, 0, SRCCOPY);
  ::ReleaseDC(hwnd_, hDC);
}


void windows::cls()
{
  ::HGDIOBJ org = ::SelectObject(hmemdc_, ::GetStockObject(DC_PEN));
  ::SelectObject(hmemdc_, ::GetStockObject(DC_BRUSH));
  ::SetDCPenColor(hmemdc_,   RGB(color::get_red(color_bg_), color::get_green(color_bg_), color::get_blue(color_bg_)));
  ::SetDCBrushColor(hmemdc_, RGB(color::get_red(color_bg_), color::get_green(color_bg_), color::get_blue(color_bg_)));
  ::Rectangle(hmemdc_, 0, 0, xsize_ * xzoom_, ysize_ * yzoom_);  // needs to be 1 pixel bigger for Windows API
  ::SelectObject(hmemdc_, org);
}


void windows::drv_pixel_set(int16_t x, int16_t y)
{
  drv_pixel_set_color(x, y, color_get());
}


void windows::drv_pixel_set_color(int16_t x, int16_t y, std::uint32_t color)
{
  for (int c = 0; c < xzoom_; ++c) {
    for (int r = 0; r < yzoom_; ++r) {
      (void)::SetPixel(hmemdc_, x * xzoom_ + c, y * yzoom_ + r, RGB(color::get_red(color), color::get_green(color), color::get_blue(color)));
    }
  }
}


std::uint32_t windows::drv_pixel_get(int16_t x, int16_t y) const
{
  COLORREF clr = ::GetPixel(hmemdc_, x * xzoom_, y * yzoom_);
  return color::rgb(GetRValue(clr), GetGValue(clr), GetBValue(clr));
}

} // namespace head
} // namespace vgx
