///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2026, PALANDesign Hannover, Germany
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
// \brief IT8951 head, ITE e-paper timing controller with 16 gray levels
// (up to 2047 x 2047, e.g. 6" 800 x 600, 9.7" 1200 x 825, 7.8" / 10.3" 1872 x 1404
// panels). The image is kept in the SDRAM of the controller, so the head has
// NO framebuffer: pixels, lines, boxes and blocks are loaded directly into the
// image buffer of the controller (LD_IMG_AREA, 4 bit per pixel) and the head only
// tracks the changed area. present() refreshes that area with the selected
// waveform mode (GC16 by default, about 450 ms) and is skipped if nothing changed;
// use dc::present_lock() to batch primitives into one refresh.
// A single pixel is a complete load sequence of 8 packets, lines, boxes and
// pixel blocks are one load each.
// All orientations are done in software. See doc/IT8951.md for the programming
// summary.
//
// io layer convention (SPI, mode 0, max. 24 MHz, 12 MHz recommended):
// every io::write() is one packet = one CS low period, the head sends the 16 bit
// preamble (6000h command, 0000h data) in front of the big endian words; the io
// option is 0 for command packets and 1 for data packets.
// io::read() option 1 reads data words: the io layer sends the read preamble 10h 00h,
// drops the dummy word of the controller and returns the big endian words in one CS
// low period. io::read() option 2 returns one byte with bit 0 = 1 while the
// controller is busy (HRDY pin low, so the inverted pin level), the head polls it
// before every packet. An io layer without HRDY read returns false, the head then
// sends without waiting (use a slow SPI clock of about 1 .. 2 MHz in this case).
// The hardware reset (RESET_N low >= 20 ms) is done by io::init().
// An io layer which can't read data words still works for drawing: pixel_get()
// returns color::none and present() waits a fixed time.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_HEAD_IT8951_H_
#define _VIC_HEAD_IT8951_H_

#include "../drv.h"


// defines the head name and version
#define VIC_HEAD_IT8951_VERSION   "IT8951 head 1.00"

namespace vic {
namespace head {


/**
 * IT8951 e-paper timing controller
 * The screen size is the LOGICAL size and must match the orientation. The physical panel is
 * e.g. 800 x 600, the portrait use is 600 x 800 with orientation_90 or 270.
 * \param Screen_Size_X Logical screen width in pixel
 * \param Screen_Size_Y Logical screen height in pixel
 * \param Orientation Orientation of the display (done in software)
 * \param Mode Waveform mode of present(): MODE_GC16 (2) for 16 gray levels, MODE_DU (1) for fast black/white updates;
 *             the mode numbers depend on the waveform of the panel firmware, see doc/IT8951.md
 * \param VCOM VCOM voltage of the panel as magnitude in mV (2000 = -2.00 V, printed on the panel cable),
 *             0 = keep the value of the controller
 */
template<std::uint16_t Screen_Size_X = 800U, std::uint16_t Screen_Size_Y = 600U,   // X, Y size, must match the orientation
         drv::orientation_type Orientation = drv::orientation_0,                    // orientation of the display
         std::uint8_t Mode = 2U,                                                    // waveform mode of present(), GC16
         std::uint16_t VCOM = 0U                                                    // mV, 0 = don't touch
>
class IT8951 final : public drv
{
  // commands
  static const std::uint16_t CMD_SYS_RUN            = 0x0001U;  // all clocks on, active state
  static const std::uint16_t CMD_STANDBY            = 0x0002U;  // clocks gated
  static const std::uint16_t CMD_SLEEP              = 0x0003U;  // all clocks off
  static const std::uint16_t CMD_REG_RD             = 0x0010U;  // address, read 1 word
  static const std::uint16_t CMD_REG_WR             = 0x0011U;  // address, value
  static const std::uint16_t CMD_MEM_BST_RD_T       = 0x0012U;  // address low / high, word count low / high
  static const std::uint16_t CMD_MEM_BST_RD_S       = 0x0013U;  // read the words
  static const std::uint16_t CMD_MEM_BST_END        = 0x0015U;
  static const std::uint16_t CMD_LD_IMG_AREA        = 0x0021U;  // argument, x, y, width, height, pixel words
  static const std::uint16_t CMD_LD_IMG_END         = 0x0022U;
  static const std::uint16_t CMD_DPY_AREA           = 0x0034U;  // x, y, width, height, mode
  static const std::uint16_t CMD_VCOM               = 0x0039U;  // 0 = get (read 1 word), 1 = set + value in mV
  static const std::uint16_t CMD_GET_DEV_INFO       = 0x0302U;  // read 20 words

