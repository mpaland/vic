///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2011-2018, PALANDesign Hannover, Germany
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
// \brief device class for BA6x LCDs
// This device class supports the famous and world bests status displays BA60,
// BA63 and BA66 by Wincor/Nixdorf!
// You can get a BA63 very easy on ebay.
// BA60 has one line, BA63 has two lines (most found in the wild) and BA66 has
// four lines:
// The displays can be connected via USB (modern version) or serial via RS232
// interface.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_DRV_BA6X_H_
#define _VIC_DRV_BA6X_H_

#include <string.h>

#include "../drv.h"
#include "../tc.h"    // include text context for alpha numeric driver


// defines the driver name and version
#define VIC_DRV_BA6X_VERSION  "Wincor BA6x driver 2.0.1"


namespace vic {
namespace head {


// BA6x model
typedef enum tag_BA6x_model_type {
  BA60 = 0,   // 1 line,  16 chars
  BA63,       // 2 lines, 20 chars
  BA66        // 4 lines, 20 chars
} BA6x_model_type;


/**
 * BA6x head is the famous Wincor alpha numeric display
 * \param Model Display model, BA60, BA63 and BA66 are possible options
 */
template<BA6x_model_type Model>
class BA6x final : public drv
{ 
  // constants
  static const std::uint8_t BA6X_MAX_CMD_LENGTH = 0x20U;    // maximum command length (device constant)
  static const std::uint8_t ESC                 = 0x1BU;    // escape char

  // vars
  const io::handle_type device_handle_;                     // logical device handle (uart or usb)
  vertex_type           cur_pos_;                           // actual cursor position

public:

  /**
   * ctor
   * \param device_handle Logical device ID (COM port number or USB ID)
   */
  BA6x(io::handle_type device_handle)
    : drv(Model == BA60 ? 16U : 20U, Model == BA60 ? 1U : Model == BA63 ? 2U : 4U,    // screen size
          Model == BA60 ? 16U : 20U, Model == BA60 ? 1U : Model == BA63 ? 2U : 4U,    // viewport size (same as screen)
          0, 0, orientation_0)
    , device_handle_(device_handle)
  { }


