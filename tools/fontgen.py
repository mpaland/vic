#!/usr/bin/env python3
###############################################################################
# \author (c) Marco Paland (info@paland.com)
#             2014-2026, PALANDesign Hannover, Germany
#
# \license The MIT License (MIT)
#
# This file is part of the vic library.
#
# \brief TTF to vic font header generator
#
# Rasterizes a TrueType/OpenType font with FreeType (freetype-py) into a vic
# proportional UNICODE font header (font::prop_ext_type chain, 1/2/4 bit per
# pixel, see src/font.h and the rendering in src/txr.h).
# The glyphs are rendered with the FreeType auto hinter in normal mode, which
# snaps the stems to the pixel grid in both directions, so a 1 pixel stem is
# one pixel column and not two half gray ones (the default TrueType interpreter
# only hints vertically, which blurs small sizes). Stems thinner than a pixel
# are darkened so that they become solid pixels (--darken).
#
# Usage:
#   python tools/fontgen.py src/font/ttf/Montserrat-Regular.ttf --size 12 --bpp 2
#   python tools/fontgen.py font.ttf --size 24 --bpp 4 --ranges 20-7E,A0-FF --name MyFont_24 --out src/font/MyFont_24.h
#   python tools/fontgen.py font.ttf --size 16 --bpp 4 --preview font16.png   (PNG sheet of the generated glyphs)
#   python tools/fontgen.py --selftest
#
# The line height (--size) is the vic 'ysize' of the font. The largest FreeType
# pixel size whose ascent + descent fits into the line height is used, the
# baseline is reported in the header. Glyphs the TTF doesn't contain are skipped.
#
# Requires Python 3, freetype-py and Pillow (pip install freetype-py pillow),
# Pillow is only needed for the --preview PNG.
#
###############################################################################

import argparse
import os
import random
import sys
import unicodedata

try:
    import freetype
except ImportError:
    sys.exit("fontgen needs freetype-py: pip install freetype-py")


LICENSE = """///////////////////////////////////////////////////////////////////////////////
// \\author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \\license The MIT License (MIT)
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
"""



###############################################################################
# bit packing (the exact layout txr::out() expects)

def bytes_per_row(xsize, bpp):
    """bytes per glyph row: rows are byte aligned, 1 + (xsize - 1) * bpp / 8"""
    return 1 + ((xsize - 1) * bpp) // 8 if xsize > 0 else 1


def pack(levels, xsize, ysize, bpp):
    """pack a row major list of intensity levels (0 .. 2^bpp - 1) MSB first, byte aligned rows"""
    bpr = bytes_per_row(xsize, bpp)
    data = bytearray(bpr * ysize)
    for y in range(ysize):
        for x in range(xsize):
            v = levels[y * xsize + x]
            data[y * bpr + ((x * bpp) >> 3)] |= v << ((8 - (x + 1) * bpp) % 8)
    return bytes(data)


def unpack(data, xsize, ysize, bpp):
    """inverse of pack(), returns the intensity levels"""
    bpr = bytes_per_row(xsize, bpp)
    mask = (1 << bpp) - 1
    return [(data[y * bpr + ((x * bpp) >> 3)] >> ((8 - (x + 1) * bpp) % 8)) & mask
            for y in range(ysize) for x in range(xsize)]


def expand(level, bpp):
    """8 bit intensity of a level, like txr: ((v + 1) << (8 - bpp)) - 1"""
    return ((level + 1) << (8 - bpp)) - 1 if level else 0


def quantize(coverage, bpp):
    """8 bit coverage to a bpp level, rounded"""
    levels = (1 << bpp) - 1
    return (coverage * levels + 127) // 255


###############################################################################
# rasterization

class Glyph(object):
    def __init__(self, code, xsize, ysize, xpos, ypos, xdist, data):
        self.code, self.xsize, self.ysize, self.xpos, self.ypos, self.xdist, self.data = code, xsize, ysize, xpos, ypos, xdist, data