  // registers
  static const std::uint16_t REG_I80CPCR            = 0x0004U;  // 1 = command parameters in packed mode
  static const std::uint16_t REG_LISAR              = 0x0208U;  // load image start address, low word, high word at + 2
  static const std::uint16_t REG_LUTAFSR            = 0x1224U;  // 0 = all LUT engines free

  // SPI preambles
  static const std::uint8_t  PREAMBLE_COMMAND       = 0x60U;    // 6000h
  static const std::uint8_t  PREAMBLE_DATA          = 0x00U;    // 0000h

  // load image argument: big endian (first pixel in the upper bits of the word), 4 bit per pixel, no rotation
  static const std::uint16_t LOAD_ARG               = 0x0120U;

  // io options
  static const std::uint32_t OPT_COMMAND            = 0U;
  static const std::uint32_t OPT_DATA               = 1U;
  static const std::uint32_t OPT_BUSY               = 2U;

  static const std::uint8_t  GRAY_WHITE             = 0x0FU;
  static const std::uint32_t RESET_DELAY            = 200U;     // ms, fallback for the HRDY poll after the reset
  static const std::size_t   PACKET_SIZE            = 2U + 256U;  // preamble + 128 pixel words

  // physical panel
  static const bool          ROTATED = (Orientation == orientation_90)  || (Orientation == orientation_270) ||
                                       (Orientation == orientation_90m) || (Orientation == orientation_270m);
  static const std::uint16_t PHYS_X  = ROTATED ? Screen_Size_Y : Screen_Size_X;
  static const std::uint16_t PHYS_Y  = ROTATED ? Screen_Size_X : Screen_Size_Y;

  static_assert((PHYS_X <= 2047U) && (PHYS_Y <= 2047U), "IT8951: width and height of a load must be less than 2048");

public:

  // waveform modes (E Ink mode declaration, the numbers depend on the waveform of the panel firmware)
  static const std::uint8_t  MODE_INIT              = 0U;       // full clear to white, about 2 s
  static const std::uint8_t  MODE_DU                = 1U;       // black / white only, about 260 ms
  static const std::uint8_t  MODE_GC16              = 2U;       // 16 gray levels with flash, about 450 ms
  static const std::uint8_t  MODE_GL16              = 3U;       // 16 gray levels, less flash on white background
  static const std::uint8_t  MODE_GLR16             = 4U;
  static const std::uint8_t  MODE_GLD16             = 5U;
  static const std::uint8_t  MODE_A2                = 6U;       // black / white only, about 120 ms (4 on the 6" "M641" firmware)
  static const std::uint8_t  MODE_DU4               = 7U;       // 4 gray levels, about 290 ms

  /**
   * Device info of the controller (GET_DEV_INFO)
   */
  typedef struct tag_device_info_type {
    bool          valid;          // false if the io layer can't read
    std::uint16_t width;          // panel width
    std::uint16_t height;         // panel height
    std::uint32_t image_buffer;   // address of the image buffer
    char          firmware[17];   // firmware version
    char          lut[17];        // LUT (waveform) version
  } device_info_type;

private:

  // vars
  const io::handle_type device_handle_;
  device_info_type      info_;
  rect_type             dirty_;                             // changed area since the last refresh (physical), empty if left >= right
  std::uint8_t          mode_;                              // waveform mode of present()
  std::uint8_t          packet_[PACKET_SIZE];               // pixel data packet
  std::size_t           packet_idx_;
  std::uint8_t          nibble_;                            // pending left pixel of a byte
  bool                  nibble_pending_;

public:

