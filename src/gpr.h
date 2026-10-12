///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2001-2018, PALANDesign Hannover, Germany
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
// \brief Graphic Primitive Renderer to render lines, arcs, boxes etc.
// This is the base class of any driver. If a driver can't provide drawing functions
// on its own, these routines are taken.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_GPR_H_
#define _VIC_GPR_H_

#include "vic_cfg.h"
#include "util.h"
#include "color.h"
#include "shader/base.h"


namespace vic {


/**
 * Graphic Primitive Renderer
 */
class gpr
{
  // class is non copyable
  const gpr& operator=(const gpr& rhs)
  { return rhs; }

  bool              anti_aliasing_;   // true if AA is enabled
  color::value_type color_;           // actual drawing color

protected:

  // virtual context/driver functions
  virtual std::uint16_t     screen_width(void) const = 0;
  virtual std::uint16_t     screen_height(void) const = 0;
  virtual shader::base*     shader_pipe(void) const = 0;
  virtual void              present(void) = 0;
  virtual void              present_lock(bool lock = true) = 0;


  /**
  * protected ctor
  * Init vars
  */
  gpr()
    : anti_aliasing_(false)   // no AA as default
    , color_(color::gray)     // default drawing color
  { }


  /**
   * Limit the border radius of rounded boxes/rectangles so that the four corner
   * discs fit into the rect without overlapping each other
   * \param rect The rect
   * \param border_radius Requested radius
   * \return The usable radius, 0 if no rounding is possible
   */
  static std::int16_t clamp_border_radius(const rect_type& rect, std::uint16_t border_radius)
  {
    const std::int32_t w = static_cast<std::int32_t>(rect.right)  - rect.left;
    const std::int32_t h = static_cast<std::int32_t>(rect.bottom) - rect.top;
    const std::int32_t rmax = ((w < h ? w : h) - 2) / 2;    // 2 * (r + 1) <= min(w, h)
    if (rmax <= 0) {
      return 0;
    }
    return static_cast<std::int16_t>(border_radius > rmax ? rmax : border_radius);
  }


  ///////////////////////////////////////////////////////////////////////////////
  // A N T I   A L I A S I N G
  //
  // Anti aliased rendering works with pixel coverage: an edge pixel gets the pen color with an
  // alpha of (pen alpha * coverage / 255), the alpha blending of the output shader (or of the
  // canvas of a sprite) mixes it with the background. Lines use the exact Xiaolin Wu approach
  // (two pixels per step whose coverage sum up to 255). Circle outlines and the filled primitives
  // (disc, disc_sector, sector, triangle_solid) sample every edge pixel with a 4 x 4 sub pixel
  // grid against the exact shape (the outline is a one pixel wide ring), fully covered pixels are
  // drawn as solid horizontal runs, so the interior still uses the native line functions of the head.
  // The sub sampling works in 1/8 pixel units (pixel center = 8 * coordinate), so shapes must
  // stay within +/- 4000 pixels to avoid overflows in the 32 bit math.

  static const std::int32_t AA_SCALE   = 8;    // 1/8 pixel units (2 * samples per axis)
  static const std::int32_t AA_SAMPLES = 4;    // samples per axis, 16 samples per pixel

  /**
   * Plot a pixel with the given coverage in the pen color
   * \param pipe The shader pipe
   * \param v Vertex
   * \param coverage Pixel coverage, 0 (nothing drawn) to 255 (fully covered)
   */
  inline void aa_pixel(shader::base* pipe, vertex_type v, std::uint32_t coverage)
  {
    if (coverage == 0U) {
      return;
    }
    const std::uint32_t alpha = coverage >= 255U ? color::get_alpha(color_) : color::get_alpha(color_) * coverage / 255U;
    if (alpha != 0U) {
      pipe->pixel_set(v, color::set_alpha(color_, static_cast<std::uint8_t>(alpha)));
    }
  }


  /**
   * Coverage based filling of a shape
   * Every pixel of the (inclusive) box is tested against the shape predicate: pixels with the
   * corners, edge centers and center inside are fully covered, pixels with all corners and the
   * center outside are empty, all other pixels are sampled with 4 x 4 sub pixels.
   * Fully covered pixels are drawn as horizontal runs via line_horz().
   * \param pipe The shader pipe
   * \param left Inclusive pixel box to scan
   * \param top Inclusive pixel box to scan
   * \param right Inclusive pixel box to scan
   * \param bottom Inclusive pixel box to scan
   * \param pred Shape predicate, pred(X, Y) returns true if the point (X, Y) in 1/8 pixel screen units is inside the shape
   */
  template<typename Pred>
  void aa_fill(shader::base* pipe, std::int32_t left, std::int32_t top, std::int32_t right, std::int32_t bottom, const Pred& pred)
  {
    for (std::int32_t y = top; y <= bottom; ++y) {
      std::int32_t run_start = 0;
      bool run = false;
      for (std::int32_t x = left; x <= right; ++x) {
        const std::int32_t X = x * AA_SCALE, Y = y * AA_SCALE;
        std::uint32_t coverage;
        const std::uint32_t corners = (pred(X - 4, Y - 4) ? 1U : 0U) + (pred(X + 4, Y - 4) ? 1U : 0U) + (pred(X - 4, Y + 4) ? 1U : 0U) + (pred(X + 4, Y + 4) ? 1U : 0U);
        if ((corners == 4U) && pred(X, Y) && pred(X - 4, Y) && pred(X + 4, Y) && pred(X, Y - 4) && pred(X, Y + 4)) {
          // corners, edge centers and the center inside: fully covered (the hole of a ring is at least 0.5 pixel wide)
          coverage = 255U;
        }
        else if ((corners == 0U) && !pred(X, Y)) {
          // all corners and the center outside: the pixel is not touched (shapes are at least one pixel wide)
          coverage = 0U;
        }
        else {
          std::uint32_t count = 0U;
          for (std::int32_t sy = -(AA_SAMPLES - 1); sy < AA_SAMPLES; sy += 2) {
            for (std::int32_t sx = -(AA_SAMPLES - 1); sx < AA_SAMPLES; sx += 2) {
              count += pred(X + sx, Y + sy) ? 1U : 0U;
            }
          }
          coverage = count * 255U / static_cast<std::uint32_t>(AA_SAMPLES * AA_SAMPLES);
        }
        if (coverage == 255U) {
          if (!run) {
            run       = true;
            run_start = x;
          }
        }
        else {
          if (run) {
            line_horz({ static_cast<std::int16_t>(run_start), static_cast<std::int16_t>(y) }, { static_cast<std::int16_t>(x - 1), static_cast<std::int16_t>(y) });
            run = false;
          }
          aa_pixel(pipe, { static_cast<std::int16_t>(x), static_cast<std::int16_t>(y) }, coverage);
        }
      }
      if (run) {
        line_horz({ static_cast<std::int16_t>(run_start), static_cast<std::int16_t>(y) }, { static_cast<std::int16_t>(right), static_cast<std::int16_t>(y) });
      }
    }
  }


