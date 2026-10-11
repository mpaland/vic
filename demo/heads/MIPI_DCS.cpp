///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief MIPI DCS TFT demo (ILI9341, ILI9488, ST7735, ST7789, ST7796, GC9A01)
// Shown: a 2.4" ILI9341 240 x 320 SPI module in landscape. io layer: option 0 =
// command, 1 = data (D/C line), SPI mode 0, 40 MHz. The reference libraries show
// these modules upright with orientation_180m (portrait) / orientation_90 (landscape).
//
///////////////////////////////////////////////////////////////////////////////

#include "head/MIPI_DCS.h"
#include "scene.h"


// other common modules:
//   head::MIPI_DCS<head::MIPI_DCS_ST7789, 240, 240, drv::orientation_0m, false, true, false, 0, 80> st7789_1_3;  // 1.3" square
//   head::MIPI_DCS<head::MIPI_DCS_ST7735, 128, 160, drv::orientation_0, true, false, false, 2, 1>  st7735_green;  // 1.8" green tab
//   head::MIPI_DCS<head::MIPI_DCS_ILI9488, 320, 480>                                                ili9488_3_5;   // 18 bit pixels on SPI
//   head::MIPI_DCS<head::MIPI_DCS_ST7796, 320, 480>                                                 st7796_4;
//   head::MIPI_DCS<head::MIPI_DCS_GC9A01, 240, 240>                                                 gc9a01_round;
static vic::head::MIPI_DCS<vic::head::MIPI_DCS_ILI9341, 320, 240, vic::drv::orientation_90> tft(reinterpret_cast<vic::io::handle_type>(0));


int main()
{
  demo_banner(tft, "ILI9341 2.4\" SPI, landscape");
  tft.init();
  demo_scene(tft);
  tft.display_brightness(200U);
  return 0;
}
