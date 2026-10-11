///////////////////////////////////////////////////////////////////////////////
// \brief Tests for the generated TTF fonts (tools/fontgen.py): data consistency
// of the font structures and rendering through the text renderer
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "helpers.h"
#include "dc.h"
#include "tc.h"
#include "head/memory.h"
#include "font/Montserrat_10_aa2.h"
#include "font/Montserrat_12_aa2.h"
#include "font/Montserrat_16_aa4.h"
#include "font/Montserrat_24_aa4.h"
#include "font/Microgramma_10_aa2.h"
#include "font/Microgramma_12_aa2.h"
#include "font/Microgramma_16_aa4.h"
#include "font/Microgramma_24_aa4.h"

using namespace vic;

namespace {

typedef head::memory<96, 40> head_type;

struct font_entry {
  const font::font_type* font;
  const char*            name;
  std::uint8_t           size;
  std::uint8_t           bpp;
};

font::Montserrat_10_aa2  montserrat_10;
font::Montserrat_12_aa2  montserrat_12;
font::Montserrat_16_aa4  montserrat_16;
font::Montserrat_24_aa4  montserrat_24;
font::Microgramma_10_aa2 microgramma_10;
font::Microgramma_12_aa2 microgramma_12;
font::Microgramma_16_aa4 microgramma_16;
font::Microgramma_24_aa4 microgramma_24;

const font_entry fonts[] = {
  { &montserrat_10,  "Montserrat_10_aa2",  10U, 2U },
  { &montserrat_12,  "Montserrat_12_aa2",  12U, 2U },
  { &montserrat_16,  "Montserrat_16_aa4",  16U, 4U },
  { &montserrat_24,  "Montserrat_24_aa4",  24U, 4U },
  { &microgramma_10, "Microgramma_10_aa2", 10U, 2U },
  { &microgramma_12, "Microgramma_12_aa2", 12U, 2U },
  { &microgramma_16, "Microgramma_16_aa4", 16U, 4U },
  { &microgramma_24, "Microgramma_24_aa4", 24U, 4U },
};
const std::size_t font_count = sizeof(fonts) / sizeof(fonts[0]);

// find a glyph in the prop_ext chain
const font::charinfo_ext_type* find_glyph(const font::info_type& info, std::uint16_t code)
{
  for (const font::prop_ext_type* b = info.family.prop_ext; !!b; b = b->next) {
    if ((code >= b->first) && (code <= b->last)) {
      return &b->char_info_ext[code - b->first];
    }
  }
  return nullptr;
}

// height of the rendered pixels (rows with any pixel)
std::int16_t rendered_height(const head_type& h)
{
  std::int16_t top = -1, bottom = -1;
  for (std::int16_t y = 0; y < static_cast<std::int16_t>(h.screen_height()); ++y) {
    for (std::int16_t x = 0; x < static_cast<std::int16_t>(h.screen_width()); ++x) {
      if (h.pixel({ x, y }) != color::none) {
        if (top < 0) { top = y; }
        bottom = y;
      }
    }
  }
  return top < 0 ? 0 : static_cast<std::int16_t>(bottom - top + 1);
}

} // namespace