  // disc predicate: a circle with radius r + 0.5 around the center (same visual size as the solid disc)
  struct aa_disc_pred
  {
    std::int32_t cx, cy, r2;
    aa_disc_pred(vertex_type center, std::uint16_t radius)
      : cx(center.x * AA_SCALE), cy(center.y * AA_SCALE)
      , r2((radius * AA_SCALE + AA_SCALE / 2) * (radius * AA_SCALE + AA_SCALE / 2))
    { }
    inline bool operator()(std::int32_t X, std::int32_t Y) const
    {
      const std::int32_t dx = X - cx, dy = Y - cy;
      return dx * dx + dy * dy <= r2;
    }
  };


  // half open angle span test [s, e) for a point in math coordinates (y up), same rules as sector()
  struct aa_span
  {
    std::int32_t span, xss, yss, xse, yse;   // sin/cos normalized to 1024
    aa_span(std::int32_t s, std::int32_t e)
      : span(e - s)
      , xss(util::cos(static_cast<std::int16_t>(s)) / 16), yss(util::sin(static_cast<std::int16_t>(s)) / 16)
      , xse(util::cos(static_cast<std::int16_t>(e)) / 16), yse(util::sin(static_cast<std::int16_t>(e)) / 16)
    { }
    inline bool operator()(std::int32_t px, std::int32_t py) const
    {
      if (span >= 360) {
        return true;
      }
      const std::int32_t cs = xss * py - yss * px, ce = xse * py - yse * px;    // cross products
      const bool ccw_s = (cs > 0) || ((cs == 0) && ((xss * px + yss * py) > 0));
      const bool ccw_e = (ce > 0) || ((ce == 0) && ((xse * px + yse * py) > 0));
      return (span < 180) ? (ccw_s && !ccw_e) : (ccw_s || !ccw_e);
    }
  };


  // sector predicate: ring from inner radius - 0.5 (inside) to outer radius + 0.5 (outside) within the angle span,
  // the outer edge is the same as the one of a disc with the outer radius, so a pie equals a disc and a ring sits
  // exactly on the outside of a disc with radius inner_radius - 1
  struct aa_sector_pred
  {
    std::int32_t cx, cy, ri2, ro2;
    aa_span      span;
    aa_sector_pred(vertex_type center, std::uint16_t inner_radius, std::uint16_t outer_radius, std::int32_t s, std::int32_t e)
      : cx(center.x * AA_SCALE), cy(center.y * AA_SCALE)
      , ri2(inner_radius ? (inner_radius * AA_SCALE - AA_SCALE / 2) * (inner_radius * AA_SCALE - AA_SCALE / 2) : 0)
      , ro2((outer_radius * AA_SCALE + AA_SCALE / 2) * (outer_radius * AA_SCALE + AA_SCALE / 2))
      , span(s, e)
    { }
    inline bool operator()(std::int32_t X, std::int32_t Y) const
    {
      const std::int32_t dx = X - cx, dy = cy - Y;   // math coordinates
      const std::int32_t d2 = dx * dx + dy * dy;
      return (d2 >= ri2) && (d2 < ro2) && span(dx, dy);
    }
  };


  // triangle predicate: inside or on the edges of a clockwise wound triangle
  struct aa_triangle_pred
  {
    std::int32_t x0, y0, x1, y1, x2, y2;
    aa_triangle_pred(vertex_type v0, vertex_type v1, vertex_type v2)
      : x0(v0.x * AA_SCALE), y0(v0.y * AA_SCALE), x1(v1.x * AA_SCALE), y1(v1.y * AA_SCALE), x2(v2.x * AA_SCALE), y2(v2.y * AA_SCALE)
    { }
    inline bool operator()(std::int32_t X, std::int32_t Y) const
    {
      // the edge functions of triangle_solid() (util::orient_2d)
      const std::int32_t w0 = (X - x2) * (y1 - y2) - (Y - y2) * (x1 - x2);
      const std::int32_t w1 = (X - x0) * (y2 - y0) - (Y - y0) * (x2 - x0);
      const std::int32_t w2 = (X - x1) * (y0 - y1) - (Y - y1) * (x0 - x1);
      return (w0 >= 0) && (w1 >= 0) && (w2 >= 0);
    }
  };


  /**
   * Anti aliased line (Xiaolin Wu algorithm), the line must not be axis aligned
   * \param v0 Start vertex
   * \param v1 End vertex
   */
  void line_aa(vertex_type v0, vertex_type v1)
  {
    shader::base* const pipe = shader_pipe();

    // work in (major, minor) coordinates, the major axis is the one with the bigger distance
    const bool steep = util::abs<std::int32_t>(v1.y - v0.y) > util::abs<std::int32_t>(v1.x - v0.x);
    std::int32_t a0 = steep ? v0.y : v0.x, b0 = steep ? v0.x : v0.y;
    std::int32_t a1 = steep ? v1.y : v1.x, b1 = steep ? v1.x : v1.y;
    if (a0 > a1) {
      util::min_max_swap(a0, a1);
      const std::int32_t t = b0; b0 = b1; b1 = t;
    }
    const std::int32_t da   = a1 - a0;                                   // > 0
    const std::int32_t step = ((b1 - b0) * 65536) / da;                  // minor increment per major step in 16.16
    std::int32_t       b16  = b0 * 65536;                                // minor coordinate in 16.16

    present_lock();
    for (std::int32_t a = a0; a <= a1; ++a, b16 += step) {
      if (a == a1) {
        // exact end point
        aa_pixel(pipe, steep ? vertex_type{ static_cast<std::int16_t>(b1), static_cast<std::int16_t>(a) } : vertex_type{ static_cast<std::int16_t>(a), static_cast<std::int16_t>(b1) }, 255U);
        break;
      }
      const std::int32_t  bi   = b16 >> 16;                                // floor of the minor coordinate
      const std::uint32_t frac = static_cast<std::uint32_t>(b16 >> 8) & 0xFFU;
      if (steep) {
        aa_pixel(pipe, { static_cast<std::int16_t>(bi),     static_cast<std::int16_t>(a) }, 255U - frac);
        aa_pixel(pipe, { static_cast<std::int16_t>(bi + 1), static_cast<std::int16_t>(a) }, frac);
      }
      else {
        aa_pixel(pipe, { static_cast<std::int16_t>(a), static_cast<std::int16_t>(bi)     }, 255U - frac);
        aa_pixel(pipe, { static_cast<std::int16_t>(a), static_cast<std::int16_t>(bi + 1) }, frac);
      }
    }
    present_lock(false);
  }


