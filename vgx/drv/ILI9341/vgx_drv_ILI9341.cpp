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
// \brief ILI9341 LCD driver
//
///////////////////////////////////////////////////////////////////////////////

#include "vgx_drv_ILI9341.h"

// defines the driver name and version
#define VGX_DRV_VERSION   "ILI9341 driver 1.00"

#include "sys/sys.h"

/** 
 * registers
 */ 
#define ILI9341_REG_SLEEP_OUT            0x11   // Sleep out register
#define ILI9341_REG_GAMMA                0x26   // Gamma register
#define ILI9341_REG_DISPLAY_OFF          0x28   // Display off register
#define ILI9341_REG_DISPLAY_ON           0x29   // Display on register
#define ILI9341_REG_COLUMN_ADDR          0x2A   // Colomn address register
#define ILI9341_REG_PAGE_ADDR            0x2B   // Page address register
#define ILI9341_REG_GRAM                 0x2C   // GRAM register
#define ILI9341_REG_MAC                  0x36   // Memory Access Control register
#define ILI9341_REG_PIXEL_FORMAT         0x3A   // Pixel Format register
#define ILI9341_REG_WDB                  0x51   // Write Brightness Display register
#define ILI9341_REG_WCD                  0x53   // Write Control Display register
#define ILI9341_REG_RGB_INTERFACE        0xB0   // RGB Interface Signal Control
#define ILI9341_REG_FRC                  0xB1   // Frame Rate Control register
#define ILI9341_REG_BPC                  0xB5   // Blanking Porch Control register
#define ILI9341_REG_DFC                  0xB6   // Display Function Control register
#define ILI9341_REG_POWER1               0xC0   // Power Control 1 register
#define ILI9341_REG_POWER2               0xC1   // Power Control 2 register
#define ILI9341_REG_VCOM1                0xC5   // VCOM Control 1 register
#define ILI9341_REG_VCOM2                0xC7   // VCOM Control 2 register
#define ILI9341_REG_POWERA               0xCB   // Power control A register
#define ILI9341_REG_POWERB               0xCF   // Power control B register
#define ILI9341_REG_PGAMMA               0xE0   // Positive Gamma Correction register
#define ILI9341_REG_NGAMMA               0xE1   // Negative Gamma Correction register
#define ILI9341_REG_DTCA                 0xE8   // Driver timing control A
#define ILI9341_REG_DTCB                 0xEA   // Driver timing control B
#define ILI9341_REG_POWER_SEQ            0xED   // Power on sequence register
#define ILI9341_REG_3GAMMA_EN            0xF2   // 3 Gamma enable register
#define ILI9341_REG_INTERFACE            0xF6   // Interface control register
#define ILI9341_REG_PRC                  0xF7   // Pump ratio control register


/** 
  * @brief  LCD Registers  
  */ 
#define LCD_SLEEP_OUT            0x11   /* Sleep out register */
#define LCD_GAMMA                0x26   /* Gamma register */
#define LCD_DISPLAY_OFF          0x28   /* Display off register */
#define LCD_DISPLAY_ON           0x29   /* Display on register */
#define LCD_COLUMN_ADDR          0x2A   /* Colomn address register */ 
#define LCD_PAGE_ADDR            0x2B   /* Page address register */ 
#define LCD_GRAM                 0x2C   /* GRAM register */   
#define LCD_MAC                  0x36   /* Memory Access Control register*/
#define LCD_PIXEL_FORMAT         0x3A   /* Pixel Format register */
#define LCD_WDB                  0x51   /* Write Brightness Display register */
#define LCD_WCD                  0x53   /* Write Control Display register*/
#define LCD_RGB_INTERFACE        0xB0   /* RGB Interface Signal Control */
#define LCD_FRC                  0xB1   /* Frame Rate Control register */
#define LCD_BPC                  0xB5   /* Blanking Porch Control register*/
#define LCD_DFC                  0xB6   /* Display Function Control register*/
#define LCD_POWER1               0xC0   /* Power Control 1 register */
#define LCD_POWER2               0xC1   /* Power Control 2 register */
#define LCD_VCOM1                0xC5   /* VCOM Control 1 register */
#define LCD_VCOM2                0xC7   /* VCOM Control 2 register */
#define LCD_POWERA               0xCB   /* Power control A register */
#define LCD_POWERB               0xCF   /* Power control B register */
#define LCD_PGAMMA               0xE0   /* Positive Gamma Correction register*/
#define LCD_NGAMMA               0xE1   /* Negative Gamma Correction register*/
#define LCD_DTCA                 0xE8   /* Driver timing control A */
#define LCD_DTCB                 0xEA   /* Driver timing control B */
#define LCD_POWER_SEQ            0xED   /* Power on sequence register */
#define LCD_3GAMMA_EN            0xF2   /* 3 Gamma enable register */
#define LCD_INTERFACE            0xF6   /* Interface control register */
#define LCD_PRC                  0xF7   /* Pump ratio control register */


