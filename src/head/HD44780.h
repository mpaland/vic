///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
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
// \brief HD44780 head, the standard alphanumeric LCD controller (16 x 2, 20 x 4 ...)
// The head is an alpha numeric head (is_graphic() = false) and implements the
// text functions of drv. See doc/HD44780.md for the programming summary.
//
// io layer convention: every byte is one io::write() call, the low byte of the
// option selects the register:
//   0 : instruction (RS = 0)
//   1 : data (RS = 1)
//   2 : instruction, high nibble only - the four single nibble transfers of the
//       4 bit init sequence. An 8 bit io layer treats option 2 like option 0.
// Bits 8..23 of the option carry the execution time of the byte in us (the
// Exec_Us template parameter), an io layer with HD44780_WAIT_IO waits that long
// after the transfer: (option >> 8) & 0xFFFF. Other io layers ignore the bits.
// The io layer drives E, R/W and RS and splits the bytes into two nibbles in
// 4 bit mode (or talks to an I2C port expander). io::read() with option 0
// returns the busy flag (bit 7) and the address counter, in 4 bit mode the io
// layer assembles the two nibbles.
// The controller needs Exec_Us after every byte (37 us for the HD44780, 43 us
// for the ST7066) and 1.52 ms after clear / home, the Wait template parameter
// selects who waits (see HD44780_wait_type).
//
// Compatible controllers: the head works unchanged with the Sitronix ST7066 (and
// ST7066U), Samsung KS0066U, Epson SED1278 and Sunplus SPLC780, they are pin and
// instruction compatible with the HD44780 (crosschecked against the ST7066
// datasheet, see doc/HD44780.md section 8). The only difference is the execution
// time of a data byte, 43 us on the ST7066 instead of 37 us, which is why
// Exec_Us defaults to 43. ROM code 0A of the ST7066 is the Japanese font
// (HD44780_ROM_A00), ROM code 0B the European font (HD44780_ROM_A02).
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_HD44780_H_
#define _VIC_HEAD_HD44780_H_

#include "../drv.h"


// defines the head name and version
#define VIC_HEAD_HD44780_VERSION   "HD44780 head 1.00"

namespace vic {
namespace head {


// bus width
typedef enum tag_HD44780_bus_type {
  HD44780_BUS8,         // 8 bit bus, DB7..DB0
  HD44780_BUS4          // 4 bit bus, DB7..DB4, two transfers per byte (done by the io layer)
} HD44780_bus_type;

// wait strategy after every byte
typedef enum tag_HD44780_wait_type {
  HD44780_WAIT_DELAY,   // the head calls io::delay(1) after every byte, works with every io layer but is slow
  HD44780_WAIT_BUSY,    // the head polls the busy flag with io::read() (falls back to the delay if the io layer can't read)
  HD44780_WAIT_IO       // the io layer blocks until the controller accepts the next byte (Exec_Us delay from the option bits 8..23 or busy poll), only clear / home are delayed by the head
} HD44780_wait_type;

// character ROM of the controller
typedef enum tag_HD44780_rom_type {
  HD44780_ROM_A00,      // Japanese standard font: ASCII, half width Katakana at A1h..DFh, some symbols at E0h..FFh
  HD44780_ROM_A02       // European standard font: A0h..FFh follow ISO 8859-1
} HD44780_rom_type;


/**
 * HD44780 display controller
 * \param Columns Visible characters per line, max 40
 * \param Lines Number of lines, 1, 2 or 4 (4 line modules are 2 line controllers, line 3 / 4 continue line 1 / 2)
 * \param Bus Bus width
 * \param Wait Wait strategy after every byte
 * \param Rom Character ROM for the UNICODE to character code mapping
 * \param Font_5x10 5 x 10 dot font instead of 5 x 8, only possible with one line
 * \param Exec_Us Execution time of an instruction or data byte in us, passed to the io layer in the
 *                option bits 8..23 (HD44780: 37, ST7066: 43, scale by 270 kHz / fOSC for other clocks)
 */
template<std::uint16_t Columns = 16U, std::uint16_t Lines = 2U,    // visible characters per line, lines
         HD44780_bus_type  Bus  = HD44780_BUS8,                    // bus width
         HD44780_wait_type Wait = HD44780_WAIT_DELAY,              // wait strategy
         HD44780_rom_type  Rom  = HD44780_ROM_A00,                 // character ROM
         bool Font_5x10 = false,                                   // 5 x 10 dot font (1 line only)
         std::uint16_t Exec_Us = 43U                               // execution time per byte in us
>
class HD44780 final : public drv
{
  static_assert((Lines == 1U) || (Lines == 2U) || (Lines == 4U), "HD44780: Lines must be 1, 2 or 4");
  static_assert(Columns <= 40U, "HD44780: Columns must be 40 or less");
  static_assert((Lines * Columns) <= 80U, "HD44780: the DDRAM holds 80 characters");
  static_assert(!Font_5x10 || (Lines == 1U), "HD44780: the 5 x 10 font is only possible with one line");