  /**
   * ctor
   * \param device_handle Device handle
   */
  IT8951(io::handle_type device_handle)
    : drv(Screen_Size_X, Screen_Size_Y,
          Screen_Size_X, Screen_Size_Y,         // no viewport support (screen = viewport)
          0, 0,                                 // no viewport support
          Orientation)
    , device_handle_(device_handle)
    , mode_(Mode)
    , packet_idx_(2U)
    , nibble_(0U)
    , nibble_pending_(false)
  {
    info_.valid        = false;
    info_.width        = 0U;
    info_.height       = 0U;
    info_.image_buffer = 0U;
    info_.firmware[0]  = '\0';
    info_.lut[0]       = '\0';
    dirty_ = { 0, 0, 0, 0 };
    packet_[0] = PREAMBLE_DATA;
    packet_[1] = 0x00U;
  }


  ~IT8951()
  {
    shutdown();
  }


  /////////////////////////////////////////////////////////////////////////////
  // M A N D A T O R Y   F U N C T I O N S

  /**
   * Init as the reference drivers (Waveshare, ITE example): run, device info, packed command parameters,
   * load address = image buffer, VCOM, then the screen is cleared to white with the INIT waveform (about 2 s)
   */
  virtual void init()
  {
    io::init(device_handle_);           // includes the hardware reset
    wait_ready(RESET_DELAY);

    write_cmd(CMD_SYS_RUN);

    // device info: panel size, image buffer address, versions
    std::uint16_t info[20];
    write_cmd(CMD_GET_DEV_INFO);
    info_.valid = read_words(info, 20U);
    if (info_.valid) {
      info_.width        = info[0];
      info_.height       = info[1];
      info_.image_buffer = (static_cast<std::uint32_t>(info[3]) << 16U) | info[2];
      for (std::size_t i = 0U; i < 8U; ++i) {
        // the strings are stored low byte first
        info_.firmware[i * 2U]      = static_cast<char>(info[4U + i] & 0xFFU);
        info_.firmware[i * 2U + 1U] = static_cast<char>(info[4U + i] >> 8U);
        info_.lut[i * 2U]           = static_cast<char>(info[12U + i] & 0xFFU);
        info_.lut[i * 2U + 1U]      = static_cast<char>(info[12U + i] >> 8U);
      }
      info_.firmware[16] = '\0';
      info_.lut[16]      = '\0';
    }

    write_reg(REG_I80CPCR, 0x0001U);    // command parameters in packed mode
    if (info_.valid) {
      // all loads go to the image buffer
      write_reg(static_cast<std::uint16_t>(REG_LISAR + 2U), static_cast<std::uint16_t>(info_.image_buffer >> 16U));
      write_reg(REG_LISAR,                                  static_cast<std::uint16_t>(info_.image_buffer));
    }

    if (VCOM != 0U) {
      // set the VCOM of the panel if the controller has another value
      std::uint16_t vcom = 0U;
      write_cmd(CMD_VCOM);
      write_word(0x0000U);
      if (!read_words(&vcom, 1U) || (vcom != VCOM)) {
        write_cmd(CMD_VCOM);
        write_word(0x0001U);
        write_word(VCOM);
      }
    }

    cls();
    refresh(MODE_INIT);
  }


  /**
   * Shutdown: sleep mode (all clocks off), init() wakes the controller up
   */
  virtual void shutdown()
  {
    write_cmd(CMD_SLEEP);
  }


  inline virtual const char* version() const
  {
    return (const char*)VIC_HEAD_IT8951_VERSION;
  }


  inline virtual bool is_graphic() const
  {
    return true;
  }


  ///////////////////////////////////////////////////////////////////////////////
  // H E A D   F U N C T I O N S
  //

  /**
   * Device info read by init(): panel size, image buffer address, firmware and LUT version
   */
  inline const device_info_type& device_info() const
  {
    return info_;
  }


