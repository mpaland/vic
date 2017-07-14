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

#include "base.h"


namespace vic {


typedef enum tag_pen_style_type {
  pen_style_solid = 0,
  pen_style_dash,
  pen_style_dot,
  pen_style_dashdot
} pen_style_type;


typedef struct tag_pen_shape_type {
  std::uint16_t       width;          // width of pen shape
  std::uint16_t       height;         // height of pen shape
  color::value_type   color;          // color of pen
  pen_style_type      style;          // style of pen
  std::uint8_t        style_ctl;      // internal style control, set to 0 on init
  const std::uint8_t* alpha;          // pen shape as alpha value array
} pen_shape_type;


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

  /**
   * Helper function to change two vertexes so that the first vertex contains top/left
   * \param v_min_y vertex
   * \param v_max_y vertex
   */
  void vertex_top_left(vertex_type& v_top_left, vertex_type& v_bottom_right) const
  {
    if (v_top_left.x > v_bottom_right.x) {
      const std::int16_t t = v_top_left.x;
      v_top_left.x     = v_bottom_right.x;
      v_bottom_right.x = t;
    }
    if (v_top_left.y > v_bottom_right.y) {
      const std::int16_t t = v_top_left.y;
      v_top_left.y     = v_bottom_right.y;
      v_bottom_right.y = t;
    }
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
        // antialising not necessary, 90� or 180�
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
   * Enable/disable anti aliasing support
   * \param enable True to enable anti aliasing
   */
  void anti_aliasing_enable(bool enable = true)
  {
    anti_aliasing_ = enable;
  }


  /**
   * Select the actual drawing pen
   * \param pen_shape Set the actual pen shape or nullptr for 1 pixel default pen (fastest)
   */
  inline void pen_set_shape(pen_shape_type* pen_shape = nullptr)
  {
    pen_shape_ = pen_shape;
  }


  /**
   * Get a stock pen
   * \param shape Receiving pen struct
   * \param style Pen style 
   * \param width Pen width
   * \param color Pen color
   */
  void pen_get_stock(pen_shape_type& shape, pen_style_type style, std::uint8_t width, color::value_type color) const
  {
    // stock pens
    static const std::uint8_t   pen_shape_width_1[1] = { 0 };         // one pixel pen
    static const std::uint8_t   pen_shape_width_2[4] = { 0, 0,
                                                         0, 0 };      // two pixel pen
    static const std::uint8_t   pen_shape_width_3[9] = { 0, 0, 0,
                                                         0, 0, 0,
                                                         0, 0, 0 };   // three pixel pen
    static const std::uint8_t* alpha[3] = { pen_shape_width_1, pen_shape_width_2, pen_shape_width_3 };

    shape = { width, width, color, style, 0U, alpha[width - 1U] };
  }


  /**
   * Render the pen at the given position
   * \param center Pen center position
   */
  inline void pen_render(vertex_type center)
  {
    if (!pen_shape_) {
      drv_pixel_set_color(center, pen_get_color(center));
    }
    else {
      switch (pen_shape_->style) {
        case pen_style_solid :
          break;
        case pen_style_dot :
          // * * * *
          if (pen_shape_->style_ctl >= pen_shape_->width * 2U) {
            pen_shape_->style_ctl = 0U;
          }
          if (pen_shape_->style_ctl++ > 0U) {
            return;
          }
          break;
        case pen_style_dash :
          // *** *** ***
          if (pen_shape_->style_ctl >= pen_shape_->width * 4U) {
            pen_shape_->style_ctl = 0U;
          }
          if (pen_shape_->style_ctl++ > pen_shape_->width * 2U) {
            return;
          }
          break;
        case pen_style_dashdot :
          // *** * *** * ***
          if (pen_shape_->style_ctl >= pen_shape_->width * 6U) {
            pen_shape_->style_ctl = 0U;
          }
          if ((pen_shape_->style_ctl >  pen_shape_->width * 2U) &&
              (pen_shape_->style_ctl != pen_shape_->width * 4U )) {
            pen_shape_->style_ctl++;
            return;
          }
          pen_shape_->style_ctl++;
          break;
        default:
          break;
      }
      std::size_t i = 0U;
      for (std::int16_t y = center.y - pen_shape_->height / 2, ye = y + pen_shape_->height; y < ye; ++y) {
        for (std::int16_t x = center.x - pen_shape_->width / 2, xe = x + pen_shape_->width; x < xe; ++x, ++i) {
          drv_pixel_set_color({ x, y }, pen_shape_->alpha[i] == 0U ? pen_shape_->color : color::mix(drv_pixel_get({ x, y }), pen_get_color({ x, y }), pen_shape_->alpha[i]));
        }
      }
    }
  }


  /**
   * Set a pixel in the actual pen color, no present is called (is a slim wrapper for drv_pixel_set_color, which is protected)
   * Uncommon to override this function, but may be done for in case of a monochrome displays
   * \param point Vertex to set
   */
  inline virtual void pixel_set(vertex_type point)
  {
    drv_pixel_set_color(point, pen_get_color(point));
  }


  /**
   * Set a pixel in the actual pen color, no present is called (is a slim wrapper for drv_pixel_set_color, which is protected)
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
   * Plot a point (one pixel) in the actual drawing (pen) color
   * \param point Vertex to plot
   */
  void plot(vertex_type point)
  {
    drv_pixel_set_color(point, pen_get_color(point));
    present();
  }


  /**
   * Plot a point (one pixel) with the given color, drawing color is not affected
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
        pen_render(v0);
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
   * This is a slow fallback implementation which should be overridden by a high speed driver implementation
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, y component is ignored
   * \return true if successful
   */
  virtual void line_horz(vertex_type v0, vertex_type v1)
  {
    // set v0 to min x
    vertex_min_x(v0, v1);

    if (pen_color_is_function()) {
      for (; v0.x <= v1.x; ++v0.x) {
        pixel_set(v0);
      }
    }
    else {
      for (; v0.x <= v1.x; ++v0.x) {
        pixel_set(v0, pen_get_color());
      }
    }
    present();
  }


  /**
   * Draw a vertical line, width is one pixel, no pen style support
   * This is a slow fallback implementation which should be overridden by a high speed driver implementation
   * \param v0 Start vertex, included in line
   * \param v1 End vertex, included in line, x component is ignored
   */
  virtual void line_vert(vertex_type v0, vertex_type v1)
  {
    // set v0 to min y
    vertex_min_y(v0, v1);

    if (pen_color_is_function()) {
      for (; v0.y <= v1.y; ++v0.y) {
        pixel_set(v0);
      }
    }
    else {
      for (; v0.y <= v1.y; ++v0.y) {
        pixel_set(v0, pen_get_color());
      }
    }
    present();
  }


  /**
   * Draw a box (filled rectangle)
   * This is a slow fallback implementation which should be overridden by a high speed driver implementation
   * \param v0 top/left vertex
   * \param v1 bottom/right vertex
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
   * Draw a box (filled rectangle)
   * This is a slow fallback implementation which should be overridden by a high speed driver implementation
   * \param v0 top/left vertex
   * \param v1 bottom/right vertex
   * \param border_radius Radius of the corner, 0 for angular
   */
  void box(vertex_type v0, vertex_type v1, std::uint16_t border_radius)
  {
    present_lock();
    box({ static_cast<std::int16_t>(v0.x + border_radius), v0.y }, { static_cast<std::int16_t>(v1.x - border_radius), static_cast<std::int16_t>(v0.y + border_radius) });
    box({ v0.x, static_cast<std::int16_t>(v0.y + border_radius) }, { v1.x, static_cast<std::int16_t>(v1.y - border_radius) });
    box({ static_cast<std::int16_t>(v0.x + border_radius), static_cast<std::int16_t>(v1.y - border_radius) }, { static_cast<std::int16_t>(v1.x - border_radius), v1.y });
    disc_sector({ static_cast<std::int16_t>(v1.x - border_radius), static_cast<std::int16_t>(v0.y + border_radius) }, border_radius, 0U);
    disc_sector({ static_cast<std::int16_t>(v0.x + border_radius), static_cast<std::int16_t>(v0.y + border_radius) }, border_radius, 1U);
    disc_sector({ static_cast<std::int16_t>(v0.x + border_radius), static_cast<std::int16_t>(v1.y - border_radius) }, border_radius, 2U);
    disc_sector({ static_cast<std::int16_t>(v1.x - border_radius), static_cast<std::int16_t>(v1.y - border_radius) }, border_radius, 3U);
    present_lock(false);
  }


  /**
   * Draw a rectangle (frame) with the current pen
   * \param v0 top/left vertex
   * \param v1 bottom/right vertex
   */
  void rectangle(vertex_type v0, vertex_type v1)
  {
    present_lock();
    line(v0, { v0.x, v1.y });
    line({ v0.x, v1.y }, v1);
    line(v1, { v1.x, v0.y });
    line({ v1.x, v0.y },  v0);
    present_lock(false);    // unlock and present
  }


  /**
   * Draw a rectangle (frame) with the current pen
   * \param v0 top/left vertex
   * \param v1 bottom/right vertex
   * \param border_radius Radius of the corner, 0 for angular
   */
  void rectangle(vertex_type v0, vertex_type v1, std::uint16_t border_radius)
  {
    present_lock();
    line({ static_cast<std::int16_t>(v1.x - border_radius), v0.y }, { static_cast<std::int16_t>(v0.x + border_radius), v0.y });
    circle({ static_cast<std::int16_t>(v0.x + border_radius), static_cast<std::int16_t>(v0.y + border_radius) }, border_radius,  90U, 180U);
    line({ v0.x, static_cast<std::int16_t>(v0.y + border_radius) }, { v0.x, static_cast<std::int16_t>(v1.y - border_radius) });
    circle({ static_cast<std::int16_t>(v0.x + border_radius), static_cast<std::int16_t>(v1.y - border_radius) }, border_radius, 180U, 270U);
    line({ static_cast<std::int16_t>(v0.x + border_radius), v1.y }, { static_cast<std::int16_t>(v1.x - border_radius), v1.y });
    circle({ static_cast<std::int16_t>(v1.x - border_radius), static_cast<std::int16_t>(v1.y - border_radius) }, border_radius, 270U, 360U);
    line({ v1.x, static_cast<std::int16_t>(v1.y - border_radius) }, { v1.x, static_cast<std::int16_t>(v0.y + border_radius) });
    circle({ static_cast<std::int16_t>(v1.x - border_radius), static_cast<std::int16_t>(v0.y + border_radius) }, border_radius,   0U,  90U);
    present_lock(false);    // unlock and present
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
    std::int16_t w0_row = util::orient_2d(v1, v2, p);
    std::int16_t w1_row = util::orient_2d(v2, v0, p);
    std::int16_t w2_row = util::orient_2d(v0, v1, p);

    // rasterize
    anti_aliasing aa0(*this), aa1(*this);
    for (p.y = min_y; p.y <= max_y; ++p.y) {
      // Barycentric coordinates at start of row
      std::int16_t w0 = w0_row, w1 = w1_row, w2 = w2_row, l_x = 0;
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
  void circle(vertex_type center, std::uint16_t radius, std::uint16_t start_angle = 0U, std::uint16_t end_angle = 360U)
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
          pen_shape_ ? pen_render(p) : (anti_aliasing_ ? aa.render(p) : pixel_set(p));
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
    //   0� = 3 o'clock
    //   0� -  89�: Q1 (top/right)
    //  90� - 179�: Q2 (top/left)
    // 180� - 269�: Q3 (bottom/left)
    // 270� - 359�: Q4 (bottom/right)

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
   * Fill routine is only working on displays which support drv_pixel_get() (or using the framebuffer ctrl)
   * \param start Start value inside region to fill
   * \param bounding_color Color of the surrounding bound or bg_color to fill all which is not bg_color
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
      {
        return stack_[--stack_count_];
      }

      // returns true if pixel is of border or drawing color
      // returns true if col != bg_color       && bounding_color == bg_color 
      // returns true if col == bounding_color && bounding_color != bg_color 
      // returns true if col == pen_color
      inline bool test(vertex_type point)
      {
        const color::value_type col = gpr_.drv_pixel_get({ point.x, point.y });
        return (col == gpr_.pen_get_color())                                      ||
               (col == bounding_color_ && bounding_color_ != gpr_.bg_get_color()) ||
               (col != bounding_color_ && bounding_color_ == gpr_.bg_get_color());
      }

      void fill(vertex_type start)
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
      floodfill(gpr& _gpr, vertex_type start, color::value_type bounding_color)
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

  /**
   * Move display area
   * This is a slow fallback implementation which should be overridden by a high speed driver implementation
   * \param source Source vertex
   * \param destination Destination vertex
   * \param width Width of the area
   * \param height Height of the area
   */
  virtual void move(vertex_type source, vertex_type destination, std::uint16_t width, std::uint16_t height)
//virtual void move(vertex_type orig_top_left, vertex_type orig_bottom_right, vertex_type dest_top_left)
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
   * Bit block image transfer to the display area
   * This is a slow fallback implementation which should be overridden by a high speed driver implementation
   * \param top_left
   * \param bottom_right
   * \param color_format
   * \param data
   */
  virtual void blitter(vertex_type top_left, vertex_type bottom_right, color::format_type color_format, const void* data)
  {
    (void)top_left; (void)bottom_right; (void)color_format; (void)data;
  }



///////////////////////////////////////////////////////////////////////////////

protected:
  bool              primitive_lock_;  // lock for rendering multiple primitives without refresh
  bool              anti_aliasing_;   // true if AA is enabled
  pen_shape_type*   pen_shape_;       // actual selected drawing pen

private:

  // non copyable
  const gpr& operator=(const gpr& rhs)
  { return rhs; }
};

} // namespace vic

#endif  // _VIC_GPR_H_