  // instructions
  static const std::uint8_t CMD_CLEAR           = 0x01U;    // clear display, DDRAM = 20h, AC = 0, 1.52 ms
  static const std::uint8_t CMD_HOME            = 0x02U;    // return home, AC = 0, shift reset, 1.52 ms
  static const std::uint8_t CMD_ENTRY_MODE      = 0x04U;    // | I/D (02h) | S (01h)
  static const std::uint8_t CMD_DISPLAY         = 0x08U;    // | D (04h) | C (02h) | B (01h)
  static const std::uint8_t CMD_SHIFT           = 0x10U;    // | S/C (08h) | R/L (04h)
  static const std::uint8_t CMD_FUNCTION        = 0x20U;    // | DL (10h) | N (08h) | F (04h)
  static const std::uint8_t CMD_CGRAM_ADDR      = 0x40U;    // | address (6 bit)
  static const std::uint8_t CMD_DDRAM_ADDR      = 0x80U;    // | address (7 bit)

  // instruction bits
  static const std::uint8_t ENTRY_INCREMENT     = 0x02U;
  static const std::uint8_t DISPLAY_ON          = 0x04U;
  static const std::uint8_t CURSOR_ON           = 0x02U;
  static const std::uint8_t BLINK_ON            = 0x01U;
  static const std::uint8_t SHIFT_DISPLAY       = 0x08U;
  static const std::uint8_t SHIFT_RIGHT         = 0x04U;
  static const std::uint8_t FUNCTION_8BIT       = 0x10U;
  static const std::uint8_t FUNCTION_2LINES     = 0x08U;
  static const std::uint8_t FUNCTION_5X10       = 0x04U;
  static const std::uint8_t BUSY_FLAG           = 0x80U;

  // io options: bits 0..7 select the register, bits 8..23 carry the execution time of the byte in us
  static const std::uint32_t OPT_INSTRUCTION    = 0U;
  static const std::uint32_t OPT_DATA           = 1U;
  static const std::uint32_t OPT_NIBBLE         = 2U;
  static const std::uint32_t OPT_EXEC_US        = static_cast<std::uint32_t>(Exec_Us) << 8U;

  // busy flag polls before giving up
  static const std::uint16_t BUSY_POLLS         = 1000U;

  // vars
  const io::handle_type device_handle_;   // logical device handle
  vertex_type           cur_pos_;         // logical cursor position, may be outside the screen
  vertex_type           ac_pos_;          // logical position of the address counter, x < 0 if unknown
  std::uint8_t          display_ctrl_;    // D, C, B bits of the display on/off control

public:

  /**
   * ctor
   * \param device_handle Logical device handle of the display interface
   */
  HD44780(io::handle_type device_handle)
    : drv(Columns, Lines,
          Columns, Lines,                 // no viewport support (screen = viewport)
          0, 0, orientation_0)
    , device_handle_(device_handle)
    , cur_pos_({ 0, 0 })
    , ac_pos_({ -1, -1 })
    , display_ctrl_(DISPLAY_ON)
  { }