class Face(object):
    """a FreeType face at one pixel size"""
    def __init__(self, path, px, bpp):
        self.face = freetype.Face(path)
        self.face.set_pixel_sizes(0, px)
        self.px = px
        self.family = (self.face.family_name or b'font').decode('latin-1').replace(' ', '')
        self.ascent = self.face.size.ascender // 64              # ascender line above the baseline
        # the auto hinter in normal mode snaps stems horizontally and vertically, mono rendering for 1 bpp
        self.flags = freetype.FT_LOAD_RENDER | freetype.FT_LOAD_FORCE_AUTOHINT | \
                     (freetype.FT_LOAD_TARGET_MONO if bpp == 1 else freetype.FT_LOAD_TARGET_NORMAL)

    def has(self, code):
        return self.face.get_char_index(code) != 0

    def render(self, code):
        """render one char, returns (xsize, ysize, xpos, ypos, coverage, xdist). xpos/ypos are relative to the
        cursor at the top/left of the line (the ascender line is the top), coverage is 0 .. 255 row major,
        xsize/ysize are 0 for an empty glyph"""
        self.face.load_char(code, self.flags)
        g = self.face.glyph
        bmp = g.bitmap
        xdist = int(round(g.advance.x / 64.0))
        if (bmp.width == 0) or (bmp.rows == 0):
            return 0, 0, 0, 0, [], xdist
        cov = []
        for y in range(bmp.rows):
            row = bmp.buffer[y * bmp.pitch:(y + 1) * bmp.pitch]
            if bmp.pixel_mode == freetype.FT_PIXEL_MODE_MONO:
                cov.extend(255 if (row[x >> 3] >> (7 - (x & 7))) & 1 else 0 for x in range(bmp.width))
            else:
                cov.extend(row[:bmp.width])
        return bmp.width, bmp.rows, g.bitmap_left, self.ascent - g.bitmap_top, cov, xdist


def ink_box(face, codes):
    """vertical ink extent (top, bottom) of the chars relative to the ascender line"""
    top, bottom = None, None
    for code in codes:
        if not face.has(code):
            continue
        xsize, ysize, xpos, ypos, cov, xdist = face.render(code)
        if ysize == 0:
            continue
        top = ypos if top is None else min(top, ypos)
        bottom = ypos + ysize if bottom is None else max(bottom, ypos + ysize)
    return top, bottom


def fit_font(path, size, bpp, codes):
    """largest FreeType pixel size whose glyphs (ink of the whole charset) fit into the line height.
    The font metrics (ascent + descent) of many TTFs contain a lot of leading, fitting the real ink
    gives considerably bigger glyphs for a given line height.
    Returns (face, pad): pad is the offset of the ascender line from the top of the line box"""
    for px in range(size * 2, 3, -1):
        face = Face(path, px, bpp)
        top, bottom = ink_box(face, codes)
        if (top is not None) and (bottom - top <= size):
            # center the ink in the line box
            pad = (size - (bottom - top)) // 2 - top
            return face, pad
    sys.exit("font doesn't fit into %d pixel" % size)


def stem_darkening(face, darken):
    """coverage scale factor so that a vertical stem becomes a solid pixel.
    At small sizes the stems of a regular weight are thinner than a pixel (e.g. 0.7 px), the hinter puts
    them into one pixel column but the coverage stays at 70 %, which quantizes to a dimmed gray level.
    The factor is the inverse of the peak coverage of the 'l' (or 'I', '|') stem, max 2.0; 1.0 for fonts
    whose stems already fill a pixel"""
    if darken != 'auto':
        return float(darken)
    for code in (ord('l'), ord('I'), ord('|'), ord('1')):
        if face.has(code):
            xsize, ysize, xpos, ypos, cov, xdist = face.render(code)
            if cov and max(cov) > 0:
                return min(2.0, 255.0 / max(cov))
    return 1.0


def rasterize(path, size, bpp, ranges, darken='auto'):
    codes = [c for lo, hi in ranges for c in range(lo, hi + 1)]
    face, pad = fit_font(path, size, bpp, codes)
    factor = stem_darkening(face, darken)
    glyphs = []
    missing = []
    clipped = 0
    for code in codes:
        if not face.has(code):
            missing.append(code)    # glyph not in the font
            continue
        xsize, ysize, xpos, ypos, cov, xdist = face.render(code)
        if factor != 1.0:
            cov = [min(255, int(c * factor + 0.5)) for c in cov]
        if ysize == 0:
            if code not in (0x20, 0xA0):
                # not a space but no ink: an empty glyph, skip it
                missing.append(code)
                continue
            glyphs.append(Glyph(code, 0, 0, 0, 0, xdist, b'\x00'))
            continue
        ypos += pad
        # clip rows outside the line box (accents above the ascender, deep descenders)
        rows = [cov[y * xsize:(y + 1) * xsize] for y in range(ysize)]
        if ypos < 0:
            rows = rows[-ypos:]
            ypos = 0
            clipped += 1
        if ypos + len(rows) > size:
            rows = rows[:size - ypos]
            clipped += 1
        ysize = len(rows)
        if ysize == 0:
            glyphs.append(Glyph(code, 0, 0, 0, 0, xdist, b'\x00'))
            continue
        if (xpos < -128) or (xpos + xsize > 127):
            sys.exit("glyph %04X exceeds the int8 x range" % code)
        if xdist > 255:
            sys.exit("glyph %04X advance %d exceeds 255" % (code, xdist))
        levels = [quantize(c, bpp) for row in rows for c in row]
        glyphs.append(Glyph(code, xsize, ysize, xpos, ypos, xdist, pack(levels, xsize, ysize, bpp)))
    return glyphs, face.ascent + pad, clipped, missing, face, factor


