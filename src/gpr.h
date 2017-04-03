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
   * Helper function to calculate (lookup) sin(x) normalized to 10000
   * \param angle Angle in degree, valid range from 0� to 360�
   * \return sin(x) * 10000 in upper int16
   */
  std::int16_t sin(std::uint16_t angle) const   // sin(x) helper function
  {
    const std::uint16_t val[90] = { 8989 
// TBD 
    
    };
    return angle <= 180U ? val[angle % 90U] : -val[angle % 90U];
  }

  /**
   * Helper function to calculate (lookup) cos(x) normalized to 10000
   * \param angle Angle in degree, valid range from 0� to 360�
   * \return cos(x) * 10000 in upper int16
   */
  std::int16_t cos(std::uint16_t angle) const   // sin(x) helper function
  {
    const std::uint16_t val[90] = { 8989 
// TBD 
    };
    return (angle <= 90U || angle >= 270) ? val[angle % 90U] : -val[angle % 90U];
  }

  /**
   * Helper function to calculate (lookup) sin(x) and cos(x), normalized to 100
   * \param angle Angle in degree, valid range from 0� to 90�
   * \return sin(x) * 100 in upper byte, cos(x) * 100 in lower byte
   */
/*
  inline std::uint16_t sin(std::uint8_t angle) const   // sin(x) helper function
  {
    const std::uint16_t angle_to_xy[91U] = {
      0x0064U, 0x0264U, 0x0364U, 0x0564U, 0x0764U, 0x0964U, 0x0A63U, 0x0C63U, 0x0E63U, 0x1063U, 0x1162U, 0x1362U, 0x1562U, 0x1661U, 0x1861U, 0x1A61U,
      0x1C60U, 0x1D60U, 0x1F5FU, 0x215FU, 0x225EU, 0x245DU, 0x255DU, 0x275CU, 0x295BU, 0x2A5BU, 0x2C5AU, 0x2D59U, 0x2F58U, 0x3057U, 0x3257U, 0x3456U,
      0x3555U, 0x3654U, 0x3853U, 0x3952U, 0x3B51U, 0x3C50U, 0x3E4FU, 0x3F4EU, 0x404DU, 0x424BU, 0x434AU, 0x4449U, 0x4548U, 0x4747U, 0x4845U, 0x4944U,
      0x4A43U, 0x4B42U, 0x4D40U, 0x4E3FU, 0x4F3EU, 0x503CU, 0x513BU, 0x5239U, 0x5338U, 0x5436U, 0x5535U, 0x5634U, 0x5732U, 0x5730U, 0x582FU, 0x592DU,
      0x5A2CU, 0x5B2AU, 0x5B29U, 0x5C27U, 0x5D25U, 0x5D24U, 0x5E22U, 0x5F21U, 0x5F1FU, 0x601DU, 0x601CU, 0x611AU, 0x6118U, 0x6116U, 0x6215U, 0x6213U,
      0x6211U, 0x6310U, 0x630EU, 0x630CU, 0x630AU, 0x6409U, 0x6407U, 0x6405U, 0x6403U, 0x6402U, 0x6400U
    };
    return angle <= 90U ? angle_to_xy[angle] : 0U;   // out of bounds returns 0
  }
*/


  inline std::int16_t orient_2d(vertex_type a, vertex_type b, vertex_type c) const
  {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
  }


  /**
   * Helper function to swap two vertexes (as coordinates)
   * \param v0 First vertex
   * \param v1 Second vertex
   */
  inline void vertex_swap(vertex_type& v0, vertex_type& v1) const
  {
    const vertex_type t = v0;
    v0 = v1;
    v1 = t; 
  }

  /**
   * Helper function to rotate a point of a given angle in respect to given center
   */
  inline vertex_type vertex_rotate(vertex_type center, vertex_type point, std::int16_t angle) const
  {
    const std::int32_t s = sin(angle);
    const std::int32_t c = cos(angle);
  
    // translate point to center
    point.x -= center.x;
    point.y -= center.y;

    // rotate point and translate back
    vertex_type rp;
    rp.x = static_cast<std::int16_t>(((std::int32_t)point.x * c - (std::int32_t)point.y * s) / 10000) + center.x;
    rp.y = static_cast<std::int16_t>(((std::int32_t)point.x * s - (std::int32_t)point.y * c) / 10000) + center.y;

    return rp;
  }


  inline void render_pen(vertex_type center)
  {
    std::size_t i = 0U;
    do {
      const vertex_type v = { center.x + pen_shape_[i].x, center.y + pen_shape_[i].y };
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

      // put the new vertex in the pipe
      pipe_[0] = pipe_[1];
      pipe_[1] = pipe_[2];
      pipe_[2] = v;

      // check the distance
      const std::int16_t dx = util::abs<std::int16_t>(pipe_[2].x - pipe_[1].x);
      const std::int16_t dy = util::abs<std::int16_t>(pipe_[2].y - pipe_[1].y);
      if (dx > 1 && dy > 1) {
        // antialising not possible, distance too far
        gpr_.drv_pixel_set_color({ v.x, v.y }, color::brightred);
        return;
      }
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
  void pen_set(const pen_type* pen_shape)
  {
    pen_shape_ = pen_shape;
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
  inline virtual color::value_type pixel_get(vertex_type point) const
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
    const std::int16_t dx = v1.x > v0.x ? v1.x - v0.x : v0.x - v1.x;
    const std::int16_t dy = v1.y > v0.y ? v1.y - v0.y : v0.y - v1.y;
    const std::int16_t sx = v1.x > v0.x ? 1 : -1;
    const std::int16_t sy = v1.y > v0.y ? 1 : -1;
          std::int16_t er = dx - dy;

    // start Bresenham line algorithm
    if (pen_shape_) {
      for (;;) {
        render_pen(v0);
        if (v0.x == v1.x && v0.y == v1.y) {
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
        if (v0.x == v1.x && v0.y == v1.y) {
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
        if (v0.x == v1.x && v0.y == v1.y) {
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
   * Draw a horizontal line, width is one pixel, no pen support
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, y component is ignored
   * \return true if successful
   */
  virtual void line_horz(vertex_type v0, vertex_type v1)
  {
    if (v0.x < v1.x) {
      for (; v0.x <= v1.x; ++v0.x) {
        pixel_set(v0);
      }
    }
    else {
      for (; v1.x <= v0.x; ++v1.x) {
        pixel_set(v1);
      }
    }
    present();
  }


  /**
   * Draw a vertical line, width is one pixel, no pen support
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, x component is ignored
   */
  virtual void line_vert(vertex_type v0, vertex_type v1)
  {
    if (v0.y < v1.y) {
      for (; v0.y <= v1.y; ++v0.y) {
        pixel_set(v0);
      }
    }
    else {
      for (; v1.y <= v0.y; ++v1.y) {
        pixel_set(v1);
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
  virtual void rect(vertex_type v0, vertex_type v1)
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
    if (v0.x > v1.x) { std::int16_t t = v0.x; v0.x = v1.x; v1.x = t; }
    if (v0.y > v1.y) { std::int16_t t = v0.y; v0.y = v1.y; v1.y = t; }
    for (; v0.y <= v1.y; ++v0.y) {
      const std::int16_t t = v0.x;
      for (v0.x = t; v0.x <= v1.x; ++v0.x) {
        pixel_set(v0);
      }
    }
    present();
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
    // check if triangle is a horizontal line
    if ((v0.y == v1.y) && (v1.y == v2.y)) {
      line_horz(v0, v1);
      line_horz(v1, v2);
      present();
      return;
    }

    // check if triangle is a vertical line
    if ((v0.x == v1.x) && (v1.x == v2.x)) {
      line_vert(v0, v1);
      line_vert(v1, v2);
      present();
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
    std::int16_t w0_row = orient_2d(v1, v2, p);
    std::int16_t w1_row = orient_2d(v2, v0, p);
    std::int16_t w2_row = orient_2d(v0, v1, p);

    anti_aliasing aa_left(*this), aa_right(*this);

    // rasterize
    for (p.y = min_y; p.y <= max_y; ++p.y) {
      // Barycentric coordinates at start of row
      std::int16_t w0 = w0_row, w1 = w1_row, w2 = w2_row;
      bool aa_inside = false;
      for (p.x = min_x; p.x <= max_x; ++p.x) {
        // if p is on or inside all edges, render the pixel
        if (w0 <= 0 && w1 <= 0 && w2 <= 0) {
          // if (w0 & w1 & w2 & 0x8000) {
          if (anti_aliasing_) {
            // AA dedection
            if (!aa_inside) {
              aa_inside = true;
              aa_left.render(p);
            }
            else {
              if (w0 + a12 > 0 || w1 + a20 > 0 || w2 + a01 > 0) {
                aa_inside = false;
                aa_right.render(p);
              }
              else {
                pixel_set(p);
              }
            }
          }
          else {
            pixel_set(p);
          }
        }
        // one step to the right
        w0 += a12; w1 += a20; w2 += a01;
      }
      // one row step
      w0_row += B12; w1_row += B20; w2_row += B01;
    }
    present();
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
   * \param x X center value
   * \param y Y center value
   * \param radius Circle radius
   * \return true if successful
   */
  void circle(vertex_type center, std::uint16_t radius)
  {
    std::int16_t xo = static_cast<std::int16_t>(radius), yo = 0, err = 1 - xo;

    if (pen_shape_) {
      // render with pen
      while (xo >= yo) {
        render_pen({ center.x + xo, center.y + yo });  // q4
        render_pen({ center.x + xo, center.y - yo });  // q1
        render_pen({ center.x + yo, center.y + xo });  // q4
        render_pen({ center.x + yo, center.y - xo });  // q1
        render_pen({ center.x - xo, center.y + yo });  // q3
        render_pen({ center.x - xo, center.y - yo });  // q2
        render_pen({ center.x - yo, center.y + xo });  // q3
        render_pen({ center.x - yo, center.y - xo });  // q2
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
    else if (anti_aliasing_) {
      // render antialiased
      anti_aliasing aa(*this);
// TBD 8 aa renders needed!
      while (xo >= yo) {
        aa.render({ center.x + xo, center.y + yo });  // q4
        aa.render({ center.x + xo, center.y - yo });  // q1
        aa.render({ center.x + yo, center.y + xo });  // q4
        aa.render({ center.x + yo, center.y - xo });  // q1
        aa.render({ center.x - xo, center.y + yo });  // q3
        aa.render({ center.x - xo, center.y - yo });  // q2
        aa.render({ center.x - yo, center.y + xo });  // q3
        aa.render({ center.x - yo, center.y - xo });  // q2
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
      while (xo >= yo) {
        pixel_set({ center.x + xo, center.y + yo });  // q4
        pixel_set({ center.x + xo, center.y - yo });  // q1
        pixel_set({ center.x + yo, center.y + xo });  // q4
        pixel_set({ center.x + yo, center.y - xo });  // q1
        pixel_set({ center.x - xo, center.y + yo });  // q3
        pixel_set({ center.x - xo, center.y - yo });  // q2
        pixel_set({ center.x - yo, center.y + xo });  // q3
        pixel_set({ center.x - yo, center.y - xo });  // q2
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
   * \return true if successful
   */
  void disc(vertex_type center, std::uint16_t radius)
  {
    std::int16_t xo = static_cast<std::int16_t>(radius), yo = 0, err = 1 - xo;
// TBD: aa support
    while (xo >= yo) {
      line_horz({ center.x + xo, center.y + yo }, { center.x - xo, center.y });
      line_horz({ center.x + yo, center.y + xo }, { center.x - yo, center.y });
      line_horz({ center.x - xo, center.y - yo }, { center.x + xo, center.y });
      line_horz({ center.x - yo, center.y - xo }, { center.x + yo, center.y });
      yo++;
      if (err < 0) {
        err += 2 * yo + 1;
      }
      else {
        xo--;
        err += 2 * (yo - xo + 1);
      }
    }
    present();
  }


  /**
   * Draw a disc section (filled quarter circle)
   * \param center Center value
   * \param radius Disc section radius
   * \param section Section number: 0: top/left, 1: top/right, 2: bottom/right, 3: bottom/left
   * \return true if successful
   */
  void disc_section(vertex_type center, std::uint16_t radius, std::uint8_t section)
  {
    std::int16_t xo = static_cast<std::int16_t>(radius), yo = 0, err = 1 - xo;
// TBD: aa suport
    while (xo >= yo) {
      switch (section) {
      case 1U:
        line_horz({ center.x, center.y - yo }, { center.x + xo, center.y });     // q1
        line_horz({ center.x, center.y - xo }, { center.x + yo, center.y });     // q1
        break;
      case 2U:
        line_horz({ center.x, center.y - yo }, { center.x - xo, center.y });     // q2
        line_horz({ center.x, center.y - xo }, { center.x - yo, center.y });     // q2
        break;
      case 3U:
        line_horz({ center.x, center.y + yo }, { center.x - xo, center.y });     // q3
        line_horz({ center.x, center.y + xo }, { center.x - yo, center.y });     // q3
        break;
      case 4U:
        line_horz({ center.x, center.y + yo }, { center.x + xo, center.y });     // q4
        line_horz({ center.x, center.y + xo }, { center.x + yo, center.y });     // q4
        break;
      default:
        break;
      }
      yo++;
      if (err < 0) {
        err += 2 * yo + 1;
      }
      else {
        xo--;
        err += 2 * (yo - xo + 1);
      }
    }
    present();
  }


  /**
   * Draw a sector (pie, filled circle piece)
   * \param center Center value
   * \param inner_radius Inner sector radius
   * \param outer_radius Outer sector radius
   * \param start_angle Start angle in degree, 0� is horizontal right, counting anticlockwise
   * \param end_angle End angle in degree
   * \return true if successful
   */
  void sector(vertex_type center, std::uint16_t inner_radius, std::uint16_t outer_radius, std::uint16_t start_angle, std::uint16_t end_angle)
  {
    // angle:
    // 0� = 3 o'clock
    //   0� -  89�: Q1 (top/right)
    //  90� - 179�: Q2 (top/left)
    // 180� - 269�: Q3 (bottom/left)
    // 270� - 359�: Q4 (bottom/right)

    bool second_half;
    std::uint16_t end_angle2 = end_angle;
    if ((end_angle > start_angle && end_angle > start_angle + 180U) ||
      (start_angle > end_angle && end_angle + 180U > start_angle)) {
      // more than 180�
      end_angle = (start_angle + 180U) % 360U;
    }

    do {
      bool q14s = (start_angle < 90U) || (start_angle >= 270U);
      bool q14e = (end_angle < 90U) || (end_angle >= 270U);
      bool q24s = (start_angle >= 90U && start_angle < 180U) || (start_angle >= 270U);
      bool q24e = (end_angle >= 90U && end_angle < 180U) || (end_angle >= 270U);

      std::int16_t xss = (std::uint8_t)(sin((q24s ? start_angle - 90U : start_angle) % 90U) >> (q24s ? 8U : 0U)) * (std::int16_t)(q14s ? 1 : -1);
      std::int16_t yss = (std::uint8_t)(sin((q24s ? start_angle - 90U : start_angle) % 90U) >> (q24s ? 0U : 8U)) * (std::int16_t)((start_angle < 180U) ? 1 : -1);
      std::int16_t xse = (std::uint8_t)(sin((q24e ? end_angle - 90U : end_angle) % 90U) >> (q24e ? 8U : 0U)) * (std::int16_t)(q14e ? 1 : -1);
      std::int16_t yse = (std::uint8_t)(sin((q24e ? end_angle - 90U : end_angle) % 90U) >> (q24e ? 0U : 8U)) * (std::int16_t)((end_angle < 180U) ? 1 : -1);

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
    std::int16_t w, h;
    if (source.x < destination.x) {
      if (source.y < destination.y) {
        for (h = height; h != 0; --h) {
          for (w = width; w != 0; --w) {
            drv_pixel_set_color({ destination.x + w, destination.y + h }, drv_pixel_get({ source.x + w, source.y + h }));
          }
        }
      }
      else {
        for (h = 0; h < height; ++h) {
          for (w = width; w != 0; --w) {
            drv_pixel_set_color({ destination.x + w, destination.y + h }, drv_pixel_get({ source.x + w, source.y + h }));
          }
        }
      }
    }
    else {
      if (source.y < destination.y) {
        for (h = height; h != 0; --h) {
          for (w = 0; w < width; ++w) {
            drv_pixel_set_color({ destination.x + w, destination.y + h }, drv_pixel_get({ source.x + w, source.y + h }));
          }
        }
      }
      else {
        for (h = 0; h < height; ++h) {
          for (w = 0; w < width; ++w) {
            drv_pixel_set_color({ destination.x + w, destination.y + h }, drv_pixel_get({ source.x + w, source.y + h }));
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

  // default pens
  const pen_type    pen_shape_width_1[1] = { { 0, 0, 0, 0 } };                                   // one pixel pen
  const pen_type    pen_shape_width_2[4] = { { 0, 0, 0, 1 }, { 1, 0, 0, 1 },
                                             { 0, 1, 0, 1 }, { 1, 1, 0, 0 } };                   // two pixel pen
  const pen_type    pen_shape_width_3[9] = { { 0, 0, 0, 1 }, { 1, 0, 0, 1 }, { 2, 0, 0, 1 },
                                             { 0, 1, 0, 1 }, { 1, 1, 0, 1 }, { 2, 1, 0, 1 },
                                             { 0, 2, 0, 1 }, { 1, 2, 0, 1 }, { 2, 2, 0, 0 } };   // three pixel pen

private:

  // non copyable
  const gpr& operator=(const gpr& rhs) { return rhs; }
};

} // namespace vgx

#endif  // _VGX_GPR_H_