TEST_CASE(fonts_consistency)
{
  for (std::size_t f = 0U; f < font_count; ++f) {
    const font::info_type& info = fonts[f].font->info();
    const std::uint8_t aa = fonts[f].bpp == 2U ? font::AA_2 : font::AA_4;
    CHECK_MSG(info.attr == (aa | font::ENCODING_UNICODE | font::TYPE_PROP), fonts[f].name);
    CHECK_MSG(info.ysize == fonts[f].size, fonts[f].name);
    CHECK_MSG(info.ydist == fonts[f].size, fonts[f].name);
    CHECK_MSG(info.baseline > 0U && info.baseline <= info.ysize, fonts[f].name);

    // walk the blocks: ordered, non overlapping, consistent glyphs
    std::size_t glyphs = 0U;
    std::int32_t last_code = -1;
    for (const font::prop_ext_type* b = info.family.prop_ext; !!b; b = b->next) {
      CHECK_MSG(b->first <= b->last, fonts[f].name);
      CHECK_MSG(static_cast<std::int32_t>(b->first) > last_code, fonts[f].name);
      last_code = b->last;
      for (std::uint32_t c = b->first; c <= b->last; ++c) {
        const font::charinfo_ext_type& g = b->char_info_ext[c - b->first];
        CHECK_MSG(g.data != nullptr, fonts[f].name);
        CHECK_MSG(g.xdist > 0U, fonts[f].name);
        CHECK_MSG(static_cast<std::int32_t>(g.xpos) + g.xsize <= 127, fonts[f].name);
        CHECK_MSG(g.ypos >= 0, fonts[f].name);
        CHECK_MSG(static_cast<std::uint32_t>(g.ypos) + g.ysize <= info.ysize, fonts[f].name);
        ++glyphs;
      }
    }
    // Montserrat: ASCII + Latin-1 minus the glyphs the font lacks, Microgramma: ASCII only
    const bool latin1 = fonts[f].name[1] == 'o';
    CHECK_MSG(glyphs >= (latin1 ? 180U : 80U), fonts[f].name);    // Microgramma lacks 13 ASCII glyphs

    // 'A', 'g' and space are present (space is empty but advances), the umlauts in the Latin-1 fonts
    CHECK_MSG(find_glyph(info, 'A') != nullptr, fonts[f].name);
    CHECK_MSG(find_glyph(info, 'g') != nullptr, fonts[f].name);
    CHECK_MSG((find_glyph(info, 0x00E4U) != nullptr) == latin1, fonts[f].name);
    const font::charinfo_ext_type* space = find_glyph(info, ' ');
    REQUIRE(space != nullptr);
    CHECK_MSG(space->xsize == 0U && space->xdist > 0U, fonts[f].name);
    // 'A' sits on the baseline, 'g' reaches at least as deep (Microgramma has no descenders)
    const font::charinfo_ext_type* a = find_glyph(info, 'A');
    const font::charinfo_ext_type* g = find_glyph(info, 'g');
    CHECK_MSG(a->ypos + a->ysize == info.baseline, fonts[f].name);
    CHECK_MSG(g->ypos + g->ysize >= info.baseline, fonts[f].name);
  }
}


TEST_CASE(fonts_render)
{
  std::int16_t height_10 = 0, height_24 = 0;
  for (std::size_t f = 0U; f < font_count; ++f) {
    head_type h;
    dc d(h);
    tc t(d);
    t.set_font(*fonts[f].font);
    t.set_color(color::white);
    CHECK_MSG(t.char_height() == fonts[f].size, fonts[f].name);
    CHECK_MSG(t.char_width() > 0U, fonts[f].name);

    // "Ag" renders inside its extent
    rect_type r;
    CHECK_EQ(t.get_extend(r, (const std::uint8_t*)"Ag"), 2U);
    CHECK_MSG(r.bottom == fonts[f].size, fonts[f].name);
    CHECK_MSG(r.right > 0 && r.right < 60, fonts[f].name);
    t.set_pos({ 4, 4 });
    t.out((const std::uint8_t*)"Ag");
    CHECK_MSG(vic_test::count_pixels_not(h, color::none) > 10U, fonts[f].name);
    bool inside = true, has_partial = false, has_full = false;
    for (std::int16_t y = 0; y < 40; ++y) {
      for (std::int16_t x = 0; x < 96; ++x) {
        const color::value_type c = h.pixel({ x, y });
        if (c == color::none) {
          continue;
        }
        if ((y < 4) || (y >= 4 + fonts[f].size) || (x < 2) || (x >= 4 + r.right + 2)) {
          inside = false;
        }
        // the AA pixels are the pen color with a reduced alpha
        const std::uint8_t alpha = color::get_alpha(c);
        if (alpha == 255U) { has_full = true; } else if (alpha > 0U) { has_partial = true; }
      }
    }
    CHECK_MSG(inside, fonts[f].name);
    CHECK_MSG(has_partial, fonts[f].name);
    if (fonts[f].size >= 16U) {
      // the strokes of the small 2 bit fonts are thinner than a pixel, so only the big fonts must have solid pixels
      CHECK_MSG(has_full, fonts[f].name);
    }

    // the rendered height of 'A' grows with the font size
    d.cls();
    t.set_pos({ 4, 4 });
    t.out(static_cast<std::uint16_t>('A'));
    if (fonts[f].size == 10U) { height_10 = rendered_height(h); }
    if (fonts[f].size == 24U) { height_24 = rendered_height(h); }
  }
  CHECK(height_10 >= 6 && height_10 <= 10);
  CHECK(height_24 >= 14 && height_24 <= 24);
  CHECK(height_24 > height_10);
}


TEST_CASE(fonts_utf8_umlaut)
{
  // the Latin-1 block is reachable through UTF-8 text
  head_type h;
  dc d(h);
  tc t(d);
  t.set_font(montserrat_12);
  t.set_color(color::white);
  t.set_pos({ 2, 2 });
  CHECK_EQ(t.out((const std::uint8_t*)"\xC3\xA4\xC3\x96"), 2U);    // ä Ö
  CHECK(vic_test::count_pixels_not(h, color::none) > 10U);
}