  // ring predicate: the one pixel wide ring of a circle outline (radius - 0.5 to radius + 0.5) within the angle span
  struct aa_ring_pred
  {
    std::int32_t cx, cy, ri2, ro2;
    aa_span      span;
    aa_ring_pred(vertex_type center, std::uint16_t radius, std::int32_t s, std::int32_t e)
      : cx(center.x * AA_SCALE), cy(center.y * AA_SCALE)
      , ri2((radius * AA_SCALE - AA_SCALE / 2) * (radius * AA_SCALE - AA_SCALE / 2))
      , ro2((radius * AA_SCALE + AA_SCALE / 2) * (radius * AA_SCALE + AA_SCALE / 2))
      , span(s, e)
    { }
    inline bool operator()(std::int32_t X, std::int32_t Y) const
    {
      const std::int32_t dx = X - cx, dy = cy - Y;   // math coordinates
      const std::int32_t d2 = dx * dx + dy * dy;
      return (d2 >= ri2) && (d2 < ro2) && span(dx, dy);
    }
  };


  /**
   * Anti aliased circle outline, a one pixel wide ring rendered with coverage sampling
   * \param center Center vertex
   * \param radius Circle radius
   * \param s Start angle (normalized, 0 - 359)
   * \param e End angle, > s, <= s + 360
   */
  void circle_aa(vertex_type center, std::uint16_t radius, std::int32_t s, std::int32_t e)
  {
    shader::base* const pipe = shader_pipe();
    present_lock();
    if (radius == 0U) {
      // a single pixel, like the solid circle
      if (aa_span(s, e)(1, 0)) {
        aa_pixel(pipe, center, 255U);
      }
    }
    else {
      const std::int32_t r1 = static_cast<std::int32_t>(radius) + 1;
      aa_fill(pipe, center.x - r1, center.y - r1, center.x + r1, center.y + r1, aa_ring_pred(center, radius, s, e));
    }
    present_lock(false);
  }


  /**
   * Anti aliased disc or disc quadrants
   * \param center Center vertex
   * \param radius Disc radius
   * \param quadrants Bit mask of the quadrants to draw, bit 0: top/right, 1: top/left, 2: bottom/left, 3: bottom/right
   */
  void disc_aa(vertex_type center, std::uint16_t radius, std::uint8_t quadrants)
  {
    shader::base* const pipe = shader_pipe();
    const aa_disc_pred pred(center, radius);
    const std::int32_t r1 = static_cast<std::int32_t>(radius) + 1;
    const std::int32_t cx = center.x, cy = center.y;
    present_lock();
    if (radius == 0U) {
      // a single pixel, like the solid disc
      aa_pixel(pipe, center, 255U);
    }
    else if (quadrants == 0x0FU) {
      aa_fill(pipe, cx - r1, cy - r1, cx + r1, cy + r1, pred);
    }
    else {
      // the center row and column belong to every quadrant (like disc_sector())
      if (quadrants & 0x01U) { aa_fill(pipe, cx,      cy - r1, cx + r1, cy,      pred); }
      if (quadrants & 0x02U) { aa_fill(pipe, cx - r1, cy - r1, cx,      cy,      pred); }
      if (quadrants & 0x04U) { aa_fill(pipe, cx - r1, cy,      cx,      cy + r1, pred); }
      if (quadrants & 0x08U) { aa_fill(pipe, cx,      cy,      cx + r1, cy + r1, pred); }
    }
    present_lock(false);
  }


public:

  /**
   * Set the drawing color
   * \param color Drawing color in ARGB format
   */
  void set_color(color::value_type color)
  {
    color_ = color;
  }


  /**
   * Get the drawing color
   * \return Actual drawing color in ARGB format
   */
  color::value_type get_color() const
  {
    return color_;
  }



///////////////////////////////////////////////////////////////////////////////
// G R A P H I C   P R I M I T I V E   F U N C T I O N S

  /**
   * Enable/disable anti aliasing
   * Anti aliased primitives render their edge pixels with a coverage dependent alpha, so alpha
   * blending must be available (output shader default or canvas sprites). Affected primitives:
   * line, polyline, triangle, triangle_solid, circle, disc, disc_sector, sector and the rounded
   * box/rectangle corners. Axis aligned lines, boxes and plain rectangles are not affected.
   * \param enable True to enable anti aliasing
   */
  void anti_aliasing_enable(bool enable = true)
  {
    anti_aliasing_ = enable;
  }


  /**
   * Set a pixel in the actual pen color, no present is called
   * \param vertex Vertex to set
   */
  inline void pixel_set(vertex_type vertex)
  {
    shader_pipe()->pixel_set(vertex, color_);
  }


  /**
   * Set a pixel in the given color, no present is called
   * \param vertex Vertex to set
   * \param color Color of the pixel in ARGB format
   */
  inline void pixel_set(vertex_type vertex, color::value_type color)
  {
    shader_pipe()->pixel_set(vertex, color);
  }


  /**
   * Return the color of the pixel (is a slim wrapper for drv_pixel_get)
   * \param vertex Vertex of the pixel
   * \return Color of pixel in ARGB format
   */
  inline color::value_type pixel_get(vertex_type vertex)
  {
    return shader_pipe()->pixel_get(vertex);
  }


  /**
   * Plot a point (one pixel) in the actual drawing (pen) color
   * \param vertex Vertex to plot
   */
  inline void plot(vertex_type vertex)
  {
    plot(vertex, color_);
  }


  /**
   * Plot a point (one pixel) in the given color
   * Basically this is pixel_set with immediate present()
   * \param vertex Vertex to plot
   * \param color Color of the pixel
   */
  inline void plot(vertex_type vertex, color::value_type color)
  {
    shader_pipe()->pixel_set(vertex, color);
    present();
  }


