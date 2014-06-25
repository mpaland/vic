// main.cpp : Defines the entry point for the demo application.

#include <Windows.h>

#include "drv/windows/vgx_drv_windows.h"


// create a single windows head
static vgx::drv_windows& get_head_drv()
{
  // create a windows head with 240x120 pixel, zero offset, screen pos at 50,50, zoom factor 2
  static vgx::drv_windows _head_drv(240, 120, 0, 0, 50, 50, 2, 2);
  return _head_drv;
}

vgx::drv& head()
{
  return get_head_drv();
}

//////////////////////////////////////////////////////////////////////////
int main()
{
  head().init();    // init head

  head().color_set(VGX_COLOR_BRIGHTBLUE);
  head().triangle_solid(30, 10, 10, 30, 60, 30);

  head().color_set(VGX_COLOR_RED);
  head().box(65, 5, 80, 15);

  head().color_set(VGX_COLOR_YELLOW);
  head().text_set_font(vgx::font_Arial_num_16x24_aa4);
  head().text_string_pos(10, 50, (const std::uint8_t*)"12,34.56-7890");

  ::Sleep(20000);

  return 0;
}