  /**
   * Set the waveform mode of present()
   * \param mode MODE_GC16, MODE_DU, MODE_A2 ...
   */
  inline void refresh_mode(std::uint8_t mode)
  {
    mode_ = mode;
  }


  /**
   * Refresh the whole screen from the image buffer, e.g. with MODE_INIT or MODE_GC16 to remove ghosting
   * \param mode Waveform mode
   */
  void refresh(std::uint8_t mode)
  {
    const rect_type full = { 0, 0, static_cast<std::int16_t>(PHYS_X), static_cast<std::int16_t>(PHYS_Y) };
    display(full, mode);
    dirty_ = { 0, 0, 0, 0 };
  }


  ///////////////////////////////////////////////////////////////////////////////
  // C O M M O N  F U N C T I O N S
  //

protected:

  /**
   * Clear the image buffer, color::none (the default) is white paper
   */
  virtual void cls(color::value_type bg_color = color::none)
  {
    const rect_type full = { 0, 0, static_cast<std::int16_t>(PHYS_X), static_cast<std::int16_t>(PHYS_Y) };
    fill(full, to_gray(bg_color));
  }


  /**
   * Refresh the changed area of the panel with the selected waveform mode, only if something changed
   */
  virtual void present()
  {
    if (dirty_.left >= dirty_.right) {
      return;
    }
    display(dirty_, mode_);
    dirty_ = { 0, 0, 0, 0 };
  }


  ///////////////////////////////////////////////////////////////////////////////
  // G R A P H I C   F U N C T I O N S
  //

  virtual void pixel_set(vertex_type vertex, color::value_type color)
  {
    if (!screen_is_inside(vertex)) {
      return;
    }
    const vertex_type p = to_phys(vertex);
    const rect_type   r = { p.x, p.y, static_cast<std::int16_t>(p.x + 1), static_cast<std::int16_t>(p.y + 1) };
    fill(r, to_gray(color));
  }


  /**
   * Read a pixel back from the image buffer (memory burst read of one word = two pixels)
   * \return Gray level as color, color::none if the io layer can't read
   */
  virtual color::value_type pixel_get(vertex_type vertex)
  {
    if (!screen_is_inside(vertex) || !info_.valid) {
      return color::none;
    }
    const vertex_type   p    = to_phys(vertex);
    const std::uint32_t addr = info_.image_buffer + static_cast<std::uint32_t>(p.y) * PHYS_X + (static_cast<std::uint32_t>(p.x) & 0xFFFEU);
    std::uint16_t data = 0U;
    write_cmd(CMD_MEM_BST_RD_T);
    write_word(static_cast<std::uint16_t>(addr));
    write_word(static_cast<std::uint16_t>(addr >> 16U));
    write_word(0x0001U);                // one word
    write_word(0x0000U);
    write_cmd(CMD_MEM_BST_RD_S);
    const bool ok = read_words(&data, 1U);
    write_cmd(CMD_MEM_BST_END);
    if (!ok) {
      return color::none;
    }
    // one byte per pixel in the image buffer, gray level in the upper nibble, the even pixel is the low byte
    return color::L4_to_color(static_cast<std::uint8_t>(((p.x & 1) ? (data >> 12U) : (data >> 4U)) & 0x0FU));
  }


  /**
   * Horizontal line, one load
   */
  virtual void line_horz(vertex_type v0, vertex_type v1, color::value_type color)
  {
    util::min_max_swap(v0.x, v1.x);
    box({ v0.x, v0.y, static_cast<std::int16_t>(v1.x + 1), static_cast<std::int16_t>(v0.y + 1) }, color);
  }


  /**
   * Vertical line, one load
   */
  virtual void line_vert(vertex_type v0, vertex_type v1, color::value_type color)
  {
    util::min_max_swap(v0.y, v1.y);
    box({ v0.x, v0.y, static_cast<std::int16_t>(v0.x + 1), static_cast<std::int16_t>(v1.y + 1) }, color);
  }


