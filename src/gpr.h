///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2001-2017, PALANDesign Hannover, Germany
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
protected:

  // virtual context/driver functions
  virtual std::uint16_t     screen_width(void) const = 0;
  virtual std::uint16_t     screen_height(void) const = 0;
  virtual shader::base*     shader_pipe(void) const = 0;
  virtual void              present(void) = 0;
  virtual void              present_lock(bool lock = true) = 0;

  /**
   * Anti Alising renderer
   */
  class anti_aliasing
  {
    gpr&        gpr_;
    vertex_type pipe_[3];   // 3 pixel pipe
    std::size_t pipe_idx_;

  public:
    anti_aliasing(gpr& _gpr)
      : gpr_(_gpr)
      , pipe_idx_(0)
    { }

    void render(vertex_type v)
    {
//WIP
      gpr_.shader_pipe()->pixel_set({v.x, v.y}, color::yellow);

      // calculate gradient
      const std::int16_t dx = v.x - pipe_[0].x;
      const std::int16_t dy = v.y - pipe_[0].y;

      // put the new vertex in the pipe
      pipe_[0] = pipe_[1];
      pipe_[1] = v;

      // check conditions
      if (util::abs<std::int16_t>(dx) > 2 || util::abs<std::int16_t>(dy) > 2) {
        // antialising not possible, distance too far
        gpr_.shader_pipe()->pixel_set({ v.x, v.y }, color::brightred);
        return;
      }
      if (dx == 0 || dy == 0) {
        // antialising not necessary, 90� or 180�
        gpr_.shader_pipe()->pixel_set({ v.x, v.y }, color::brightgreen);
        return;
      }

#if 0
      // ---->
      // |
      // v

      //  0
      //    V
      if (dx == 1 && dy == 1) {
        pixel_set(v.x - 1, v.y, 50 % );
        pixel_set(v.x, v.y - 1, 50 % );
      }

      //  V
      //    0
      if (dx == -1 && dy == -1) {
        pixel_set(v.x + 1, v.y, 50 % );
        pixel_set(v.x, v.y + 1, 50 % );
      }

      //    V
      //  0
      if (dx == 1 && dy == -1) {
        pixel_set(v.x - 1, v.y, 50 % );
        pixel_set(v.x, v.y + 1, 50 % );
      }

      //    0
      //  V
      if (dx == -1 && dy == 1) {
        pixel_set(v.x + 1, v.y, 50 % );
        pixel_set(v.x, v.y - 1, 50 % );
      }

      //  0
      //  1
      //    V
      if (dx == 1 && dy == 2) {
        pixel_set(v.x - 1, v.y, 50 % );
        pixel_set(v.x, v.y - 1, 50 % );
      }

 // table
//    []   []
//    dx   dy   p1.x   p1.y   p1.m   p2.x   p2.y   p2.m   p3.x   p3.y   p3.m
       1,   1,   -1,    0,     50,    0,     -1,    50,    0,     0,      0,
      -1,  -1,    1,    0,     50,    0,      1,    50,    0,     0,      0,
       1,  -1,   -1,    0,     50,    0,      1,    50,    0,     0,      0,
      -1,   1,    1,    0,     50,    0,     -1,    50,    0,     0,      0,

       2,   2,   -1,    0,     50,    0,     -1,    50,    0,     -1,    50,
      -2,  -2,    1,    0,     50,    0,      1,    50,    0,      1,    50,
       2,  -2,   -1,    0,     50,    0,      1,    50,    0,      1,    50,
      -2,   2,    1,    0,     50,    0,     -1,    50,    0,     -1,    50,

       1,   2,   -1,    0,     50,    0,     -1,    50,    0,     -1,    50,
      -1,  -2,    1,    0,     50,    0,      1,    50,    0,      1,    50,
       1,  -2,   -1,    0,     50,    0,      1,    50,    0,      1,    50,
      -1,   2,    1,    0,     50,    0,     -1,    50,    0,     -1,    50,

       2,   1,   -1,    0,     50,    0,     -1,    50,    0,     -1,    50,
      -2,  -1,    1,    0,     50,    0,      1,    50,    0,      1,    50,
       2,  -1,   -1,    0,     50,    0,      1,    50,    0,      1,    50,
      -2,   1,    1,    0,     50,    0,     -1,    50,    0,     -1,    50,


    drv_pixel_set_color(v.x + p1.x, v.y + p1.y, color::mix(color_, drv_pixel_get(v.x + p1.x, v.y + p1.y), p1.m));

#endif



/*

      // calculate the gradient for "bluring the tail"
      std::int16_t dx = abs<std::int16_t>(pipe_[2].x - pipe_[0].x);
      std::int16_t dy = abs<std::int16_t>(pipe_[2].y - pipe_[0].y);

      if (dy * 2 <= dx) {
        // 0 - 30�
      }
      if (dy <= dx) {
        // 30 - 45�
      }
      if (dx * 2 <= dy) {
        // 90 - 75�
      }
      if (dx <= dy) {
        // 45 - 30�
      }


  //     _ _|_ _
  //    |       |
  //    |       |_
  //    |       |
  //    |_ _ _ _|
  //
  //

      // render anti aliasing
      switch (0) {
      case 0:
        break;
      case 30:
        pixel_set(x - 2, y);
        pixel_set(x - 1, y, 50 % );
        pixel_set(x, y + 1, 50 % );

        pixel_set(x, y, 75 % );
        break;
        //      drv_pixel_set_color(x0, y0 + 1, color::mix(color_, drv_pixel_get(x0, y0 + 1), lum));
      }
  */
    }
  };


