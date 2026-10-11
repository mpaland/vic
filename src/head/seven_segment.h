///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2020, PALANDesign Hannover, Germany
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
// \brief Generic seven segment head
// An alpha numeric head with one row of 7-segment digits. The segment patterns
// are kept in a buffer and written to the device with present(), one byte per
// digit in the common 'dp g f e d c b a' bit order (bit 0 = segment a, bit 7 = dp).
// Use it with any serial segment driver which accepts raw segment data.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_SEVEN_SEGMENT_H_
#define _VIC_HEAD_SEVEN_SEGMENT_H_

#include "../drv.h"


// defines the head name and version
#define VIC_HEAD_SEVEN_SEGMENT_VERSION   "Seven segment head 1.00"


namespace vic {
namespace head {


/**
 * 7-segment head
 * \param Columns Number of columns (digits)
 */
template<std::uint16_t Columns>
class seven_segment : public drv
{
  const io::handle_type device_handle_;     // logical device handle
  std::uint8_t          segments_[Columns]; // segment buffer, one byte per digit
  vertex_type           pos_;               // cursor position

public:

  // segment bits
  static const std::uint8_t SEG_A  = 0x01U;
  static const std::uint8_t SEG_B  = 0x02U;
  static const std::uint8_t SEG_C  = 0x04U;
  static const std::uint8_t SEG_D  = 0x08U;
  static const std::uint8_t SEG_E  = 0x10U;
  static const std::uint8_t SEG_F  = 0x20U;
  static const std::uint8_t SEG_G  = 0x40U;
  static const std::uint8_t SEG_DP = 0x80U;


  /**
   * ctor
   * \param device_handle Logical device handle of the segment driver interface
   */
  seven_segment(io::handle_type device_handle)
    : drv(Columns, 1U,
          Columns, 1U)
    , device_handle_(device_handle)
    , pos_({ 0, 0 })
  {
    for (std::uint16_t i = 0U; i < Columns; ++i) {
      segments_[i] = 0U;
    }
  }


  /**
   * dtor
   */
  ~seven_segment()
  {
    shutdown();
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S

  virtual void init()
  {
    io::init(device_handle_);
    cls();
    present();
  }


  virtual void shutdown()
  {
    cls();
    present();
  }


  virtual inline const char* version() const
  {
    return (const char*)VIC_HEAD_SEVEN_SEGMENT_VERSION;
  }


  virtual inline bool is_graphic() const
  {
    // alpha numeric display
    return false;
  }


  /**
   * Get the segment pattern of a digit
   * \param column The digit column
   * \return Segment pattern, bit 0 = a ... bit 6 = g, bit 7 = dp
   */
  inline std::uint8_t segments(std::uint16_t column) const
  {
    return column < Columns ? segments_[column] : 0U;
  }


  /**
   * Get the segment pattern of a character
   * \param ch Character
   * \param valid Set to true if the character is displayable
   * \return Segment pattern
   */
  static std::uint8_t char_to_segments(std::uint16_t ch, bool& valid)
  {
    // digits 0 - 9, A - F, encoding is dp g f e d c b a
    static const std::uint8_t digit[16] = { 0x3FU, 0x06U, 0x5BU, 0x4FU, 0x66U, 0x6DU, 0x7DU, 0x07U,
                                            0x7FU, 0x6FU, 0x77U, 0x7CU, 0x39U, 0x5EU, 0x79U, 0x71U };
    valid = true;
    if ((ch >= '0') && (ch <= '9')) {
      return digit[ch - '0'];
    }
    if ((ch >= 'A') && (ch <= 'F')) {
      return digit[ch - 'A' + 10U];
    }
    if ((ch >= 'a') && (ch <= 'f')) {
      return digit[ch - 'a' + 10U];
    }
    switch (ch) {
      case ' ' : return 0x00U;
      case '-' : return SEG_G;
      case '_' : return SEG_D;
      case 'o' : return static_cast<std::uint8_t>(SEG_C | SEG_D | SEG_E | SEG_G);
      case 'r' : return static_cast<std::uint8_t>(SEG_E | SEG_G);
      case 'H' : return static_cast<std::uint8_t>(SEG_B | SEG_C | SEG_E | SEG_F | SEG_G);
      case 'L' : return static_cast<std::uint8_t>(SEG_D | SEG_E | SEG_F);
      case 'P' : return static_cast<std::uint8_t>(SEG_A | SEG_B | SEG_E | SEG_F | SEG_G);
      case 'U' : return static_cast<std::uint8_t>(SEG_B | SEG_C | SEG_D | SEG_E | SEG_F);
      default  : break;
    }
    valid = false;
    return 0x00U;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

protected:

  virtual void cls(color::value_type bg_color = color::none)
  {
    (void)bg_color;
    for (std::uint16_t i = 0U; i < Columns; ++i) {
      segments_[i] = 0U;
    }
    pos_ = { 0, 0 };
  }


  virtual void present()
  {
    // write all segments to the device
    io::write(device_handle_, 0U, segments_, Columns, nullptr, 0U);
  }


  ///////////////////////////////////////////////////////////////////////////////
  // A L P H A   T E X T   F U N C T I O N S
  //

  /**
   * Output a single ASCII/UNICODE char at the actual cursor position
   * A '.' sets the decimal point of the previous digit and doesn't move the cursor,
   * unknown chars are ignored
   * \param ch Output character in 16 bit ASCII/UNICODE (NOT UTF-8) format, 00-7F is compatible with ASCII
   */
  virtual void text_out(std::uint16_t ch)
  {
    if (ch < 0x20U) {
      // ignore non characters
      return;
    }

    if (ch == '.') {
      // decimal point of the previous digit
      const std::int16_t col = static_cast<std::int16_t>(pos_.x - 1);
      if (screen_is_inside({ col, pos_.y })) {
        segments_[col] |= SEG_DP;
      }
      return;
    }

    bool valid = false;
    const std::uint8_t seg = char_to_segments(ch, valid);
    if (!valid) {
      return;
    }
    if (screen_is_inside(pos_)) {
      segments_[pos_.x] = seg;
    }
    // always advance the cursor
    pos_.x++;
  }


  virtual void text_set_pos(vertex_type pos)
  {
    pos_ = pos;
  }


  virtual void text_clear_eol()
  {
    for (std::int16_t x = pos_.x < 0 ? 0 : pos_.x; x < static_cast<std::int16_t>(Columns); ++x) {
      segments_[x] = 0U;
    }
  }


  virtual void text_clear_sol()
  {
    for (std::int16_t x = 0; (x <= pos_.x) && (x < static_cast<std::int16_t>(Columns)); ++x) {
      segments_[x] = 0U;
    }
  }


  virtual void text_clear_line()
  {
    for (std::uint16_t i = 0U; i < Columns; ++i) {
      segments_[i] = 0U;
    }
  }
};

} // namespace head
} // namespace vic

#endif  // _VIC_HEAD_SEVEN_SEGMENT_H_