  /**
   * Box (filled rectangle), one load
   * \param rect Box bounding, right and bottom are outside the box
   * \param color Box color
   */
  virtual void box(const rect_type& rect, color::value_type color)
  {
    rect_type r = rect;
    if (!clip(r)) {
      return;
    }
    fill(phys_rect(r), to_gray(color));
  }


  /**
   * Block of pixels, one load
   * \param rect Destination rect, right and bottom are outside the rect
   * \param pixels Pixel block in ARGB format, row major
   * \param stride Distance between two rows in pixels
   */
  virtual void blit(const rect_type& rect, const color::value_type* pixels, std::size_t stride)
  {
    rect_type r = rect;
    if (!clip(r)) {
      return;
    }
    const rect_type p = phys_rect(r);
    load_begin(p);
    for (std::int16_t y = p.top; y < p.bottom; ++y) {
      // dummy pixels in front of an unaligned start, the controller drops them
      std::uint_fast16_t n = static_cast<std::uint_fast16_t>(p.left) & 0x03U;
      for (std::uint_fast16_t i = 0U; i < n; ++i) {
        stream_pixel(GRAY_WHITE);
      }
      for (std::int16_t x = p.left; x < p.right; ++x, ++n) {
        const vertex_type v = from_phys({ x, y });
        stream_pixel(to_gray(pixels[static_cast<std::size_t>(v.y - rect.top) * stride + static_cast<std::size_t>(v.x - rect.left)]));
      }
      // pad the row to a whole word
      for (; (n & 0x03U) != 0U; ++n) {
        stream_pixel(GRAY_WHITE);
      }
    }
    load_end(p);
  }


private:

  /**
   * Gray level of a color: 0 = black .. 15 = white, color::none is white paper
   */
  static inline std::uint8_t to_gray(color::value_type c)
  {
    return (c == color::none) ? GRAY_WHITE : color::color_to_L4(c);
  }


  /**
   * Clip a logical rect to the screen
   * \return False if nothing is left
   */
  static inline bool clip(rect_type& r)
  {
    if (r.left < 0) { r.left = 0; }
    if (r.top  < 0) { r.top  = 0; }
    if (r.right  > static_cast<std::int16_t>(Screen_Size_X)) { r.right  = static_cast<std::int16_t>(Screen_Size_X); }
    if (r.bottom > static_cast<std::int16_t>(Screen_Size_Y)) { r.bottom = static_cast<std::int16_t>(Screen_Size_Y); }
    return (r.left < r.right) && (r.top < r.bottom);
  }


  /**
   * Map a logical screen vertex to the physical panel, the 'm' orientations mirror the panel
   * top/bottom after the rotation
   */
  static inline vertex_type to_phys(vertex_type v)
  {
    const std::int16_t mx = static_cast<std::int16_t>(PHYS_X - 1U);
    const std::int16_t my = static_cast<std::int16_t>(PHYS_Y - 1U);
    switch (Orientation) {
      case orientation_0    : return { v.x, v.y };
      case orientation_0m   : return { v.x, static_cast<std::int16_t>(my - v.y) };
      case orientation_180  : return { static_cast<std::int16_t>(mx - v.x), static_cast<std::int16_t>(my - v.y) };
      case orientation_180m : return { static_cast<std::int16_t>(mx - v.x), v.y };
      case orientation_90   : return { static_cast<std::int16_t>(mx - v.y), v.x };
      case orientation_90m  : return { static_cast<std::int16_t>(mx - v.y), static_cast<std::int16_t>(my - v.x) };
      case orientation_270  : return { v.y, static_cast<std::int16_t>(my - v.x) };
      case orientation_270m :
      default               : return { v.y, v.x };
    }
  }


