#!/usr/bin/env python3
###############################################################################
# \author (c) Marco Paland (info@paland.com)
#             2014-2026, PALANDesign Hannover, Germany
#
# \license The MIT License (MIT)
#
# This file is part of the vic library.
#
# \brief TTF to vic vector (outline) font header generator
#
# Extracts the quadratic TrueType outlines of a font (freetype-py, unscaled
# font units) into a vic vector font header (font::vector_type, see
# src/font.h). The font is rasterized at any pixel size at runtime by the text
# renderer (src/txr.h), one font header serves all sizes.
#
# The glyph data of a char starts with the vertical hints and the horizontal
# hints, each as their word count followed by the hint entries, then the
# contours follow: the point count and the points as x, y pairs in font units,
# y pointing up. The LSB of x holds the
# TrueType on-curve flag: x_stored = (x << 1) | on_curve. Like in TrueType, two
# consecutive off-curve points have an implied on-curve point in their middle.
# Composite glyphs (accented chars) are stored decomposed. Contours which are
# moved copies of the last full contour (the dots of a dot matrix font) are
# stored as their moves: a negative count -k followed by k dx, dy pairs.
#
# Vertical hints (autohinting): the horizontal edges of a glyph
# (straight horizontal lines and the flat extrema of curves) are found here,
# a bottom edge (filled above) and a top edge (filled below) close together
# form a stem (the bar of a 'T', the crossbar of an 'e', the top of an 'o').
# The renderer snaps the edges to pixel rows and rounds the stem thickness to
# whole pixels, the points between the edges are interpolated, so horizontal
# strokes are crisp rows instead of two gray half rows. Edges close to a zone
# (baseline, x-height, cap height, ascender, descender) are aligned to the
# zone, so all glyphs share the same rows there, overshoots stay relative.
# Entries, LSB of the first word: 0 = edge (y << 1, zone), 1 = stem
# ((y_bottom << 1) | 1, zone_bottom, y_top, zone_top), sorted by y, zone is
# -32768 for an edge without zone.
# Horizontal hints: the same for x, the straight vertical edges and stems (the
# stems of an 'H') are snapped to pixel columns. Round sides (of an 'o') are
# not, snapping them distorts small round glyphs, they stay anti-aliased. There
# are no zones in x, the entries are: 0 = edge (x << 1), 1 = stem
# ((x_left << 1) | 1, x_right), sorted by x.
#
# The line box is the ink box of the whole charset (like tools/fontgen.py
# does), so a vector font at size N has the glyph size of the N pixel bitmap
# font: 'units' is the line height and 'ascent' the top of the line box above
# the baseline, both in font units.
#
# Usage:
#   python tools/vecfontgen.py src/font/ttf/Montserrat-Regular.ttf
#   python tools/vecfontgen.py font.ttf --ranges 20-7E --name MyFont_vec --out src/font/MyFont_vec.h
#   python tools/vecfontgen.py src/font/ttf/Doto-Variable.ttf --axes ROND=100,wght=700
#   python tools/vecfontgen.py --selftest
#
# Requires Python 3 and freetype-py (pip install freetype-py)
# CFF (cubic) OpenType fonts are not supported, use the TrueType version.
#
###############################################################################

import argparse
import os
import sys
import unicodedata

try:
    import freetype
except ImportError:
    sys.exit("vecfontgen needs freetype-py: pip install freetype-py")


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
# encoding

def moved(base, pts):
    """the move (dx, dy) if the contour pts is a moved copy of the contour base, else None"""
    if not base or len(base) != len(pts):
        return None
    dx, dy = pts[0][0] - base[0][0], pts[0][1] - base[0][1]
    same = all(p[0] - b[0] == dx and p[1] - b[1] == dy and p[2] == b[2] for b, p in zip(base, pts))
    return (dx, dy) if same else None