def blocks_of(glyphs):
    """consecutive codes form one prop_ext block, returns [(first_index, last_index)]"""
    blocks = []
    start = 0
    for i in range(1, len(glyphs) + 1):
        if (i == len(glyphs)) or (glyphs[i].code != glyphs[i - 1].code + 1):
            blocks.append((start, i - 1))
            start = i
    return blocks


###############################################################################
# header output

def char_name(code):
    try:
        return unicodedata.name(chr(code))
    except ValueError:
        return ''


def write_header(path, name, glyphs, size, bpp, baseline, ttf, family):
    aa = { 1: 'AA_NONE', 2: 'AA_2', 4: 'AA_4' }[bpp]
    guard = '_VIC_FONT_%s_H_' % name.upper()
    out = []
    w = out.append
    w(LICENSE)
    w('// \\brief %s, %d pixel line height, proportional, UNICODE, %d bit per pixel\n' % (family, size, bpp))
    w('// Generated by tools/fontgen.py from %s, do not edit. The license of the\n' % os.path.basename(ttf))
    w('// font file applies to the glyph data.\n')
    w('//\n')
    w('///////////////////////////////////////////////////////////////////////////////\n\n')
    w('#ifndef %s\n#define %s\n\n#include "../font.h"\n\n\nnamespace vic {\nnamespace font {\n\n\n' % (guard, guard))
    w('class %s final : public font_type\n{\n  virtual const info_type& info() const\n  {\n' % name)
    # glyph data
    w('    static const std::uint8_t %s_none[1] = { 0x00 };\n\n' % name)
    for g in glyphs:
        if g.xsize == 0:
            continue
        w('    static const std::uint8_t %s_%04X[%d] = { /* code %04X, %s */\n' % (name, g.code, len(g.data), g.code, char_name(g.code)))
        bpr = bytes_per_row(g.xsize, bpp)
        for y in range(g.ysize):
            row = g.data[y * bpr:(y + 1) * bpr]
            w('      ' + ', '.join('0x%02X' % b for b in row) + (',' if y < g.ysize - 1 else '') + '\n')
        w('    };\n\n')
    # char info
    w('    static const charinfo_ext_type %s_charinfo[%d] = {\n' % (name, len(glyphs)))
    for i, g in enumerate(glyphs):
        data = ('%s_%04X' % (name, g.code)) if g.xsize else ('%s_none' % name)
        w('      { %3d, %3d, %4d, %4d, %3d, %s }%s /* code %04X, %s */\n' %
          (g.xsize, g.ysize, g.xpos, g.ypos, g.xdist, data, ',' if i < len(glyphs) - 1 else ' ', g.code, char_name(g.code)))
    w('    };\n\n')
    # blocks, the last block first so that the next pointers are defined
    blocks = blocks_of(glyphs)
    for k in range(len(blocks), 0, -1):
        first, last = blocks[k - 1]
        nxt = ('&%s_block%d' % (name, k + 1)) if k < len(blocks) else 'nullptr'
        w('    static const prop_ext_type %s_block%d = {\n' % (name, k))
        w('      0x%04X,                            // first character\n' % glyphs[first].code)
        w('      0x%04X,                            // last character\n' % glyphs[last].code)
        w('      &%s_charinfo[%d],%s// address of the first character\n' % (name, first, ' ' * max(1, 20 - len(name) - len(str(first)))))
        w('      %s%s// next block\n' % (nxt, ' ' * max(1, 35 - len(nxt))))
        w('    };\n\n')
    w('    static const info_type %s_info = {\n' % name)
    w('      %s | ENCODING_UNICODE | TYPE_PROP,%s// type of font\n' % (aa, ' ' * max(1, 11 - len(aa))))
    w('      %dU,%s// height of font (line)\n' % (size, ' ' * (37 - len(str(size)))))
    w('      %dU,%s// y distance (line advance)\n' % (size, ' ' * (37 - len(str(size)))))
    w('      %dU,%s// baseline\n' % (baseline, ' ' * (37 - len(str(baseline)))))
    w('      &%s_block1\n' % name)
    w('    };\n\n')
    w('    return %s_info;\n  }\n};\n\n\n} // namespace font\n} // namespace vic\n\n#endif  // %s\n' % (name, guard))
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(''.join(out))


###############################################################################
# preview