  /**
   * Map a physical panel vertex to the logical screen (inverse of to_phys)
   */
  static inline vertex_type from_phys(vertex_type p)
  {
    const std::int16_t mx = static_cast<std::int16_t>(PHYS_X - 1U);
    const std::int16_t my = static_cast<std::int16_t>(PHYS_Y - 1U);
    switch (Orientation) {
      case orientation_0    : return { p.x, p.y };
      case orientation_0m   : return { p.x, static_cast<std::int16_t>(my - p.y) };
      case orientation_180  : return { static_cast<std::int16_t>(mx - p.x), static_cast<std::int16_t>(my - p.y) };
      case orientation_180m : return { static_cast<std::int16_t>(mx - p.x), p.y };
      case orientation_90   : return { p.y, static_cast<std::int16_t>(mx - p.x) };
      case orientation_90m  : return { static_cast<std::int16_t>(my - p.y), static_cast<std::int16_t>(mx - p.x) };
      case orientation_270  : return { static_cast<std::int16_t>(my - p.y), p.x };
      case orientation_270m :
      default               : return { p.y, p.x };
    }
  }


  /**
   * Physical rect of a (clipped) logical rect, right and bottom are outside
   */
  static inline rect_type phys_rect(const rect_type& r)
  {
    const vertex_type a = to_phys({ r.left, r.top });
    const vertex_type b = to_phys({ static_cast<std::int16_t>(r.right - 1), static_cast<std::int16_t>(r.bottom - 1) });
    const rect_type   p = { (a.x < b.x) ? a.x : b.x, (a.y < b.y) ? a.y : b.y,
                            static_cast<std::int16_t>(((a.x < b.x) ? b.x : a.x) + 1), static_cast<std::int16_t>(((a.y < b.y) ? b.y : a.y) + 1) };
    return p;
  }


  /**
   * Fill a physical rect of the image buffer with one gray level
   */
  void fill(const rect_type& p, std::uint8_t gray)
  {
    // bytes of a row: dummy pixels in front of an unaligned start, padded to a whole word
    const std::size_t  row   = ((static_cast<std::size_t>(p.left) & 0x03U) + static_cast<std::size_t>(p.right - p.left) + 3U) / 4U * 2U;
    const std::uint8_t value = static_cast<std::uint8_t>(gray * 0x11U);
    load_begin(p);
    for (std::int16_t y = p.top; y < p.bottom; ++y) {
      for (std::size_t i = 0U; i < row; ++i) {
        stream_byte(value);
      }
    }
    load_end(p);
  }


  /**
   * Start a load of a physical rect into the image buffer
   */
  void load_begin(const rect_type& p)
  {
    write_cmd(CMD_LD_IMG_AREA);
    write_word(LOAD_ARG);
    write_word(static_cast<std::uint16_t>(p.left));
    write_word(static_cast<std::uint16_t>(p.top));
    write_word(static_cast<std::uint16_t>(p.right - p.left));
    write_word(static_cast<std::uint16_t>(p.bottom - p.top));
    packet_idx_     = 2U;
    nibble_pending_ = false;
  }


  /**
   * Finish a load: send the remaining pixels, end command, extend the changed area
   */
  void load_end(const rect_type& p)
  {
    stream_flush();
    write_cmd(CMD_LD_IMG_END);
    if (dirty_.left >= dirty_.right) {
      dirty_ = p;
    }
    else {
      if (p.left   < dirty_.left)   { dirty_.left   = p.left; }
      if (p.top    < dirty_.top)    { dirty_.top    = p.top; }
      if (p.right  > dirty_.right)  { dirty_.right  = p.right; }
      if (p.bottom > dirty_.bottom) { dirty_.bottom = p.bottom; }
    }
  }


  /**
   * Add one pixel to the load, the first pixel is the upper nibble of a byte
   */
  inline void stream_pixel(std::uint8_t gray)
  {
    if (nibble_pending_) {
      nibble_pending_ = false;
      stream_byte(static_cast<std::uint8_t>(nibble_ | gray));
    }
    else {
      nibble_pending_ = true;
      nibble_ = static_cast<std::uint8_t>(static_cast<unsigned>(gray) << 4U);
    }
  }


  /**
   * Add one byte (two pixels) to the load, a full packet is sent
   */
  inline void stream_byte(std::uint8_t data)
  {
    packet_[packet_idx_++] = data;
    if (packet_idx_ == PACKET_SIZE) {
      stream_flush();
    }
  }