def encode_items(contours):
    """contours: list of [(x, y, on_curve), ...] -> the int16 words of the contours: a full contour (count,
    points) or a run of moved copies of the last full contour (-count, moves), one list per item"""
    items, base = [], None
    for pts in contours:
        move = moved(base, pts)
        if move is None:
            base = pts
            items.append([len(pts)] + [v for x, y, on in pts for v in ((x << 1) | (1 if on else 0), y)])
        elif items[-1][0] < 0:
            items[-1][0] -= 1
            items[-1] += list(move)
        else:
            items.append([-1] + list(move))
    return items


def encode_contours(contours):
    return [v for item in encode_items(contours) for v in item]


def decode_contours(words):
    """inverse of encode_contours()"""
    contours, base = [], None
    i = 0
    while i < len(words):
        n = words[i]
        i += 1
        if n < 0:
            for _ in range(-n):
                contours.append([(x + words[i], y + words[i + 1], on) for x, y, on in base])
                i += 2
            continue
        pts = []
        for _ in range(n):
            xs, y = words[i], words[i + 1]
            on = xs & 1
            pts.append(((xs - on) // 2, y, bool(on)))
            i += 2
        contours.append(pts)
        base = pts
    return contours


###############################################################################
# vertical hints

NO_ZONE = -32768


class Edge(object):
    def __init__(self, y, top, x0, x1):
        self.y, self.top, self.x0, self.x1 = y, top, x0, x1


def segments(pts):
    """explicit segments of a TrueType contour: ('L', p0, p1) and ('Q', p0, ctrl, p1), the implied on-curve points resolved"""
    def mid(a, b):
        return ((a[0] + b[0]) / 2.0, (a[1] + b[1]) / 2.0)
    if pts[0][2]:
        start, seq = pts[0][:2], pts[1:]
    elif pts[-1][2]:
        start, seq = pts[-1][:2], pts[:-1]
    else:
        start, seq = mid(pts[0], pts[-1]), pts
    segs, cur, ctrl = [], start, None
    for x, y, on in seq:
        if on:
            segs.append(('Q', cur, ctrl, (x, y)) if ctrl else ('L', cur, (x, y)))
            cur, ctrl = (x, y), None
        else:
            if ctrl:
                m = mid(ctrl, (x, y))
                segs.append(('Q', cur, ctrl, m))
                cur = m
            ctrl = (x, y)
    segs.append(('Q', cur, ctrl, start) if ctrl else ('L', cur, start))
    return [sg for sg in segs if tuple(sg[1]) != tuple(sg[-1])]


def flatten(pts, steps=8):
    """polygon of a TrueType contour, the curves flattened"""
    poly = []
    for sg in segments(pts):
        if sg[0] == 'Q':
            p0, c, p1 = sg[1], sg[2], sg[3]
            for k in range(steps):
                t = k / float(steps)
                poly.append(((1 - t) ** 2 * p0[0] + 2 * (1 - t) * t * c[0] + t * t * p1[0],
                             (1 - t) ** 2 * p0[1] + 2 * (1 - t) * t * c[1] + t * t * p1[1]))
        else:
            poly.append(sg[1])
    return poly


def filled(polys, x, y):
    """nonzero winding test of a point"""
    w = 0
    for poly in polys:
        for k in range(len(poly)):
            (x0, y0), (x1, y1) = poly[k], poly[(k + 1) % len(poly)]
            if (y0 <= y) != (y1 <= y) and x0 + (y - y0) * (x1 - x0) / (y1 - y0) > x:
                w += 1 if y1 > y0 else -1
    return w != 0


def horizontal(dx, dy):
    return dx != 0 and abs(dy) <= 0.05 * abs(dx)


def glyph_edges(contours, em, extrema=True):
    """the horizontal edges of a glyph: straight lines and (extrema) curve extrema with a horizontal tangent"""
    # TrueType: outer contours are clockwise, the fill is right of the direction of travel
    area = 0.0
    for pts in contours:
        for k in range(len(pts)):
            x0, y0 = pts[k][:2]
            x1, y1 = pts[(k + 1) % len(pts)][:2]
            area += x0 * y1 - x1 * y0
    flip = area > 0     # counter clockwise outer contours (PostScript orientation)
    edges = []
    for pts in contours:
        segs = segments(pts)
        for sg in segs:
            if sg[0] == 'L':
                (x0, y0), (x1, y1) = sg[1], sg[2]
                if abs(x1 - x0) >= 0.02 * em and horizontal(x1 - x0, y1 - y0):
                    edges.append(Edge((y0 + y1) / 2.0, (x1 > x0) != flip, min(x0, x1), max(x0, x1)))
        for k in range(len(segs)):
            a, b = segs[k], segs[(k + 1) % len(segs)]
            if not extrema or (a[0] == 'L' and b[0] == 'L'):
                continue
            p = a[-1]
            pin = a[2] if a[0] == 'Q' else a[1]     # control point (or start) before p
            pout = b[2]                             # control point (or end) after p
            tin, tout = (p[0] - pin[0], p[1] - pin[1]), (pout[0] - p[0], pout[1] - p[1])
            if horizontal(*tin) and horizontal(*tout) and tin[0] * tout[0] > 0:
                edges.append(Edge(p[1], (tin[0] > 0) != flip, p[0] - 0.05 * em, p[0] + 0.05 * em))
    # merge edges at about the same height and side (a stem side with a kink), the longer one gives the height
    merged = []
    for e in sorted(edges, key=lambda e: (e.top, e.y)):
        m = merged[-1] if merged else None
        if m and m.top == e.top and abs(m.y - e.y) <= 0.01 * em:
            m.y = m.y if m.x1 - m.x0 >= e.x1 - e.x0 else e.y
            m.x0, m.x1 = min(m.x0, e.x0), max(m.x1, e.x1)
        else:
            merged.append(e)
    # drop the edges inside the fill of overlapping contours (the hidden ends of the crossbar of an 'H'): no empty side
    polys = [flatten(pts) for pts in contours]
    d = max(1.0, 0.005 * em)

    def visible(e):
        return any(not filled(polys, e.x0 + f * (e.x1 - e.x0), e.y + (d if e.top else -d)) for f in (0.1, 0.3, 0.5, 0.7, 0.9))
    return [e for e in merged if visible(e)]


def font_zones(face):
    """(top zones, bottom zones) in font units: x-height, cap height, ascender / baseline, descender"""
    def bbox(code):
        index = face.get_char_index(code)
        if index == 0:
            return None
        face.load_glyph(index, freetype.FT_LOAD_NO_SCALE)
        b = face.glyph.outline.get_bbox()
        return b.yMin, b.yMax
    tops, bottoms = [], [0]
    for code, top in ((ord('x'), True), (ord('H'), True), (ord('d'), True), (ord('p'), False)):
        b = bbox(code)
        if b:
            (tops if top else bottoms).append(b[1] if top else b[0])
    return sorted(set(tops)), sorted(set(bottoms))


def zone_of(e, zones, em):
    """the zone of an edge (the nearest within 3% em), NO_ZONE if there is none"""
    tops, bottoms = zones or ([], [])
    near = [z for z in (tops if e.top else bottoms) if abs(e.y - z) <= 0.03 * em]
    return min(near, key=lambda z: abs(e.y - z)) if near else NO_ZONE


def glyph_stems(contours, em, extrema=True):
    """the stems (bottom edge, top edge) and the unpaired edges of a glyph"""
    edges = glyph_edges(contours, em, extrema)
    # stems: a bottom edge (filled above) with the nearest top edge (filled below) above it, overlapping in x
    pairs = []
    for b in edges:
        for t in edges:
            if not b.top and t.top and 0 < t.y - b.y <= 0.2 * em and b.x0 < t.x1 and t.x0 < b.x1:
                pairs.append((t.y - b.y, b, t))
    used, stems = set(), []
    for d, b, t in sorted(pairs, key=lambda p: p[0]):
        if id(b) not in used and id(t) not in used:
            used.update((id(b), id(t)))
            stems.append((b, t))
    return stems, [e for e in edges if id(e) not in used]


def glyph_hints(contours, em, zones):
    """the vertical hint words of a glyph, zones None: no zone words and no curve extrema (the x hints)"""
    def zone(e):
        return zone_of(e, zones, em)

    stems, edges = glyph_stems(contours, em, zones is not None)
    # entries without overlaps (the renderer needs increasing y): the zone edges first, then the longest edges
    # (the stem of an 'i' wins over the extrema of its wider dot)
    entries = [(int(round(b.y)), int(round(t.y)), zone(b), zone(t), min(b.x1 - b.x0, t.x1 - t.x0)) for b, t in stems]
    entries += [(int(round(e.y)), int(round(e.y)), zone(e), None, e.x1 - e.x0) for e in edges]
    kept = []
    for en in sorted(entries, key=lambda en: (en[2] == NO_ZONE and en[3] in (None, NO_ZONE), -en[4])):
        if all(en[1] < k[0] or en[0] > k[1] for k in kept):
            kept.append(en)
    words = []
    for yb, yt, zb, zt, _ in sorted(kept):
        if zt is None and yt == yb:
            words += [yb << 1, zb] if zones else [yb << 1]
        else:
            words += [(yb << 1) | 1, zb, yt, zt] if zones else [(yb << 1) | 1, yt]
    return words


def swap(contours):
    """the contours with x and y swapped: the vertical edges become horizontal edges"""
    return [[(y, x, on) for x, y, on in pts] for pts in contours]


def glyph_xhints(contours, em):
    """the horizontal hint words of a glyph: the vertical edges and stems (x and y swapped), without zones"""
    return glyph_hints(swap(contours), em, None)


def std_widths(charset, em, extrema):
    """standard stem widths of the charset (a list of contours): a width within 0.5% em of a more frequent width is
    mapped to it, so similar stems have the same pixel width at every size (74 and 73 units can round to 2 and 1)"""
    count = {}
    for contours in charset:
        for b, t in glyph_stems(contours, em, extrema)[0]:
            w = int(round(t.y - b.y))
            count[w] = count.get(w, 0) + 1
    std = {}
    for w in sorted(count, key=lambda w: (-count[w], w)):
        for v in count:
            if v not in std and abs(v - w) <= 0.005 * em:
                std[v] = w
    return std


def normalize_stems(contours, em, zones, std):
    """the contours with the stems moved to their standard width: the outline points on the free edge of a stem
    (not the zone edge) are moved by a few font units"""
    for b, t in glyph_stems(contours, em, zones is not None)[0]:
        w = int(round(t.y - b.y))
        d = std.get(w, w) - w
        free_top = zone_of(b, zones, em) != NO_ZONE or zone_of(t, zones, em) == NO_ZONE
        e, d = (t, d) if free_top else (b, -d)
        if d and e.y == int(e.y):
            x0, x1 = e.x0 - 0.2 * em, e.x1 + 0.2 * em
            contours = [[(x, y + d if y == e.y and x0 <= x <= x1 else y, on) for x, y, on in pts] for pts in contours]
    return contours


###############################################################################
# outline extraction

class Glyph(object):
    def __init__(self, code, xdist, contours, bbox):
        self.code, self.xdist, self.contours, self.bbox = code, xdist, contours, bbox
        self.set_hints([], [])

    def set_hints(self, hints, xhints):
        """the glyph data: the vertical hints, the horizontal hints (each with its word count), the contours (empty for a space)"""
        self.hints, self.xhints = hints, xhints
        self.words = [len(hints)] + hints + [len(xhints)] + xhints + encode_contours(self.contours) if self.contours else []


def simplify(pts):
    """the contour without redundant points (a variable font keeps them for the interpolation): duplicates, joints of
    2 font units between two curves, on-curve points implied by their off-curve neighbors (within 1/2 font unit)"""
    changed = True
    while changed and len(pts) > 2:
        changed = False
        n = len(pts)
        for k in range(n):
            a, p, b, c = pts[k - 1], pts[k], pts[(k + 1) % n], pts[(k + 2) % n]
            if p[2] and b[2] and p[:2] == b[:2]:
                del pts[k]                                              # duplicate
            elif p[2] and b[2] and not a[2] and not c[2] and abs(p[0] - b[0]) <= 2 and abs(p[1] - b[1]) <= 2:
                pts[k] = ((p[0] + b[0]) // 2, (p[1] + b[1]) // 2, True)  # joint: its middle
                del pts[(k + 1) % n]
            elif p[2] and not a[2] and not b[2] and abs(2 * p[0] - a[0] - b[0]) <= 1 and abs(2 * p[1] - a[1] - b[1]) <= 1:
                del pts[k]                                              # implied
            else:
                continue
            changed = True
            break
    return pts


def extract(path, ranges, axes=None):
    face = freetype.Face(path)
    if axes:
        # instance of a variable font: the design coordinates of the given axes, the others at their default
        info = face.get_variation_info()
        face.set_var_design_coords([axes.get(a.tag, a.default) for a in info.axes])
    family = (face.family_name or b'font').decode('latin-1').replace(' ', '')
    glyphs, missing = [], []
    for lo, hi in ranges:
        for code in range(lo, hi + 1):
            index = face.get_char_index(code)
            if index == 0:
                missing.append(code)
                continue
            face.load_glyph(index, freetype.FT_LOAD_NO_SCALE)
            g = face.glyph
            outline = g.outline
            contours = []
            start = 0
            for end in outline.contours:
                pts = []
                for k in range(start, end + 1):
                    tag = outline.tags[k]
                    if (tag & 3) == 2:
                        sys.exit("%s: cubic (CFF) outlines are not supported, use the TrueType version" % path)
                    x, y = outline.points[k]
                    pts.append((int(x), int(y), bool(tag & 1)))
                contours.append(simplify(pts))
                start = end + 1
            bbox = None
            if contours:
                b = outline.get_bbox()
                bbox = (b.xMin, b.yMin, b.xMax, b.yMax)
            elif code not in (0x20, 0xA0):
                missing.append(code)    # not a space but no ink: an empty glyph, skip it
                continue
            glyphs.append(Glyph(code, int(g.metrics.horiAdvance), contours, bbox))
    return face, family, glyphs, missing


def line_box(glyphs):
    """(units, ascent): line height and the top of the line box in font units, the ink box of the charset"""
    top = max(g.bbox[3] for g in glyphs if g.bbox)
    bottom = min(g.bbox[1] for g in glyphs if g.bbox)
    return top - bottom, top


###############################################################################
# header

def char_name(code):
    try:
        return unicodedata.name(chr(code))
    except ValueError:
        return 'U+%04X' % code


def blocks_of(glyphs):
    """runs of consecutive codes: list of (first index, last index)"""
    blocks = []
    for i, g in enumerate(glyphs):
        if blocks and glyphs[blocks[-1][1]].code + 1 == g.code and g.code != 0x80:
            blocks[-1] = (blocks[-1][0], i)
        else:
            blocks.append((i, i))
    return blocks


EXTENDED = '#if !VIC_FONT_VECTOR_ASCII_ONLY   // the chars beyond ASCII\n'


def data_size(glyphs):
    """about the bytes of the glyph data: the words and the char info"""
    return sum(2 * len(g.words) for g in glyphs) + 8 * len(glyphs)


def write_header(path, name, glyphs, units, ascent, size, ttf, family, axes=None):
    guard = '_VIC_FONT_%s_H_' % name.upper()
    baseline = (ascent * size + units // 2) // units
    out = []
    w = out.append
    w(LICENSE)
    w('// \\brief %s, vector (outline) font, scalable, proportional, UNICODE\n' % family)
    w('// Generated by tools/vecfontgen.py from %s, do not edit. The license of the\n' %
      (os.path.basename(ttf) + (' (%s)' % axes if axes else '')))
    w('// font file applies to the glyph data.\n')
    w('// Select it with set_font(font, size), size is the line height in pixel (default %d)\n' % size)
    w('// VIC_FONT_VECTOR_ASCII_ONLY 1 (vic_cfg.h) keeps the ASCII chars only: about %d instead of %d byte\n' %
      (data_size([g for g in glyphs if g.code < 0x80]), data_size(glyphs)))
    w('//\n')
    w('///////////////////////////////////////////////////////////////////////////////\n\n')
    w('#ifndef %s\n#define %s\n\n#include "../font.h"\n#include "../vic_cfg.h"\n\n\nnamespace vic {\nnamespace font {\n\n\n' % (guard, guard))
    w('class %s final : public font_type\n{\n  virtual const info_type& info() const\n  {\n' % name)
    w('    static const std::int16_t %s_none[1] = { 0 };\n\n' % name)
    ext = False
    for g in glyphs:
        if not g.words:
            continue
        if g.code >= 0x80 and not ext:
            w(EXTENDED)
            ext = True
        w('    static const std::int16_t %s_%04X[%d] = { /* code %04X, %s */\n' % (name, g.code, len(g.words), g.code, char_name(g.code)))
        lines = [', '.join(str(v) for v in [len(h)] + h) for h in (g.hints, g.xhints)]
        for words in encode_items(g.contours):
            # one contour (or run of copies) per line, wrapped after 12 points
            for k in range(0, len(words), 24):
                lines.append(', '.join(str(v) for v in words[k:k + 24]))
        w(',\n'.join('      ' + l for l in lines) + '\n')
        w('    };\n\n')
    if ext:
        w('#endif\n\n')
    w('    static const vector_charinfo_type %s_charinfo[] = {\n' % name)
    ext = False
    for g in glyphs:
        if g.code >= 0x80 and not ext:
            w(EXTENDED)
            ext = True
        data = ('%s_%04X' % (name, g.code)) if g.words else ('%s_none' % name)
        w('      { %4d, %4d, %s }, /* code %04X, %s */\n' % (g.xdist, len(g.words), data, g.code, char_name(g.code)))
    if ext:
        w('#endif\n')
    w('    };\n\n')
    blocks = blocks_of(glyphs)
    ascii = sum(1 for first, last in blocks if glyphs[first].code < 0x80)
    for k in range(len(blocks), 0, -1):
        first, last = blocks[k - 1]
        if k == len(blocks) and k > ascii:
            w(EXTENDED)
        nxt = ('&%s_block%d' % (name, k + 1)) if k < len(blocks) else 'nullptr'
        w('    static const vector_block_type %s_block%d = {\n' % (name, k))
        w('      0x%04X,                            // first character\n' % glyphs[first].code)
        w('      0x%04X,                            // last character\n' % glyphs[last].code)
        w('      &%s_charinfo[%d],%s// address of the first character\n' % (name, first, ' ' * max(1, 20 - len(name) - len(str(first)))))
        if k == ascii and k < len(blocks):
            # the last ASCII block ends the chain without the chars beyond ASCII
            w('#if VIC_FONT_VECTOR_ASCII_ONLY\n      nullptr                            // next block\n#else\n')
            w('      %s%s// next block\n#endif\n' % (nxt, ' ' * max(1, 35 - len(nxt))))
        else:
            w('      %s%s// next block\n' % (nxt, ' ' * max(1, 35 - len(nxt))))
        w('    };\n\n')
        if k == ascii + 1:
            w('#endif\n\n')
    w('    static const vector_type %s_vector = {\n' % name)
    w('      %dU,%s// line height in font units\n' % (units, ' ' * (37 - len(str(units)))))
    w('      %d,%s// ascent (top of the line box above the baseline) in font units\n' % (ascent, ' ' * (38 - len(str(ascent)))))
    w('      &%s_block1\n' % name)
    w('    };\n\n')
    w('    static const info_type %s_info = {\n' % name)
    w('      FORMAT_VECTOR | ENCODING_UNICODE | TYPE_PROP, // type of font\n')
    w('      %dU,%s// default height of font (line)\n' % (size, ' ' * (37 - len(str(size)))))
    w('      %dU,%s// default y distance (line advance)\n' % (size, ' ' * (37 - len(str(size)))))
    w('      %dU,%s// baseline at the default height\n' % (baseline, ' ' * (37 - len(str(baseline)))))
    w('      &%s_vector\n' % name)
    w('    };\n\n')
    w('    return %s_info;\n  }\n};\n\n\n} // namespace font\n} // namespace vic\n\n#endif  // %s\n' % (name, guard))
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(''.join(out))


###############################################################################
# self test

def selftest():
    # a square: bottom and top are zone edges; a bar: a stem without zones
    em = 1000
    square = [[(0, 0, True), (0, 700, True), (500, 700, True), (500, 0, True)]]
    hints = glyph_hints(square, em, ([525, 700], [0]))
    assert hints == [0 << 1, 0, 700 << 1, 700], hints
    bar = [[(0, 300, True), (0, 380, True), (500, 380, True), (500, 300, True)]]
    hints = glyph_hints(bar, em, ([525, 700], [0]))
    assert hints == [(300 << 1) | 1, NO_ZONE, 380, NO_ZONE], hints
    # an 'o' made of quadratic arcs (outer clockwise, inner counter clockwise): top and bottom of the ring are stems
    def ring(r, cx, cy, ccw):
        pts = [(cx + r, cy, True), (cx + r, cy + r, False), (cx, cy + r, True), (cx - r, cy + r, False),
               (cx - r, cy, True), (cx - r, cy - r, False), (cx, cy - r, True), (cx + r, cy - r, False)]
        return pts if ccw else pts[::-1]
    o = [ring(267, 300, 262, False), ring(190, 300, 262, True)]
    hints = glyph_hints(o, em, ([525, 700], [0]))
    assert hints == [(-5 << 1) | 1, 0, 72, NO_ZONE, (452 << 1) | 1, NO_ZONE, 529, 525], hints
    # x hints: the vertical stems of the square sides are too far apart, the round sides of the ring are no stems
    assert glyph_xhints(square, em) == [0 << 1, 500 << 1], glyph_xhints(square, em)
    assert glyph_xhints(o, em) == [], glyph_xhints(o, em)
    # an 'H' of overlapping rectangles: the hidden ends of the crossbar are no edges
    h = [[(100, 0, True), (100, 700, True), (180, 700, True), (180, 0, True)],
         [(420, 0, True), (420, 700, True), (500, 700, True), (500, 0, True)],
         [(170, 320, True), (170, 390, True), (430, 390, True), (430, 320, True)]]
    assert glyph_xhints(h, em) == [(100 << 1) | 1, 180, (420 << 1) | 1, 500], glyph_xhints(h, em)
    assert glyph_hints(h, em, ([525, 700], [0])) == [0 << 1, 0, (320 << 1) | 1, NO_ZONE, 390, NO_ZONE, 700 << 1, 700], \
        glyph_hints(h, em, ([525, 700], [0]))
    # redundant points: duplicate, joint of 2 units between curves, implied on-curve points -> 8 off-curve points
    dot = [(249, 176, True), (249, 176, True), (239, 176, False), (224, 161, False), (224, 151, True), (224, 149, True),
           (224, 139, False), (239, 124, False), (249, 124, True), (251, 124, True), (261, 124, False), (276, 139, False),
           (276, 149, True), (276, 151, True), (276, 161, False), (261, 176, False), (251, 176, True)]
    assert simplify(dot) == [(239, 176, False), (224, 161, False), (224, 139, False), (239, 124, False),
                             (261, 124, False), (276, 139, False), (276, 161, False), (261, 176, False)], simplify(dot)
    assert simplify([(0, 0, True), (0, 700, True), (500, 700, True), (500, 0, True)]) ==         [(0, 0, True), (0, 700, True), (500, 700, True), (500, 0, True)]
    contours = [[(0, 0, True), (-5, 7, False), (32767 // 2, -32768, True)], [(1, -1, False)]]
    assert decode_contours(encode_contours(contours)) == contours
    # moved copies of a contour: a run of 2 copies, a different contour, a copy of that one
    sq = [(0, 0, True), (0, 10, True), (10, 10, True)]
    tri = [(0, 0, True), (5, 9, False), (9, 0, True)]
    contours = [sq, [(x + 20, y, on) for x, y, on in sq], [(x, y - 30, on) for x, y, on in sq], tri,
                [(x + 1, y + 2, on) for x, y, on in tri]]
    words = encode_contours(contours)
    assert words == [3, 1, 0, 1, 10, 21, 10, -2, 20, 0, 0, -30, 3, 1, 0, 10, 9, 19, 0, -1, 1, 2], words
    assert decode_contours(words) == contours
    for x in range(-2000, 2000):
        assert decode_contours(encode_contours([[(x, 0, x % 3 == 0)]])) == [[(x, 0, x % 3 == 0)]]
    print('selftest ok')


###############################################################################

def parse_ranges(text):
    ranges = []
    for part in text.split(','):
        lo, hi = part.split('-') if '-' in part else (part, part)
        ranges.append((int(lo, 16), int(hi, 16)))
    return ranges


def main():
    ap = argparse.ArgumentParser(description='TTF to vic vector font header generator')
    ap.add_argument('ttf', nargs='?', help='TrueType font file (quadratic outlines)')
    ap.add_argument('--ranges', default='20-7E,A0-FF', help='hex code point ranges, default ASCII + Latin-1')
    ap.add_argument('--size', type=int, default=16, help='default line height in pixel, used by set_font() without a size')
    ap.add_argument('--name', help='class name, default <Family>_vec')
    ap.add_argument('--out', help='output header, default src/font/<name>.h')
    ap.add_argument('--axes', help='instance of a variable font, design coordinates: ROND=100,wght=700')
    ap.add_argument('--no-hints', action='store_true', help="don't generate the vertical hints")
    ap.add_argument('--selftest', action='store_true', help='run the encode/decode self test')
    args = ap.parse_args()

    if args.selftest:
        selftest()
        return
    if not args.ttf:
        ap.error('ttf is required')

    axes = dict((a.split('=')[0], float(a.split('=')[1])) for a in args.axes.split(',')) if args.axes else None
    face, family, glyphs, missing = extract(args.ttf, parse_ranges(args.ranges), axes)
    if not args.no_hints:
        em, zones = face.units_per_EM, font_zones(face)
        # standard stem widths: horizontal stems, then the vertical stems (x and y swapped)
        std = std_widths([g.contours for g in glyphs], em, True)
        for g in glyphs:
            moved = normalize_stems(g.contours, em, zones, std)
            if g.bbox:
                # the line box follows a moved top or bottom edge
                ys = [(p[1], q[1]) for a, b in zip(g.contours, moved) for p, q in zip(a, b) if p[1] != q[1]]
                g.bbox = (g.bbox[0], min([g.bbox[1]] + [n for o, n in ys if o == g.bbox[1]]),
                          g.bbox[2], max([g.bbox[3]] + [n for o, n in ys if o == g.bbox[3]]))
            g.contours = moved
        std = std_widths([swap(g.contours) for g in glyphs], em, False)
        for g in glyphs:
            g.contours = swap(normalize_stems(swap(g.contours), em, None, std))
            if g.contours:
                g.set_hints(glyph_hints(g.contours, em, zones), glyph_xhints(g.contours, em))
    units, ascent = line_box(glyphs)
    for g in glyphs:
        for pts in g.contours:
            for x, y, on in pts:
                if not (-16384 <= x < 16384 and -32768 <= y < 32768):
                    sys.exit('%s: coordinate out of range in code %04X' % (args.ttf, g.code))
    name = args.name or '%s_vec' % family
    out = args.out or os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'src', 'font', name + '.h')
    write_header(out, name, glyphs, units, ascent, args.size, args.ttf, family, args.axes)
    data = data_size(glyphs)
    hints = sum(2 * (len(g.hints) + len(g.xhints)) for g in glyphs)
    print('%s: %d glyphs in %d blocks, %d units per em, line height %d units, ascent %d, about %d bytes (%d hints) -> %s' %
          (name, len(glyphs), len(blocks_of(glyphs)), face.units_per_EM, units, ascent, data, hints, os.path.normpath(out)))
    if missing:
        print('  missing in the font, skipped: ' + ' '.join('%04X' % c for c in missing))


if __name__ == '__main__':
    main()