  /**
   * Draw a line with the actual selected pen (with pen color)
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line
   */
  void line(vertex_type v0, vertex_type v1)
  {
    // axis aligned lines are horizontal/vertical lines, which the head may draw natively
    if (v0.y == v1.y) {
      line_horz(v0, v1);
      return;
    }
    if (v0.x == v1.x) {
      line_vert(v0, v1);
      return;
    }
    if (anti_aliasing_) {
      line_aa(v0, v1);
      return;
    }

    // precalc constants, 32 bit math: the error term is twice the distance and the
    // distance itself may exceed the int16 range
    const std::int32_t dx = v1.x > v0.x ? v1.x - v0.x : v0.x - v1.x;
    const std::int32_t dy = v1.y > v0.y ? v1.y - v0.y : v0.y - v1.y;
    const std::int16_t sx = v1.x > v0.x ? 1 : -1;
    const std::int16_t sy = v1.y > v0.y ? 1 : -1;
          std::int32_t er = dx - dy;

    // Bresenham with run length emission: all pixels of a run along the major axis
    // are drawn with one line_horz/line_vert call (a native head command if available),
    // single pixel runs are plotted directly
    {
      shader::base* const pipe = shader_pipe();
      const bool x_major = dx >= dy;
      vertex_type run = v0;     // start of the current run
      present_lock();
      for (;;) {
        const bool last = (v0 == v1);
        const std::int32_t er2 = er * 2;
        const bool step_x = !last && (er2 + dy > 0);
        const bool step_y = !last && (er2 < dx);
        // the run ends at the end of the line or when the minor coordinate changes
        if (last || (x_major ? step_y : step_x)) {
          if (run == v0) {
            pipe->pixel_set(v0, color_);
          }
          else if (x_major) {
            line_horz(run, v0);
          }
          else {
            line_vert(run, v0);
          }
          if (last) {
            break;
          }
        }
        if (step_x) {
          er -= dy;
          v0.x = static_cast<std::int16_t>(v0.x + sx);
        }
        if (step_y) {
          er += dx;
          v0.y = static_cast<std::int16_t>(v0.y + sy);
        }
        if (x_major ? step_y : step_x) {
          run = v0;
        }
      }
      present_lock(false);
    }
  }


  /**
   * Draw a horizontal line, width is one pixel
   * This is should be done by a high speed driver implementation if no shader is used
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, y component is ignored
   * \return true if successful
   */
  virtual void line_horz(vertex_type v0, vertex_type v1)
  {
    shader::base* const pipe = shader_pipe();
    util::min_max_swap(v0.x, v1.x);   // set v0 to min x, keep the y of v0
    for (; v0.x <= v1.x; ++v0.x) {
      pipe->pixel_set(v0, color_);
    }
    present();
  }


  /**
   * Draw a vertical line, width is one pixel
   * This is should be done by a high speed driver implementation if no shader is used
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, x component is ignored
   */
  virtual void line_vert(vertex_type v0, vertex_type v1)
  {
    shader::base* const pipe = shader_pipe();
    util::min_max_swap(v0.y, v1.y);   // set v0 to min y, keep the x of v0
    for (; v0.y <= v1.y; ++v0.y) {
      pipe->pixel_set(v0, color_);
    }
    present();
  }


  /**
   * Draw a box (filled rectangle)
   * This is should be done by a high speed driver implementation if no shader is used
   * Top, left is inside the box. Bottom, right is outside the box
   * \param rect Box bounding
   */
  virtual void box(const rect_type& rect)
  {
    shader::base* const pipe = shader_pipe();
    for (std::int16_t y = rect.top; y < rect.bottom; ++y) {
      for (std::int16_t x = rect.left; x < rect.right; ++x) {
        pipe->pixel_set({ x, y }, color_);
      }
    }
    present();
  }


  /**
   * Draw a box (filled rectangle) with a corner radius
   * \param rect Box bounding
   * \param border_radius Radius of the corner, 0 for angular
   */
  void box(const rect_type& rect, std::uint16_t border_radius)
  {
    const std::int16_t r = clamp_border_radius(rect, border_radius);
    if (r <= 0) {
      // no radius (or too small rect): plain box
      box(rect);
      return;
    }

    // decomposition without overlapping areas, so every pixel is written exactly once
    // (important for alpha colors): three bands and four quarter discs
    const std::int16_t x0 = rect.left,  x1 = static_cast<std::int16_t>(rect.right - 1);   // inside coordinates
    const std::int16_t y0 = rect.top,   y1 = static_cast<std::int16_t>(rect.bottom - 1);
    present_lock();
    box({ static_cast<std::int16_t>(x0 + r + 1), y0,                                   static_cast<std::int16_t>(x1 - r), static_cast<std::int16_t>(y0 + r + 1) });  // top band
    box({ x0,                                    static_cast<std::int16_t>(y0 + r + 1), rect.right,                         static_cast<std::int16_t>(y1 - r)     });  // middle band
    box({ static_cast<std::int16_t>(x0 + r + 1), static_cast<std::int16_t>(y1 - r),     static_cast<std::int16_t>(x1 - r), rect.bottom                           });  // bottom band
    disc_sector({ static_cast<std::int16_t>(x1 - r), static_cast<std::int16_t>(y0 + r) }, static_cast<std::uint16_t>(r), 0U);   // top/right
    disc_sector({ static_cast<std::int16_t>(x0 + r), static_cast<std::int16_t>(y0 + r) }, static_cast<std::uint16_t>(r), 1U);   // top/left
    disc_sector({ static_cast<std::int16_t>(x0 + r), static_cast<std::int16_t>(y1 - r) }, static_cast<std::uint16_t>(r), 2U);   // bottom/left
    disc_sector({ static_cast<std::int16_t>(x1 - r), static_cast<std::int16_t>(y1 - r) }, static_cast<std::uint16_t>(r), 3U);   // bottom/right
    present_lock(false);    // unlock and present
  }


  /**
   * Draw a box (filled rectangle) with a corner radius, vertex to rect wrapper
   * \param v0 First corner vertex
   * \param v1 Second corner vertex
   * \param border_radius Radius of the corner, 0 for angular
   */
  inline void box(vertex_type v0, vertex_type v1, std::uint16_t border_radius)
  {
    rect_type r;
    r.normalize(v0, v1);
    box(r, border_radius);
  }