public:

  /**
   * ctor
   * Init vars
   */
  gpr()
    : anti_aliasing_(false)   // no AA as default
    , color_(color::gray)     // default drawing color
  { }


  /**
   * Set drawing color
   * \param color Drawing color
   */
  void set_color(color::value_type color)
  { color_ = color; }


  /**
   * Get drawing color
   */
  color::value_type get_color() const
  { return color_; }



///////////////////////////////////////////////////////////////////////////////
// G R A P H I C   P R I M I T I V E   F U N C T I O N S

  /**
   * Enable/disable anti aliasing support
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
   * \param color Color of the pixel
   */
  inline void pixel_set(vertex_type vertex, color::value_type color)
  {
    shader_pipe()->pixel_set(vertex, color);
  }


  /**
   * Return the color of the pixel (is a slim wrapper for drv_pixel_get)
   * \param point Vertex of the pixel
   * \return Color of pixel in ARGB format
   */
  inline color::value_type pixel_get(vertex_type point)
  {
    return shader_pipe()->pixel_get(point);
  }


  /**
   * Plot a point (one pixel) in the actual drawing (pen) color
   * \param point Vertex to plot
   */
  inline void plot(vertex_type point)
  {
    plot(point, color_);
  }


  /**
   * Plot a point (one pixel) in the given color
   * \param point Vertex to plot
   * \param color Color of the pixel
   */
  inline void plot(vertex_type point, color::value_type color)
  {
    shader_pipe()->pixel_set(point, color);
    present();
  }


  /**
   * Draw a line with the actual selected pen (with pen color)
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line
   */
  void line(vertex_type v0, vertex_type v1)
  {
    // precalc constants
    const std::int16_t dx = v1.x > v0.x ? v1.x - v0.x : v0.x - v1.x;
    const std::int16_t dy = v1.y > v0.y ? v1.y - v0.y : v0.y - v1.y;
    const std::int16_t sx = v1.x > v0.x ? 1 : -1;
    const std::int16_t sy = v1.y > v0.y ? 1 : -1;
          std::int16_t er = dx - dy;

    // start Bresenham line algorithm
    if (anti_aliasing_) {
      anti_aliasing aa(*this);
      for (;;) {
        aa.render(v0);
        if (v0 == v1) {
          break;
        }
        std::int16_t er2 = er * 2;
        if (er2 + dy > 0) {
          er -= dy;
          v0.x += sx;
        }
        if (er2 < dx) {
          er += dx;
          v0.y += sy;
        }
      }
    }
    else {
      for (;;) {
        shader_pipe()->pixel_set(v0, color_);
        if (v0 == v1) {
          break;
        }
        std::int16_t er2 = er * 2;
        if (er2 + dy > 0) {
          er -= dy;
          v0.x += sx;
        }
        if (er2 < dx) {
          er += dx;
          v0.y += sy;
        }
      }
    }
    present();
  }


  /**
   * Draw a horizontal line, width is one pixel
   * This is should be done by a high speed driver implementation if no shader is used
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, y component is ignored
   * \return true if successful
   */
  inline void line_horz(vertex_type v0, vertex_type v1)
  {
    util::vertex_min_x(v0, v1);   // set v0 to min x
    for (; v0.x <= v1.x; ++v0.x) {
      shader_pipe()->pixel_set(v0, color_);
    }
    present();
  }


  /**
   * Draw a vertical line, width is one pixel
   * This is should be done by a high speed driver implementation if no shader is used
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, x component is ignored
   */
  inline void line_vert(vertex_type v0, vertex_type v1)
  {
    util::vertex_min_y(v0, v1);   // set v0 to min y
    for (; v0.y <= v1.y; ++v0.y) {
      shader_pipe()->pixel_set(v0, color_);
    }
    present();
  }


  /**
   * Draw a box (filled rectangle)
   * This is should be done by a high speed driver implementation if no shader is used
   * \param rect Box bounding
   */
  void box(rect_type rect)
  {
    for (std::int16_t y = rect.top; y <= rect.bottom; ++y) {
      for (std::int16_t x = rect.left; x <= rect.right; ++x) {
        shader_pipe()->pixel_set({ x, y }, color_);
      }
    }
    present();
  }


  /**
   * Draw a box (filled rectangle) with a corner radius
   * \param rect Box bounding
   * \param border_radius Radius of the corner, 0 for angular
   */
  void box(rect_type rect, std::uint16_t border_radius)
  {
    present_lock();
    box({ static_cast<std::int16_t>(rect.left + border_radius), rect.top, static_cast<std::int16_t>(rect.right - border_radius), static_cast<std::int16_t>(rect.top + border_radius) });
    box({ rect.left, static_cast<std::int16_t>(rect.top + border_radius), rect.right, static_cast<std::int16_t>(rect.bottom - border_radius) });
    box({ static_cast<std::int16_t>(rect.left + border_radius), static_cast<std::int16_t>(rect.bottom - border_radius), static_cast<std::int16_t>(rect.right - border_radius), rect.bottom });
    disc_sector({ static_cast<std::int16_t>(rect.right - border_radius), static_cast<std::int16_t>(rect.top    + border_radius) }, border_radius, 0U);
    disc_sector({ static_cast<std::int16_t>(rect.left  + border_radius), static_cast<std::int16_t>(rect.top    + border_radius) }, border_radius, 1U);
    disc_sector({ static_cast<std::int16_t>(rect.left  + border_radius), static_cast<std::int16_t>(rect.bottom - border_radius) }, border_radius, 2U);
    disc_sector({ static_cast<std::int16_t>(rect.right - border_radius), static_cast<std::int16_t>(rect.bottom - border_radius) }, border_radius, 3U);
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
    util::vertex_top_left(v0, v1);
    box({ v0.x, v0.y, v1.x, v1.y }, border_radius);
  }


  /**
   * Draw a rectangle (frame) with the current pen
   * \param rect Rectangle
   */
  void rectangle(rect_type rect)
  {
    present_lock();
    line(rect.top_left(), { rect.right, rect.top });
    line({ rect.right, rect.top }, rect.bottom_right());
    line(rect.bottom_right(), { rect.left, rect.bottom });
    line({ rect.left, rect.bottom }, rect.top_left());
    present_lock(false);    // unlock and present
  }


  /**
   * Draw a rectangle (frame) with the current pen, vertex to rect wrapper
   * \param v0 top/left vertex
   * \param v1 bottom/right vertex
   */
  void rectangle(vertex_type v0, vertex_type v1)
  {
    util::vertex_top_left(v0, v1);
    rectangle({ v0.x, v0.y, v1.x, v1.y });
  }


  /**
   * Draw a rectangle (frame) with a corner radius
   * \param rect Rectangle
   * \param border_radius Radius of the corner, 0 for angular
   */
  void rectangle(rect_type rect, std::uint16_t border_radius)
  {
    present_lock();
    line({ static_cast<std::int16_t>(rect.right - border_radius), rect.top }, { static_cast<std::int16_t>(rect.left + border_radius), rect.top });
    circle({ static_cast<std::int16_t>(rect.left + border_radius), static_cast<std::int16_t>(rect.top + border_radius) }, border_radius, 90U, 180U);
    line({ rect.left, static_cast<std::int16_t>(rect.top + border_radius) }, { rect.left, static_cast<std::int16_t>(rect.bottom - border_radius) });
    circle({ static_cast<std::int16_t>(rect.left + border_radius), static_cast<std::int16_t>(rect.bottom - border_radius) }, border_radius, 180U, 270U);
    line({ static_cast<std::int16_t>(rect.left + border_radius), rect.bottom }, { static_cast<std::int16_t>(rect.right - border_radius), rect.bottom });
    circle({ static_cast<std::int16_t>(rect.right - border_radius), static_cast<std::int16_t>(rect.bottom - border_radius) }, border_radius, 270U, 360U);
    line({ rect.right, static_cast<std::int16_t>(rect.bottom - border_radius) }, { rect.right, static_cast<std::int16_t>(rect.top + border_radius) });
    circle({ static_cast<std::int16_t>(rect.right - border_radius), static_cast<std::int16_t>(rect.top + border_radius) }, border_radius, 0U,  90U);
    present_lock(false);    // unlock and present
  }


  /**
   * Draw a rectangle (frame) with a corner radius, vertex to rect wrapper
   * \param v0 top/left vertex
   * \param v1 bottom/right vertex
   * \param border_radius Radius of the corner, 0 for angular
   */
  void rectangle(vertex_type v0, vertex_type v1, std::uint16_t border_radius)
  {
    util::vertex_top_left(v0, v1);
    rectangle({ v0.x, v0.y, v1.x, v1.y }, border_radius);
  }


  /**
   * Draw multiple connected lines
   * \param vertexes Pointer to array of vertexes, at least 2
   * \param vertex_count Number of vertexes in the array, at least 2
   */
  void polyline(const vertex_type* vertexes, std::size_t vertex_count)
  {
    if (vertex_count-- < 2U) {
      return;
    }
    present_lock();
    for (std::size_t n = 0U; n < vertex_count; ++n) {
      line(vertexes[n], vertexes[vertex_count + 1U]);
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

    // check if triangle is a horizontal line
    if ((v0.y == v1.y) && (v1.y == v2.y)) {
      line_horz(v0, v1);
      line_horz(v1, v2);
      present_lock(false);
      return;
    }

    // check if triangle is a vertical line
    if ((v0.x == v1.x) && (v1.x == v2.x)) {
      line_vert(v0, v1);
      line_vert(v1, v2);
      present_lock(false);
      return;
    }

    // compute triangle bounding box
    const std::int16_t min_x = util::min3(v0.x, v1.x, v2.x);
    const std::int16_t min_y = util::min3(v0.y, v1.y, v2.y);
    const std::int16_t max_x = util::max3(v0.x, v1.x, v2.x);
    const std::int16_t max_y = util::max3(v0.y, v1.y, v2.y);

    // triangle setup
    const std::int16_t a01 = v0.y - v1.y, B01 = v1.x - v0.x;
    const std::int16_t a12 = v1.y - v2.y, B12 = v2.x - v1.x;
    const std::int16_t a20 = v2.y - v0.y, B20 = v0.x - v2.x;

    // Barycentric coordinates at minX/minY corner
    vertex_type p = { min_x, min_y };
    std::int32_t w0_row = util::orient_2d(v1, v2, p);
    std::int32_t w1_row = util::orient_2d(v2, v0, p);
    std::int32_t w2_row = util::orient_2d(v0, v1, p);

    // rasterize
    anti_aliasing aa0(*this), aa1(*this);
    for (p.y = min_y; p.y <= max_y; ++p.y) {
      // Barycentric coordinates at start of row
      std::int32_t w0 = w0_row, w1 = w1_row, w2 = w2_row;
      std::int16_t l_x = 0;
      bool inside = false;
      for (p.x = min_x; p.x <= max_x; ++p.x) {
        // if p is on or inside all edges, render the pixel
        if (!inside && w0 <= 0 && w1 <= 0 && w2 <= 0) {
          inside = true;
          l_x = p.x;
        }
        if (inside && (w0 + a12 > 0 || w1 + a20 > 0 || w2 + a01 > 0)) {
          line_horz({ l_x, p.y }, { p.x, p.y });
          if (anti_aliasing_) {
            aa0.render({ l_x, p.y });
            aa1.render({ p.x, p.y });
          }
          break;
        }
        // one step to the right
        w0 += a12; w1 += a20; w2 += a01;
      }
      // one row step
      w0_row += B12; w1_row += B20; w2_row += B01;
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
  void circle(vertex_type center, std::uint16_t radius, std::uint16_t start_angle = 0U, std::uint16_t end_angle = 359U)
  {
    const std::int16_t xs = center.x + radius * util::cos(static_cast<std::int16_t>(start_angle)) / 16384;
    const std::int16_t ys = center.y - radius * util::sin(static_cast<std::int16_t>(start_angle)) / 16384;
    const std::int16_t xe = center.x + radius * util::cos(static_cast<std::int16_t>(end_angle))   / 16384;
    const std::int16_t ye = center.y - radius * util::sin(static_cast<std::int16_t>(end_angle))   / 16384;

    const std::uint_fast16_t qs = (start_angle / 90U);
    const std::uint_fast16_t qe = (end_angle - 1U) / 90U;
    const bool               os = (start_angle / 45U) & 1U;
    const bool               oe = ((end_angle - 1U) / 45U) & 1U;

    // aa renderer
    anti_aliasing aa(*this);

    bool render = false;
    for (std::uint_fast16_t q = qs; q <= qe; ++q) {
      std::int16_t r = static_cast<std::int16_t>(radius), x = 0, y = -r, err = 2 - r * 2;
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
          anti_aliasing_ ? aa.render(p) : shader_pipe()->pixel_set(p, color_);
        }
        r = err;
        if (r <= x)
          err += ++x * 2 + 1;
        if (r > y || err > x)
          err += ++y * 2 + 1;
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
    anti_aliasing aa0(*this), aa1(*this), aa2(*this), aa3(*this);

    radius++;
    const std::int16_t radius_sqr = radius * radius;
    for (std::int16_t y = -radius; y <= 0; ++y) {
      for (std::int16_t x = -radius; x <= 0; ++x) {
        if (x * x + y * y < radius_sqr) {
          line_horz({ static_cast<std::int16_t>(center.x - x), static_cast<std::int16_t>(center.y + y) }, { static_cast<std::int16_t>(center.x + x), static_cast<std::int16_t>(center.y + y) });
          if (y != 0) {
            line_horz({ static_cast<std::int16_t>(center.x - x), static_cast<std::int16_t>(center.y - y) }, { static_cast<std::int16_t>(center.x + x), static_cast<std::int16_t>(center.y - y) });
          }
          if (anti_aliasing_) {
            aa0.render({ static_cast<std::int16_t>(center.x + x), static_cast<std::int16_t>(center.y + y) });
            aa1.render({ static_cast<std::int16_t>(center.x - x), static_cast<std::int16_t>(center.y + y) });
            aa2.render({ static_cast<std::int16_t>(center.x + x), static_cast<std::int16_t>(center.y - y) });
            aa3.render({ static_cast<std::int16_t>(center.x - x), static_cast<std::int16_t>(center.y - y) });
          }
          break;
        }
      }
    }
    present();
  }


  /**
   * Draw a disc sector (filled quarter circle)
   * \param center Center value
   * \param radius Disc radius
   * \param quadrant Quadrant number: 0: top/right, 1: top/left, 2: bottom/left, 3: bottom/right
   */
  void disc_sector(vertex_type center, std::uint16_t radius, std::uint8_t quadrant)
  {
    anti_aliasing aa(*this);

    radius++;
    const std::int16_t radius_sqr = radius * radius;
    for (std::int16_t y = -radius; y <= 0; ++y) {
      for (std::int16_t x = -radius; x <= 0; ++x) {
        if (x * x + y * y < radius_sqr) {
          switch (quadrant) {
            case 0 :
              line_horz({ static_cast<std::int16_t>(center.x), static_cast<std::int16_t>(center.y + y) }, { static_cast<std::int16_t>(center.x - x), static_cast<std::int16_t>(center.y + y) });
              if (anti_aliasing_) {
                aa.render({ static_cast<std::int16_t>(center.x - x), static_cast<std::int16_t>(center.y + y) });
              }
              break;
            case 1 :
              line_horz({ static_cast<std::int16_t>(center.x), static_cast<std::int16_t>(center.y + y) }, { static_cast<std::int16_t>(center.x + x), static_cast<std::int16_t>(center.y + y) });
              if (anti_aliasing_) {
                aa.render({ static_cast<std::int16_t>(center.x + x), static_cast<std::int16_t>(center.y + y) });
              }
              break;
            case 2 :
              line_horz({ static_cast<std::int16_t>(center.x), static_cast<std::int16_t>(center.y - y) }, { static_cast<std::int16_t>(center.x + x), static_cast<std::int16_t>(center.y - y) });
              if (anti_aliasing_) {
                aa.render({ static_cast<std::int16_t>(center.x + x), static_cast<std::int16_t>(center.y - y) });
              }
              break;
            case 3 :
              line_horz({ static_cast<std::int16_t>(center.x), static_cast<std::int16_t>(center.y - y) }, { static_cast<std::int16_t>(center.x - x), static_cast<std::int16_t>(center.y - y) });
              if (anti_aliasing_) {
                aa.render({ static_cast<std::int16_t>(center.x - x), static_cast<std::int16_t>(center.y - y) });
              }
              break;
            default :
              break;
          }
          break;
        }
      }
    }
    present();
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
    present_lock();

    // angle:
    //   0° = 3 o'clock
    //   0° -  89°: Q1 (top/right)
    //  90° - 179°: Q2 (top/left)
    // 180° - 269°: Q3 (bottom/left)
    // 270° - 359°: Q4 (bottom/right)

    bool second_half = false;
    std::uint16_t end_angle2 = end_angle;
    if (end_angle > 180U) {
      end_angle = 180U;
    }

    // aa renderer
    anti_aliasing aa0(*this), aa1(*this), aa2(*this), aa3(*this);

    do {
      const std::int16_t xss = util::cos(static_cast<std::int16_t>(start_angle));  // no division of scaled sin/cos to use the full range
      const std::int16_t yss = util::sin(static_cast<std::int16_t>(start_angle));
      const std::int16_t xse = util::cos(static_cast<std::int16_t>(end_angle));
      const std::int16_t yse = util::sin(static_cast<std::int16_t>(end_angle));

      for (std::int16_t yp = center.y - outer_radius; yp <= center.y + outer_radius; yp++) {
        std::int16_t inside = 0, lxp = 0;
        for (std::int16_t xp = center.x - outer_radius; xp <= center.x + outer_radius; xp++) {
          // check if xp/yp is within the sector
          std::int16_t xr = xp - center.x;
          std::int16_t yr = center.y - yp;   // * -1 for coords to screen conversion
          if (((xr * xr + yr * yr) >= inner_radius * inner_radius) &&
              ((xr * xr + yr * yr) <  outer_radius * outer_radius) &&
              !((yss * xr) >  (xss * yr)) &&
               ((yse * xr) >= (xse * yr))
             ) {
            if (!(inside & 0x01U)) {
              inside++;
              lxp = xp;
            }
          }
          else {
            if (inside & 0x01U) {
              line_horz({ lxp, yp }, { static_cast<std::int16_t>(xp - 1), yp });
              if (anti_aliasing_ && !(inside & 0x02U)) {
                aa0.render({ lxp, yp });
                aa1.render({ static_cast<std::int16_t>(xp - 1), yp });
              }
              if (anti_aliasing_ && (inside & 0x02U)) {
                aa2.render({ lxp, yp });
                aa3.render({ static_cast<std::int16_t>(xp - 1), yp });
              }
              inside++;
            }
          }
        }
      }
      // second half (Q3/Q4) necessary?
      second_half = false;
      if (end_angle != end_angle2) {
        start_angle = 180U;
        end_angle   = end_angle2;
        second_half = true;
      }
    } while (second_half);
    present_lock(false);
  }


  /**
   * Fill region up to the bounding color with the drawing color
   * Fill routine is only working on displays which support pixel_get()
   * \param start Start value inside region to fill
   * \param bounding_color Color of the surrounding bound or bg_color to fill all what is of bg_color
   * \return true if successful
   */
  void fill(vertex_type start, color::value_type bounding_color)
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
      color::value_type  bounding_color_;
      std::size_t        stack_count_;
      segment_type       stack_[VIC_GPR_FILL_STACK_SIZE / 8];

      inline void stack_push(segment_type segment)
      {
        if (stack_count_ < (VIC_GPR_FILL_STACK_SIZE / 8)) {
          stack_[stack_count_++] = segment;
        }
      }

      inline segment_type& stack_pop()
      { return stack_[--stack_count_]; }

      // returns true if pixel is of border or drawing color
      // returns true if col != bg_color       && bounding_color == bg_color 
      // returns true if col == bounding_color && bounding_color != bg_color 
      // returns true if col == pen_color
      inline bool test(vertex_type point) const
      {
        const color::value_type col = gpr_.shader_pipe()->pixel_get({ point.x, point.y });
        return (col == gpr_.color_);          //                        ||
//             (col == bounding_color_ && bounding_color_ != gpr_.bg_get_color()) ||
//             (col != bounding_color_ && bounding_color_ == gpr_.bg_get_color());
      }

      void fill(vertex_type& start)
      {
        stack_push({ start.x, static_cast<std::int16_t>(start.x + 1), start.y, 0, 1U, 1U });
        gpr_.pixel_set({ start.x, start.y });

        do {
          segment_type r = stack_pop();
          std::int16_t x_start = r.x_start, x_end = r.x_end;
          if (r.scan_left) { // if we should extend the segment towards the left...
            while (x_start > 0 && !test({ static_cast<std::int16_t>(x_start - 1), r.y })) {
              gpr_.pixel_set({ --x_start, r.y });
            }
          }
          if (r.scan_right) {
            while (x_end < gpr_.screen_width() && !test({ x_end, r.y })) {
              gpr_.pixel_set({ x_end++, r.y });
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
            gpr_.pixel_set({ x, y });       // fill the cell
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
      floodfill(gpr& _gpr, vertex_type& start, color::value_type bounding_color)
      : gpr_(_gpr)
      , bounding_color_(bounding_color)
      , stack_count_(0U)
      {
        fill(start);
      }
    };

    floodfill _floodfill(*this, start, bounding_color);
    present();
  }


///////////////////////////////////////////////////////////////////////////////

protected:
  bool                anti_aliasing_;   // true if AA is enabled
  color::value_type   color_;           // actual drawing color

private:

  // non copyable
  const gpr& operator=(const gpr& rhs)
  { return rhs; }
};

} // namespace vic

#endif  // _VIC_GPR_H_
