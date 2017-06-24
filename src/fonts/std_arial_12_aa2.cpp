///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2001-2014, PALANDesign Hannover, Germany
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
// \brief Standard font, Arial like, 12 pixel, proportional, UNICODE
//
///////////////////////////////////////////////////////////////////////////////

#include "vic_fonts.h"


namespace vic {

const std::uint8_t font_std_arial_12_aa2_0020[  1] = { /* code 0020, SPACE */
  0x00
};

const std::uint8_t font_std_arial_12_aa2_0021[  7] = { /* code 0021, EXCLAMATION MARK */
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0x00,
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_0022[  2] = { /* code 0022, QUOTATION MARK */
  0xCC,
  0xC8
};

const std::uint8_t font_std_arial_12_aa2_0023[  7] = { /* code 0023, NUMBER SIGN */
  0x33,
  0x33,
  0xFF,
  0x55,
  0xFF,
  0xCC,
  0xCC
};

const std::uint8_t font_std_arial_12_aa2_0024[ 16] = { /* code 0024, DOLLAR SIGN */
  0x7F, 0x40,
  0xDD, 0xC0,
  0xEC, 0x00,
  0x1F, 0x40,
  0x0C, 0xC0,
  0xDD, 0xC0,
  0x7F, 0x40,
  0x0C, 0x00
};

const std::uint8_t font_std_arial_12_aa2_0025[ 14] = { /* code 0025, PERCENT SIGN */
  0x74, 0xC0,
  0xFE, 0x40,
  0x77, 0x00,
  0x03, 0x00,
  0x0A, 0xD0,
  0x0F, 0xF0,
  0x19, 0xD0
};

const std::uint8_t font_std_arial_12_aa2_0026[ 14] = { /* code 0026, AMPERSAND */
  0x1F, 0x40,
  0x30, 0xC0,
  0x26, 0x80,
  0x2D, 0x00,
  0xC3, 0x60,
  0xD2, 0xC0,
  0x7E, 0xA0
};

const std::uint8_t font_std_arial_12_aa2_0027[  2] = { /* code 0027, APOSTROPHE */
  0xC0,
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_0028[  9] = { /* code 0028, LEFT PARENTHESIS */
  0x18,
  0x30,
  0x90,
  0xC0,
  0xC0,
  0xC0,
  0x90,
  0x30,
  0x18
};

const std::uint8_t font_std_arial_12_aa2_0029[  9] = { /* code 0029, RIGHT PARENTHESIS */
  0x90,
  0x30,
  0x18,
  0x0C,
  0x0C,
  0x0C,
  0x18,
  0x30,
  0x90
};

const std::uint8_t font_std_arial_12_aa2_002A[  3] = { /* code 002A, ASTERISK */
  0xB8,
  0x74,
  0x44
};

const std::uint8_t font_std_arial_12_aa2_002B[ 10] = { /* code 002B, PLUS SIGN */
  0x0C, 0x00,
  0x0C, 0x00,
  0xFF, 0xC0,
  0x0C, 0x00,
  0x0C, 0x00
};

const std::uint8_t font_std_arial_12_aa2_002C[  2] = { /* code 002C, COMMA */
  0xC0,
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_002D[  1] = { /* code 002D, HYPHEN-MINUS */
  0xF0
};

const std::uint8_t font_std_arial_12_aa2_002E[  1] = { /* code 002E, FULL STOP */
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_002F[  7] = { /* code 002F, SOLIDUS */
  0x0C,
  0x18,
  0x24,
  0x30,
  0x60,
  0x90,
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_0030[  7] = { /* code 0030, DIGIT ZERO */
  0x7C,
  0xC3,
  0xC3,
  0xC3,
  0xC3,
  0xC3,
  0x7D
};

const std::uint8_t font_std_arial_12_aa2_0031[  7] = { /* code 0031, DIGIT ONE */
  0x30,
  0xB0,
  0x30,
  0x30,
  0x30,
  0x30,
  0x30
};

const std::uint8_t font_std_arial_12_aa2_0032[  7] = { /* code 0032, DIGIT TWO */
  0x7D,
  0xC3,
  0x03,
  0x05,
  0x08,
  0x60,
  0xFF
};

const std::uint8_t font_std_arial_12_aa2_0033[  7] = { /* code 0033, DIGIT THREE */
  0x7D,
  0xC3,
  0x03,
  0x1E,
  0x03,
  0xC3,
  0x7D
};

const std::uint8_t font_std_arial_12_aa2_0034[ 14] = { /* code 0034, DIGIT FOUR */
  0x07, 0x00,
  0x0F, 0x00,
  0x33, 0x00,
  0x93, 0x00,
  0xFF, 0xC0,
  0x03, 0x00,
  0x03, 0x00
};

const std::uint8_t font_std_arial_12_aa2_0035[  7] = { /* code 0035, DIGIT FIVE */
  0x3F,
  0x60,
  0xBD,
  0xC3,
  0x03,
  0xC2,
  0x7C
};

const std::uint8_t font_std_arial_12_aa2_0036[  7] = { /* code 0036, DIGIT SIX */
  0x3D,
  0x83,
  0xED,
  0xD3,
  0xC3,
  0xC3,
  0x3D
};

const std::uint8_t font_std_arial_12_aa2_0037[  7] = { /* code 0037, DIGIT SEVEN */
  0xFF,
  0x06,
  0x0C,
  0x18,
  0x24,
  0x30,
  0x30
};

const std::uint8_t font_std_arial_12_aa2_0038[  7] = { /* code 0038, DIGIT EIGHT */
  0x7D,
  0xC3,
  0xC3,
  0x7D,
  0xC3,
  0xC3,
  0x7D
};

const std::uint8_t font_std_arial_12_aa2_0039[  7] = { /* code 0039, DIGIT NINE */
  0x7C,
  0xC3,
  0xC3,
  0xC7,
  0x7B,
  0xC2,
  0x7C
};

const std::uint8_t font_std_arial_12_aa2_003A[  5] = { /* code 003A, COLON */
  0xC0,
  0x00,
  0x00,
  0x00,
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_003B[  6] = { /* code 003B, SEMICOLON */
  0xC0,
  0x00,
  0x00,
  0x00,
  0xC0,
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_003C[  5] = { /* code 003C, LESS-THAN SIGN */
  0x08,
  0x74,
  0xD0,
  0x74,
  0x08
};

const std::uint8_t font_std_arial_12_aa2_003D[  3] = { /* code 003D, EQUALS SIGN */
  0xFF,
  0x00,
  0xFF
};

const std::uint8_t font_std_arial_12_aa2_003E[  5] = { /* code 003E, GREATER-THAN SIGN */
  0x80,
  0x74,
  0x1C,
  0x74,
  0x80
};

const std::uint8_t font_std_arial_12_aa2_003F[ 14] = { /* code 003F, QUESTION MARK */
  0x3F, 0x40,
  0xD1, 0xC0,
  0x01, 0xC0,
  0x06, 0x00,
  0x0C, 0x00,
  0x00, 0x00,
  0x0C, 0x00
};

const std::uint8_t font_std_arial_12_aa2_0040[ 18] = { /* code 0040, COMMERCIAL AT */
  0x0B, 0xF4,
  0x34, 0x1D,
  0x57, 0xB7,
  0xCD, 0xE3,
  0xCC, 0xD3,
  0xCD, 0xCD,
  0x9B, 0xF4,
  0x34, 0x0A,
  0x0B, 0xF4
};

const std::uint8_t font_std_arial_12_aa2_0041[ 14] = { /* code 0041, LATIN CAPITAL LETTER A */
  0x07, 0x40,
  0x0A, 0x80,
  0x0C, 0xC0,
  0x24, 0x60,
  0x3F, 0xF0,
  0x60, 0x24,
  0xC0, 0x0C
};

const std::uint8_t font_std_arial_12_aa2_0042[  7] = { /* code 0042, LATIN CAPITAL LETTER B */
  0xFC,
  0xC3,
  0xC2,
  0xFE,
  0xC3,
  0xC3,
  0xFD
};

const std::uint8_t font_std_arial_12_aa2_0043[ 14] = { /* code 0043, LATIN CAPITAL LETTER C */
  0x2F, 0x00,
  0x91, 0xC0,
  0xC0, 0x00,
  0xC0, 0x00,
  0xC0, 0x40,
  0x91, 0x80,
  0x2F, 0x00
};

const std::uint8_t font_std_arial_12_aa2_0044[ 14] = { /* code 0044, LATIN CAPITAL LETTER D */
  0xFF, 0x40,
  0xC1, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC1, 0xC0,
  0xFF, 0x40
};

const std::uint8_t font_std_arial_12_aa2_0045[ 14] = { /* code 0045, LATIN CAPITAL LETTER E */
  0xFF, 0xC0,
  0xC0, 0x00,
  0xC0, 0x00,
  0xFF, 0xC0,
  0xC0, 0x00,
  0xC0, 0x00,
  0xFF, 0xC0
};

const std::uint8_t font_std_arial_12_aa2_0046[  7] = { /* code 0046, LATIN CAPITAL LETTER F */
  0xFF,
  0xC0,
  0xC0,
  0xFC,
  0xC0,
  0xC0,
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_0047[ 14] = { /* code 0047, LATIN CAPITAL LETTER G */
  0x7F, 0x00,
  0xD1, 0xC0,
  0xC0, 0x00,
  0xC3, 0xC0,
  0xC0, 0xC0,
  0xD1, 0xC0,
  0x7F, 0x40
};

const std::uint8_t font_std_arial_12_aa2_0048[ 14] = { /* code 0048, LATIN CAPITAL LETTER H */
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xFF, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0
};

const std::uint8_t font_std_arial_12_aa2_0049[  7] = { /* code 0049, LATIN CAPITAL LETTER I */
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_004A[  7] = { /* code 004A, LATIN CAPITAL LETTER J */
  0x03,
  0x03,
  0x03,
  0x03,
  0x03,
  0xC3,
  0x7D
};

const std::uint8_t font_std_arial_12_aa2_004B[ 14] = { /* code 004B, LATIN CAPITAL LETTER K */
  0xC1, 0x80,
  0xC2, 0x00,
  0xCC, 0x00,
  0xE8, 0x00,
  0xDC, 0x00,
  0xC3, 0x00,
  0xC1, 0x80
};

const std::uint8_t font_std_arial_12_aa2_004C[  7] = { /* code 004C, LATIN CAPITAL LETTER L */
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xFF
};

const std::uint8_t font_std_arial_12_aa2_004D[ 14] = { /* code 004D, LATIN CAPITAL LETTER M */
  0xC0, 0x0C,
  0xD0, 0x1C,
  0xE0, 0x2C,
  0xD8, 0x9C,
  0xCD, 0xCC,
  0xCB, 0x8C,
  0xC7, 0x4C
};

const std::uint8_t font_std_arial_12_aa2_004E[ 14] = { /* code 004E, LATIN CAPITAL LETTER N */
  0xD0, 0xC0,
  0xF0, 0xC0,
  0xE4, 0xC0,
  0xCC, 0xC0,
  0xC6, 0xC0,
  0xC3, 0xC0,
  0xC1, 0xC0
};

const std::uint8_t font_std_arial_12_aa2_004F[ 14] = { /* code 004F, LATIN CAPITAL LETTER O */
  0x2E, 0x00,
  0x91, 0x80,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0x91, 0x80,
  0x2E, 0x00
};

const std::uint8_t font_std_arial_12_aa2_0050[  7] = { /* code 0050, LATIN CAPITAL LETTER P */
  0xFD,
  0xC3,
  0xC3,
  0xFD,
  0xC0,
  0xC0,
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_0051[ 16] = { /* code 0051, LATIN CAPITAL LETTER Q */
  0x2E, 0x00,
  0x91, 0x80,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0x96, 0x40,
  0x3F, 0xC0,
  0x00, 0x00
};

const std::uint8_t font_std_arial_12_aa2_0052[ 14] = { /* code 0052, LATIN CAPITAL LETTER R */
  0xFF, 0x40,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xFF, 0x40,
  0xC2, 0x00,
  0xC1, 0x80,
  0xC0, 0xC0
};

const std::uint8_t font_std_arial_12_aa2_0053[  7] = { /* code 0053, LATIN CAPITAL LETTER S */
  0x7D,
  0xC3,
  0xD0,
  0x2D,
  0x03,
  0xC3,
  0x7D
};

const std::uint8_t font_std_arial_12_aa2_0054[ 14] = { /* code 0054, LATIN CAPITAL LETTER T */
  0xFF, 0xC0,
  0x0C, 0x00,
  0x0C, 0x00,
  0x0C, 0x00,
  0x0C, 0x00,
  0x0C, 0x00,
  0x0C, 0x00
};

const std::uint8_t font_std_arial_12_aa2_0055[ 14] = { /* code 0055, LATIN CAPITAL LETTER U */
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xD1, 0xC0,
  0x3F, 0x00
};

const std::uint8_t font_std_arial_12_aa2_0056[ 21] = { /* code 0056, LATIN CAPITAL LETTER V */
  0x20, 0x02, 0x00,
  0x20, 0x02, 0x00,
  0x08, 0x08, 0x00,
  0x08, 0x08, 0x00,
  0x06, 0x24, 0x00,
  0x03, 0x70, 0x00,
  0x01, 0xD0, 0x00
};

const std::uint8_t font_std_arial_12_aa2_0057[ 21] = { /* code 0057, LATIN CAPITAL LETTER W */
  0xC1, 0xD0, 0xC0,
  0x92, 0xA1, 0x80,
  0x63, 0x72, 0x40,
  0x33, 0x33, 0x00,
  0x37, 0x27, 0x00,
  0x2E, 0x2E, 0x00,
  0x1D, 0x1D, 0x00
};

const std::uint8_t font_std_arial_12_aa2_0058[ 14] = { /* code 0058, LATIN CAPITAL LETTER X */
  0x91, 0x80,
  0x33, 0x00,
  0x2E, 0x00,
  0x0C, 0x00,
  0x2E, 0x00,
  0x33, 0x00,
  0x91, 0x80
};

const std::uint8_t font_std_arial_12_aa2_0059[ 14] = { /* code 0059, LATIN CAPITAL LETTER Y */
  0x80, 0x80,
  0x22, 0x00,
  0x26, 0x00,
  0x0C, 0x00,
  0x0C, 0x00,
  0x0C, 0x00,
  0x0C, 0x00
};

const std::uint8_t font_std_arial_12_aa2_005A[ 14] = { /* code 005A, LATIN CAPITAL LETTER Z */
  0xFF, 0xF0,
  0x00, 0x50,
  0x02, 0x40,
  0x09, 0x00,
  0x24, 0x00,
  0x90, 0x00,
  0xFF, 0xF0
};

const std::uint8_t font_std_arial_12_aa2_005B[  9] = { /* code 005B, LEFT SQUARE BRACKET */
  0xF0,
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xF0
};

const std::uint8_t font_std_arial_12_aa2_005C[  7] = { /* code 005C, REVERSE SOLIDUS */
  0xC0,
  0x90,
  0x60,
  0x30,
  0x24,
  0x18,
  0x0C
};

const std::uint8_t font_std_arial_12_aa2_005D[  9] = { /* code 005D, RIGHT SQUARE BRACKET */
  0xF0,
  0x30,
  0x30,
  0x30,
  0x30,
  0x30,
  0x30,
  0x30,
  0xF0
};

const std::uint8_t font_std_arial_12_aa2_005E[  3] = { /* code 005E, CIRCUMFLEX ACCENT */
  0x30,
  0xA8,
  0xCC
};

const std::uint8_t font_std_arial_12_aa2_005F[  2] = { /* code 005F, LOW LINE */
  0xFF, 0xC0
};

const std::uint8_t font_std_arial_12_aa2_0060[  2] = { /* code 0060, GRAVE ACCENT */
  0x80,
  0x20
};

const std::uint8_t font_std_arial_12_aa2_0061[  5] = { /* code 0061, LATIN SMALL LETTER A */
  0xBE,
  0x03,
  0x6F,
  0xD7,
  0xBB
};

const std::uint8_t font_std_arial_12_aa2_0062[  7] = { /* code 0062, LATIN SMALL LETTER B */
  0xC0,
  0xC0,
  0xED,
  0xD7,
  0xC3,
  0xD7,
  0xED
};

const std::uint8_t font_std_arial_12_aa2_0063[  5] = { /* code 0063, LATIN SMALL LETTER C */
  0x7D,
  0xD3,
  0xC0,
  0xD3,
  0x7D
};

const std::uint8_t font_std_arial_12_aa2_0064[  7] = { /* code 0064, LATIN SMALL LETTER D */
  0x03,
  0x03,
  0x7B,
  0xD7,
  0xC3,
  0xD7,
  0x7B
};

const std::uint8_t font_std_arial_12_aa2_0065[  5] = { /* code 0065, LATIN SMALL LETTER E */
  0x7C,
  0xC3,
  0xFF,
  0xC0,
  0x7E
};

const std::uint8_t font_std_arial_12_aa2_0066[  7] = { /* code 0066, LATIN SMALL LETTER F */
  0x1C,
  0x34,
  0xFC,
  0x30,
  0x30,
  0x30,
  0x30
};

const std::uint8_t font_std_arial_12_aa2_0067[  7] = { /* code 0067, LATIN SMALL LETTER G */
  0x7B,
  0xD7,
  0xC3,
  0xD7,
  0x7B,
  0xC7,
  0x7D
};

const std::uint8_t font_std_arial_12_aa2_0068[  7] = { /* code 0068, LATIN SMALL LETTER H */
  0xC0,
  0xC0,
  0xED,
  0xD7,
  0xC3,
  0xC3,
  0xC3
};

const std::uint8_t font_std_arial_12_aa2_0069[  7] = { /* code 0069, LATIN SMALL LETTER I */
  0xC0,
  0x00,
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_006A[  9] = { /* code 006A, LATIN SMALL LETTER J */
  0x30,
  0x00,
  0x30,
  0x30,
  0x30,
  0x30,
  0x30,
  0x70,
  0x90
};

const std::uint8_t font_std_arial_12_aa2_006B[  7] = { /* code 006B, LATIN SMALL LETTER K */
  0xC0,
  0xC0,
  0xC6,
  0xD8,
  0xFC,
  0xC9,
  0xC2
};

const std::uint8_t font_std_arial_12_aa2_006C[  7] = { /* code 006C, LATIN SMALL LETTER L */
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_006D[ 10] = { /* code 006D, LATIN SMALL LETTER M */
  0xED, 0xB4,
  0xD3, 0x5C,
  0xC3, 0x0C,
  0xC3, 0x0C,
  0xC3, 0x0C
};

const std::uint8_t font_std_arial_12_aa2_006E[  5] = { /* code 006E, LATIN SMALL LETTER N */
  0xED,
  0xD7,
  0xC3,
  0xC3,
  0xC3
};

const std::uint8_t font_std_arial_12_aa2_006F[  5] = { /* code 006F, LATIN SMALL LETTER O */
  0x7D,
  0xD7,
  0xC3,
  0xD7,
  0x7D
};

const std::uint8_t font_std_arial_12_aa2_0070[  7] = { /* code 0070, LATIN SMALL LETTER P */
  0xED,
  0xD7,
  0xC3,
  0xD7,
  0xED,
  0xC0,
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_0071[  7] = { /* code 0071, LATIN SMALL LETTER Q */
  0x7B,
  0xD7,
  0xC3,
  0xD7,
  0x7B,
  0x03,
  0x03
};

const std::uint8_t font_std_arial_12_aa2_0072[  5] = { /* code 0072, LATIN SMALL LETTER R */
  0xEC,
  0xD0,
  0xC0,
  0xC0,
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_0073[  5] = { /* code 0073, LATIN SMALL LETTER S */
  0x7E,
  0xE0,
  0x3C,
  0x0B,
  0xBD
};

const std::uint8_t font_std_arial_12_aa2_0074[  7] = { /* code 0074, LATIN SMALL LETTER T */
  0x10,
  0x30,
  0xFC,
  0x30,
  0x30,
  0x30,
  0x3C
};

const std::uint8_t font_std_arial_12_aa2_0075[  5] = { /* code 0075, LATIN SMALL LETTER U */
  0xC3,
  0xC3,
  0xC3,
  0xD7,
  0x7B
};

const std::uint8_t font_std_arial_12_aa2_0076[ 10] = { /* code 0076, LATIN SMALL LETTER V */
  0xC0, 0xC0,
  0x62, 0x40,
  0x33, 0x00,
  0x2A, 0x00,
  0x1D, 0x00
};

const std::uint8_t font_std_arial_12_aa2_0077[ 10] = { /* code 0077, LATIN SMALL LETTER W */
  0xDD, 0xC0,
  0xEE, 0xC0,
  0xAE, 0x80,
  0xBF, 0x80,
  0x7B, 0x40
};

const std::uint8_t font_std_arial_12_aa2_0078[  5] = { /* code 0078, LATIN SMALL LETTER X */
  0x96,
  0x3C,
  0x28,
  0x3C,
  0x96
};

const std::uint8_t font_std_arial_12_aa2_0079[ 14] = { /* code 0079, LATIN SMALL LETTER Y */
  0xC0, 0xC0,
  0x62, 0x40,
  0x33, 0x00,
  0x2E, 0x00,
  0x0C, 0x00,
  0x0C, 0x00,
  0x34, 0x00
};

const std::uint8_t font_std_arial_12_aa2_007A[  5] = { /* code 007A, LATIN SMALL LETTER Z */
  0xFC,
  0x18,
  0x30,
  0x90,
  0xFC
};

const std::uint8_t font_std_arial_12_aa2_007B[  9] = { /* code 007B, LEFT CURLY BRACKET */
  0x2C,
  0x30,
  0x30,
  0x30,
  0xD0,
  0x30,
  0x30,
  0x30,
  0x2C
};

const std::uint8_t font_std_arial_12_aa2_007C[  8] = { /* code 007C, VERTICAL LINE */
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_007D[  9] = { /* code 007D, RIGHT CURLY BRACKET */
  0xE0,
  0x30,
  0x30,
  0x30,
  0x1C,
  0x30,
  0x30,
  0x30,
  0xE0
};

const std::uint8_t font_std_arial_12_aa2_007E[  3] = { /* code 007E, TILDE */
  0x00,
  0xB2,
  0x4E
};

/* Start of unicode area <Latin-1 Supplement> */
const std::uint8_t font_std_arial_12_aa2_00A3[ 14] = { /* code 00A3, POUND SIGN */
  0x1F, 0x40,
  0x30, 0xC0,
  0x30, 0x00,
  0xFC, 0x00,
  0x30, 0x00,
  0x60, 0x40,
  0xBF, 0x80
};

const std::uint8_t font_std_arial_12_aa2_00A4[  4] = { /* code 00A4, CURRENCY SIGN */
  0xFF,
  0xC3,
  0xC3,
  0xFF
};

const std::uint8_t font_std_arial_12_aa2_00A5[ 14] = { /* code 00A5, YEN SIGN */
  0x80, 0x80,
  0x22, 0x00,
  0x26, 0x00,
  0xFF, 0xC0,
  0x0C, 0x00,
  0xFF, 0xC0,
  0x0C, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00A6[  8] = { /* code 00A6, BROKEN BAR */
  0xC0,
  0xC0,
  0xC0,
  0x00,
  0x00,
  0xC0,
  0xC0,
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_00A7[  9] = { /* code 00A7, SECTION SIGN */
  0x7D,
  0xC3,
  0x60,
  0xD8,
  0xC3,
  0x26,
  0x09,
  0xC3,
  0x7D
};

const std::uint8_t font_std_arial_12_aa2_00A8[  1] = { /* code 00A8, DIAERESIS */
  0xCC
};

const std::uint8_t font_std_arial_12_aa2_00A9[ 14] = { /* code 00A9, COPYRIGHT SIGN */
  0x7F, 0xF4,
  0xD0, 0x1C,
  0xC7, 0x8C,
  0xCE, 0x0C,
  0xC7, 0x8C,
  0xD0, 0x1C,
  0x7F, 0xF4
};

const std::uint8_t font_std_arial_12_aa2_00AA[  3] = { /* code 00AA, FEMININE ORDINAL INDICATOR */
  0xBC,
  0x5C,
  0xFC
};

const std::uint8_t font_std_arial_12_aa2_00B0[  3] = { /* code 00B0, DEGREE SIGN */
  0xB8,
  0xCC,
  0xB8
};

const std::uint8_t font_std_arial_12_aa2_00B1[ 12] = { /* code 00B1, PLUS-MINUS SIGN */
  0x0C, 0x00,
  0x0C, 0x00,
  0xFF, 0xC0,
  0x0C, 0x00,
  0x0C, 0x00,
  0xFF, 0xC0
};

const std::uint8_t font_std_arial_12_aa2_00B2[  3] = { /* code 00B2, SUPERSCRIPT TWO */
  0xBC,
  0x04,
  0xBC
};

const std::uint8_t font_std_arial_12_aa2_00B3[  3] = { /* code 00B3, SUPERSCRIPT THREE */
  0xBC,
  0x38,
  0xBC
};

const std::uint8_t font_std_arial_12_aa2_00B4[  2] = { /* code 00B4, ACUTE ACCENT */
  0x20,
  0x80
};

const std::uint8_t font_std_arial_12_aa2_00B5[  7] = { /* code 00B5, MICRO SIGN */
  0xC3,
  0xC3,
  0xC3,
  0xC7,
  0xFB,
  0xC0,
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_00B6[ 16] = { /* code 00B6, PILCROW SIGN */
  0x7F, 0xC0,
  0xF3, 0x00,
  0xF3, 0x00,
  0xB3, 0x00,
  0x33, 0x00,
  0x33, 0x00,
  0x33, 0x00,
  0x33, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00B7[  1] = { /* code 00B7, MIDDLE DOT */
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_00B8[  3] = { /* code 00B8, CEDILLA */
  0x30,
  0x30,
  0xE0
};

const std::uint8_t font_std_arial_12_aa2_00B9[  3] = { /* code 00B9, SUPERSCRIPT ONE */
  0x70,
  0xB0,
  0x30
};

const std::uint8_t font_std_arial_12_aa2_00BA[  3] = { /* code 00BA, MASCULINE ORDINAL INDICATOR */
  0xB8,
  0xCC,
  0xB8
};

const std::uint8_t font_std_arial_12_aa2_00BB[  8] = { /* code 00BB, RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
  0x99, 0x00,
  0x12, 0x40,
  0x12, 0x40,
  0x99, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00BC[ 14] = { /* code 00BC, VULGAR FRACTION ONE QUARTER */
  0x1C, 0x18,
  0x2C, 0x30,
  0x0C, 0x80,
  0x01, 0x40,
  0x02, 0x1C,
  0x0C, 0x3C,
  0x24, 0x0C
};

const std::uint8_t font_std_arial_12_aa2_00BD[ 14] = { /* code 00BD, VULGAR FRACTION ONE HALF */
  0x70, 0x30,
  0xB0, 0xC0,
  0x32, 0x40,
  0x06, 0x00,
  0x0C, 0xBC,
  0x30, 0x04,
  0x90, 0x7C
};

const std::uint8_t font_std_arial_12_aa2_00BE[ 14] = { /* code 00BE, VULGAR FRACTION THREE QUARTERS */
  0xBC, 0x0C,
  0x38, 0x20,
  0xBC, 0x50,
  0x00, 0x80,
  0x03, 0x0C,
  0x09, 0x3C,
  0x18, 0x0C
};

const std::uint8_t font_std_arial_12_aa2_00BF[  7] = { /* code 00BF, INVERTED QUESTION MARK */
  0x0C,
  0x00,
  0x0C,
  0x24,
  0xD0,
  0xC3,
  0x7D
};

const std::uint8_t font_std_arial_12_aa2_00C0[ 18] = { /* code 00C0, LATIN CAPITAL LETTER A WITH GRAVE */
  0x02, 0x00,
  0x00, 0x80,
  0x07, 0x40,
  0x0A, 0x80,
  0x0C, 0xC0,
  0x24, 0x60,
  0x3F, 0xF0,
  0x60, 0x24,
  0xC0, 0x0C
};

const std::uint8_t font_std_arial_12_aa2_00C1[ 18] = { /* code 00C1, LATIN CAPITAL LETTER A WITH ACUTE */
  0x00, 0x80,
  0x02, 0x00,
  0x07, 0x40,
  0x0A, 0x80,
  0x0C, 0xC0,
  0x24, 0x60,
  0x3F, 0xF0,
  0x60, 0x24,
  0xC0, 0x0C
};

const std::uint8_t font_std_arial_12_aa2_00C2[ 18] = { /* code 00C2, LATIN CAPITAL LETTER A WITH CIRCUMFLEX */
  0x07, 0x40,
  0x0D, 0x80,
  0x07, 0x40,
  0x0A, 0x80,
  0x0C, 0xC0,
  0x24, 0x60,
  0x3F, 0xF0,
  0x60, 0x24,
  0xC0, 0x0C
};

const std::uint8_t font_std_arial_12_aa2_00C3[ 18] = { /* code 00C3, LATIN CAPITAL LETTER A WITH TILDE */
  0x1D, 0xC0,
  0x37, 0x40,
  0x07, 0x40,
  0x0A, 0x80,
  0x0C, 0xC0,
  0x24, 0x60,
  0x3F, 0xF0,
  0x60, 0x24,
  0xC0, 0x0C
};

const std::uint8_t font_std_arial_12_aa2_00C4[ 16] = { /* code 00C4, LATIN CAPITAL LETTER A WITH DIAERESIS */
  0x0C, 0xC0,
  0x07, 0x40,
  0x0A, 0x80,
  0x0C, 0xC0,
  0x24, 0x60,
  0x3F, 0xF0,
  0x60, 0x24,
  0xC0, 0x0C
};

const std::uint8_t font_std_arial_12_aa2_00C5[ 18] = { /* code 00C5, LATIN CAPITAL LETTER A WITH RING ABOVE */
  0x0B, 0x80,
  0x0C, 0xC0,
  0x0B, 0x80,
  0x0A, 0x80,
  0x0C, 0xC0,
  0x24, 0x60,
  0x3F, 0xF0,
  0x60, 0x24,
  0xC0, 0x0C
};

const std::uint8_t font_std_arial_12_aa2_00C6[ 21] = { /* code 00C6, LATIN CAPITAL LETTER AE */
  0x01, 0xFF, 0xF0,
  0x03, 0x30, 0x00,
  0x06, 0x30, 0x00,
  0x0C, 0x3F, 0xF0,
  0x1F, 0xF0, 0x00,
  0x30, 0x30, 0x00,
  0x60, 0x3F, 0xF0
};

const std::uint8_t font_std_arial_12_aa2_00C7[ 20] = { /* code 00C7, LATIN CAPITAL LETTER C WITH CEDILLA */
  0x2F, 0x00,
  0x91, 0xC0,
  0xC0, 0x00,
  0xC0, 0x00,
  0xC0, 0x40,
  0x91, 0x80,
  0x2F, 0x00,
  0x03, 0x00,
  0x03, 0x00,
  0x0E, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00C8[ 18] = { /* code 00C8, LATIN CAPITAL LETTER E WITH GRAVE */
  0x08, 0x00,
  0x02, 0x00,
  0xFF, 0xC0,
  0xC0, 0x00,
  0xC0, 0x00,
  0xFF, 0xC0,
  0xC0, 0x00,
  0xC0, 0x00,
  0xFF, 0xC0
};

const std::uint8_t font_std_arial_12_aa2_00C9[ 18] = { /* code 00C9, LATIN CAPITAL LETTER E WITH ACUTE */
  0x02, 0x00,
  0x08, 0x00,
  0xFF, 0xC0,
  0xC0, 0x00,
  0xC0, 0x00,
  0xFF, 0xC0,
  0xC0, 0x00,
  0xC0, 0x00,
  0xFF, 0xC0
};

const std::uint8_t font_std_arial_12_aa2_00CA[ 18] = { /* code 00CA, LATIN CAPITAL LETTER E WITH CIRCUMFLEX */
  0x1D, 0x00,
  0x36, 0x00,
  0xFF, 0xC0,
  0xC0, 0x00,
  0xC0, 0x00,
  0xFF, 0xC0,
  0xC0, 0x00,
  0xC0, 0x00,
  0xFF, 0xC0
};

const std::uint8_t font_std_arial_12_aa2_00CB[ 16] = { /* code 00CB, LATIN CAPITAL LETTER E WITH DIAERESIS */
  0x33, 0x00,
  0xFF, 0xC0,
  0xC0, 0x00,
  0xC0, 0x00,
  0xFF, 0xC0,
  0xC0, 0x00,
  0xC0, 0x00,
  0xFF, 0xC0
};

const std::uint8_t font_std_arial_12_aa2_00CC[  9] = { /* code 00CC, LATIN CAPITAL LETTER I WITH GRAVE */
  0x80,
  0x20,
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_00CD[  9] = { /* code 00CD, LATIN CAPITAL LETTER I WITH ACUTE */
  0x20,
  0x80,
  0x30,
  0x30,
  0x30,
  0x30,
  0x30,
  0x30,
  0x30
};

const std::uint8_t font_std_arial_12_aa2_00CE[ 18] = { /* code 00CE, LATIN CAPITAL LETTER I WITH CIRCUMFLEX */
  0x1D, 0x00,
  0x36, 0x00,
  0x03, 0x00,
  0x03, 0x00,
  0x03, 0x00,
  0x03, 0x00,
  0x03, 0x00,
  0x03, 0x00,
  0x03, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00CF[  8] = { /* code 00CF, LATIN CAPITAL LETTER I WITH DIAERESIS */
  0xCC,
  0x30,
  0x30,
  0x30,
  0x30,
  0x30,
  0x30,
  0x30
};

const std::uint8_t font_std_arial_12_aa2_00D0[ 14] = { /* code 00D0, LATIN CAPITAL LETTER ETH */
  0x3F, 0xD0,
  0x30, 0x70,
  0x30, 0x30,
  0xFF, 0x30,
  0x30, 0x30,
  0x30, 0x70,
  0x3F, 0xD0
};

const std::uint8_t font_std_arial_12_aa2_00D1[ 18] = { /* code 00D1, LATIN CAPITAL LETTER N WITH TILDE */
  0x77, 0x00,
  0xDD, 0x00,
  0xD0, 0xC0,
  0xF0, 0xC0,
  0xE4, 0xC0,
  0xCC, 0xC0,
  0xC6, 0xC0,
  0xC3, 0xC0,
  0xC1, 0xC0
};

const std::uint8_t font_std_arial_12_aa2_00D2[ 18] = { /* code 00D2, LATIN CAPITAL LETTER O WITH GRAVE */
  0x08, 0x00,
  0x02, 0x00,
  0x2E, 0x00,
  0x91, 0x80,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0x91, 0x80,
  0x2E, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00D3[ 18] = { /* code 00D3, LATIN CAPITAL LETTER O WITH ACUTE */
  0x02, 0x00,
  0x08, 0x00,
  0x2E, 0x00,
  0x91, 0x80,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0x91, 0x80,
  0x2E, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00D4[ 18] = { /* code 00D4, LATIN CAPITAL LETTER O WITH CIRCUMFLEX */
  0x1D, 0x00,
  0x36, 0x00,
  0x2E, 0x00,
  0x91, 0x80,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0x91, 0x80,
  0x2E, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00D5[ 18] = { /* code 00D5, LATIN CAPITAL LETTER O WITH TILDE */
  0x77, 0x00,
  0xDD, 0x00,
  0x2E, 0x00,
  0x91, 0x80,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0x91, 0x80,
  0x2E, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00D6[ 16] = { /* code 00D6, LATIN CAPITAL LETTER O WITH DIAERESIS */
  0x33, 0x00,
  0x2E, 0x00,
  0x91, 0x80,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0x91, 0x80,
  0x2E, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00D7[ 10] = { /* code 00D7, MULTIPLICATION SIGN */
  0x91, 0xC0,
  0x7B, 0x00,
  0x2D, 0x00,
  0x7B, 0x00,
  0x91, 0xC0
};

const std::uint8_t font_std_arial_12_aa2_00D8[ 16] = { /* code 00D8, LATIN CAPITAL LETTER O WITH STROKE */
  0x3E, 0x40,
  0x93, 0x00,
  0xC6, 0xC0,
  0xCC, 0xC0,
  0xE4, 0xC0,
  0xB1, 0x80,
  0xBF, 0x00,
  0x00, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00D9[ 18] = { /* code 00D9, LATIN CAPITAL LETTER U WITH GRAVE */
  0x20, 0x00,
  0x08, 0x00,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xD1, 0xC0,
  0x3F, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00DA[ 18] = { /* code 00DA, LATIN CAPITAL LETTER U WITH ACUTE */
  0x08, 0x00,
  0x20, 0x00,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xD1, 0xC0,
  0x3F, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00DB[ 18] = { /* code 00DB, LATIN CAPITAL LETTER U WITH CIRCUMFLEX */
  0x1D, 0x00,
  0x36, 0x00,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xD1, 0xC0,
  0x3F, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00DC[ 16] = { /* code 00DC, LATIN CAPITAL LETTER U WITH DIAERESIS */
  0x33, 0x00,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xC0, 0xC0,
  0xD1, 0xC0,
  0x3F, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00DD[ 18] = { /* code 00DD, LATIN CAPITAL LETTER Y WITH ACUTE */
  0x02, 0x00,
  0x08, 0x00,
  0x80, 0x80,
  0x22, 0x00,
  0x26, 0x00,
  0x0C, 0x00,
  0x0C, 0x00,
  0x0C, 0x00,
  0x0C, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00DE[ 14] = { /* code 00DE, LATIN CAPITAL LETTER THORN */
  0xC0, 0x00,
  0xFF, 0x40,
  0xC1, 0xC0,
  0xC0, 0xC0,
  0xC1, 0xC0,
  0xFF, 0x40,
  0xC0, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00DF[  7] = { /* code 00DF, LATIN SMALL LETTER SHARP S */
  0x74,
  0xEC,
  0xD8,
  0xF0,
  0xCE,
  0xDB,
  0xDD
};

const std::uint8_t font_std_arial_12_aa2_00E0[  8] = { /* code 00E0, LATIN SMALL LETTER A WITH GRAVE */
  0x20,
  0x08,
  0x00,
  0xBE,
  0x03,
  0x6F,
  0xD7,
  0xBB
};

const std::uint8_t font_std_arial_12_aa2_00E1[  8] = { /* code 00E1, LATIN SMALL LETTER A WITH ACUTE */
  0x08,
  0x20,
  0x00,
  0xBE,
  0x03,
  0x6F,
  0xD7,
  0xBB
};

const std::uint8_t font_std_arial_12_aa2_00E2[ 16] = { /* code 00E2, LATIN SMALL LETTER A WITH CIRCUMFLEX */
  0x1D, 0x00,
  0x36, 0x00,
  0x00, 0x00,
  0xBE, 0x00,
  0x03, 0x00,
  0x6F, 0x00,
  0xD7, 0x00,
  0xBB, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00E3[  8] = { /* code 00E3, LATIN SMALL LETTER A WITH TILDE */
  0x77,
  0xDD,
  0x00,
  0xBE,
  0x03,
  0x6F,
  0xD7,
  0xBB
};

const std::uint8_t font_std_arial_12_aa2_00E4[  7] = { /* code 00E4, LATIN SMALL LETTER A WITH DIAERESIS */
  0x33,
  0x00,
  0xBE,
  0x03,
  0x6F,
  0xD7,
  0xBB
};

const std::uint8_t font_std_arial_12_aa2_00E5[  9] = { /* code 00E5, LATIN SMALL LETTER A WITH RING ABOVE */
  0x2E,
  0x33,
  0x2E,
  0x00,
  0xBE,
  0x03,
  0x6F,
  0xD7,
  0xBB
};

const std::uint8_t font_std_arial_12_aa2_00E6[ 10] = { /* code 00E6, LATIN SMALL LETTER AE */
  0xBE, 0xF0,
  0x03, 0x0C,
  0x6F, 0xFC,
  0xD7, 0x00,
  0xBD, 0xF8
};

const std::uint8_t font_std_arial_12_aa2_00E7[  8] = { /* code 00E7, LATIN SMALL LETTER C WITH CEDILLA */
  0x7D,
  0xD3,
  0xC0,
  0xD3,
  0x7D,
  0x0C,
  0x0C,
  0x38
};

const std::uint8_t font_std_arial_12_aa2_00E8[  8] = { /* code 00E8, LATIN SMALL LETTER E WITH GRAVE */
  0x20,
  0x08,
  0x00,
  0x7C,
  0xC3,
  0xFF,
  0xC0,
  0x7E
};

const std::uint8_t font_std_arial_12_aa2_00E9[  8] = { /* code 00E9, LATIN SMALL LETTER E WITH ACUTE */
  0x08,
  0x20,
  0x00,
  0x7C,
  0xC3,
  0xFF,
  0xC0,
  0x7E
};

const std::uint8_t font_std_arial_12_aa2_00EA[ 16] = { /* code 00EA, LATIN SMALL LETTER E WITH CIRCUMFLEX */
  0x1D, 0x00,
  0x36, 0x00,
  0x00, 0x00,
  0x7C, 0x00,
  0xC3, 0x00,
  0xFF, 0x00,
  0xC0, 0x00,
  0x7E, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00EB[  7] = { /* code 00EB, LATIN SMALL LETTER E WITH DIAERESIS */
  0xCC,
  0x00,
  0x7C,
  0xC3,
  0xFF,
  0xC0,
  0x7E
};

const std::uint8_t font_std_arial_12_aa2_00EC[  8] = { /* code 00EC, LATIN SMALL LETTER I WITH GRAVE */
  0x80,
  0x20,
  0x00,
  0xC0,
  0xC0,
  0xC0,
  0xC0,
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_00ED[  8] = { /* code 00ED, LATIN SMALL LETTER I WITH ACUTE */
  0x20,
  0x80,
  0x00,
  0x30,
  0x30,
  0x30,
  0x30,
  0x30
};

const std::uint8_t font_std_arial_12_aa2_00EE[ 16] = { /* code 00EE, LATIN SMALL LETTER I WITH CIRCUMFLEX */
  0x1D, 0x00,
  0x36, 0x00,
  0x00, 0x00,
  0x30, 0x00,
  0x30, 0x00,
  0x30, 0x00,
  0x30, 0x00,
  0x30, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00EF[  7] = { /* code 00EF, LATIN SMALL LETTER I WITH DIAERESIS */
  0xCC,
  0x00,
  0x30,
  0x30,
  0x30,
  0x30,
  0x30
};

const std::uint8_t font_std_arial_12_aa2_00F0[  7] = { /* code 00F0, LATIN SMALL LETTER ETH */
  0x28,
  0x08,
  0x7E,
  0xD7,
  0xC3,
  0xD7,
  0x7D
};

const std::uint8_t font_std_arial_12_aa2_00F1[  8] = { /* code 00F1, LATIN SMALL LETTER N WITH TILDE */
  0x77,
  0xDD,
  0x00,
  0xED,
  0xD7,
  0xC3,
  0xC3,
  0xC3
};

const std::uint8_t font_std_arial_12_aa2_00F2[  8] = { /* code 00F2, LATIN SMALL LETTER O WITH GRAVE */
  0x20,
  0x08,
  0x00,
  0x7D,
  0xD7,
  0xC3,
  0xD7,
  0x7D
};

const std::uint8_t font_std_arial_12_aa2_00F3[  8] = { /* code 00F3, LATIN SMALL LETTER O WITH ACUTE */
  0x08,
  0x20,
  0x00,
  0x7D,
  0xD7,
  0xC3,
  0xD7,
  0x7D
};

const std::uint8_t font_std_arial_12_aa2_00F4[ 16] = { /* code 00F4, LATIN SMALL LETTER O WITH CIRCUMFLEX */
  0x1D, 0x00,
  0x36, 0x00,
  0x00, 0x00,
  0x7D, 0x00,
  0xD7, 0x00,
  0xC3, 0x00,
  0xD7, 0x00,
  0x7D, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00F5[  8] = { /* code 00F5, LATIN SMALL LETTER O WITH TILDE */
  0x77,
  0xDD,
  0x00,
  0x7D,
  0xD7,
  0xC3,
  0xD7,
  0x7D
};

const std::uint8_t font_std_arial_12_aa2_00F6[  7] = { /* code 00F6, LATIN SMALL LETTER O WITH DIAERESIS */
  0xCC,
  0x00,
  0x7D,
  0xD7,
  0xC3,
  0xD7,
  0x7D
};

const std::uint8_t font_std_arial_12_aa2_00F7[ 10] = { /* code 00F7, DIVISION SIGN */
  0x0C, 0x00,
  0x00, 0x00,
  0xFF, 0xC0,
  0x00, 0x00,
  0x0C, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00F8[ 12] = { /* code 00F8, LATIN SMALL LETTER O WITH STROKE */
  0x00, 0x00,
  0x1E, 0x80,
  0x37, 0x80,
  0x39, 0xC0,
  0x39, 0xC0,
  0x2B, 0x40
};

const std::uint8_t font_std_arial_12_aa2_00F9[  8] = { /* code 00F9, LATIN SMALL LETTER U WITH GRAVE */
  0x20,
  0x08,
  0x00,
  0xC3,
  0xC3,
  0xC3,
  0xD7,
  0x7B
};

const std::uint8_t font_std_arial_12_aa2_00FA[  8] = { /* code 00FA, LATIN SMALL LETTER U WITH ACUTE */
  0x08,
  0x20,
  0x00,
  0xC3,
  0xC3,
  0xC3,
  0xD7,
  0x7B
};

const std::uint8_t font_std_arial_12_aa2_00FB[ 16] = { /* code 00FB, LATIN SMALL LETTER U WITH CIRCUMFLEX */
  0x1D, 0x00,
  0x36, 0x00,
  0x00, 0x00,
  0x30, 0xC0,
  0x30, 0xC0,
  0x30, 0xC0,
  0x35, 0xC0,
  0x1E, 0xC0
};

const std::uint8_t font_std_arial_12_aa2_00FC[  7] = { /* code 00FC, LATIN SMALL LETTER U WITH DIAERESIS */
  0x33,
  0x00,
  0xC3,
  0xC3,
  0xC3,
  0xD7,
  0x7B
};

const std::uint8_t font_std_arial_12_aa2_00FD[ 20] = { /* code 00FD, LATIN SMALL LETTER Y WITH ACUTE */
  0x02, 0x00,
  0x08, 0x00,
  0x00, 0x00,
  0xC0, 0xC0,
  0x62, 0x40,
  0x33, 0x00,
  0x2E, 0x00,
  0x0C, 0x00,
  0x0C, 0x00,
  0x34, 0x00
};

const std::uint8_t font_std_arial_12_aa2_00FE[  9] = { /* code 00FE, LATIN SMALL LETTER THORN */
  0xC0,
  0xC0,
  0xED,
  0xC7,
  0xC3,
  0xC7,
  0xED,
  0xC0,
  0xC0
};

const std::uint8_t font_std_arial_12_aa2_00FF[ 18] = { /* code 00FF, LATIN SMALL LETTER Y WITH DIAERESIS */
  0x33, 0x00,
  0x00, 0x00,
  0xC0, 0xC0,
  0x62, 0x40,
  0x33, 0x00,
  0x2E, 0x00,
  0x0C, 0x00,
  0x0C, 0x00,
  0x34, 0x00
};

const font_charinfo_ext_type font_std_arial_12_aa2_charinfo[183] = {
   {   1,   1,   0,   9,   3, font_std_arial_12_aa2_0020 } /* code 0020, SPACE */
  ,{   1,   7,   1,   2,   3, font_std_arial_12_aa2_0021 } /* code 0021, EXCLAMATION MARK */
  ,{   3,   2,   0,   2,   3, font_std_arial_12_aa2_0022 } /* code 0022, QUOTATION MARK */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_0023 } /* code 0023, NUMBER SIGN */
  ,{   5,   8,   0,   2,   5, font_std_arial_12_aa2_0024 } /* code 0024, DOLLAR SIGN */
  ,{   6,   7,   1,   2,   8, font_std_arial_12_aa2_0025 } /* code 0025, PERCENT SIGN */
  ,{   6,   7,   0,   2,   6, font_std_arial_12_aa2_0026 } /* code 0026, AMPERSAND */
  ,{   1,   2,   0,   2,   2, font_std_arial_12_aa2_0027 } /* code 0027, APOSTROPHE */
  ,{   3,   9,   0,   2,   3, font_std_arial_12_aa2_0028 } /* code 0028, LEFT PARENTHESIS */
  ,{   3,   9,   0,   2,   3, font_std_arial_12_aa2_0029 } /* code 0029, RIGHT PARENTHESIS */
  ,{   3,   3,   0,   2,   4, font_std_arial_12_aa2_002A } /* code 002A, ASTERISK */
  ,{   5,   5,   0,   4,   5, font_std_arial_12_aa2_002B } /* code 002B, PLUS SIGN */
  ,{   1,   2,   1,   8,   3, font_std_arial_12_aa2_002C } /* code 002C, COMMA */
  ,{   2,   1,   0,   6,   3, font_std_arial_12_aa2_002D } /* code 002D, HYPHEN-MINUS */
  ,{   1,   1,   1,   8,   3, font_std_arial_12_aa2_002E } /* code 002E, FULL STOP */
  ,{   3,   7,   0,   2,   3, font_std_arial_12_aa2_002F } /* code 002F, SOLIDUS */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_0030 } /* code 0030, DIGIT ZERO */
  ,{   2,   7,   1,   2,   5, font_std_arial_12_aa2_0031 } /* code 0031, DIGIT ONE */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_0032 } /* code 0032, DIGIT TWO */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_0033 } /* code 0033, DIGIT THREE */
  ,{   5,   7,   0,   2,   5, font_std_arial_12_aa2_0034 } /* code 0034, DIGIT FOUR */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_0035 } /* code 0035, DIGIT FIVE */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_0036 } /* code 0036, DIGIT SIX */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_0037 } /* code 0037, DIGIT SEVEN */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_0038 } /* code 0038, DIGIT EIGHT */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_0039 } /* code 0039, DIGIT NINE */
  ,{   1,   5,   1,   4,   3, font_std_arial_12_aa2_003A } /* code 003A, COLON */
  ,{   1,   6,   1,   4,   3, font_std_arial_12_aa2_003B } /* code 003B, SEMICOLON */
  ,{   3,   5,   1,   3,   5, font_std_arial_12_aa2_003C } /* code 003C, LESS-THAN SIGN */
  ,{   4,   3,   0,   4,   5, font_std_arial_12_aa2_003D } /* code 003D, EQUALS SIGN */
  ,{   3,   5,   1,   3,   5, font_std_arial_12_aa2_003E } /* code 003E, GREATER-THAN SIGN */
  ,{   5,   7,   0,   2,   5, font_std_arial_12_aa2_003F } /* code 003F, QUESTION MARK */
  ,{   8,   9,   1,   2,   9, font_std_arial_12_aa2_0040 } /* code 0040, COMMERCIAL AT */
  ,{   7,   7,   0,   2,   8, font_std_arial_12_aa2_0041 } /* code 0041, LATIN CAPITAL LETTER A */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_0042 } /* code 0042, LATIN CAPITAL LETTER B */
  ,{   5,   7,   0,   2,   6, font_std_arial_12_aa2_0043 } /* code 0043, LATIN CAPITAL LETTER C */
  ,{   5,   7,   1,   2,   7, font_std_arial_12_aa2_0044 } /* code 0044, LATIN CAPITAL LETTER D */
  ,{   5,   7,   1,   2,   6, font_std_arial_12_aa2_0045 } /* code 0045, LATIN CAPITAL LETTER E */
  ,{   4,   7,   1,   2,   6, font_std_arial_12_aa2_0046 } /* code 0046, LATIN CAPITAL LETTER F */
  ,{   5,   7,   1,   2,   7, font_std_arial_12_aa2_0047 } /* code 0047, LATIN CAPITAL LETTER G */
  ,{   5,   7,   1,   2,   7, font_std_arial_12_aa2_0048 } /* code 0048, LATIN CAPITAL LETTER H */
  ,{   1,   7,   1,   2,   3, font_std_arial_12_aa2_0049 } /* code 0049, LATIN CAPITAL LETTER I */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_004A } /* code 004A, LATIN CAPITAL LETTER J */
  ,{   5,   7,   1,   2,   6, font_std_arial_12_aa2_004B } /* code 004B, LATIN CAPITAL LETTER K */
  ,{   4,   7,   1,   2,   5, font_std_arial_12_aa2_004C } /* code 004C, LATIN CAPITAL LETTER L */
  ,{   7,   7,   0,   2,   8, font_std_arial_12_aa2_004D } /* code 004D, LATIN CAPITAL LETTER M */
  ,{   5,   7,   1,   2,   7, font_std_arial_12_aa2_004E } /* code 004E, LATIN CAPITAL LETTER N */
  ,{   5,   7,   1,   2,   7, font_std_arial_12_aa2_004F } /* code 004F, LATIN CAPITAL LETTER O */
  ,{   4,   7,   1,   2,   6, font_std_arial_12_aa2_0050 } /* code 0050, LATIN CAPITAL LETTER P */
  ,{   5,   8,   1,   2,   7, font_std_arial_12_aa2_0051 } /* code 0051, LATIN CAPITAL LETTER Q */
  ,{   6,   7,   1,   2,   7, font_std_arial_12_aa2_0052 } /* code 0052, LATIN CAPITAL LETTER R */
  ,{   4,   7,   1,   2,   6, font_std_arial_12_aa2_0053 } /* code 0053, LATIN CAPITAL LETTER S */
  ,{   5,   7,   0,   2,   5, font_std_arial_12_aa2_0054 } /* code 0054, LATIN CAPITAL LETTER T */
  ,{   5,   7,   1,   2,   7, font_std_arial_12_aa2_0055 } /* code 0055, LATIN CAPITAL LETTER U */
  ,{   9,   7,  -1,   2,  10, font_std_arial_12_aa2_0056 } /* code 0056, LATIN CAPITAL LETTER V */
  ,{   9,   7,   0,   2,  10, font_std_arial_12_aa2_0057 } /* code 0057, LATIN CAPITAL LETTER W */
  ,{   5,   7,   0,   2,   5, font_std_arial_12_aa2_0058 } /* code 0058, LATIN CAPITAL LETTER X */
  ,{   5,   7,   1,   2,   7, font_std_arial_12_aa2_0059 } /* code 0059, LATIN CAPITAL LETTER Y */
  ,{   6,   7,   0,   2,   6, font_std_arial_12_aa2_005A } /* code 005A, LATIN CAPITAL LETTER Z */
  ,{   2,   9,   1,   2,   3, font_std_arial_12_aa2_005B } /* code 005B, LEFT SQUARE BRACKET */
  ,{   3,   7,   0,   2,   3, font_std_arial_12_aa2_005C } /* code 005C, REVERSE SOLIDUS */
  ,{   2,   9,   0,   2,   3, font_std_arial_12_aa2_005D } /* code 005D, RIGHT SQUARE BRACKET */
  ,{   3,   3,   0,   2,   3, font_std_arial_12_aa2_005E } /* code 005E, CIRCUMFLEX ACCENT */
  ,{   5,   1,   0,  10,   5, font_std_arial_12_aa2_005F } /* code 005F, LOW LINE */
  ,{   2,   2,   0,   2,   3, font_std_arial_12_aa2_0060 } /* code 0060, GRAVE ACCENT */
  ,{   4,   5,   0,   4,   5, font_std_arial_12_aa2_0061 } /* code 0061, LATIN SMALL LETTER A */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_0062 } /* code 0062, LATIN SMALL LETTER B */
  ,{   4,   5,   0,   4,   5, font_std_arial_12_aa2_0063 } /* code 0063, LATIN SMALL LETTER C */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_0064 } /* code 0064, LATIN SMALL LETTER D */
  ,{   4,   5,   0,   4,   5, font_std_arial_12_aa2_0065 } /* code 0065, LATIN SMALL LETTER E */
  ,{   4,   7,   0,   2,   4, font_std_arial_12_aa2_0066 } /* code 0066, LATIN SMALL LETTER F */
  ,{   4,   7,   0,   4,   5, font_std_arial_12_aa2_0067 } /* code 0067, LATIN SMALL LETTER G */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_0068 } /* code 0068, LATIN SMALL LETTER H */
  ,{   1,   7,   0,   2,   2, font_std_arial_12_aa2_0069 } /* code 0069, LATIN SMALL LETTER I */
  ,{   2,   9,  -1,   2,   2, font_std_arial_12_aa2_006A } /* code 006A, LATIN SMALL LETTER J */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_006B } /* code 006B, LATIN SMALL LETTER K */
  ,{   1,   7,   0,   2,   2, font_std_arial_12_aa2_006C } /* code 006C, LATIN SMALL LETTER L */
  ,{   7,   5,   0,   4,   8, font_std_arial_12_aa2_006D } /* code 006D, LATIN SMALL LETTER M */
  ,{   4,   5,   0,   4,   5, font_std_arial_12_aa2_006E } /* code 006E, LATIN SMALL LETTER N */
  ,{   4,   5,   0,   4,   5, font_std_arial_12_aa2_006F } /* code 006F, LATIN SMALL LETTER O */
  ,{   4,   7,   0,   4,   5, font_std_arial_12_aa2_0070 } /* code 0070, LATIN SMALL LETTER P */
  ,{   4,   7,   0,   4,   5, font_std_arial_12_aa2_0071 } /* code 0071, LATIN SMALL LETTER Q */
  ,{   3,   5,   0,   4,   3, font_std_arial_12_aa2_0072 } /* code 0072, LATIN SMALL LETTER R */
  ,{   4,   5,   0,   4,   5, font_std_arial_12_aa2_0073 } /* code 0073, LATIN SMALL LETTER S */
  ,{   3,   7,  -1,   2,   3, font_std_arial_12_aa2_0074 } /* code 0074, LATIN SMALL LETTER T */
  ,{   4,   5,   0,   4,   5, font_std_arial_12_aa2_0075 } /* code 0075, LATIN SMALL LETTER U */
  ,{   5,   5,   0,   4,   6, font_std_arial_12_aa2_0076 } /* code 0076, LATIN SMALL LETTER V */
  ,{   5,   5,   0,   4,   6, font_std_arial_12_aa2_0077 } /* code 0077, LATIN SMALL LETTER W */
  ,{   4,   5,   0,   4,   5, font_std_arial_12_aa2_0078 } /* code 0078, LATIN SMALL LETTER X */
  ,{   5,   7,   0,   4,   6, font_std_arial_12_aa2_0079 } /* code 0079, LATIN SMALL LETTER Y */
  ,{   3,   5,   0,   4,   4, font_std_arial_12_aa2_007A } /* code 007A, LATIN SMALL LETTER Z */
  ,{   3,   9,   0,   2,   3, font_std_arial_12_aa2_007B } /* code 007B, LEFT CURLY BRACKET */
  ,{   1,   8,   1,   2,   3, font_std_arial_12_aa2_007C } /* code 007C, VERTICAL LINE */
  ,{   3,   9,   0,   2,   3, font_std_arial_12_aa2_007D } /* code 007D, RIGHT CURLY BRACKET */
  ,{   4,   3,   0,   4,   5, font_std_arial_12_aa2_007E } /* code 007E, TILDE */
  ,{   5,   7,   0,   2,   5, font_std_arial_12_aa2_00A3 } /* code 00A3, POUND SIGN */
  ,{   4,   4,   1,   4,   5, font_std_arial_12_aa2_00A4 } /* code 00A4, CURRENCY SIGN */
  ,{   5,   7,   0,   2,   5, font_std_arial_12_aa2_00A5 } /* code 00A5, YEN SIGN */
  ,{   1,   8,   1,   2,   3, font_std_arial_12_aa2_00A6 } /* code 00A6, BROKEN BAR */
  ,{   4,   9,   0,   2,   5, font_std_arial_12_aa2_00A7 } /* code 00A7, SECTION SIGN */
  ,{   3,   1,   0,   2,   3, font_std_arial_12_aa2_00A8 } /* code 00A8, DIAERESIS */
  ,{   7,   7,   0,   2,   7, font_std_arial_12_aa2_00A9 } /* code 00A9, COPYRIGHT SIGN */
  ,{   3,   3,   0,   2,   3, font_std_arial_12_aa2_00AA } /* code 00AA, FEMININE ORDINAL INDICATOR */
  ,{   3,   3,   1,   2,   4, font_std_arial_12_aa2_00B0 } /* code 00B0, DEGREE SIGN */
  ,{   5,   6,   0,   3,   5, font_std_arial_12_aa2_00B1 } /* code 00B1, PLUS-MINUS SIGN */
  ,{   3,   3,   0,   2,   3, font_std_arial_12_aa2_00B2 } /* code 00B2, SUPERSCRIPT TWO */
  ,{   3,   3,   0,   2,   3, font_std_arial_12_aa2_00B3 } /* code 00B3, SUPERSCRIPT THREE */
  ,{   2,   2,   1,   2,   3, font_std_arial_12_aa2_00B4 } /* code 00B4, ACUTE ACCENT */
  ,{   4,   7,   0,   4,   5, font_std_arial_12_aa2_00B5 } /* code 00B5, MICRO SIGN */
  ,{   5,   8,   0,   2,   5, font_std_arial_12_aa2_00B6 } /* code 00B6, PILCROW SIGN */
  ,{   1,   1,   1,   5,   3, font_std_arial_12_aa2_00B7 } /* code 00B7, MIDDLE DOT */
  ,{   2,   3,   0,   9,   3, font_std_arial_12_aa2_00B8 } /* code 00B8, CEDILLA */
  ,{   2,   3,   1,   2,   3, font_std_arial_12_aa2_00B9 } /* code 00B9, SUPERSCRIPT ONE */
  ,{   3,   3,   0,   2,   3, font_std_arial_12_aa2_00BA } /* code 00BA, MASCULINE ORDINAL INDICATOR */
  ,{   5,   4,   0,   5,   5, font_std_arial_12_aa2_00BB } /* code 00BB, RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
  ,{   8,   7,   0,   2,   8, font_std_arial_12_aa2_00BC } /* code 00BC, VULGAR FRACTION ONE QUARTER */
  ,{   7,   7,   1,   2,   8, font_std_arial_12_aa2_00BD } /* code 00BD, VULGAR FRACTION ONE HALF */
  ,{   8,   7,   0,   2,   8, font_std_arial_12_aa2_00BE } /* code 00BE, VULGAR FRACTION THREE QUARTERS */
  ,{   4,   7,   1,   4,   6, font_std_arial_12_aa2_00BF } /* code 00BF, INVERTED QUESTION MARK */
  ,{   7,   9,   0,   0,   6, font_std_arial_12_aa2_00C0 } /* code 00C0, LATIN CAPITAL LETTER A WITH GRAVE */
  ,{   7,   9,   0,   0,   6, font_std_arial_12_aa2_00C1 } /* code 00C1, LATIN CAPITAL LETTER A WITH ACUTE */
  ,{   7,   9,   0,   0,   6, font_std_arial_12_aa2_00C2 } /* code 00C2, LATIN CAPITAL LETTER A WITH CIRCUMFLEX */
  ,{   7,   9,   0,   0,   6, font_std_arial_12_aa2_00C3 } /* code 00C3, LATIN CAPITAL LETTER A WITH TILDE */
  ,{   7,   8,   0,   1,   6, font_std_arial_12_aa2_00C4 } /* code 00C4, LATIN CAPITAL LETTER A WITH DIAERESIS */
  ,{   7,   9,   0,   0,   6, font_std_arial_12_aa2_00C5 } /* code 00C5, LATIN CAPITAL LETTER A WITH RING ABOVE */
  ,{  10,   7,  -1,   2,   9, font_std_arial_12_aa2_00C6 } /* code 00C6, LATIN CAPITAL LETTER AE */
  ,{   5,  10,   1,   2,   6, font_std_arial_12_aa2_00C7 } /* code 00C7, LATIN CAPITAL LETTER C WITH CEDILLA */
  ,{   5,   9,   1,   0,   6, font_std_arial_12_aa2_00C8 } /* code 00C8, LATIN CAPITAL LETTER E WITH GRAVE */
  ,{   5,   9,   1,   0,   6, font_std_arial_12_aa2_00C9 } /* code 00C9, LATIN CAPITAL LETTER E WITH ACUTE */
  ,{   5,   9,   1,   0,   6, font_std_arial_12_aa2_00CA } /* code 00CA, LATIN CAPITAL LETTER E WITH CIRCUMFLEX */
  ,{   5,   8,   1,   1,   6, font_std_arial_12_aa2_00CB } /* code 00CB, LATIN CAPITAL LETTER E WITH DIAERESIS */
  ,{   2,   9,   1,   0,   3, font_std_arial_12_aa2_00CC } /* code 00CC, LATIN CAPITAL LETTER I WITH GRAVE */
  ,{   2,   9,   0,   0,   3, font_std_arial_12_aa2_00CD } /* code 00CD, LATIN CAPITAL LETTER I WITH ACUTE */
  ,{   5,   9,  -2,   0,   3, font_std_arial_12_aa2_00CE } /* code 00CE, LATIN CAPITAL LETTER I WITH CIRCUMFLEX */
  ,{   3,   8,   0,   1,   3, font_std_arial_12_aa2_00CF } /* code 00CF, LATIN CAPITAL LETTER I WITH DIAERESIS */
  ,{   6,   7,   0,   2,   6, font_std_arial_12_aa2_00D0 } /* code 00D0, LATIN CAPITAL LETTER ETH */
  ,{   5,   9,   1,   0,   6, font_std_arial_12_aa2_00D1 } /* code 00D1, LATIN CAPITAL LETTER N WITH TILDE */
  ,{   5,   9,   1,   0,   7, font_std_arial_12_aa2_00D2 } /* code 00D2, LATIN CAPITAL LETTER O WITH GRAVE */
  ,{   5,   9,   1,   0,   7, font_std_arial_12_aa2_00D3 } /* code 00D3, LATIN CAPITAL LETTER O WITH ACUTE */
  ,{   5,   9,   1,   0,   7, font_std_arial_12_aa2_00D4 } /* code 00D4, LATIN CAPITAL LETTER O WITH CIRCUMFLEX */
  ,{   5,   9,   1,   0,   7, font_std_arial_12_aa2_00D5 } /* code 00D5, LATIN CAPITAL LETTER O WITH TILDE */
  ,{   5,   8,   1,   1,   7, font_std_arial_12_aa2_00D6 } /* code 00D6, LATIN CAPITAL LETTER O WITH DIAERESIS */
  ,{   6,   5,   0,   3,   5, font_std_arial_12_aa2_00D7 } /* code 00D7, MULTIPLICATION SIGN */
  ,{   5,   8,   1,   2,   7, font_std_arial_12_aa2_00D8 } /* code 00D8, LATIN CAPITAL LETTER O WITH STROKE */
  ,{   5,   9,   1,   0,   6, font_std_arial_12_aa2_00D9 } /* code 00D9, LATIN CAPITAL LETTER U WITH GRAVE */
  ,{   5,   9,   1,   0,   6, font_std_arial_12_aa2_00DA } /* code 00DA, LATIN CAPITAL LETTER U WITH ACUTE */
  ,{   5,   9,   1,   0,   6, font_std_arial_12_aa2_00DB } /* code 00DB, LATIN CAPITAL LETTER U WITH CIRCUMFLEX */
  ,{   5,   8,   1,   1,   6, font_std_arial_12_aa2_00DC } /* code 00DC, LATIN CAPITAL LETTER U WITH DIAERESIS */
  ,{   5,   9,   1,   0,   6, font_std_arial_12_aa2_00DD } /* code 00DD, LATIN CAPITAL LETTER Y WITH ACUTE */
  ,{   5,   7,   1,   2,   6, font_std_arial_12_aa2_00DE } /* code 00DE, LATIN CAPITAL LETTER THORN */
  ,{   4,   7,   1,   2,   6, font_std_arial_12_aa2_00DF } /* code 00DF, LATIN SMALL LETTER SHARP S */
  ,{   4,   8,   0,   1,   5, font_std_arial_12_aa2_00E0 } /* code 00E0, LATIN SMALL LETTER A WITH GRAVE */
  ,{   4,   8,   0,   1,   5, font_std_arial_12_aa2_00E1 } /* code 00E1, LATIN SMALL LETTER A WITH ACUTE */
  ,{   5,   8,   0,   1,   5, font_std_arial_12_aa2_00E2 } /* code 00E2, LATIN SMALL LETTER A WITH CIRCUMFLEX */
  ,{   4,   8,   0,   1,   5, font_std_arial_12_aa2_00E3 } /* code 00E3, LATIN SMALL LETTER A WITH TILDE */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_00E4 } /* code 00E4, LATIN SMALL LETTER A WITH DIAERESIS */
  ,{   4,   9,   0,   0,   5, font_std_arial_12_aa2_00E5 } /* code 00E5, LATIN SMALL LETTER A WITH RING ABOVE */
  ,{   7,   5,   0,   4,   8, font_std_arial_12_aa2_00E6 } /* code 00E6, LATIN SMALL LETTER AE */
  ,{   4,   8,   0,   4,   5, font_std_arial_12_aa2_00E7 } /* code 00E7, LATIN SMALL LETTER C WITH CEDILLA */
  ,{   4,   8,   0,   1,   5, font_std_arial_12_aa2_00E8 } /* code 00E8, LATIN SMALL LETTER E WITH GRAVE */
  ,{   4,   8,   0,   1,   5, font_std_arial_12_aa2_00E9 } /* code 00E9, LATIN SMALL LETTER E WITH ACUTE */
  ,{   5,   8,   0,   1,   5, font_std_arial_12_aa2_00EA } /* code 00EA, LATIN SMALL LETTER E WITH CIRCUMFLEX */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_00EB } /* code 00EB, LATIN SMALL LETTER E WITH DIAERESIS */
  ,{   2,   8,   0,   1,   3, font_std_arial_12_aa2_00EC } /* code 00EC, LATIN SMALL LETTER I WITH GRAVE */
  ,{   2,   8,  -1,   1,   3, font_std_arial_12_aa2_00ED } /* code 00ED, LATIN SMALL LETTER I WITH ACUTE */
  ,{   5,   8,  -1,   1,   3, font_std_arial_12_aa2_00EE } /* code 00EE, LATIN SMALL LETTER I WITH CIRCUMFLEX */
  ,{   3,   7,  -1,   2,   3, font_std_arial_12_aa2_00EF } /* code 00EF, LATIN SMALL LETTER I WITH DIAERESIS */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_00F0 } /* code 00F0, LATIN SMALL LETTER ETH */
  ,{   4,   8,   0,   1,   5, font_std_arial_12_aa2_00F1 } /* code 00F1, LATIN SMALL LETTER N WITH TILDE */
  ,{   4,   8,   0,   1,   5, font_std_arial_12_aa2_00F2 } /* code 00F2, LATIN SMALL LETTER O WITH GRAVE */
  ,{   4,   8,   0,   1,   5, font_std_arial_12_aa2_00F3 } /* code 00F3, LATIN SMALL LETTER O WITH ACUTE */
  ,{   5,   8,   0,   1,   5, font_std_arial_12_aa2_00F4 } /* code 00F4, LATIN SMALL LETTER O WITH CIRCUMFLEX */
  ,{   4,   8,   0,   1,   5, font_std_arial_12_aa2_00F5 } /* code 00F5, LATIN SMALL LETTER O WITH TILDE */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_00F6 } /* code 00F6, LATIN SMALL LETTER O WITH DIAERESIS */
  ,{   5,   5,   0,   4,   5, font_std_arial_12_aa2_00F7 } /* code 00F7, DIVISION SIGN */
  ,{   5,   6,  -1,   3,   5, font_std_arial_12_aa2_00F8 } /* code 00F8, LATIN SMALL LETTER O WITH STROKE */
  ,{   4,   8,   0,   1,   5, font_std_arial_12_aa2_00F9 } /* code 00F9, LATIN SMALL LETTER U WITH GRAVE */
  ,{   4,   8,   0,   1,   5, font_std_arial_12_aa2_00FA } /* code 00FA, LATIN SMALL LETTER U WITH ACUTE */
  ,{   5,   8,  -1,   1,   5, font_std_arial_12_aa2_00FB } /* code 00FB, LATIN SMALL LETTER U WITH CIRCUMFLEX */
  ,{   4,   7,   0,   2,   5, font_std_arial_12_aa2_00FC } /* code 00FC, LATIN SMALL LETTER U WITH DIAERESIS */
  ,{   5,  10,   0,   1,   5, font_std_arial_12_aa2_00FD } /* code 00FD, LATIN SMALL LETTER Y WITH ACUTE */
  ,{   4,   9,   0,   2,   5, font_std_arial_12_aa2_00FE } /* code 00FE, LATIN SMALL LETTER THORN */
  ,{   5,   9,   0,   2,   5, font_std_arial_12_aa2_00FF } /* code 00FF, LATIN SMALL LETTER Y WITH DIAERESIS */
};

const font_prop_ext_type font_std_arial_12_aa2_prop3 = {
   0x00B0 /* first character */
  ,0x00FF /* last character  */
  ,&font_std_arial_12_aa2_charinfo[103] /* address of first character */
  ,nullptr /* pointer to next font_prop_ext_type */
};

const font_prop_ext_type font_std_arial_12_aa2_prop2 = {
   0x00A3 /* first character */
  ,0x00AA /* last character  */
  ,&font_std_arial_12_aa2_charinfo[ 95] /* address of first character */
  ,&font_std_arial_12_aa2_prop3 /* pointer to next font_prop_ext_type */
};

const font_prop_ext_type font_std_arial_12_aa2_prop1 = {
   0x0020 /* first character */
  ,0x007E /* last character  */
  ,&font_std_arial_12_aa2_charinfo[  0] /* address of first character */
  ,&font_std_arial_12_aa2_prop2 /* pointer to next font_prop_ext_type */
};

const font_type font_std_arial_8x12_aa2 = {
  AA_2 | ENCODING_UNICODE | TYPE_PROP   /* type of font    */
  ,12 /* height of font  */
  ,12 /* space of font y */
  ,5  /* Baseline */
  ,&font_std_arial_12_aa2_prop1
};

} // namespace vic