  /**
   * dtor
   * Shutdown the driver
   */
  ~BA6x()
  {
    shutdown();
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   D R I V E R   F U N C T I O N S
  //

  virtual void init()
  {
    // init the interface
    io::init(device_handle_);

    // BX6x needs about 750 ms time to get ready
    io::delay(750U);

    // clear rx buffer
    std::uint8_t dummy[16];
    std::size_t  dummy_len = 16U;
    while (io::read(device_handle_, 0U, dummy, dummy_len, 0U));

    // clear display
    cls();

    // explicit cursor home
    text_set_pos({ 0, 0 });
  }


  virtual void shutdown()
  {
    // clear buffer
    cls();
  }


  inline virtual const char* version() const
  {
    return (const char*)VIC_DRV_BA6X_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    // BA6x is an alpha numeric display
    return false;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N   F U N C T I O N S
  //

protected:

  inline virtual void cls(color::value_type)
  {
    // send 1B 5B 32 4A
    static const std::uint8_t cmd[4] = { ESC, 0x5BU, 0x32U, 0x4AU };

    // send command
    write_command(cmd, 4U);
  }


  ///////////////////////////////////////////////////////////////////////////////
  // A L P H A   T E X T   F U N C T I O N S
  //

  /**
   * Output a single ASCII/UNICODE char at the actual cursor position
   * The cursor position is moved by the char width (distance)
   * \param ch Output character in 16 bit ASCII/UNICODE (NOT UTF-8) format, 00-7F is compatible with ASCII
   */
  virtual void text_out(std::uint16_t ch)
  {
    if (ch < 0x20U) {
      // ignore non characters
      return;
    }

    // check limits
    if (screen_is_inside(cur_pos_)) {
      // send command
      write_command(reinterpret_cast<std::uint8_t*>(&ch), 1U);
    }

    // inc x and reposition cursor if an implicit CR occured at end of the line 
    if (++cur_pos_.x == static_cast<std::int16_t>(screen_width())) {
      text_set_pos(cur_pos_);
    }
  }


  /**
   * Render an UTF-8 / ASCII coded string at the actual cursor position
   * \param string Output string in UTF-8/ASCII format, zero terminated
   * \return Number of written characters, not bytes (as an UTF-8 character may consist out of more bytes)
   */
  virtual std::uint16_t text_out(const std::uint8_t* string)
  {
    // the base class function might be used, but the char by char write command overhead is too big,
    // so assemble the data here and pass it directly
    // CAUTION: No Unicode conversion is done, data must be in display format!
    std::uint16_t len = (std::uint16_t)strlen(reinterpret_cast<const char*>(string));
    std::uint16_t off = 0U;

    // check if string is (partly) in screen
    if ((cur_pos_.x + len >= 0) && (cur_pos_.x < screen_width()) &&
        (cur_pos_.y >= 0) && (cur_pos_.y < screen_height())) {
      // calc offset
      if (cur_pos_.x < 0) {
        off = 0 - cur_pos_.x;
      }
      // calc visible len
      if (cur_pos_.x < 0) {
        len = cur_pos_.x + len;
      }
      else if (cur_pos_.x + len > screen_width()) {
        len = screen_width() - cur_pos_.x;  // limit string to screen size
      }
      if (len > screen_width()) {
        len = screen_width();
      }

      // send command
      write_command(string + off, len);
      return len;
    }
    return 0U;
  }


  /**
   * Set the new text position
   * \param pos Position in chars on text displays (0/0 is left/top)
   */
  virtual void text_set_pos(vertex_type pos)
  {
    cur_pos_ = pos;       // store pos

    if (pos.x < 0) {      // position cursor to col 0 if outside screen to get partial string offset right
      pos.x = 0;
    }

    if (!screen_is_inside(pos)) {
      return;
    }

    pos.x++; pos.y++;   // 1/1 is top left in BA6x, so add 1

    // send 1B 5B XX 3B YY 48
    std::uint8_t cmd[8];
    std::uint8_t len = 0U;

    cmd[len++] = ESC;
    cmd[len++] = 0x5BU;

    // y
    if (pos.y >= 10) {
      cmd[len++] = 0x30U + static_cast<std::uint8_t>(pos.y / 10U);
    }
    cmd[len++] = 0x30U + static_cast<std::uint8_t>(pos.y % 10U);
    cmd[len++] = 0x3BU;
  
    // x
    if (pos.x >= 10) {
      cmd[len++] = 0x30U + static_cast<std::uint8_t>(pos.x / 10U);
    }
    cmd[len++] = 0x30U + static_cast<std::uint8_t>(pos.x % 10U);
    cmd[len++] = 0x48U;

    // send command
    write_command(cmd, len);
  }


  /**
   * Clear actual line from cursor pos (included) to end of line
   */
  inline virtual void text_clear_eol()
  {
    // send 1B 5B 32 4A
    static const std::uint8_t cmd[4] = { ESC, 0x5BU, 0x30U, 0x4BU };

    // send command
    write_command(cmd, 4U);
  }


/////////////////////////////////////////////////////////////////////////////
// D R I V E R   A P I   I N T E R F A C E

public:

  // country-specific character set
  typedef enum tag_country_code_type {
    USA           = 0x00,
    France        = 0x01,
    Germany       = 0x02,
    GreatBritain  = 0x03,
    Denmark1      = 0x04,
    Sweden        = 0x05,
    Italy         = 0x06,
    Spain1        = 0x07,
    Japan         = 0x08,
    Norway        = 0x09,
    Denmark2      = 0x0A,
    Spain2        = 0x0B,
    LatinAmerica  = 0x0C,
    CP437         = 0x30,   // Standard, Latin-America
    CP850         = 0x31,   // Latin 1, International, Scandinavia, Latin-America
    CP852         = 0x32,   // Latin 2, Hungary, Poland, Czechia, Slowakia
    CP857         = 0x33,   // Latin 5, Turkey
    CP858         = 0x34,   // Latin 1 + € char, International, Scandinavia, Latin-America
    CP866         = 0x35,   // Latin/Cyrillic, Russia
    CP737         = 0x36,   // Latin/Greek 2, Greece
    CP862         = 0x37,   // Latin/Hebrew, Israel
    IBM813        = 0x38,   // Latin/Greek 2, Greece
    Katakana      = 0x63,   // Katakana, Japan
    UserPage      = 0x73    // User definable page
  } country_code_type;


  /**
   * Set the country-specific character set
   * \return true if successful
   */
  inline void set_country_code(country_code_type code)
  {
    // send 1B 52 CC
    const std::uint8_t cmd[3] = { ESC, 0x52U, static_cast<std::uint8_t>(code) };

    // send command
    write_command(cmd, 3U);
  }


  /**
   * Perform a display self test
   * \return true if successful
   */
  bool self_test() const
  {
    // make sure, the display is idle
    io::delay(200U);

    // command: 00H, 10H, 00H
    const std::uint8_t cmd[3] = { 0x00U, 0x10U, 0x00U };

    // send data to display
    io::write(device_handle_, 0U, cmd, 3U, nullptr, 0U);

    // wait that display finishes the test (ca. 30 sec)
    std::uint8_t response[4];
    std::size_t  len = 4U;
    if (!read_response(response, len, 30000U) || len != 4U || response[0] != 0x04U || (response[1] & 0x0A0U)) {
      // invalid response, error or device not ready
      return false;
    }
    return true;
  }


  /**
   * Check if display is ready
   * \return true if display is ready
   */
  bool device_ready() const
  {
    // status request command: 00H, 20H, 00H, 00H
    const std::uint8_t cmd[3] = { 0x00U, 0x20U, 0x00U };

    // send command to display
    io::write(device_handle_, 0U, cmd, 3U, nullptr, 0U);

    // read and check status response
    // expected response: 04H, Status Byte 0, Status Byte 1, Status Byte 2 (SB0 oer SB1 != 0 is error)
    std::uint8_t response[4];
    std::size_t  len = 4U;
    if (!read_response(response, len) || len != 4U || response[0] != 0x04U || (response[1] & 0x0A0U)) {
      // invalid response, error or device not ready
      return false;
    }

    // ready to receive commands
    return true;
  }


private:

  void write_command(const std::uint8_t* data, std::uint16_t data_count) const
  {
    // write data command is 02H, 00H, Data Count, Data Bytes
    // maximum length of one message the display can handle is BA6X_MAX_CMD_LENGTH
    std::uint16_t offset = 0U;
    std::uint8_t  msg[BA6X_MAX_CMD_LENGTH];

    do {
      const std::uint16_t blk_size = data_count - offset > BA6X_MAX_CMD_LENGTH - 3U ? BA6X_MAX_CMD_LENGTH - 3U : data_count - offset;

      msg[0] = 0x02U;
      msg[1] = 0x00U;
      msg[2] = static_cast<std::uint8_t>(blk_size);
      memcpy(&msg[3], &data[offset], blk_size);

      // send data to display
      bool res = io::write(device_handle_, 0U, msg, blk_size + 3U, nullptr, 0U);
      std::uint8_t response[4];
      std::size_t  len = 4U;
      if (!(res && (read_response(response, len)) && (len == 4U) && (response[0] == 0x04U) && (!(response[1] & 0xA0U)))) {
        // abort
        return;
      }
      // process next command segment
      offset += blk_size;
    } while (offset < data_count);
  }


  /**
   * Read response
   * \param data_count On input the minimum chars to read, on return the available chars
   * \return true if response is positive/successful
   */
  bool read_response(std::uint8_t* data, std::size_t& data_count, std::uint32_t timeout = 100U) const
  {
    std::size_t offset   = 0U;
    std::size_t max_size = data_count;
    std::size_t count    = max_size;

    while (io::read(device_handle_, 0U, &data[offset], count, timeout)) {
      offset += (std::int16_t)count;
      count = max_size - offset;
      if (offset >= data_count) {
        data_count = offset;
        return true;
      }
    }

    // timeout or error
    return false;
  }

};

} // namespace head
} // namespace vic

#endif // _VIC_DRV_BA6X_H_