def write_preview(path, glyphs, size, bpp, baseline, scale=3, per_row=32):
    """PNG sheet of the glyphs decoded from the packed data, so it shows exactly what the header holds"""
    try:
        from PIL import Image
    except ImportError:
        sys.exit("the preview needs Pillow: pip install pillow")
    cell_w = max([g.xdist for g in glyphs] + [1]) + 2
    rows = (len(glyphs) + per_row - 1) // per_row
    im = Image.new('L', (per_row * cell_w, rows * (size + 2)), 40)
    px = im.load()
    for i, g in enumerate(glyphs):
        cx = (i % per_row) * cell_w + 1
        cy = (i // per_row) * (size + 2) + 1
        for x in range(g.xdist):            # line box of the glyph
            for y in range(size):
                px[cx + x, cy + y] = 64 if y != baseline else 96
        if g.xsize:
            levels = unpack(g.data, g.xsize, g.ysize, bpp)
            for y in range(g.ysize):
                for x in range(g.xsize):
                    v = expand(levels[y * g.xsize + x], bpp)
                    if v:
                        X, Y = cx + g.xpos + x, cy + g.ypos + y
                        if 0 <= X < im.width and 0 <= Y < im.height:
                            px[X, Y] = max(px[X, Y], v)
    im = im.resize((im.width * scale, im.height * scale), Image.NEAREST)
    im.save(path)


###############################################################################
# self test

def selftest():
    rnd = random.Random(1)
    for bpp in (1, 2, 4):
        for _ in range(200):
            xsize, ysize = rnd.randint(1, 40), rnd.randint(1, 40)
            levels = [rnd.randint(0, (1 << bpp) - 1) for _ in range(xsize * ysize)]
            assert unpack(pack(levels, xsize, ysize, bpp), xsize, ysize, bpp) == levels
            assert len(pack(levels, xsize, ysize, bpp)) == bytes_per_row(xsize, bpp) * ysize
    # the expansion matches txr: full level is 255, zero stays zero
    for bpp in (1, 2, 4):
        assert expand((1 << bpp) - 1, bpp) == 255 and expand(0, bpp) == 0
    assert quantize(255, 2) == 3 and quantize(0, 4) == 0 and quantize(128, 1) == 1 and quantize(100, 1) == 0
    # the Arial_std_12_aa2 '"' glyph: 3 x 2 pixel, rows 0xCC 0xC8 decode to 3,0,3 / 3,0,2
    assert unpack(b'\xCC\xC8', 3, 2, 2) == [3, 0, 3, 3, 0, 2]
    print('selftest ok')


###############################################################################

def parse_ranges(text):
    ranges = []
    for part in text.split(','):
        lo, hi = part.split('-') if '-' in part else (part, part)
        ranges.append((int(lo, 16), int(hi, 16)))
    return ranges


def main():
    ap = argparse.ArgumentParser(description='TTF to vic font header generator')
    ap.add_argument('ttf', nargs='?', help='TrueType/OpenType font file')
    ap.add_argument('--size', type=int, help='line height in pixel (vic ysize)')
    ap.add_argument('--bpp', type=int, choices=(1, 2, 4), default=2, help='bit per pixel: 1 (mono), 2 or 4 (anti aliased)')
    ap.add_argument('--ranges', default='20-7E,A0-FF', help='hex code point ranges, default ASCII + Latin-1')
    ap.add_argument('--name', help='class name, default <Family>_<size>_aa<bpp>')
    ap.add_argument('--out', help='output header, default src/font/<name>.h')
    ap.add_argument('--darken', default='auto', help='stem darkening factor: auto (default, stems become solid pixels), 1 = off, or a factor like 1.5')
    ap.add_argument('--preview', help='write a PNG preview sheet of the generated glyphs')
    ap.add_argument('--selftest', action='store_true', help='run the pack/unpack self test')
    args = ap.parse_args()

    if args.selftest:
        selftest()
        return
    if not args.ttf or not args.size:
        ap.error('ttf and --size are required')

    glyphs, baseline, clipped, missing, face, factor = rasterize(args.ttf, args.size, args.bpp, parse_ranges(args.ranges), args.darken)
    name = args.name or '%s_%d_aa%d' % (face.family, args.size, args.bpp)
    out = args.out or os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'src', 'font', name + '.h')
    write_header(out, name, glyphs, args.size, args.bpp, baseline, args.ttf, face.family)
    data = sum(len(g.data) for g in glyphs if g.xsize) + 8 * len(glyphs)
    print('%s: %d glyphs in %d blocks, baseline %d, FreeType size %d, stem darkening %.2f, %d clipped, about %d bytes -> %s' %
          (name, len(glyphs), len(blocks_of(glyphs)), baseline, face.px, factor, clipped, data, os.path.normpath(out)))
    if missing:
        print('  missing in the font, skipped: ' + ' '.join('%04X' % c for c in missing))
    if args.preview:
        write_preview(args.preview, glyphs, args.size, args.bpp, baseline)


if __name__ == '__main__':
    main()
