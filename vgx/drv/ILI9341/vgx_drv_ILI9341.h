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
// \brief ILI9341 driver
// The ILI9341 is a complete LCD display controller for resolutions up to
// 320x240 pixelwith integrated RAM
// 
// MCU interface
// The ILI provides different system interfaces: 8bit data bus, 16 bit data bus, SPI
//
// RGB interface
// The ILI has the possibility to use the data pins as RGB input together with
// the VSYNV, HSYNC etc. signals. In this mode, SPI/serial interface has to be used
// for communication
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VGX_DRV_ILI9341_H_
#define _VGX_DRV_ILI9341_H_

#include "vgx_drv.h"


namespace vgx {
namespace head {


class ili9341 : public drv
{
public:
/////////////////////////////////////////////////////////////////////////////
// M A N D A T O R Y   F U N C T I O N S

  /**
   * ctor
   * \param xsize Screen width
   * \param ysize Screen height
   * \param xoffset X offset of the screen, relative to top/left corner
   * \param yoffset Y offset of the screen, relative to top/left corner
   */
  ili9341(std::uint16_t xsize, std::uint16_t ysize, std::int16_t xoffset, std::int16_t yoffset,
          std::uint8_t spi_id, std::uint8_t dio_id,
          bool rgb_interface)
    : drv(xsize, ysize, xoffset, yoffset)
    , spi_id_(spi_id)
    , dio_id_(dio_id)
    , rgb_interface_(rgb_interface)
  { }

  /**
   * dtor
   * Deinit the driver
   */
  ~ili9341()
  { deinit(); }

  // mandatory driver functions
  virtual void init();                                    // driver init
  virtual void deinit();                                  // driver deinit
  virtual void brightness_set(std::uint8_t level);        // set display brightness/backlight
  virtual const char* version() const;                    // get driver name and version
  virtual void cls();                                     // clear display, all pixels off (black)

/////////////////////////////////////////////////////////////////////////////
// M A N D A T O R Y   F U N C T I O N S   F O R   G R A P H I C   H E A D S

  /**
   * Set pixel (in actual drawing color)
   * \param x X value
   * \param y Y value
   */
  virtual void pixel_set(std::int16_t x, std::int16_t y);

  /**
   * Set pixel in given color, the color doesn't change the actual drawing color
   * \param x X value
   * \param y Y value
   * \param color Color of pixel in ARGB format
   */
  virtual void pixel_set_color(std::int16_t x, std::int16_t y, std::uint32_t color);

  /**
   * Get pixel color
   * \param x X value
   * \param y Y value
   * \return Color of pixel in ARGB format
   */
  virtual std::uint32_t pixel_get(std::int16_t x, std::int16_t y) const;


private:
  std::uint8_t  spi_id_;
  std::uint8_t  dio_id_;
  bool          rgb_interface_;

  void write_command(std::uint8_t cmd);
  void write_data(std::uint8_t cmd);

  void power_on();
  void display_on();
  void display_off();

};

} // namespace head
} // namespace vgx

#endif  // _VGX_DRV_SKELETON_H_