  /**
   * Draw a rectangle (frame) with the current pen
   * \param rect Rectangle
   */
  void rectangle(const rect_type& rect)
  {
    if ((rect.right <= rect.left) || (rect.bottom <= rect.top)) {
      // empty rect
      return;
    }
    const std::int16_t x1 = static_cast<std::int16_t>(rect.right - 1);    // inside coordinates
    const std::int16_t y1 = static_cast<std::int16_t>(rect.bottom - 1);

    // every frame pixel is written exactly once (corners belong to the horizontal lines)
    present_lock();
    line_horz({ rect.left, rect.top }, { x1, rect.top });
    if (y1 > rect.top) {
      line_horz({ rect.left, y1 }, { x1, y1 });
    }
    if (y1 - rect.top > 1) {
      line_vert({ rect.left, static_cast<std::int16_t>(rect.top + 1) }, { rect.left, static_cast<std::int16_t>(y1 - 1) });
      if (x1 > rect.left) {
        line_vert({ x1, static_cast<std::int16_t>(rect.top + 1) }, { x1, static_cast<std::int16_t>(y1 - 1) });
      }
    }
    present_lock(false);    // unlock and present
  }


  /**
   * Draw a rectangle (frame) with the current pen, vertex to rect wrapper
   * \param v0 First corner vertex
   * \param v1 Second corner vertex
   */
  void rectangle(vertex_type v0, vertex_type v1)
  {
    rect_type r;
    r.normalize(v0, v1);
    rectangle(r);
  }


  /**
   * Draw a rectangle (frame) with a corner radius
   * \param rect Rectangle
   * \param border_radius Radius of the corner, 0 for angular
   */
  void rectangle(const rect_type& rect, std::uint16_t border_radius)
  {
    const std::int16_t r = clamp_border_radius(rect, border_radius);
    if (r <= 0) {
      // no radius (or too small rect): plain rectangle
      rectangle(rect);
      return;
    }

    const std::int16_t x0 = rect.left,  x1 = static_cast<std::int16_t>(rect.right - 1);   // inside coordinates
    const std::int16_t y0 = rect.top,   y1 = static_cast<std::int16_t>(rect.bottom - 1);
    const std::uint16_t ur = static_cast<std::uint16_t>(r);

    // The quarter circles are half open arcs: they include their start angle point but not
    // their end angle point (the 90°, 180°, 270° and 360° points). These four points are
    // covered by the straight lines, so every frame pixel is written exactly once.
    present_lock();
    line_horz({ static_cast<std::int16_t>(x0 + r + 1), y0 }, { static_cast<std::int16_t>(x1 - r),     y0 });   // top,    includes the 90° point of the top/right arc
    line_vert({ x0, static_cast<std::int16_t>(y0 + r)     }, { x0, static_cast<std::int16_t>(y1 - r - 1) });   // left,   includes the 180° point of the top/left arc
    line_horz({ static_cast<std::int16_t>(x0 + r),     y1 }, { static_cast<std::int16_t>(x1 - r - 1), y1 });   // bottom, includes the 270° point of the bottom/left arc
    line_vert({ x1, static_cast<std::int16_t>(y0 + r + 1) }, { x1, static_cast<std::int16_t>(y1 - r)     });   // right,  includes the 360° point of the bottom/right arc
    circle({ static_cast<std::int16_t>(x1 - r), static_cast<std::int16_t>(y0 + r) }, ur,   0U,  90U);   // top/right
    circle({ static_cast<std::int16_t>(x0 + r), static_cast<std::int16_t>(y0 + r) }, ur,  90U, 180U);   // top/left
    circle({ static_cast<std::int16_t>(x0 + r), static_cast<std::int16_t>(y1 - r) }, ur, 180U, 270U);   // bottom/left
    circle({ static_cast<std::int16_t>(x1 - r), static_cast<std::int16_t>(y1 - r) }, ur, 270U, 360U);   // bottom/right
    present_lock(false);    // unlock and present
  }


  /**
   * Draw a rectangle (frame) with a corner radius, vertex to rect wrapper
   * \param v0 First corner vertex
   * \param v1 Second corner vertex
   * \param border_radius Radius of the corner, 0 for angular
   */
  void rectangle(vertex_type v0, vertex_type v1, std::uint16_t border_radius)
  {
    rect_type r;
    r.normalize(v0, v1);
    rectangle(r, border_radius);
  }


  /**
   * Draw multiple connected lines
   * \param vertices Pointer to array of vertices, at least 2
   * \param vertex_count Number of vertices in the array, at least 2
   */
  void polyline(const vertex_type* vertices, std::size_t vertex_count)
  {
    if (vertex_count < 2U) {
      return;
    }
    present_lock();
    for (std::size_t n = 0U; n + 1U < vertex_count; ++n) {
      line(vertices[n], vertices[n + 1U]);
    }
    present_lock(false);    // present
  }


  /**
   * Draw a triangle
   * \param v0 value, included in triangle
   * \param v1 value, included in triangle
   * \param v2 value, included in triangle
   */
  void triangle(vertex_type v0, vertex_type v1, vertex_type v2)
  {
    present_lock();
    line(v0, v1);
    line(v1, v2);
    line(v2, v0);
    present_lock(false);  // present
  }