namespace vgx {
namespace head {


void ili9341::init()
{
  power_on();
  display_on();
}


void ili9341::deinit()
{
  brightness_set(0U);     // display off
}


void ili9341::brightness_set(std::uint8_t level)
{
  (void)level;
}


const char* ili9341::version() const
{
  return (const char*)VGX_DRV_VERSION;
}


void ili9341::cls()
{
}


void ili9341::pixel_set(int16_t x, int16_t y)
{
  (void)x; (void)y;
}


void ili9341::pixel_set_color(int16_t x, int16_t y, std::uint32_t color)
{
  (void)x; (void)y; (void)color;
}


std::uint32_t ili9341::pixel_get(int16_t x, int16_t y) const
{
  (void)x; (void)y;
  return 0U;
}


void ili9341::write_command(std::uint8_t cmd)
{
  // set the D/CX line to 0 for command
  out_dio_set(dio_id_, false);

  // send command to SPI and wait until finished
  out_spi(spi_id_, &cmd, 1U, nullptr, 0U);
}


void ili9341::write_data(std::uint8_t cmd)
{
  // set the D/CX line to 1 for command
  out_dio_set(dio_id_, true);

  // send command to SPI and wait until finished
  out_spi(spi_id_, &cmd, 1U, nullptr, 0U);
} 


/**
 * poweron sequence as described in ILI9341 datasheet
 */
void ili9341::power_on()
{
  write_command(0xCA);
  write_data(0xC3);
  write_data(0x08);
  write_data(0x50);
  write_command(LCD_POWERB);
  write_data(0x00);
  write_data(0xC1);
  write_data(0x30);
  write_command(LCD_POWER_SEQ);
  write_data(0x64);
  write_data(0x03);
  write_data(0x12);
  write_data(0x81);
  write_command(LCD_DTCA);
  write_data(0x85);
  write_data(0x00);
  write_data(0x78);
  write_command(LCD_POWERA);
  write_data(0x39);
  write_data(0x2C);
  write_data(0x00);
  write_data(0x34);
  write_data(0x02);
  write_command(LCD_PRC);
  write_data(0x20);
  write_command(LCD_DTCB);
  write_data(0x00);
  write_data(0x00);
  write_command(LCD_FRC);
  write_data(0x00);
  write_data(0x1B);
  write_command(LCD_DFC);
  write_data(0x0A);
  write_data(0xA2);
  write_command(LCD_POWER1);
  write_data(0x10);
  write_command(LCD_POWER2);
  write_data(0x10);
  write_command(LCD_VCOM1);
  write_data(0x45);
  write_data(0x15);
  write_command(LCD_VCOM2);
  write_data(0x90);
  write_command(LCD_MAC);
  write_data(0xC8);
  write_command(LCD_3GAMMA_EN);
  write_data(0x00);
  write_command(LCD_RGB_INTERFACE);
  write_data(0xC2);
  write_command(LCD_DFC);
  write_data(0x0A);
  write_data(0xA7);
  write_data(0x27);
  write_data(0x04);

  /* colomn address set */
  write_command(LCD_COLUMN_ADDR);
  write_data(0x00);
  write_data(0x00);
  write_data(0x00);
  write_data(0xEF);

  /* Page Address Set */
  write_command(LCD_PAGE_ADDR);
  write_data(0x00);
  write_data(0x00);
  write_data(0x01);
  write_data(0x3F);
  write_command(LCD_INTERFACE);
  write_data(0x01);
  write_data(0x00);
  write_data(0x06);

  write_command(LCD_GRAM);
  volatile int a = 0;
  for (std::uint16_t n = 0U; n < 20000U; n++) {
    a++;
  }

  write_command(LCD_GAMMA);
  write_data(0x01);

  write_command(LCD_PGAMMA);
  write_data(0x0F);
  write_data(0x29);
  write_data(0x24);
  write_data(0x0C);
  write_data(0x0E);
  write_data(0x09);
  write_data(0x4E);
  write_data(0x78);
  write_data(0x3C);
  write_data(0x09);
  write_data(0x13);
  write_data(0x05);
  write_data(0x17);
  write_data(0x11);
  write_data(0x00);
  write_command(LCD_NGAMMA);
  write_data(0x00);
  write_data(0x16);
  write_data(0x1B);
  write_data(0x04);
  write_data(0x11);
  write_data(0x07);
  write_data(0x31);
  write_data(0x33);
  write_data(0x42);
  write_data(0x05);
  write_data(0x0C);
  write_data(0x0A);
  write_data(0x28);
  write_data(0x2F);
  write_data(0x0F);

  write_command(LCD_SLEEP_OUT);
  for (std::uint16_t n = 0U; n < 20000U; n++) {
    a++;
  }

  write_command(LCD_DISPLAY_ON);

  /* GRAM start writing */
  write_command(LCD_GRAM);
 }


/**
  * @brief  Enables the Display.
  * @param  None
  * @retval None
  */
void ili9341::display_on(void)
{
  write_command(LCD_DISPLAY_ON);
}


/**
  * @brief  Disables the Display.
  * @param  None
  * @retval None
  */
void ili9341::display_off(void)
{
  /* Display Off */
  write_command(LCD_DISPLAY_OFF);
}

} // namespace head
} // namespace vgx
