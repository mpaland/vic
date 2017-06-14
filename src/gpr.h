///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2001-2017, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// This file is part of the vgx library.
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

#ifndef _VGX_GPR_H_
#define _VGX_GPR_H_

#include "base.h"


namespace vgx {


typedef struct struct_pen_type {
  std::int8_t   x;
  std::int8_t   y;
  std::uint8_t  alpha;
  std::uint8_t  next;
} pen_type;


/**
 * Graphic Primitive Renderer
 */
class gpr : virtual public base
{
protected:

  /**
   * Helper function to swap two vertexes (as coordinates)
   * \param v0 First vertex
   * \param v1 Second vertex
   */
  inline void vertex_swap(vertex_type& v0, vertex_type& v1) const
  {
    const vertex_type vt = v0;
    v0 = v1;
    v1 = vt;
  }

  /**
   * Helper function to swap two vertexes so that first vertex contains min x
   * \param v_min_x vertex
   * \param v_max_x vertex
   */
  inline void vertex_min_x(vertex_type& v_min_x, vertex_type& v_max_x) const
  {
    if (v_min_x.x > v_max_x.x) {
      vertex_swap(v_min_x, v_max_x);
    }
  }

  /**
   * Helper function to swap two vertexes that first vertex contains min y
   * \param v_min_y vertex
   * \param v_max_y vertex
   */
  inline void vertex_min_y(vertex_type& v_min_y, vertex_type& v_max_y) const
  {
    if (v_min_y.y > v_max_y.y) {
      vertex_swap(v_min_y, v_max_y);
  }
  }


  inline void render_pen(vertex_type center)
  {
    std::size_t i = 0U;
    do {
      const vertex_type v = { static_cast<std::int16_t>(center.x + pen_shape_[i].x), static_cast<std::int16_t>(center.y + pen_shape_[i].y) };
      drv_pixel_set_color(v, pen_shape_[i].alpha == 0U ? color_pen_get(v) : color::mix(drv_pixel_get(v), color_pen_get(v), pen_shape_[i].alpha));
    } while (pen_shape_[i++].next);
  }


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