  /**
   * Draw a solid (filled) triangle
   * \param v0 value, included in triangle
   * \param v1 value, included in triangle
   * \param v2 value, included in triangle
   */
  void triangle_solid(vertex_type v0, vertex_type v1, vertex_type v2)
  {
    present_lock();

    // ensure v0 is min_x
    v0.min_x(v1);
    v0.min_x(v2);

    // ensure clockwise vertex winding
    const std::int32_t r = util::orient_2d(v1, v0, v2);
    // check if the area of the triangle is 0 (straight line)
    if (r > 0) {
      // v1 is on the right side, swap to get it on the left side (clockwise vertex winding)
      v1.swap(v2);
    }
    else if (r == 0) {
      // v1 is on a line with v0 and v2
      line(v0, v1);
      line(v1, v2);
      present_lock(false);
      return;
    }

    if (anti_aliasing_) {
      // coverage based rendering over the bounding box
      aa_fill(shader_pipe(), v0.x, util::min3(v0.y, v1.y, v2.y), util::max2(v1.x, v2.x), util::max3(v0.y, v1.y, v2.y), aa_triangle_pred(v0, v1, v2));
      present_lock(false);
      return;
    }

    // compute triangle bounding box
    const std::int16_t min_x = v0.x;
    const std::int16_t max_x = util::max2(v1.x, v2.x);
    const std::int16_t min_y = util::min3(v0.y, v1.y, v2.y);
    const std::int16_t max_y = util::max3(v0.y, v1.y, v2.y);

    // triangle setup
    const std::int16_t a01 = v0.y - v1.y, b01 = v1.x - v0.x;
    const std::int16_t a12 = v1.y - v2.y, b12 = v2.x - v1.x;
    const std::int16_t a20 = v2.y - v0.y, b20 = v0.x - v2.x;

    // Barycentric coordinates at minX/minY corner
    vertex_type p = { min_x, min_y };
    std::int32_t w0_row = util::orient_2d(p, v1, v2);
    std::int32_t w1_row = util::orient_2d(p, v2, v0);
    std::int32_t w2_row = util::orient_2d(p, v0, v1);

    // rasterize
    for (p.y = min_y; p.y <= max_y; ++p.y) {
      // Barycentric coordinates at start of row
      std::int32_t w0 = w0_row, w1 = w1_row, w2 = w2_row;
      std::int16_t l_x = 0;
      bool inside = false;
      for (p.x = min_x; p.x <= max_x; ++p.x) {
        // if p is on or inside all edges, render the pixel
        if (!inside && w0 >= 0 && w1 >= 0 && w2 >= 0) {
          inside = true;
          l_x = p.x;
        }

        // one step to the right
        w0 += a12; w1 += a20; w2 += a01;

        if (inside && (w0 < 0 || w1 < 0 || w2 < 0)) {
          line_horz({ l_x, p.y }, p);
          break;
        }
      }
      // one row step
      w0_row += b12; w1_row += b20; w2_row += b01;
    }
    present_lock(false);
  }


  /**
   * Draw a circle piece with the current pen or anti aliased
   * \param center Center vertex
   * \param radius Circle radius
   * \param start_angle Start angle in degree, 0 is horizontal right, counting anticlockwise
   * \param end_angle End angle in degree
   */
  void circle(vertex_type center, std::uint16_t radius, std::uint16_t start_angle = 0U, std::uint16_t end_angle = 360U)
  {
    // validate the angles: start is reduced to 0 - 359, end is limited to 360
    // the arc is drawn from start to end (counting counterclockwise), so start must be less than end
    start_angle = static_cast<std::uint16_t>(start_angle % 360U);
    if (end_angle > 360U) {
      end_angle = 360U;
    }
    if (start_angle >= end_angle) {
      return;
    }
    if (anti_aliasing_) {
      circle_aa(center, radius, start_angle, end_angle);
      return;
    }

    const std::int16_t xs = static_cast<std::int16_t>(center.x + radius * util::cos(static_cast<std::int16_t>(start_angle)) / 16384);
    const std::int16_t ys = static_cast<std::int16_t>(center.y - radius * util::sin(static_cast<std::int16_t>(start_angle)) / 16384);
    const std::int16_t xe = static_cast<std::int16_t>(center.x + radius * util::cos(static_cast<std::int16_t>(end_angle))   / 16384);
    const std::int16_t ye = static_cast<std::int16_t>(center.y - radius * util::sin(static_cast<std::int16_t>(end_angle))   / 16384);

    const std::uint_fast16_t qs = (start_angle / 90U);
    const std::uint_fast16_t qe = (end_angle - 1U) / 90U;
    const bool               os = (start_angle / 45U) & 1U;
    const bool               oe = ((end_angle - 1U) / 45U) & 1U;

    shader::base* const pipe = shader_pipe();

    bool render = false;
    for (std::uint_fast16_t q = qs; q <= qe; ++q) {
      std::int16_t r = static_cast<std::int16_t>(radius), x = 0, y = static_cast<std::int16_t>(-r), err = static_cast<std::int16_t>(2 - r * 2);
      vertex_type p;
      do {
        switch (q) {
          case 0 :
            p = { static_cast<std::int16_t>(center.x - y), static_cast<std::int16_t>(center.y - x) };
            if (!render && (q == qs) && (((p.x <= xs) && os) || ((p.y <= ys) && !os))) {
              render = true;
            }
            if (render && (q == qe) && (((p.x < xe) && oe) || ((p.y < ye) && !oe))) {
              present();
              return;
            }
            break;
          case 1 :
            p = { static_cast<std::int16_t>(center.x - x), static_cast<std::int16_t>(center.y + y) };
            if (!render && (q == qs) && (((p.x <= xs) && !os) || ((p.y >= ys) && os))) {
              render = true;
            }
            if (render && (q == qe) && (((p.x < xe) && !oe) || ((p.y > ye) && oe))) {
              present();
              return;
            }
            break;
          case 2 :
            p = { static_cast<std::int16_t>(center.x + y), static_cast<std::int16_t>(center.y + x) };
            if (!render && (q == qs) && (((p.x >= xs) && os) || ((p.y >= ys) && !os))) {
              render = true;
            }
            if (render && (q == qe) && (((p.x > xe) && oe) || ((p.y > ye) && !oe))) {
              present();
              return;
            }
            break;
          case 3 :
            p = { static_cast<std::int16_t>(center.x + x), static_cast<std::int16_t>(center.y - y) };
            if (!render && (q == qs) && (((p.x >= xs) && !os) || ((p.y <= ys) && os))) {
              render = true;
            }
            if (render && (q == qe) && (((p.x > xe) && !oe) || ((p.y < ye) && oe))) {
              present();
              return;
            }
            break;
          default:
            p = { 0, 0 };
            break;
        }
        if (render) {
          pipe->pixel_set(p, color_);
        }
        r = err;
        if (r <= x)
          err = static_cast<std::int16_t>(err + ++x * 2 + 1);
        if (r > y || err > x)
          err = static_cast<std::int16_t>(err + ++y * 2 + 1);
      } while (y < 0);
    }
    present();
  }