  /**
   * dtor
   * Shutdown the driver
   */
  ~HD44780()
  {
    shutdown();
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   D R I V E R   F U N C T I O N S
  //

  /**
   * Initialization by instruction (datasheet figures 23 / 24), works regardless of the internal reset
   */
  virtual void init()
  {
    io::init(device_handle_);
    io::delay(50U);         // > 40 ms after VCC rises to 2.7 V

    // the controller is in 8 bit mode after power on, the first three function sets are single transfers
    // (one nibble on the 4 bit bus) and must not wait for the busy flag
    const std::uint8_t  reset8  = CMD_FUNCTION | FUNCTION_8BIT;
    const std::uint32_t opt     = ((Bus == HD44780_BUS4) ? OPT_NIBBLE : OPT_INSTRUCTION) | OPT_EXEC_US;
    io::write(device_handle_, opt, &reset8, 1U, nullptr, 0U);
    io::delay(5U);          // > 4.1 ms
    io::write(device_handle_, opt, &reset8, 1U, nullptr, 0U);
    io::delay(1U);          // > 100 us
    io::write(device_handle_, opt, &reset8, 1U, nullptr, 0U);
    io::delay(1U);
    if (Bus == HD44780_BUS4) {
      // switch to the 4 bit bus, one more single nibble transfer
      const std::uint8_t reset4 = CMD_FUNCTION;
      io::write(device_handle_, OPT_NIBBLE | OPT_EXEC_US, &reset4, 1U, nullptr, 0U);
      io::delay(1U);
    }

    // function set: bus width, lines and font can't be changed after this point
    write_cmd(static_cast<std::uint8_t>(CMD_FUNCTION |
                                        ((Bus == HD44780_BUS8) ? FUNCTION_8BIT   : 0U) |
                                        ((Lines > 1U)          ? FUNCTION_2LINES : 0U) |
                                        (Font_5x10             ? FUNCTION_5X10   : 0U)));
    write_cmd(CMD_DISPLAY);                             // display off
    write_cmd(CMD_CLEAR, true);                         // clear
    write_cmd(static_cast<std::uint8_t>(CMD_ENTRY_MODE | ENTRY_INCREMENT));        // increment, no display shift
    display_ctrl_ = DISPLAY_ON;
    write_cmd(static_cast<std::uint8_t>(CMD_DISPLAY | display_ctrl_));             // display on, cursor off
    cur_pos_ = { 0, 0 };
    ac_pos_  = { 0, 0 };
  }


  virtual void shutdown()
  {
    // display off, the DDRAM content is retained
    write_cmd(CMD_DISPLAY);
  }


  inline virtual const char* version() const
  {
    return (const char*)VIC_HEAD_HD44780_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    // HD44780 is an alpha numeric display
    return false;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N   F U N C T I O N S
  //

protected:

  /**
   * Clear the display, the cursor is at (0, 0) afterwards
   */
  virtual void cls(color::value_type bg_color = color::none)
  {
    (void)bg_color;
    write_cmd(CMD_CLEAR, true);
    cur_pos_ = { 0, 0 };
    ac_pos_  = { 0, 0 };
  }


  ///////////////////////////////////////////////////////////////////////////////
  // A L P H A   T E X T   F U N C T I O N S
  //

  /**
   * Output a single ASCII/UNICODE char at the actual cursor position
   * The cursor position is moved by one, characters outside the screen are not sent
   * \param ch Output character in 16 bit ASCII/UNICODE (NOT UTF-8) format, 00-7F is compatible with ASCII
   */
  virtual void text_out(std::uint16_t ch)
  {
    if ((ch < 0x20U) && (ch > 0x07U)) {
      // ignore control chars, 00h..07h are the user defined CGRAM characters
      return;
    }

    if (screen_is_inside(cur_pos_)) {
      // set the address counter if the controller is not already there
      if (ac_pos_ != cur_pos_) {
        set_address(cur_pos_);
      }
      write_data(to_rom(ch));
      ac_pos_ = { static_cast<std::int16_t>(cur_pos_.x + 1), cur_pos_.y };
    }
    cur_pos_.x++;
  }


  /**
   * Set the new text position
   * \param pos Position in chars (0/0 is left/top), may be outside the screen
   */
  virtual void text_set_pos(vertex_type pos)
  {
    cur_pos_ = pos;
    if (screen_is_inside(pos)) {
      set_address(pos);
    }
  }


  /**
   * Clear the actual line from the cursor position (included) to the end of the line
   */
  virtual void text_clear_eol()
  {
    fill_spaces(cur_pos_.x, static_cast<std::int16_t>(Columns - 1U), cur_pos_.y);
  }


  /**
   * Clear the actual line from the start to the cursor position (included)
   */
  virtual void text_clear_sol()
  {
    fill_spaces(0, cur_pos_.x, cur_pos_.y);
  }


  /**
   * Clear the actual line
   */
  virtual void text_clear_line()
  {
    fill_spaces(0, static_cast<std::int16_t>(Columns - 1U), cur_pos_.y);
  }


  /////////////////////////////////////////////////////////////////////////////
  // D I S P L A Y   C O N T R O L
  //

public:

  /**
   * Enable/disable the display, the DDRAM content is retained
   * \param enable True to switch the display on
   */
  virtual void display_enable(bool enable = true)
  {
    display_ctrl_ = static_cast<std::uint8_t>(enable ? (display_ctrl_ | DISPLAY_ON) : (display_ctrl_ & ~DISPLAY_ON));
    write_cmd(static_cast<std::uint8_t>(CMD_DISPLAY | display_ctrl_));
  }


  /**
   * Cursor display
   * \param show True to show the underline cursor at the cursor position
   * \param blink True to blink the character at the cursor position
   */
  void cursor_set(bool show, bool blink = false)
  {
    display_ctrl_ = static_cast<std::uint8_t>((display_ctrl_ & DISPLAY_ON) | (show ? CURSOR_ON : 0U) | (blink ? BLINK_ON : 0U));
    write_cmd(static_cast<std::uint8_t>(CMD_DISPLAY | display_ctrl_));
  }


  /**
   * Define a user character in the CGRAM, it is displayed with the character codes 00h..07h
   * \param index Character index 0..7
   * \param pattern 8 rows of 5 bits (bit 4 is the left dot), row 7 is the cursor line and normally 0
   */
  void char_define(std::uint8_t index, const std::uint8_t* pattern)
  {
    write_cmd(static_cast<std::uint8_t>(CMD_CGRAM_ADDR | ((index & 0x07U) << 3U)));
    for (std::uint_fast8_t row = 0U; row < 8U; ++row) {
      write_data(static_cast<std::uint8_t>(pattern[row] & 0x1FU));
    }
    // back to the DDRAM
    ac_pos_ = { -1, -1 };
    if (screen_is_inside(cur_pos_)) {
      set_address(cur_pos_);
    }
  }


  /**
   * Shift the whole display by one character without changing the DDRAM, the logical positions of the
   * head don't follow the shift, cls() or display_home() restore them
   * \param right True to shift right, false to shift left
   */
  void display_shift(bool right)
  {
    write_cmd(static_cast<std::uint8_t>(CMD_SHIFT | SHIFT_DISPLAY | (right ? SHIFT_RIGHT : 0U)));
  }


  /**
   * Return home: undo the display shift, the cursor is at (0, 0) afterwards
   */
  void display_home()
  {
    write_cmd(CMD_HOME, true);
    cur_pos_ = { 0, 0 };
    ac_pos_  = { 0, 0 };
  }


private:

  /**
   * DDRAM address of a screen position
   * Lines 1 and 2 start at 00h and 40h, lines 3 and 4 of a 4 line module continue lines 1 and 2
   */
  static inline std::uint8_t ddram_address(vertex_type pos)
  {
    const std::uint8_t line = static_cast<std::uint8_t>(((pos.y & 1) ? 0x40U : 0x00U) + ((pos.y & 2) ? Columns : 0U));
    return static_cast<std::uint8_t>(line + pos.x);
  }


  /**
   * Set the address counter to a screen position (which must be inside the screen)
   */
  inline void set_address(vertex_type pos)
  {
    write_cmd(static_cast<std::uint8_t>(CMD_DDRAM_ADDR | ddram_address(pos)));
    ac_pos_ = pos;
  }


  /**
   * Write spaces from x0 to x1 (both included) in line y and put the address counter back to the cursor
   */
  void fill_spaces(std::int16_t x0, std::int16_t x1, std::int16_t y)
  {
    if ((y < 0) || (y >= static_cast<std::int16_t>(Lines))) {
      return;
    }
    if (x0 < 0) {
      x0 = 0;
    }
    if (x1 >= static_cast<std::int16_t>(Columns)) {
      x1 = static_cast<std::int16_t>(Columns - 1U);
    }
    if (x0 > x1) {
      return;
    }
    const vertex_type start = { x0, y };
    if (ac_pos_ != start) {
      set_address(start);
    }
    for (std::int_fast16_t x = x0; x <= x1; ++x) {
      write_data(0x20U);
    }
    ac_pos_ = { static_cast<std::int16_t>(x1 + 1), y };
    if (screen_is_inside(cur_pos_) && (ac_pos_ != cur_pos_)) {
      set_address(cur_pos_);
    }
  }


  /**
   * Map a UNICODE character to the character code of the ROM
   * 00h..07h are the CGRAM characters, 20h..7Fh are ASCII (A00: 5Ch is yen, 7Eh and 7Fh are arrows)
   */
  static std::uint8_t to_rom(std::uint16_t ch)
  {
    if (ch < 0x80U) {
      return static_cast<std::uint8_t>(ch);
    }
    if (Rom == HD44780_ROM_A02) {
      // ISO 8859-1 block
      if (ch <= 0xFFU) {
        return static_cast<std::uint8_t>(ch);
      }
    }
    else {
      // A00: half width Katakana U+FF61..U+FF9F are linear at A1h..DFh, a few Latin symbols exist
      if ((ch >= 0xFF61U) && (ch <= 0xFF9FU)) {
        return static_cast<std::uint8_t>(ch - 0xFF61U + 0xA1U);
      }
      switch (ch) {
        case 0x00A5U : return 0x5CU;    // yen
        case 0x00B0U : return 0xDFU;    // degree
        case 0x00B7U : return 0xA5U;    // middle dot
        case 0x00DFU : return 0xE2U;    // sharp s
        case 0x00E4U : return 0xE1U;    // a umlaut
        case 0x00F6U : return 0xEFU;    // o umlaut
        case 0x00F7U : return 0xFDU;    // division sign
        case 0x00FCU : return 0xF5U;    // u umlaut
        case 0x2190U : return 0x7FU;    // left arrow
        case 0x2192U : return 0x7EU;    // right arrow
        default      : break;
      }
    }
    return static_cast<std::uint8_t>('?');
  }


  /**
   * Write an instruction and wait for the controller
   * \param cmd Instruction byte
   * \param slow True for clear / home (1.52 ms), false for the 37 us instructions
   */
  inline void write_cmd(std::uint8_t cmd, bool slow = false)
  {
    io::write(device_handle_, OPT_INSTRUCTION | OPT_EXEC_US, &cmd, 1U, nullptr, 0U);
    wait(slow);
  }


  /**
   * Write a data byte (DDRAM / CGRAM) and wait for the controller
   */
  inline void write_data(std::uint8_t data)
  {
    io::write(device_handle_, OPT_DATA | OPT_EXEC_US, &data, 1U, nullptr, 0U);
    wait(false);
  }


  /**
   * Wait until the controller accepts the next byte, according to the Wait strategy
   */
  void wait(bool slow)
  {
    switch (Wait) {
      case HD44780_WAIT_BUSY :
        for (std::uint_fast16_t n = 0U; n < BUSY_POLLS; ++n) {
          std::uint8_t status = 0U;
          std::size_t  len    = 1U;
          if (!io::read(device_handle_, OPT_INSTRUCTION, &status, len, 0U) || (len != 1U)) {
            // the io layer can't read the busy flag, fall back to the delay
            io::delay(slow ? 2U : 1U);
            return;
          }
          if (!(status & BUSY_FLAG)) {
            return;
          }
        }
        break;
      case HD44780_WAIT_IO :
        // the io layer waits for the short instructions
        if (slow) {
          io::delay(2U);
        }
        break;
      case HD44780_WAIT_DELAY :
      default :
        io::delay(slow ? 2U : 1U);
        break;
    }
  }

};

} // namespace head
} // namespace vic

#endif  // _VIC_HEAD_HD44780_H_