    inline void render(vertex_type v)
    {
//WIP
      gpr_.drv_pixel_set_color({v.x, v.y}, color::yellow);

      // calculate gradient
      const std::int16_t dx = v.x - pipe_[0].x;
      const std::int16_t dy = v.y - pipe_[0].y;

      // put the new vertex in the pipe
      pipe_[0] = pipe_[1];
      pipe_[1] = v;

      // check conditions
      if (util::abs<std::int16_t>(dx) > 2 || util::abs<std::int16_t>(dy) > 2) {
        // antialising not possible, distance too far
        gpr_.drv_pixel_set_color({ v.x, v.y }, color::brightred);
        return;
      }
      if (dx == 0 || dy == 0) {
        // antialising not necessary, 90° or 180°
        gpr_.drv_pixel_set_color({ v.x, v.y }, color::brightgreen);
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
        // 0 - 30°
      }
      if (dy <= dx) {
        // 30 - 45°
      }
      if (dx * 2 <= dy) {
        // 90 - 75°
      }
      if (dx <= dy) {
        // 45 - 30°
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
    : primitive_lock_(false)
    , anti_aliasing_(false)     // no AA as default
    , pen_shape_(nullptr)       // 1 pixel pen as default
  { }


///////////////////////////////////////////////////////////////////////////////
// G R A P H I C   P R I M I T I V E   F U N C T I O N S

  /**
   * Select the actual drawing pen
   * \param pen_shape Set the actual pen shape or nullptr for 1 pixel default pen (fastest)
   */
  inline void pen_set(const pen_type* pen_shape)
  {
    pen_shape_ = pen_shape;
  }


  /**
   * Return a stock pen
   * \param width Pen width
   */
  inline const pen_type* pen_get_stock(std::uint8_t width) const
  {
    // stock pens
    static const pen_type    pen_shape_width_1[1] = { { 0, 0, 0, 0 } };                                   // one pixel pen
    static const pen_type    pen_shape_width_2[4] = { { 0, 0, 0, 1 }, { 1, 0, 0, 1 },
                                                      { 0, 1, 0, 1 }, { 1, 1, 0, 0 } };                   // two pixel pen
    static const pen_type    pen_shape_width_3[9] = { { 0, 0, 0, 1 }, { 1, 0, 0, 1 }, { 2, 0, 0, 1 },
                                                      { 0, 1, 0, 1 }, { 1, 1, 0, 1 }, { 2, 1, 0, 1 },
                                                      { 0, 2, 0, 1 }, { 1, 2, 0, 1 }, { 2, 2, 0, 0 } };   // three pixel pen

    const pen_type* stock_pen[4] = { nullptr, pen_shape_width_1, pen_shape_width_2, pen_shape_width_3 };
    return stock_pen[width];
  }


  /**
   * Set pixel in the actual pen color, no present is called (is a slim wrapper for drv_pixel_set_color
   * Uncommon to override this function, but may be done for in case of a monochrome displays
   * \param point Vertex to set
   */
  inline virtual void pixel_set(vertex_type point)
  {
    drv_pixel_set_color(point, color_pen_get(point));
  }


  /**
   * Set pixel in the actual pen color, no present is called (is a slim wrapper for drv_pixel_set_color
   * Uncommon to override this function, but may be done for in case of a monochrome displays
   * \param point Vertex to set
   * \param color Color of the pixel
   */
  inline virtual void pixel_set(vertex_type point, color::value_type color)
  {
    drv_pixel_set_color(point, color);
  }


  /**
   * Return the color of the pixel (is a slim wrapper for drv_pixel_get)
   * \param point Vertex of the pixel
   * \return Color of pixel in ARGB format
   */
  inline virtual color::value_type pixel_get(vertex_type point)
  {
    return drv_pixel_get(point);
  }


  /**
   * Plot a point in the actual drawing (pen) color
   * \param point Vertex to plot
   */
  void plot(vertex_type point)
  {
    drv_pixel_set_color(point, color_pen_get(point));
    present();
  }


  /**
   * Plot a point with a given color, drawing color is not affected
   * \param point Vertex to plot
   * \param color Color of the pixel
   */
  void plot(vertex_type point, color::value_type color)
  {
    drv_pixel_set_color(point, color);
    present();
  }


  /**
   * Draw a line with the actual selected pen in drawing (pen) color
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
    if (pen_shape_) {
      for (;;) {
        render_pen(v0);
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
    else if (anti_aliasing_) {
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
        pixel_set(v0);
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
   * Draw a horizontal line, width is one pixel, no pen style support
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, y component is ignored
   * \return true if successful
   */
  virtual void line_horz(vertex_type v0, vertex_type v1)
  {
    // set v0 to min x
    vertex_min_x(v0, v1);

    if (color_pen_is_function()) {
      for (; v0.x <= v1.x; ++v0.x) {
        pixel_set(v0);
      }
    }
    else {
      for (; v0.x <= v1.x; ++v0.x) {
        pixel_set(v0, color_pen_get());
      }
    }
    present();
  }


  /**
   * Draw a vertical line, width is one pixel, no pen style support
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, x component is ignored
   */
  virtual void line_vert(vertex_type v0, vertex_type v1)
  {
    // set v0 to min y
    vertex_min_y(v0, v1);

    if (color_pen_is_function()) {
      for (; v0.y <= v1.y; ++v0.y) {
        pixel_set(v0);
      }
    }
    else {
      for (; v0.y <= v1.y; ++v0.y) {
        pixel_set(v0, color_pen_get());
      }
    }
    present();
  }


  /**
   * Draw a rectangle (frame) with the current pen
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in rect
   * \param y1 Y end value, included in rect
   * \return true if successful
   */
  virtual void rectangle(vertex_type v0, vertex_type v1)
  {
    present_lock();
    line(v0, { v1.x, v0.y });
    line(v1, { v1.x, v0.y });
    line(v0, { v0.x, v1.y });
    line(v1, { v0.x, v1.y });
    present_lock(false);    // unlock and present
  }


  /**
   * Draw a box (filled rectangle)
   * \param x0 X start value
   * \param y0 Y start value
   * \param x1 X end value, included in box
   * \param y1 Y end value, included in box
   * \return true if successful
   */
  virtual void box(vertex_type v0, vertex_type v1)
  {
    // set v0 to min y
    vertex_min_y(v0, v1);

    present_lock();
    for (; v0.y <= v1.y; ++v0.y) {
      line_horz(v0, v1);
    }
    present_lock(false);    // unlock and present
  }


  /**
   * Draw a multiple lines
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
    line(v0, v2);
    line(v1, v2);
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
    std::int16_t w0_row = util::orient_2d(v1, v2, p);
    std::int16_t w1_row = util::orient_2d(v2, v0, p);
    std::int16_t w2_row = util::orient_2d(v0, v1, p);

    // rasterize
    anti_aliasing aa0(*this), aa1(*this);
    for (p.y = min_y; p.y <= max_y; ++p.y) {
      // Barycentric coordinates at start of row
      std::int16_t w0 = w0_row, w1 = w1_row, w2 = w2_row, l_x;
      bool inside = false;
      for (p.x = min_x; p.x <= max_x; ++p.x) {
        // if p is on or inside all edges, render the pixel
        if (!inside && w0 <= 0 && w1 <= 0 && w2 <= 0) {
          // if (w0 & w1 & w2 & 0x8000) {
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
   * Draw an arc (Bezier curve)
   * \param v0 Start value, included in arc
   * \param v1 Mid value, included in arc
   * \param v2 End value, included in arc
   */
  void arc(vertex_type v0, vertex_type v1, vertex_type v2)
  {
    // sign of gradient must not change
    if ((v0.x - v1.x) * (v2.x - v1.x) > 0 ||
        (v0.y - v1.y) * (v2.y - v1.y) > 0) {
      return;
    }

    std::int16_t sx = v2.x - v1.x, sy = v2.y - v1.y;
    std::int32_t xx = v0.x - v1.x, yy = v0.y - v1.y, xy;  // relative values for checks
    std::int32_t dx, dy, err, cur = xx * sy - yy * sx;    // curvature
    anti_aliasing aa(*this);

    // sign of gradient must not change
    if (xx * sx > 0 || yy * sy > 0) {
      return;
    }

    if (sx * (std::int32_t)sx + sy * (std::int32_t)sy > xx * xx + yy * yy) {      // begin with longer part
      v2.x = v0.x; v0.x = sx + v1.x; v2.y = v0.y; v0.y = sy + v1.y; cur = -cur;   // swap
    }
    if (cur != 0) {                                   // no straight line
      xx += sx; xx *= sx = v0.x < v2.x ? 1 : -1;      // x step direction
      yy += sy; yy *= sy = v0.y < v2.y ? 1 : -1;      // y step direction
      xy = 2 * xx * yy;
      xx *= xx;
      yy *= yy;                                       // differences 2nd degree
      if (cur * sx * sy < 0) {                        // negated curvature?
        xx = -xx; yy = -yy; xy = -xy; cur = -cur;
      }
      dx = 4 * sy * cur * (v1.x - v0.x) + xx - xy;    // differences 1st degree
      dy = 4 * sx * cur * (v0.y - v1.y) + yy - xy;
      xx += xx;
      yy += yy;
      err = dx + dy + xy;                             // error 1st step
      do {
        pen_shape_ ? render_pen(v0) : anti_aliasing_ ? aa.render(v0) : pixel_set(v0);
        if (v0.x == v2.x && v0.y == v2.y) {
          return;                                     // curve finished
        }
        v1.y = 2 * err < dx;                          // save value for test of y step
        if (2 * err > dy) {
          v0.x += sx; dx -= xy; err += dy += yy;      // x step
        }
        if (v1.y) {
          v0.y += sy; dy -= xy; err += dx += xx;      // y step
        }
      } while (dy < dx);                              // gradient negates -> algorithm fails
    }
    line(v0, v2);                                     // plot remaining part to end
    present();
  }


  /**
   * Draw a circle
   * \param center Center vertex
   * \param radius Circle radius
   */
  void circle(vertex_type center, std::uint16_t radius)
  {
    std::int16_t xo = static_cast<std::int16_t>(radius), yo = 0, err = 1 - xo;

    if (pen_shape_ || !anti_aliasing_) {
      // render with pen
      while (xo >= yo) {
        render_pen({ static_cast<std::int16_t>(center.x + xo), static_cast<std::int16_t>(center.y + yo) });  // q4
        render_pen({ static_cast<std::int16_t>(center.x + xo), static_cast<std::int16_t>(center.y - yo) });  // q1
        render_pen({ static_cast<std::int16_t>(center.x + yo), static_cast<std::int16_t>(center.y + xo) });  // q4
        render_pen({ static_cast<std::int16_t>(center.x + yo), static_cast<std::int16_t>(center.y - xo) });  // q1
        render_pen({ static_cast<std::int16_t>(center.x - xo), static_cast<std::int16_t>(center.y + yo) });  // q3
        render_pen({ static_cast<std::int16_t>(center.x - xo), static_cast<std::int16_t>(center.y - yo) });  // q2
        render_pen({ static_cast<std::int16_t>(center.x - yo), static_cast<std::int16_t>(center.y + xo) });  // q3
        render_pen({ static_cast<std::int16_t>(center.x - yo), static_cast<std::int16_t>(center.y - xo) });  // q2
        yo++;
        if (err < 0) {
          err += 2 * yo + 1;
        }
        else {
          xo--;
          err += 2 * (yo - xo + 1);
        }
      }
    }
    else {
      // render antialiased
      anti_aliasing aa0(*this), aa1(*this), aa2(*this), aa3(*this),
                    aa4(*this), aa5(*this), aa6(*this), aa7(*this);
      while (xo >= yo) {
        aa0.render({ static_cast<std::int16_t>(center.x + xo), static_cast<std::int16_t>(center.y + yo) });  // q4
        aa1.render({ static_cast<std::int16_t>(center.x + xo), static_cast<std::int16_t>(center.y - yo) });  // q1
        aa2.render({ static_cast<std::int16_t>(center.x + yo), static_cast<std::int16_t>(center.y + xo) });  // q4
        aa3.render({ static_cast<std::int16_t>(center.x + yo), static_cast<std::int16_t>(center.y - xo) });  // q1
        aa4.render({ static_cast<std::int16_t>(center.x - xo), static_cast<std::int16_t>(center.y + yo) });  // q3
        aa5.render({ static_cast<std::int16_t>(center.x - xo), static_cast<std::int16_t>(center.y - yo) });  // q2
        aa6.render({ static_cast<std::int16_t>(center.x - yo), static_cast<std::int16_t>(center.y + xo) });  // q3
        aa7.render({ static_cast<std::int16_t>(center.x - yo), static_cast<std::int16_t>(center.y - xo) });  // q2
        yo++;
        if (err < 0) {
          err += 2 * yo + 1;
        }
        else {
          xo--;
          err += 2 * (yo - xo + 1);
        }
      }
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
          line_horz({ static_cast<std::int16_t>(center.x - x), static_cast<std::int16_t>(center.y - y) }, { static_cast<std::int16_t>(center.x + x), static_cast<std::int16_t>(center.y - y) });
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
   * \param sector Sector number: 0: top/left, 1: top/right, 2: bottom/right, 3: bottom/left
   */
  void disc_sector(vertex_type center, std::uint16_t radius, std::uint8_t sector)
  {
    anti_aliasing aa(*this);

    radius++;
    const std::int16_t radius_sqr = radius * radius;
    for (std::int16_t y = -radius; y <= 0; ++y) {
      for (std::int16_t x = -radius; x <= 0; ++x) {
        if (x * x + y * y < radius_sqr) {
          switch (sector) {
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
   * \param start_angle Start angle in degree, 0° is horizontal right, counting anticlockwise
   * \param end_angle End angle in degree
   */
  void sector(vertex_type center, std::uint16_t inner_radius, std::uint16_t outer_radius, std::uint16_t start_angle, std::uint16_t end_angle)
  {
    // angle:
    //   0° = 3 o'clock
    //   0° -  89°: Q1 (top/right)
    //  90° - 179°: Q2 (top/left)
    // 180° - 269°: Q3 (bottom/left)
    // 270° - 359°: Q4 (bottom/right)

    bool second_half;
    std::uint16_t end_angle2 = end_angle;
    if ((end_angle > start_angle && end_angle > start_angle + 180U) ||
      (start_angle > end_angle && end_angle + 180U > start_angle)) {
      // more than 180°
      end_angle = (start_angle + 180U) % 360U;
    }
// TBD: sine calc and AA
    do {
      bool q14s = (start_angle < 90U) || (start_angle >= 270U);
      bool q14e = (end_angle < 90U) || (end_angle >= 270U);
      bool q24s = (start_angle >= 90U && start_angle < 180U) || (start_angle >= 270U);
      bool q24e = (end_angle >= 90U && end_angle < 180U) || (end_angle >= 270U);

      std::int16_t xss = (std::uint8_t)(util::sin((q24s ? start_angle - 90U : start_angle) % 90U) >> (q24s ? 8U : 0U)) * (std::int16_t)(q14s ? 1 : -1);
      std::int16_t yss = (std::uint8_t)(util::sin((q24s ? start_angle - 90U : start_angle) % 90U) >> (q24s ? 0U : 8U)) * (std::int16_t)((start_angle < 180U) ? 1 : -1);
      std::int16_t xse = (std::uint8_t)(util::sin((q24e ? end_angle - 90U : end_angle) % 90U) >> (q24e ? 8U : 0U)) * (std::int16_t)(q14e ? 1 : -1);
      std::int16_t yse = (std::uint8_t)(util::sin((q24e ? end_angle - 90U : end_angle) % 90U) >> (q24e ? 0U : 8U)) * (std::int16_t)((end_angle < 180U) ? 1 : -1);

      for (std::int16_t yp = center.y - outer_radius; yp <= center.y + outer_radius; yp++) {
        for (std::int16_t xp = center.x - outer_radius; xp <= center.x + outer_radius; xp++) {
          // check if xp/yp is within the sector
          std::int16_t xr = xp - center.x;
          std::int16_t yr = center.y - yp;   // * -1 for coords to screen conversion
          if (((xr * xr + yr * yr) >= inner_radius * inner_radius) &&
            ((xr * xr + yr * yr) < outer_radius * outer_radius) &&
            !((yss * xr) > (xss * yr)) &&
            ((yse * xr) >= (xse * yr))
            ) {
            pixel_set({ xp, yp });
          }
        }
      }
      // second half necessary?
      second_half = false;
      if (end_angle != end_angle2) {
        start_angle = end_angle;
        end_angle = end_angle2;
        second_half = true;
      }
    } while (second_half);
    present();
  }

  ///////////////////////////////////////////////////////////////////////////////

  /**
   *  Fill region up to the bounding color with the drawing color
   * \param start Start value inside region to fill
   * \param bounding_color Color of the surrounding bound
   * \return true if successful
   */
  void fill(vertex_type start, color::value_type bounding_color)
  {
    // TBD
    (void)start; (void)bounding_color;
    present();
  }

  ///////////////////////////////////////////////////////////////////////////////


  /**
   * Move display area
   * \param source Source vertex
   * \param destination Destination vertex
   * \param width Width of the area
   * \param height Height of the area
   * \return true if successful
   */
  virtual void move(vertex_type source, vertex_type destination, std::uint16_t width, std::uint16_t height)
  {
    std::uint16_t w, h;
    if (source.x < destination.x) {
      if (source.y < destination.y) {
        for (h = height; h != 0U; --h) {
          for (w = width; w != 0U; --w) {
            drv_pixel_set_color({ static_cast<std::int16_t>(destination.x + w), static_cast<std::int16_t>(destination.y + h) }, drv_pixel_get({ static_cast<std::int16_t>(source.x + w), static_cast<std::int16_t>(source.y + h) }));
          }
        }
      }
      else {
        for (h = 0U; h < height; ++h) {
          for (w = width; w != 0U; --w) {
            drv_pixel_set_color({ static_cast<std::int16_t>(destination.x + w), static_cast<std::int16_t>(destination.y + h) }, drv_pixel_get({ static_cast<std::int16_t>(source.x + w), static_cast<std::int16_t>(source.y + h) }));
          }
        }
      }
    }
    else {
      if (source.y < destination.y) {
        for (h = height; h != 0U; --h) {
          for (w = 0U; w < width; ++w) {
            drv_pixel_set_color({ static_cast<std::int16_t>(destination.x + w), static_cast<std::int16_t>(destination.y + h) }, drv_pixel_get({ static_cast<std::int16_t>(source.x + w), static_cast<std::int16_t>(source.y + h) }));
          }
        }
      }
      else {
        for (h = 0U; h < height; ++h) {
          for (w = 0U; w < width; ++w) {
            drv_pixel_set_color({ static_cast<std::int16_t>(destination.x + w), static_cast<std::int16_t>(destination.y + h) }, drv_pixel_get({ static_cast<std::int16_t>(source.x + w), static_cast<std::int16_t>(source.y + h) }));
          }
        }
      }
    }
    present();
  }


  /**
   * Enable/disable anti aliasing support
   * \param enable True to enable anti aliasing
   */
  void anti_aliasing_set(bool enable)
  {
    anti_aliasing_ = enable;
  }


///////////////////////////////////////////////////////////////////////////////

protected:
  bool              primitive_lock_;  // lock for rendering multiple primitives without refresh
  bool              anti_aliasing_;   // true if AA is enabled

  const pen_type*   pen_shape_;       // actual selected drawing pen

private:

  // non copyable
  const gpr& operator=(const gpr& rhs) { return rhs; }
};

} // namespace vgx

#endif  // _VGX_GPR_H_