  /**
   * Draw a disc (filled circle)
   * \param center Center value
   * \param radius Disc radius
   */
  void disc(vertex_type center, std::uint16_t radius)
  {
    if (anti_aliasing_) {
      disc_aa(center, radius, 0x0FU);
      return;
    }
    // the disc is the filled circle(): the same midpoint algorithm gives the
    // horizontal extent of every row, so disc and circle have identical outlines
    present_lock();
    std::int32_t x = 0, y = -static_cast<std::int32_t>(radius), err = 2 - 2 * static_cast<std::int32_t>(radius), row = -1;
    do {
      if (x != row) {
        // first vertex of a new row: y is the (negative) horizontal extent of the row
        row = x;
        const std::int16_t ext = static_cast<std::int16_t>(-y);
        line_horz({ static_cast<std::int16_t>(center.x - ext), static_cast<std::int16_t>(center.y - row) }, { static_cast<std::int16_t>(center.x + ext), static_cast<std::int16_t>(center.y - row) });
        if (row != 0) {
          line_horz({ static_cast<std::int16_t>(center.x - ext), static_cast<std::int16_t>(center.y + row) }, { static_cast<std::int16_t>(center.x + ext), static_cast<std::int16_t>(center.y + row) });
        }
      }
      const std::int32_t r = err;
      if (r <= x) {
        err += ++x * 2 + 1;
      }
      if (r > y || err > x) {
        err += ++y * 2 + 1;
      }
    } while (y < 0);
    if (row != radius) {
      // the loop ends before the last row (the axis points) is reached, draw the single pixel rows
      line_horz({ center.x, static_cast<std::int16_t>(center.y - radius) }, { center.x, static_cast<std::int16_t>(center.y - radius) });
      line_horz({ center.x, static_cast<std::int16_t>(center.y + radius) }, { center.x, static_cast<std::int16_t>(center.y + radius) });
    }
    present_lock(false);
  }


  /**
   * Draw a disc sector (filled quarter circle)
   * \param center Center value
   * \param radius Disc radius
   * \param quadrant Quadrant number: 0: top/right, 1: top/left, 2: bottom/left, 3: bottom/right
   */
  void disc_sector(vertex_type center, std::uint16_t radius, std::uint8_t quadrant)
  {
    if (quadrant > 3U) {
      return;
    }
    if (anti_aliasing_) {
      disc_aa(center, radius, static_cast<std::uint8_t>(1U << quadrant));
      return;
    }
    // same midpoint algorithm as disc(), only one side of each row is drawn
    const std::int16_t sx = (quadrant == 0U || quadrant == 3U) ? 1 : -1;   // right or left half
    const std::int16_t sy = (quadrant == 0U || quadrant == 1U) ? -1 : 1;   // top or bottom half

    present_lock();
    std::int32_t x = 0, y = -static_cast<std::int32_t>(radius), err = 2 - 2 * static_cast<std::int32_t>(radius), row = -1;
    do {
      if (x != row) {
        row = x;
        const std::int16_t ext = static_cast<std::int16_t>(-y);
        line_horz({ center.x, static_cast<std::int16_t>(center.y + sy * row) }, { static_cast<std::int16_t>(center.x + sx * ext), static_cast<std::int16_t>(center.y + sy * row) });
      }
      const std::int32_t r = err;
      if (r <= x) {
        err += ++x * 2 + 1;
      }
      if (r > y || err > x) {
        err += ++y * 2 + 1;
      }
    } while (y < 0);
    if (row != radius) {
      // the loop ends before the last row (the axis point) is reached
      line_horz({ center.x, static_cast<std::int16_t>(center.y + sy * radius) }, { center.x, static_cast<std::int16_t>(center.y + sy * radius) });
    }
    present_lock(false);
  }


  /**
   * Draw a sector (pie, filled circle piece)
   * \param center Center value
   * \param inner_radius Inner sector radius
   * \param outer_radius Outer sector radius
   * \param start_angle Start angle in degree, 0 is horizontal right, counting anticlockwise
   * \param end_angle End angle in degree
   */
  void sector(vertex_type center, std::uint16_t inner_radius, std::uint16_t outer_radius, std::uint16_t start_angle, std::uint16_t end_angle)
  {
    // angle:
    //   0° = 3 o'clock, counting counterclockwise
    //   0° -  89°: Q1 (top/right)
    //  90° - 179°: Q2 (top/left)
    // 180° - 269°: Q3 (bottom/left)
    // 270° - 359°: Q4 (bottom/right)
    // The angle interval is half open: the start ray is part of the sector, the end ray is not.
    // So adjacent sectors tile without gaps or double drawn pixels. The inner radius is inside,
    // the outer radius is outside the sector.

    if (inner_radius >= outer_radius) {
      return;
    }

    // normalize the angles: start to 0 - 359, span to 1 - 360 (end <= start means wrap around)
    const std::int32_t s    = start_angle % 360U;
    std::int32_t      span = static_cast<std::int32_t>(end_angle) - static_cast<std::int32_t>(start_angle);
    if (span <= 0) {
      span = static_cast<std::int32_t>(end_angle % 360U) - s;
      if (span <= 0) {
        if (span == 0) {
          return;   // zero span
        }
        span += 360;
      }
    }
    if (span > 360) {
      span = 360;
    }
    const std::int32_t e = s + span;

    if (anti_aliasing_) {
      shader::base* const pipe = shader_pipe();
      const std::int32_t r1 = static_cast<std::int32_t>(outer_radius) + 1;
      present_lock();
      aa_fill(pipe, center.x - r1, center.y - r1, center.x + r1, center.y + r1, aa_sector_pred(center, inner_radius, outer_radius, s, e));
      present_lock(false);
      return;
    }

    // direction vectors of start and end ray, normalized to 16384 (no division to use the full range)
    const std::int32_t xss = util::cos(static_cast<std::int16_t>(s)), yss = util::sin(static_cast<std::int16_t>(s));
    const std::int32_t xse = util::cos(static_cast<std::int16_t>(e)), yse = util::sin(static_cast<std::int16_t>(e));

    const std::int32_t ri2 = static_cast<std::int32_t>(inner_radius) * inner_radius;
    const std::int32_t ro2 = static_cast<std::int32_t>(outer_radius) * outer_radius;

    present_lock();
    for (std::int32_t yp = center.y - outer_radius; yp <= center.y + outer_radius; ++yp) {
      bool inside = false;
      std::int32_t lxp = 0;
      for (std::int32_t xp = center.x - outer_radius; xp <= center.x + outer_radius + 1; ++xp) {
        // check if xp/yp is within the sector
        const std::int32_t xr = xp - center.x;
        const std::int32_t yr = center.y - yp;   // screen to math coords
        const std::int32_t d2 = xr * xr + yr * yr;
        bool in = (d2 >= ri2) && (d2 < ro2);
        if (in && (span < 360)) {
          // the center pixel (only reached if inner_radius is 0) has no angle, it is treated
          // as lying on the 0° ray, so it belongs to exactly one sector of a tiled pie
          const std::int32_t px = d2 ? xr : 1, py = d2 ? yr : 0;
          // p is in the half open half plane [a, a + 180°) if it's strictly left of the ray a or on the ray a itself
          const std::int32_t cs = xss * py - yss * px, ce = xse * py - yse * px;    // cross products
          const bool ccw_s = (cs > 0) || ((cs == 0) && ((xss * px + yss * py) > 0));
          const bool ccw_e = (ce > 0) || ((ce == 0) && ((xse * px + yse * py) > 0));
          // span < 180: [s, e) = [s, s + 180) and not [e, e + 180)
          // span >= 180: [s, e) = [s, s + 180) or not [e, e + 180)
          in = (span < 180) ? (ccw_s && !ccw_e) : (ccw_s || !ccw_e);
        }
        if (in) {
          if (!inside) {
            inside = true;
            lxp    = xp;
          }
        }
        else if (inside) {
          line_horz({ static_cast<std::int16_t>(lxp), static_cast<std::int16_t>(yp) }, { static_cast<std::int16_t>(xp - 1), static_cast<std::int16_t>(yp) });
          inside = false;
        }
      }
    }
    present_lock(false);
  }


