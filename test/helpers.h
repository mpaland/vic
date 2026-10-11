///////////////////////////////////////////////////////////////////////////////
// \brief Test helpers: pixel raster assertions for the memory head
//
// A raster is given as ASCII-art rows, e.g.
//   CHECK_RASTER(head, PAL_X, ".XX.",
//                             "X..X",
//                             ".XX.");
// where '.' maps to the palette background (color::none by default) and every
// other char maps to a palette color. All rows must have the same length.
// In exact mode (default) every pixel of the head outside the raster must equal
// the background, so drawing outside the expected area is detected.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_TEST_HELPERS_H_
#define _VIC_TEST_HELPERS_H_

#include <cstdio>
#include <cstring>
#include <initializer_list>

#include "vic_test.h"
#include "util.h"
#include "color.h"
#include "shader/base.h"
#include "head/memory.h"


namespace vic_test {


// raster palette
struct palette
{
  struct entry {
    char                    ch;
    vic::color::value_type  color;
  };

  entry                   e[8];
  std::size_t             n;
  vic::color::value_type  background;

  bool lookup(char ch, vic::color::value_type& c) const
  {
    if (ch == '.') {
      c = background;
      return true;
    }
    for (std::size_t i = 0U; i < n; ++i) {
      if (e[i].ch == ch) {
        c = e[i].color;
        return true;
      }
    }
    return false;
  }