  inline void stream_flush()
  {
    if (packet_idx_ > 2U) {
      wait_ready();
      io::write(device_handle_, OPT_DATA, packet_, packet_idx_, nullptr, 0U);
      packet_idx_ = 2U;
    }
  }


  /**
   * Refresh a physical rect of the panel from the image buffer and wait until the display engine is done
   */
  void display(const rect_type& p, std::uint8_t mode)
  {
    write_cmd(CMD_DPY_AREA);
    write_word(static_cast<std::uint16_t>(p.left));
    write_word(static_cast<std::uint16_t>(p.top));
    write_word(static_cast<std::uint16_t>(p.right - p.left));
    write_word(static_cast<std::uint16_t>(p.bottom - p.top));
    write_word(mode);

    // poll the LUT engine status (max. 5 s), fixed delay if the io layer can't read
    for (std::uint_fast16_t n = 0U; n < 500U; ++n) {
      std::uint16_t busy = 0U;
      if (!read_reg(REG_LUTAFSR, busy)) {
        io::delay((mode == MODE_INIT) ? 2000U : 500U);
        return;
      }
      if (busy == 0U) {
        return;
      }
      io::delay(10U);
    }
  }


  /**
   * Wait until the controller accepts the next packet (HRDY high, max. 3 s)
   * \param fallback Delay in ms if the io layer can't read the HRDY pin
   */
  void wait_ready(std::uint32_t fallback = 0U) const
  {
    for (std::uint_fast16_t n = 0U; n < 3000U; ++n) {
      std::uint8_t busy = 0U;
      std::size_t  len  = 1U;
      if (!io::read(device_handle_, OPT_BUSY, &busy, len, 0U) || (len != 1U)) {
        if (fallback != 0U) {
          io::delay(fallback);
        }
        return;
      }
      if ((busy & 0x01U) == 0U) {
        return;
      }
      io::delay(1U);
    }
  }


  /**
   * Send a command packet
   */
  inline void write_cmd(std::uint16_t cmd) const
  {
    const std::uint8_t packet[4] = { PREAMBLE_COMMAND, 0x00U, static_cast<std::uint8_t>(cmd >> 8U), static_cast<std::uint8_t>(cmd) };
    wait_ready();
    io::write(device_handle_, OPT_COMMAND, packet, 4U, nullptr, 0U);
  }


  /**
   * Send a data packet with one word (command argument, register value)
   */
  inline void write_word(std::uint16_t data) const
  {
    const std::uint8_t packet[4] = { PREAMBLE_DATA, 0x00U, static_cast<std::uint8_t>(data >> 8U), static_cast<std::uint8_t>(data) };
    wait_ready();
    io::write(device_handle_, OPT_DATA, packet, 4U, nullptr, 0U);
  }


  /**
   * Read data words in one packet (the io layer sends the read preamble and drops the dummy word)
   * \param data Receives the words
   * \param count Word count, max. 20
   * \return False if the io layer can't read
   */
  bool read_words(std::uint16_t* data, std::size_t count) const
  {
    std::uint8_t buffer[40];
    std::size_t  len = count * 2U;
    wait_ready();
    if (!io::read(device_handle_, OPT_DATA, buffer, len, 0U) || (len != count * 2U)) {
      return false;
    }
    for (std::size_t i = 0U; i < count; ++i) {
      data[i] = static_cast<std::uint16_t>((static_cast<std::uint16_t>(buffer[i * 2U]) << 8U) | buffer[i * 2U + 1U]);
    }
    return true;
  }


  inline void write_reg(std::uint16_t addr, std::uint16_t value) const
  {
    write_cmd(CMD_REG_WR);
    write_word(addr);
    write_word(value);
  }


  inline bool read_reg(std::uint16_t addr, std::uint16_t& value) const
  {
    write_cmd(CMD_REG_RD);
    write_word(addr);
    return read_words(&value, 1U);
  }

};

} // namespace head
} // namespace vic

#endif  // _VIC_HEAD_IT8951_H_