  /**
   * Fill a region with the drawing color
   * This routine is only working on displays which support the pixel_get() function
   * \param start Start vertex inside the region to fill
   * \param color Color of the surrounding bound or the background color, depending on use_color_as_bounding
   * \param use_color_as_bounding True to use color as bouning color, false to use color as background color which should be filled
   */
  void fill(vertex_type start, color::value_type color, bool use_color_as_bounding = true)
  {
    class floodfill
    {
      // a stack entry take 8 byte
      typedef struct tag_segment_type
      {
        std::int16_t x_start;
        std::int16_t x_end;
        std::int16_t y;
        std::int8_t  dir;             // 0: no previous segment, -1: above the previous segment, 1: below the previous segment
        std::uint8_t scan_left  : 1;
        std::uint8_t scan_right : 1;
      } segment_type;

      gpr&               gpr_;
      shader::base*      pipe_;                  // cached shader pipe
      color::value_type  color_;
      bool               use_color_as_bounding_;
      std::size_t        stack_count_;
      segment_type       stack_[VIC_GPR_FILL_STACK_SIZE / 8U];

      inline void stack_push(segment_type segment)
      {
        if (stack_count_ < (VIC_GPR_FILL_STACK_SIZE / 8U)) {
          stack_[stack_count_++] = segment;
        }
      }

      inline segment_type& stack_pop()
      { return stack_[--stack_count_]; }

      // returns true if pixel is of : drawing color or
      // use_color_as_bounding_ == true  : bounding color
      // use_color_as_bounding_ == false : not of background color
      inline bool test(const vertex_type& point) const
      {
        const color::value_type col = pipe_->pixel_get(point);
        return (col == gpr_.color_)                         ||
               ((col == color_) &&  use_color_as_bounding_) ||
               ((col != color_) && !use_color_as_bounding_);
      }

      void fill(const vertex_type& start)
      {
        stack_push({ start.x, static_cast<std::int16_t>(start.x + 1), start.y, 0, 1U, 1U });
        pipe_->pixel_set({ start.x, start.y }, gpr_.color_);

        do {
          segment_type r = stack_pop();
          std::int16_t x_start = r.x_start, x_end = r.x_end;
          if (r.scan_left) { // if we should extend the segment towards the left...
            while (x_start > 0 && !test({ static_cast<std::int16_t>(x_start - 1), r.y })) {
              pipe_->pixel_set({ --x_start, r.y }, gpr_.color_);
            }
          }
          if (r.scan_right) {
            while (x_end < gpr_.screen_width() && !test({ x_end, r.y })) {
              pipe_->pixel_set({ x_end++, r.y }, gpr_.color_);
            }
          }
          // at this point, the segment from startX (inclusive) to endX (exclusive) is filled. compute the region to ignore
          r.x_start--;  // since the segment is bounded on either side by filled cells or array edges, we can extend the size of
          r.x_end++;    // the region that we're going to ignore in the adjacent lines by one
          // scan above and below the segment and add any new segments we find
          if (r.y > 0) {
            add_line(x_start, x_end, r.y - 1, r.x_start, r.x_end, -1, r.dir <= 0);
          }
          if (r.y < gpr_.screen_height() - 1) {
            add_line(x_start, x_end, r.y + 1, r.x_start, r.x_end,  1, r.dir >= 0);
          }
        } while (stack_count_ != 0U);
      }

      void add_line(std::int16_t x_start, std::int16_t x_end, std::int16_t y, std::int16_t ignore_start, std::int16_t ignore_end, std::int8_t dir, bool is_next_in_dir)
      {
        std::int16_t region_start = -1, x;
        for (x = x_start; x < x_end; ++x) { // scan the width of the parent segment
          if ((is_next_in_dir || x < ignore_start || x >= ignore_end) && !test({ x, y })) {   // if we're outside the region we should ignore and the cell is clear
            pipe_->pixel_set({ x, y }, gpr_.color_);       // fill the cell
            if (region_start < 0)
              region_start = x;             // and start a new segment if we haven't already
          }
          else if (region_start >= 0) {     // otherwise, if we shouldn't fill this cell and we have a current segment...
            stack_push({ region_start, x, y, dir, static_cast<std::uint8_t>(region_start == x_start ? 1U : 0U), 0U });   // push the segment
            region_start = -1;              // and end it
          }
          if (!is_next_in_dir && x < ignore_end && x >= ignore_start) {
            x = ignore_end - 1;             // skip over the ignored region
          }
        }
        if (region_start >= 0) {
          stack_push({ region_start, x, y, dir, static_cast<std::uint8_t>(region_start == x_start ? 1U : 0U), 1U });
        }
      }

    public:
      floodfill(gpr& _gpr, vertex_type& start, color::value_type color, bool use_color_as_bounding)
        : gpr_(_gpr)
        , pipe_(_gpr.shader_pipe())
        , color_(color)
        , use_color_as_bounding_(use_color_as_bounding)
        , stack_count_(0U)
      {
        fill(start);
      }
    };

    floodfill _floodfill(*this, start, color, use_color_as_bounding);
    present();
  }

};

} // namespace vic

#endif  // _VIC_GPR_H_