  char reverse(vic::color::value_type c) const
  {
    if (c == background) {
      return '.';
    }
    for (std::size_t i = 0U; i < n; ++i) {
      if (e[i].color == c) {
        return e[i].ch;
      }
    }
    return '?';
  }
};

// stock palettes
static const palette PAL_X  = { { { 'X', vic::color::white } }, 1U, vic::color::none };
static const palette PAL_RG = { { { 'R', vic::color::red }, { 'G', vic::color::green }, { 'B', vic::color::blue }, { 'W', vic::color::white } }, 4U, vic::color::none };


// print the complete head content as raster
template<typename Head>
inline void dump_raster(const Head& head, const palette& pal, const char* title)
{
  std::printf("    %s (%ux%u):\n", title, static_cast<unsigned>(head.screen_width()), static_cast<unsigned>(head.screen_height()));
  for (std::int16_t y = 0; y < static_cast<std::int16_t>(head.screen_height()); ++y) {
    std::printf("    ");
    for (std::int16_t x = 0; x < static_cast<std::int16_t>(head.screen_width()); ++x) {
      std::printf("%c", pal.reverse(head.pixel({ x, y })));
    }
    std::printf("\n");
  }
}


// compare the head content against the raster rows
template<typename Head>
inline bool check_raster(const char* file, int line, const Head& head,
                         std::initializer_list<const char*> rows, const palette& pal,
                         vic::vertex_type origin = { 0, 0 }, bool exact = true)
{
  ++ctx().checks;

  const std::int16_t height = static_cast<std::int16_t>(rows.size());
  const std::int16_t width  = height ? static_cast<std::int16_t>(std::strlen(*rows.begin())) : 0;

  bool ok = true;
  std::int16_t ry = 0;
  for (std::initializer_list<const char*>::const_iterator it = rows.begin(); it != rows.end(); ++it, ++ry) {
    const char* row = *it;
    if (static_cast<std::int16_t>(std::strlen(row)) != width) {
      fail_header(file, line, "raster");
      std::printf("  -- row %d has a different length\n", static_cast<int>(ry));
      return false;
    }
    for (std::int16_t rx = 0; rx < width; ++rx) {
      vic::color::value_type expected;
      if (!pal.lookup(row[rx], expected)) {
        fail_header(file, line, "raster");
        std::printf("  -- unknown palette char '%c'\n", row[rx]);
        return false;
      }
      const vic::vertex_type v = { static_cast<std::int16_t>(origin.x + rx), static_cast<std::int16_t>(origin.y + ry) };
      if (head.pixel(v) != expected) {
        ok = false;
      }
    }
  }

  if (ok && exact) {
    // everything outside the raster must be background
    for (std::int16_t y = 0; y < static_cast<std::int16_t>(head.screen_height()); ++y) {
      for (std::int16_t x = 0; x < static_cast<std::int16_t>(head.screen_width()); ++x) {
        const bool inside = (x >= origin.x) && (x < origin.x + width) && (y >= origin.y) && (y < origin.y + height);
        if (!inside && (head.pixel({ x, y }) != pal.background)) {
          ok = false;
        }
      }
    }
  }

  if (!ok) {
    fail_header(file, line, "raster mismatch");
    std::printf("\n    expected (origin %d,%d):\n", static_cast<int>(origin.x), static_cast<int>(origin.y));
    for (std::initializer_list<const char*>::const_iterator it = rows.begin(); it != rows.end(); ++it) {
      std::printf("    %s\n", *it);
    }
    dump_raster(head, pal, "actual");
  }
  return ok;
}


// count pixels of the given color
template<typename Head>
inline std::size_t count_pixels(const Head& head, vic::color::value_type c)
{
  std::size_t cnt = 0U;
  for (std::int16_t y = 0; y < static_cast<std::int16_t>(head.screen_height()); ++y) {
    for (std::int16_t x = 0; x < static_cast<std::int16_t>(head.screen_width()); ++x) {
      if (head.pixel({ x, y }) == c) {
        ++cnt;
      }
    }
  }
  return cnt;
}


// count pixels which are NOT of the given color
template<typename Head>
inline std::size_t count_pixels_not(const Head& head, vic::color::value_type c)
{
  return static_cast<std::size_t>(head.screen_width()) * head.screen_height() - count_pixels(head, c);
}


// check that the given pixels have the given colors
template<typename Head>
inline bool check_pixels(const char* file, int line, const Head& head, std::initializer_list<vic::pixel_type> pixels)
{
  ++ctx().checks;
  bool ok = true;
  for (std::initializer_list<vic::pixel_type>::const_iterator it = pixels.begin(); it != pixels.end(); ++it) {
    const vic::color::value_type actual = head.pixel(it->vertex);
    if (actual != it->color) {
      if (ok) {
        fail_header(file, line, "pixels");
        std::printf("\n");
      }
      ok = false;
      std::printf("    pixel (%d,%d): expected 0x%08X, actual 0x%08X\n", static_cast<int>(it->vertex.x), static_cast<int>(it->vertex.y),
                  static_cast<unsigned>(it->color), static_cast<unsigned>(actual));
    }
  }
  return ok;
}


// check that two heads have identical content
template<typename Head1, typename Head2>
inline bool check_same_buffer(const char* file, int line, const Head1& a, const Head2& b, const palette& pal = PAL_X)
{
  ++ctx().checks;
  if ((a.screen_width() != b.screen_width()) || (a.screen_height() != b.screen_height())) {
    fail_header(file, line, "same buffer");
    std::printf("  -- different screen sizes\n");
    return false;
  }
  bool ok = true;
  for (std::int16_t y = 0; y < static_cast<std::int16_t>(a.screen_height()); ++y) {
    for (std::int16_t x = 0; x < static_cast<std::int16_t>(a.screen_width()); ++x) {
      if (a.pixel({ x, y }) != b.pixel({ x, y })) {
        ok = false;
      }
    }
  }
  if (!ok) {
    fail_header(file, line, "same buffer");
    std::printf("\n");
    dump_raster(a, pal, "head a");
    dump_raster(b, pal, "head b");
  }
  return ok;
}


// a shader that just passes all pixels, forces the dc to use the gpr path instead of the head fast path
class passthrough_shader : public vic::shader::base
{
public:
  std::size_t count;

  passthrough_shader()
    : count(0U)
  { }

  virtual void pixel_set(vic::vertex_type vertex, vic::color::value_type color)
  {
    ++count;
    next_->pixel_set(vertex, color);
  }
};


} // namespace vic_test


#define CHECK_RASTER(head, pal, ...)                  vic_test::check_raster(__FILE__, __LINE__, (head), { __VA_ARGS__ }, (pal))
#define CHECK_RASTER_AT(head, pal, origin, ...)       vic_test::check_raster(__FILE__, __LINE__, (head), { __VA_ARGS__ }, (pal), (origin))
#define CHECK_RASTER_PART(head, pal, origin, ...)     vic_test::check_raster(__FILE__, __LINE__, (head), { __VA_ARGS__ }, (pal), (origin), false)
#define CHECK_PIXELS(head, ...)                       vic_test::check_pixels(__FILE__, __LINE__, (head), { __VA_ARGS__ })
#define CHECK_SAME_BUFFER(a, b)                       vic_test::check_same_buffer(__FILE__, __LINE__, (a), (b))

#endif  // _VIC_TEST_HELPERS_H_
