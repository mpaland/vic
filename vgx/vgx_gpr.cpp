///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2001-2014, PALANDesign Hannover, Germany
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
// \brief Graphic Primitive Renderer of the vgxlib to render lines, arcs, boxes etc.
//
///////////////////////////////////////////////////////////////////////////////

#include "vgx_gpr.h"


namespace vgx {

// ANTI ALIASING
bool gpr::anti_aliasing_set(bool enable)
{
#if defined(VGX_CFG_ANTIALIASING)
  anti_aliasing_ = enable;
#endif
  return true;
}


// CLIPPING
#if defined(VGX_CFG_CLIPPING)
bool vgx_clipping_set(int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
{
  clippling_x0_ = x0;
  clippling_y0_ = y0;
  clippling_x1_ = x1;
  clippling_y1_ = y1;
  return true;
}


bool clipping_clear()
{
  drv_.clipping_clear();
}
#endif


///////////////////////////////////////////////////////////////////////////////
// N A T I V E   D R A W I N G   F U N C T I O N S

// draw a line from x0/y0 to x1/y1
void gpr::drv_line(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
{
  std::int16_t dx, dy;
  std::int16_t sx, sy;
  std::int16_t er, er2;

  // check for straight lines
  if (x0 == x1 && y0 < y1) {
    for (; y0 <= y1; ++y0) {
      pixel_set(x0, y0);
    }
    return;
  }
  if (x0 == x1 && y0 > y1) {
    for (; y0 >= y1; --y0) {
      pixel_set(x0, y0);
    }
    return;
  }
  if (y0 == y1 && x0 < x1) {
    for (; x0 <= x1; ++x0) {
      pixel_set(x0, y0);
    }
    return;
  }
  if (y0 == y1 && x0 > x1) {
    for (; x0 >= x1; --x0) {
      pixel_set(x0, y0);
    }
    return;
  }

  // start Bresenham line algorithm
  dx = x1 > x0 ? x1 - x0 : x0 - x1;
  sx = x1 > x0 ? 1 : -1;
  dy = y1 > y0 ? y1 - y0 : y0 - y1;
  sy = y1 > y0 ? 1 : -1;
  er = dx - dy;

  for(;;) {
    pixel_set(x0, y0);
    if (x0 == x1 && y0 == y1) {
      return;
    }
    er2 = er * 2;
    if (er2 + dy > 0) {
      er -= dy;
      x0 += sx;
    }
    if (er2 < dx) {
      er += dx;
      y0 += sy;
    }
  }
}


// draw a horizontal line from x0/y0 to x1/y0
void gpr::drv_line_horz(std::int16_t x0, std::int16_t y0, std::int16_t x1)
{
  if (x0 < x1) {
    for (; x0 <= x1; ++x0)
      pixel_set(x0, y0);
  }
  else {
    for (; x1 <= x0; ++x1)
      pixel_set(x1, y0);
  }
}


// draw a vertical line from x0/y0 to x0/y1
void gpr::drv_line_vert(std::int16_t x0, std::int16_t y0, std::int16_t y1)
{
  if (y0 < y1) {
    for (; y0 <= y1; ++y0)
      pixel_set(x0, y0);
  }
  else {
    for (; y1 <= y0; ++y1)
      pixel_set(x0, y1);
  }
}


void gpr::drv_line_width(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::uint16_t width)
{
  std::int16_t dx, dy;
  std::int16_t sx, sy;
  std::int16_t er, er2;

// TBD: calculate wd without using float
  std::int16_t wd, w;
  wd = (int16_t)width;

  // start Bresenham line algorithm
  dx = x1 > x0 ? x1 - x0 : x0 - x1;
  dy = y1 > y0 ? y1 - y0 : y0 - y1;
  sx = x1 > x0 ? 1 : -1;
  sy = y1 > y0 ? 1 : -1;
  er = dx - dy;

  for(;;) {
    if (dx > dy) {
      for (w = y0 - (wd >> 1U); w < y0 + wd - (wd >> 1U); w++)
        pixel_set(x0, w);
    }
    else {
      for (w = x0 - (wd >> 1U); w < x0 + wd - (wd >> 1U); w++)
        pixel_set(w, y0);
    }
    if (x0 == x1 && y0 == y1) {
      return;
    }
    er2 = er * 2;
    if (er2 + dy > 0) {
      er -= dy;
      x0 += sx;
    }
    if (er2 < dx) {
      er += dx;
      y0 += sy;
    }
  }
}


// rect (frame) function
void gpr::drv_rect(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
{
  drv_line_horz(x0, y0, x1);
  drv_line_horz(x0, y1, x1);
  drv_line_vert(x0, y0, y1);
  drv_line_vert(x1, y0, y1);
}


// draw a solid box
void gpr::drv_box(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
{
  std::int16_t tmp;
  if (x0 > x1) { tmp = x0; x0 = x1; x1 = tmp; }
  if (y0 > y1) { tmp = y0; y0 = y1; y1 = tmp; }

  for (tmp = x0; y0 <= y1; ++y0) {
    for (x0 = tmp; x0 <= x1; ++x0) {
      pixel_set(x0, y0);
    }
  }
}


void gpr::drv_triangle(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2)
{
  drv_line(x0, y0, x1, y1);
  drv_line(x0, y0, x2, y2);
  drv_line(x1, y1, x2, y2);
}


void gpr::drv_triangle_solid(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2)
{
  /* draw a solid triangle using two Bresenham line algorithms in a y-value loop
     sort is:    p0
                /  \
               p1--p2
     lines run from p0 to p1 and p2. When y reaches p1 or p2 (ymin), according line is exchanged
  */

  // check special cases
  if ((y0 == y1) && (y0 == y2)) {
    drv_line_horz(x0, y0, x1);
    drv_line_horz(x0, y0, x2);
    return;
  }
  if ((x0 == x1) && (x0 == x2)) {
    drv_line_vert(x0, y0, y1);
    drv_line_vert(x0, y0, y2);
    return;
  }

  // sort points
  std::int16_t tmp;
  if (y1 < y0) { tmp = y0; y0 = y1; y1 = tmp; tmp = x0; x0 = x1; x1 = tmp; }
  if (y2 < y0) { tmp = y0; y0 = y2; y2 = tmp; tmp = x0; x0 = x2; x2 = tmp; }
  if (x1 > x2) { tmp = y1; y1 = y2; y2 = tmp; tmp = x1; x1 = x2; x2 = tmp; }

  std::int16_t xl = y0 != y1 ? x0 : x1;
  std::int16_t xr = y0 != y2 ? x0 : x2;
  std::int16_t ymax = y1 < y2 ? y2 : y1;
  std::int16_t ymin = y1 < y2 ? y1 : y2;
  std::int16_t dx0 = x1 > x0 ? x1 - x0 : x0 - x1;
  std::int16_t dy0 = y1 > y0 ? y1 - y0 : y0 - y1;
  std::int16_t sx0 = x1 > x0 ? 1 : -1;
  std::int16_t er0 = dx0 - dy0;
  std::int16_t dx1 = x2 > x0 ? x2 - x0 : x0 - x2;
  std::int16_t dy1 = y2 > y0 ? y2 - y0 : y0 - y2;
  std::int16_t sx1 = x2 > x0 ? 1 : -1;
  std::int16_t er1 = dx1 - dy1;
  std::int16_t dx2 = x2 > x1 ? x2 - x1 : x1 - x2;
  std::int16_t dy2 = y2 > y1 ? y2 - y1 : y1 - y2;
  std::int16_t s2  = y2 > y1 ? 1 : -1;
  std::int16_t er2 = dx2 - dy2;
  std::int16_t err;

  for(std::int16_t y = y0; y <= ymax; y++) {
    drv_line_horz(xl, y, xr);

    if (y == ymin) {
      // exchange params
      if (y1 > y2) {
        dx1 = dx2; dy1 = dy2; sx1 = s2; er1 = er2; xr = x2;
      }
      else {
        dx0 = dx2; dy0 = dy2; sx0 = s2; er0 = er2; xl = x1;
      }
    }

    do {
      err = er0 * 2;
      if (err + dy0 > 0) {
        er0 -= dy0;
        xl  += sx0;
      }
    } while (dy0 && (err >= dx0));
    er0 += dx0;

    do {
      err = er1 * 2;
      if (err + dy1 > 0) {
        er1 -= dy1;
        xr  += sx1;
      }
    } while (dy1 && (err >= dx1));
    er1 += dx1;
  }
}


void gpr::drv_arc(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2)
{
  std::int16_t sx = x2 - x1, sy = y2 - y1;
  std::int32_t xx = x0 - x1, yy = y0 - y1, xy;        // relative values for checks
  std::int32_t dx, dy, err, cur = xx * sy - yy * sx;  // curvature

  // sign of gradient must not change
  if (xx * sx > 0 || yy * sy > 0) {
    return;
  }

  if (sx * (std::int32_t)sx + sy * (std::int32_t)sy > xx * xx + yy * yy) {  // begin with longer part
    x2 = x0; x0 = sx + x1; y2 = y0; y0 = sy + y1; cur = -cur;               // swap
  }  
  if (cur != 0) {                                   // no straight line
    xx += sx; xx *= sx = x0 < x2 ? 1 : -1;          // x step direction
    yy += sy; yy *= sy = y0 < y2 ? 1 : -1;          // y step direction
    xy  = 2 * xx * yy;
    xx *= xx;
    yy *= yy;                                       // differences 2nd degree
    if (cur * sx * sy < 0) {                        // negated curvature?
      xx = -xx; yy = -yy; xy = -xy; cur = -cur;
    }
    dx = 4 * sy * cur * (x1 - x0) + xx - xy;        // differences 1st degree
    dy = 4 * sx * cur * (y0 - y1) + yy - xy;
    xx += xx;
    yy += yy;
    err = dx + dy + xy;                             // error 1st step
    do {
      pixel_set(x0, y0);
      if (x0 == x2 && y0 == y2) {
        return;                                     // curve finished
      }
      y1 = 2 * err < dx;                            // save value for test of y step
      if (2 * err > dy) {
        x0 += sx; dx -= xy; err += dy += yy;        // x step
      }
      if (y1) {
        y0 += sy; dy -= xy; err += dx += xx;        // y step
      }
    } while (dy < dx );                             // gradient negates -> algorithm fails
  }
  drv_line(x0, y0, x2, y2);                         // plot remaining part to end
}


void gpr::drv_circle(std::int16_t x, std::int16_t y, std::uint16_t r)
{
  std::int16_t xo = (std::int16_t)r, yo = 0, err = 1 - xo;

  while (xo >= yo) {
    pixel_set(x + xo, y + yo);  // q4
    pixel_set(x + yo, y + xo);  // q4
    pixel_set(x - xo, y + yo);  // q3
    pixel_set(x - yo, y + xo);  // q3
    pixel_set(x - xo, y - yo);  // q2
    pixel_set(x - yo, y - xo);  // q2
    pixel_set(x + xo, y - yo);  // q1
    pixel_set(x + yo, y - xo);  // q1
    yo++;
    if (err < 0) {
      err += 2 * yo + 1;
    } else {
      xo--;
      err += 2 * (yo - xo + 1);
    }
  }
}


void gpr::drv_disc(std::int16_t x, std::int16_t y, std::uint16_t r)
{
  std::int16_t xo = (int16_t)r, yo = 0, err = 1 - xo;

  while (xo >= yo) {
    drv_line_horz(x + xo, y + yo, x - xo);
    drv_line_horz(x + yo, y + xo, x - yo);
    drv_line_horz(x - xo, y - yo, x + xo);
    drv_line_horz(x - yo, y - xo, x + yo);
    yo++;
    if (err < 0) {
      err += 2 * yo + 1;
    } else {
      xo--;
      err += 2 * (yo - xo + 1);
    }
  }
}


void gpr::drv_disc_section(std::int16_t x, std::int16_t y, std::uint16_t r, std::uint8_t section)
{
  std::int16_t xo = (std::int16_t)r, yo = 0, err = 1 - xo;

  while (xo >= yo) {
    switch (section) {
      case 1U :
        drv_line_horz(x, y - yo, x + xo);     // q1
        drv_line_horz(x, y - xo, x + yo);     // q1
        break;
      case 2U :
        drv_line_horz(x, y - yo, x - xo);     // q2
        drv_line_horz(x, y - xo, x - yo);     // q2
        break;
      case 3U :
        drv_line_horz(x, y + yo, x - xo);     // q3
        drv_line_horz(x, y + xo, x - yo);     // q3
        break;
      case 4U :
        drv_line_horz(x, y + yo, x + xo);     // q4
        drv_line_horz(x, y + xo, x + yo);     // q4
        break;
      default :
        break;
    }
    yo++;
    if (err < 0) {
      err += 2 * yo + 1;
    } else {
      xo--;
      err += 2 * (yo - xo + 1);
    }
  }
}


void gpr::drv_sector(std::int16_t x, std::int16_t y, std::uint16_t inner_radius, std::uint16_t outer_radius, std::uint16_t start_angle, std::uint16_t end_angle)
{
  // angle:
  // 0° = 3 o'clock
  //   0° -  89°: Q1 (top/right)
  //  90° - 179°: Q2 (top/left)
  // 180° - 269°: Q3 (bottom/left)
  // 270° - 359°: Q4 (bottom/right)

  bool second_half;
  std::uint16_t end_angle2 = end_angle;
  if ((end_angle > start_angle && end_angle > start_angle + 180U) || 
      (start_angle > end_angle && end_angle + 180U > start_angle)) {
    // more than 180°
    end_angle  = (start_angle + 180U) % 360U;
  }

  do {
    bool q14s = (start_angle < 90U) || (start_angle >= 270U);
    bool q14e = (end_angle   < 90U) || (end_angle   >= 270U);
    bool q24s = (start_angle >= 90U && start_angle < 180U) || (start_angle >= 270U);
    bool q24e = (end_angle   >= 90U && end_angle   < 180U) || (end_angle   >= 270U);

    std::int16_t xss = (std::uint8_t)(sin((q24s ? start_angle - 90U : start_angle) % 90U) >> (q24s ? 8U : 0U)) * (std::int16_t)(q14s ? 1 : -1);
    std::int16_t yss = (std::uint8_t)(sin((q24s ? start_angle - 90U : start_angle) % 90U) >> (q24s ? 0U : 8U)) * (std::int16_t)((start_angle < 180U) ? 1 : -1);
    std::int16_t xse = (std::uint8_t)(sin((q24e ? end_angle   - 90U : end_angle)   % 90U) >> (q24e ? 8U : 0U)) * (std::int16_t)(q14e ? 1 : -1);
    std::int16_t yse = (std::uint8_t)(sin((q24e ? end_angle   - 90U : end_angle)   % 90U) >> (q24e ? 0U : 8U)) * (std::int16_t)((end_angle   < 180U) ? 1 : -1);

    for (std::int16_t yp = y - outer_radius; yp <= y + outer_radius; yp++) {
      for (std::int16_t xp = x - outer_radius; xp <= x + outer_radius; xp++) {
        // check if xp/yp is within the sector
        std::int16_t xr = xp - x;
        std::int16_t yr = y - yp;   // * -1 for coords to screen conversion
        if ( ((xr * xr + yr * yr) >= inner_radius * inner_radius) &&
             ((xr * xr + yr * yr) <  outer_radius * outer_radius) &&
            !((yss * xr) >  (xss * yr)) &&
             ((yse * xr) >= (xse * yr))
           ) {
          pixel_set(xp, yp);
        }
      }
    }
    // second half necessary?
    second_half = false;
    if (end_angle != end_angle2) {
      start_angle = end_angle;
      end_angle   = end_angle2;
      second_half = true;
    }
  } while (second_half);
}


///////////////////////////////////////////////////////////////////////////////
// A N T I   A L I A S I N G   S U P P O R T
///////////////////////////////////////////////////////////////////////////////
#if defined(VGX_CFG_ANTIALIASING)

// draws an anti aliased line from x0/y0 to x1/y1
void gpr::drv_line_aa(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
{
  // using Xiaolin Wu's anti aliasing line algorithm
  // adaption of code from Michael Abrash in Dr. Dobbs journal, 1992 June 01

  std::uint16_t ErrorAdj, ErrorAcc, ErrorAccTemp;
  std::int16_t  DeltaX, DeltaY, XDir;

  // make sure the line runs top to bottom
  if (y0 > y1) {
    std::int16_t temp;
    temp = y0; y0 = y1; y1 = temp;
    temp = x0; x0 = x1; x1 = temp;
  }

  // draw the initial pixel, which is always exactly intersected by the line and so needs no weighting
  pixel_set(x0, y0);

  if ((DeltaX = x1 - x0) >= 0) {
    XDir = 1;
  } else {
    XDir = -1;
    DeltaX = -DeltaX; // make DeltaX positive
  }
  // special-case horizontal, vertical, and diagonal lines, which require no weighting
  // because they go right through the center of every pixel
  if ((DeltaY = y1 - y0) == 0) {
    // horizontal line
    drv_line_horz(x0, y0, x1);
    return;
  }
  if (DeltaX == 0) {
    // vertical line
    drv_line_vert(x0, y0, y1);
    return;
  }
  if (DeltaX == DeltaY) {
    // diagonal line
    do {
      x0 += XDir;
      y0++;
      pixel_set(x0, y0);
    } while (--DeltaY != 0);
    return;
  }

  // line is not horizontal, diagonal, or vertical
  ErrorAcc = 0U;    // initialize the line error accumulator to 0

  // Is this an X-major or Y-major line?
  if (DeltaY > DeltaX) {
    // Y-major line; calculate 16-bit fixed-point fractional part of a
    // pixel that X advances each time Y advances 1 pixel, truncating the
    // result so that we won't overrun the endpoint along the X axis
    ErrorAdj = ((std::int32_t)DeltaX << 16U) / (std::int32_t)DeltaY;
    // draw all pixels other than the first and last
    while (--DeltaY) {
      ErrorAccTemp = ErrorAcc;    // remember current accumulated error
      ErrorAcc += ErrorAdj;       // calculate error for next pixel
      if (ErrorAcc <= ErrorAccTemp) {
        // The error accumulator turned over, so advance the X coord
        x0 += XDir;
      }
      y0++; // Y-major, so always advance Y
      // the IntensityBits most significant bits of ErrorAcc give us the intensity weighting
      // for this pixel, and the complement of the weighting for the paired pixel
      // # of bits by which to shift ErrorAcc to get intensity level
      #define INTENSITY_SHIFT (16U - 8U)
      std::uint8_t lum = (std::uint8_t)(ErrorAcc >> INTENSITY_SHIFT);
      pixel_set_color(x0, y0,        color_mix(color_, pixel_get(x0, y0), lum ^ 0xFFU));
      pixel_set_color(x0 + XDir, y0, color_mix(color_, pixel_get(x0 + XDir, y0), lum));
    }
    // draw the final pixel, which is always exactly intersected by the line and so needs no weighting
    pixel_set(x1, y1);
    return;
  }

  // It's an X-major line; calculate 16-bit fixed-point fractional part of a
  // pixel that Y advances each time X advances 1 pixel, truncating the
  // result to avoid overrunning the endpoint along the X axis
  ErrorAdj = ((std::uint32_t)DeltaY << 16U) / (std::uint32_t)DeltaX;
  // draw all pixels other than the first and last
  while (--DeltaX) {
    ErrorAccTemp = ErrorAcc;    // remember current accumulated error
    ErrorAcc += ErrorAdj;       // calculate error for next pixel
    if (ErrorAcc <= ErrorAccTemp) {
      // the error accumulator turned over, so advance the Y coord
      y0++;
    }
    x0 += XDir;   // X-major, so always advance X
    // the IntensityBits most significant bits of ErrorAcc give us the intensity weighting
    // for this pixel, and the complement of the weighting for the paired pixel
    std::uint8_t lum = (std::uint8_t)(ErrorAcc >> INTENSITY_SHIFT);
    pixel_set_color(x0, y0,     color_mix(color_, pixel_get(x0, y0), lum ^ 0xFFU));
    pixel_set_color(x0, y0 + 1, color_mix(color_, pixel_get(x0, y0 + 1), lum));
  }
  // draw the final pixel, which is always exactly intersected by the line and so needs no weighting
  pixel_set(x1, y1);
}


// draws an anti aliased arc
void gpr::drv_arc_aa(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2)
{
  //Linear-constrain circle anti-aliased algorithm, copyright by Konstantin Kirillov, license: MIT
#if 0
  int I=255;  // intensity range. Determines algorithms' accuracy.

    //No precalculation is required.
    //Calculation of the 1/8 of circle will take
    // R/sq(2) steps = R/sq(2)*(3mult+1div+8add) ~ 1.4R(3mult+1div)
    // All operations are of integer type.
        
        
    //Input: uses precalculated variables D and I.
    public void drawArch( int R ) {
       int R2=R*R;
       int y=0;
       int x=R;

       int B=x*x;
       int xTop=x+1;
       int T=xTop*xTop;

       while( y<x ) {
           int E=R2-y*y;
           int L=E-B;
           int U=T-E;
           if(L<0){ //We had Wu's lemma before: if( dnew < d ) x--;
             xTop=x;
             x--;
             T=B;
             U=-L;
             B=x*x;
             L=E-B;
           }
           int u=I*U/(U+L);
           
           //good for debug:
           con("x="+x+" y="+y+" E="+E+" B="+B+" T="+T+" L="+L+" U="+U+" u="+u);
           
           //These two statements are not a part of the algorithm:
           //Each language, OS, or framework has own ways to put a "pixel".
           putpixel(x,      y,     u,  doDemo);
           putpixel(xTop,   y, (I-u), !doDemo);
           
           y++;
       }    
    }
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Calculation of an arc
    //----------------------------------------------------------------

    
    //----------------------------------------------------------------
    //End of Algorithm.
    //==========================================================================
#endif
}


// draw an anti aliased triangle
void gpr::drv_triangle_aa(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2)
{
  drv_line_aa(x0, y0, x1, y1);
  drv_line_aa(x0, y0, x2, y2);
  drv_line_aa(x1, y1, x2, y2);
}


// draw an anti aliased solid triangle
void gpr::drv_triangle_solid_aa(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2)
{
  drv_triangle_solid(x0, y0, x1, y1, x2, y2);
  drv_line_aa(x0, y0, x1, y1);
  drv_line_aa(x0, y0, x2, y2);
  drv_line_aa(x1, y1, x2, y2);
}


// draw an anti aliased circle
void gpr::drv_circle_aa(std::int16_t x, std::int16_t y, std::uint16_t r)
{
  // TBD: render an AA circle
}

// draw an anti aliased disc
void gpr::drv_disc_aa(std::int16_t x, std::int16_t y, std::uint16_t r)
{
  // TBD: render an AA disc
}

// draw an anti aliased section
void gpr::drv_disc_section_aa(std::int16_t x, std::int16_t y, std::uint16_t r, std::uint8_t section)
{
  // TBD: render an AA section
}
#endif  // VGX_CFG_ANTIALIASING


void gpr::drv_fill(std::int16_t x, std::int16_t y, std::uint32_t bounding_color)
{
  // TBD: fill region up to the bounding border_color with the drawing color
}


// move area
void gpr::drv_move(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::uint16_t width, std::uint16_t height)
{
  std::uint16_t w, h;

  if (x0 < x1) {
    if (y0 < y1) {
      for (h = height; h != 0U; --h) {
        for (w = width; w != 0U; --w) {
          pixel_set_color(x1 + w, y1 + h, pixel_get(x0 + w, y0 + h));
    }
      }
    }
    else {
      for (h = 0U; h < height; ++h) {
        for (w = width; w != 0U; --w) {
          pixel_set_color(x1 + w, y1 + h, pixel_get(x0 + w, y0 + h));
    }
  }
    }
  }
  else {
    if (y0 < y1) {
      for (h = height; h != 0U; --h) {
        for (w = 0U; w < width; ++w) {
          pixel_set_color(x1 + w, y1 + h, pixel_get(x0 + w, y0 + h));
    }
      }
    }
    else {
      for (h = 0U; h < height; ++h) {
        for (w = 0U; w < width; ++w) {
          pixel_set_color(x1 + w, y1 + h, pixel_get(x0 + w, y0 + h));
    }
  }
}
  }
}


///////////////////////////////////////////////////////////////////////////////

// draw a point
bool gpr::plot(std::int16_t x, std::int16_t y)
{
  pixel_set(x, y);
  primitive_done();
  return true;
}


// draw a solid line from x0/y0 to x1/y1
bool gpr::line(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
{
#if defined(VGX_CFG_ANTIALIASING)
  if (anti_aliasing_) {
    drv_line_aa(x0, y0, x1, y1);
  }
  else {
    drv_line(x0, y0, x1, y1);
  }
#else
  drv_line(x0, y0, x1, y1);
#endif
  primitive_done();
  return true;
}


// draw a horizontal line from x0/y0 to x1/y0
bool gpr::line_horz(std::int16_t x0, std::int16_t y0, std::int16_t x1)
{
  drv_line_horz(x0, y0, x1);
  primitive_done();
  return true;
}


// draw a vertical line from x0/y0 to x0/y1
bool gpr::line_vert(std::int16_t x0, std::int16_t y0, std::int16_t y1)
{
  drv_line_vert(x0, y0, y1);
  primitive_done();
  return true;
}


// rect (frame) function
bool gpr::rect(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
{
  drv_rect(x0, y0, x1, y1);
  primitive_done();
  return true;
}


// draw a solid box
bool gpr::box(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1)
{
  drv_box(x0, y0, x1, y1);
  primitive_done();
  return true;
}


bool gpr::box_gradient(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, const gradient_type& gradient)
{
  std::uint16_t seg;
  seg = gradient.horizontal ? x1 - x0 : y1 - y0;
  seg = seg / (gradient.color_count - 1U);

  std::uint16_t s = 0U, c = 0U;
  if (gradient.horizontal) {
    // horizontal gradient
    for (std::int16_t x = x0; x <= x1; ++x) {
      color_set(color_rgb((std::uint8_t)((((std::uint16_t)color_get_red  (gradient.colors[c]) * (seg - s)) + ((std::uint16_t)color_get_red  (gradient.colors[c + 1U]) * s)) / seg),
                          (std::uint8_t)((((std::uint16_t)color_get_green(gradient.colors[c]) * (seg - s)) + ((std::uint16_t)color_get_green(gradient.colors[c + 1U]) * s)) / seg),
                          (std::uint8_t)((((std::uint16_t)color_get_blue (gradient.colors[c]) * (seg - s)) + ((std::uint16_t)color_get_blue (gradient.colors[c + 1U]) * s)) / seg)
                         ));
      drv_line_vert(x, y0, y1);
      if (s++ >= seg) {
        s = 0U;
        c++;
      }
    }
  }
  else {
    // vertical gradient
    for (std::int16_t y = y0; y <= y1; ++y) {
      color_set(color_rgb((std::uint8_t)((((std::uint16_t)color_get_red  (gradient.colors[c]) * (seg - s)) + ((std::uint16_t)color_get_red  (gradient.colors[c + 1U]) * s)) / seg),
                          (std::uint8_t)((((std::uint16_t)color_get_green(gradient.colors[c]) * (seg - s)) + ((std::uint16_t)color_get_green(gradient.colors[c + 1U]) * s)) / seg),
                          (std::uint8_t)((((std::uint16_t)color_get_blue (gradient.colors[c]) * (seg - s)) + ((std::uint16_t)color_get_blue (gradient.colors[c + 1U]) * s)) / seg)
                         ));
      drv_line_horz(x0, y, x1);
      if (s++ >= seg) {
        s = 0U;
        c++;
      }
    }
  }
  primitive_done();
  return true;
}


// draw a polygon
bool gpr::polygon(const vertex_type* vertexes, std::uint16_t vertex_count)
{
  if (!vertex_count || --vertex_count < 2U) {
    return false;
  }
#if defined(VGX_CFG_ANTIALIASING)
  if (anti_aliasing_) {
    drv_line_aa(vertexes[0].x, vertexes[0].y, vertexes[vertex_count].x, vertexes[vertex_count].y);
    for (; vertex_count > 0U; --vertex_count) {
      drv_line_aa(vertexes[vertex_count].x, vertexes[vertex_count].y, vertexes[vertex_count - 1U].x, vertexes[vertex_count - 1U].y);
    }
  }
  else {
    drv_line(vertexes[0].x, vertexes[0].y, vertexes[vertex_count].x, vertexes[vertex_count].y);
    for (; vertex_count > 0U; --vertex_count) {
      drv_line(vertexes[vertex_count].x, vertexes[vertex_count].y, vertexes[vertex_count - 1U].x, vertexes[vertex_count - 1U].y);
    }
  }
#else
  drv_line(vertexes[0].x, vertexes[0].y, vertexes[vertex_count].x, vertexes[vertex_count].y);
  for (; vertex_count > 0U; --vertex_count) {
    drv_line(vertexes[vertex_count].x, vertexes[vertex_count].y, vertexes[vertex_count - 1U].x, vertexes[vertex_count - 1U].y);
  }
#endif
  primitive_done();
  return true;
}


// triangle function
bool gpr::triangle(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2)
{
#if defined(VGX_CFG_ANTIALIASING)
  if (anti_aliasing_) {
    drv_triangle_aa(x0, y0, x1, y1, x2, y2);
  }
  else {
    drv_triangle(x0, y0, x1, y1, x2, y2);
  }
#else
    drv_triangle(x0, y0, x1, y1, x2, y2);
#endif
  primitive_done();
  return true;
}


bool gpr::triangle_solid(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2)
{
#if defined(VGX_CFG_ANTIALIASING)
  if (anti_aliasing_) {
    drv_triangle_solid_aa(x0, y0, x1, y1, x2, y2);
  }
  else {
    drv_triangle_solid(x0, y0, x1, y1, x2, y2);
  }
#else
    drv_triangle_solid(x0, y0, x1, y1, x2, y2);
#endif
  primitive_done();
  return true;
}


// arc (belzier) functions
bool gpr::arc(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::int16_t x2, std::int16_t y2)
{
  // sign of gradient must not change
  if ((x0 - x1) * (x2 - x1) > 0 ||
      (y0 - y1) * (y2 - y1) > 0) {
    return false;
  }
#if defined(VGX_CFG_ANTIALIASING)
  if (anti_aliasing_) {
    drv_arc_aa(x0, y0, x1, y1, x2, y2);
  }
  else {
    drv_arc(x0, y0, x1, y1, x2, y2);
  }
#else
  drv_arc(x0, y0, x1, y1, x2, y2);
#endif
  primitive_done();
  return true;
}


// draw a circle
bool gpr::circle(std::int16_t x, std::int16_t y, std::uint16_t radius)
{
#if defined(VGX_CFG_ANTIALIASING)
  if (anti_aliasing_) {
    drv_circle_aa(x, y, radius);
  }
  else {
    drv_circle(x, y, radius);
  }
#else
  drv_circle(x, y, radius);
#endif
  primitive_done();
  return true;
}


// draw a disc (filled circle)
bool gpr::disc(std::int16_t x, std::int16_t y, std::uint16_t radius)
{
#if defined(VGX_CFG_ANTIALIASING)
  if (anti_aliasing_) {
    drv_disc_aa(x, y, radius);
  }
  else {
    drv_disc(x, y, radius);
  }
#else
  drv_disc(x, y, radius);
#endif
  primitive_done();
  return true;
}


// disc section
bool gpr::disc_section(std::int16_t x, std::int16_t y, std::uint16_t radius, std::uint8_t section)
{
  drv_disc_section(x, y, radius, section);
  primitive_done();
  return true;
}


// sector
bool gpr::sector(std::int16_t x, std::int16_t y, std::uint16_t inner_radius, std::uint16_t outer_radius, std::uint16_t start_angle, std::uint16_t end_angle)
{
  drv_sector(x, y, inner_radius, outer_radius, start_angle, end_angle);
  primitive_done();
  return true;
}


// fill function, fill region up to bounding color
bool gpr::fill(std::int16_t x, std::int16_t y, std::uint32_t bounding_color)
{
  drv_fill(x, y, bounding_color);
  primitive_done();
  return true;
}


///////////////////////////////////////////////////////////////////////////////

// blitter
bool gpr::blitter(std::int16_t x, std::int16_t y, std::uint16_t width, std::uint16_t height, std::uint8_t color_depth, std::uint8_t* data)
{
  // TBD: blittering
  return false;
}


// move area
bool gpr::move(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::uint16_t width, std::uint16_t height)
{
  drv_move(x0, y0, x1, y1, width, height);
  primitive_done();
  return true;
}


///////////////////////////////////////////////////////////////////////////////
// T E X T   F U N C T I O N S

bool gpr::drv_text_set_font(const font_type& font)
{
  return true;
}


bool gpr::drv_text_set_pos(std::int16_t, std::int16_t)
{ 
  return true;
}


void gpr::drv_text_set_mode(text_mode_type)
{
}


/**
 * Draw a single ASCII/UNICODE char at the actual cursor position
 * \param ch Character in Unicode format, 20-7F is compatible with ASCII
 */
void gpr::drv_text_char(std::uint16_t ch)
{
#if defined(VGX_CFG_FONT)
  std::uint8_t color_depth = (text_font_->font_attr & VGX_FONT_AA_MASK);
  std::uint8_t color_mask  = (1U << color_depth) - 1U;

  // handling of special chars
  if ((char)ch == '\n') {
    // LF: X = 0, Y = next line
    text_x_act_ = text_x_set_;
    text_y_act_ = text_y_act_ + text_font_->ysize;
    return;
  }
  else if ((char)ch == '\r') {
    // CR: X = 0
    text_x_act_ = text_x_set_;
    return;
  }
  else if (ch < 0x0020U) {
    // ignore all other codes
    return;
  }

  if ((text_font_->font_attr & VGX_FONT_ENCODING_MASK) == VGX_FONT_ENCODING_UNICODE) {
    // extended (UNICODE) font
    const font_prop_ext_type* font_prop_ext = text_font_->font_type_type.font_prop_ext;
    do {
      if (ch >= font_prop_ext->first && ch <= font_prop_ext->last) {
        // found char
        const font_charinfo_ext_type* info = &font_prop_ext->char_info_ext[ch - font_prop_ext->first];
        std::uint16_t x, y, d = 0;
        for (y = 0U; y < info->ysize; ++y) {
          d = (1U + ((info->xsize - 1U) * color_depth / 8U)) * y;
          for (x = 0U; x < info->xsize ; ++x) {
            std::uint8_t intensity = (info->data[d + ((x * color_depth) >> 3U)] >> ((8U - (x + 1U) * color_depth) % 8U)) & color_mask;
            if (intensity) {
              if (color_depth == VGX_FONT_AA_NONE) {
                pixel_set(text_x_act_ + info->xpos + x, text_y_act_ + info->ypos + y);
              }
              else {
                std::uint32_t fg = color_;
                std::uint32_t bg = pixel_get(text_x_act_+ info->xpos + x, text_y_act_ + info->ypos + y);
                pixel_set_color(text_x_act_ + info->xpos + x, text_y_act_ + info->ypos + y, color_mix(fg, bg, color_depth == 2U ? intensity << 6U : intensity << 4U));
              }
            }
          }
        }
        text_x_act_ +=  info->xdist;
        return;
      }
      font_prop_ext = font_prop_ext->next;
    } while (font_prop_ext);
    // char not found
    return;
  }
  else {
    // normal (ASCII) font
    if ((text_font_->font_attr & VGX_FONT_TYPE_MASK) == VGX_FONT_TYPE_PROP) {
      // prop font
      const font_prop_type* font_prop = text_font_->font_type_type.font_prop;
      do {
        if (ch >= font_prop->first && ch <= font_prop->last) {
          // found char
          const font_charinfo_type* info = &font_prop->char_info[ch - font_prop->first];
          std::uint16_t x, y, d;
          for (y = 0U; y < text_font_->ysize; ++y) {
            d = (1U + ((info->xsize - 1U) * color_depth / 8U)) * y;
            for (x = 0U; x < info->xsize ; ++x) {
              uint8_t intensity = (info->data[d + ((x * color_depth) >> 3U)] >> ((8U - (x + 1U) * color_depth) % 8U)) & color_mask;
              if (intensity) {
                if (color_depth == VGX_FONT_AA_NONE) {
                  pixel_set(text_x_act_ + x, text_y_act_ + y);
                }
                else {
                  std::uint32_t fg = color_;
                  std::uint32_t bg = pixel_get(text_x_act_ + x, text_y_act_ + y);
                  pixel_set_color(text_x_act_ + x, text_y_act_ + y, color_mix(fg, bg, color_depth == 2U ? intensity << 6U : intensity << 4U));
                }
              }
            }
          }
          text_x_act_ += info->xdist;
          return;
        }
        font_prop = font_prop->next;
      } while (font_prop);
      // char not found
      return;
    }
    else {
      // mono font
      const font_mono_type* font_mono = text_font_->font_type_type.font_mono;
      if (ch >= font_mono->first && ch <= font_mono->last) {
        std::uint16_t x, y, d;
        for (y = 0U; y < text_font_->ysize; ++y) {
          d = (ch - font_mono->first) * text_font_->ysize * font_mono->bytes_per_line + y * font_mono->bytes_per_line;
          for (x = 0U; x < font_mono->xsize; ++x) {
            uint8_t intensity = (font_mono->data[d + ((x * color_depth) >> 3U)] >> ((8U - (x + 1U) * color_depth) % 8U)) & color_mask;
            if (intensity) {
              if (color_depth == VGX_FONT_AA_NONE) {
                pixel_set(text_x_act_ + x, text_y_act_ + y);
              }
              else {
              }
            }
          }
        }
        text_x_act_ += font_mono->xsize;   // x-distance is xsize
        return;
      }
    }
  }
#endif  // VGX_CFG_FONT
}


/**
 * Render an ASCII/UTF-8 coded string
 * \param string Zero terminated text string in ASCII or UTF-8 format
 * \return Number of chars rendered
 */
uint16_t gpr::drv_text_string(const std::uint8_t* string)
{
  std::uint16_t ch, cnt = 0U;
  while (*string) {
    if ((*string & 0x80U) == 0x00U) {
      // 1 byte sequence (ASCII char)
      ch = (std::uint16_t)(*string++ & 0x7FU);
    }
    else if ((*string & 0xE0U) == 0xC0U) {
      // 2 byte sequence
      ch = (((std::uint16_t)*string & 0x001FU) << 6U) | ((std::uint16_t)*(string + 1U) & 0x003FU);
      string += 2U;
    } else if ((*string & 0xF0U) == 0xE0U) {
      // 3 byte UTF-8 sequence
      ch = (((std::uint16_t)*string & 0x000FU) << 12U) | (((std::uint16_t)*(string + 1U) & 0x003FU) << 6U) | ((std::uint16_t)*(string + 2U) & 0x003FU);
      string += 3U;
    } else {
      // unknown sequence
      string++;
      continue;
    }
    drv_text_char(ch);
    cnt++;
  }
  return cnt;
}


///////////////////////////////////////////////////////////////////////////////
// text functions

// select active font
bool gpr::text_set_font(const font_type& font)
{
  text_font_ = &font;
  return drv_text_set_font(font);
}


// set cursor position
bool gpr::text_set_pos(std::int16_t x, std::int16_t y)
{
  text_x_act_ = text_x_set_ = x;
  text_y_act_ = y;
  return drv_text_set_pos(x, y);
}


// set mode
bool gpr::text_set_mode(text_mode_type mode)
{
  text_mode_ = mode;
  drv_text_set_mode(mode);
  return true;
}


/**
 * Draw a single ASCII/UNICODE char at the actual cursor position
 * \param ch Character in Unicode format, 20-7F is compatible with ASCII
 */
void gpr::text_char(std::uint16_t ch)
{
  drv_text_char(ch);
  primitive_done();
}


/**
 * Render an ASCII/UTF-8 coded string
 * \param string Zero terminated text string in ASCII or UTF-8 format
 * \return Number of chars rendered
 */
uint16_t gpr::text_string(const std::uint8_t* string)
{
  std::uint16_t cnt = drv_text_string(string);
  primitive_done();
  return cnt;
}


/**
 * Render an ASCII/UTF-8 coded string at given position
 * \param x X-position
 * \param y Y-position
 * \param string Zero terminated text string in ASCII or UTF-8 format
 * \return Number of chars rendered
 */
uint16_t gpr::text_string_pos(std::int16_t x, std::int16_t y, const std::uint8_t* string)
{
  text_set_pos(x, y);
  std::uint16_t cnt = drv_text_string(string);
  primitive_done();
  return cnt;
}


/**
 * Helper function to calculate (lookup) sin(x) and cos(x), normalized to 100
 * Due to speed there's no input value check, so make sure angle is between 0-90
 * \param angle Angle in degree, valid range from 0° to 90°
 * \return sin(x) * 100 in upper byte, cos(x) * 100 in lower byte
 */
std::uint16_t gpr::sin(std::uint8_t angle) const
{
  const std::uint16_t angle_to_xy[91U] = {
    0x0064U, 0x0264U, 0x0364U, 0x0564U, 0x0764U, 0x0964U, 0x0A63U, 0x0C63U, 0x0E63U, 0x1063U, 0x1162U, 0x1362U, 0x1562U, 0x1661U, 0x1861U, 0x1A61U,
    0x1C60U, 0x1D60U, 0x1F5FU, 0x215FU, 0x225EU, 0x245DU, 0x255DU, 0x275CU, 0x295BU, 0x2A5BU, 0x2C5AU, 0x2D59U, 0x2F58U, 0x3057U, 0x3257U, 0x3456U,
    0x3555U, 0x3654U, 0x3853U, 0x3952U, 0x3B51U, 0x3C50U, 0x3E4FU, 0x3F4EU, 0x404DU, 0x424BU, 0x434AU, 0x4449U, 0x4548U, 0x4747U, 0x4845U, 0x4944U,
    0x4A43U, 0x4B42U, 0x4D40U, 0x4E3FU, 0x4F3EU, 0x503CU, 0x513BU, 0x5239U, 0x5338U, 0x5436U, 0x5535U, 0x5634U, 0x5732U, 0x5730U, 0x582FU, 0x592DU,
    0x5A2CU, 0x5B2AU, 0x5B29U, 0x5C27U, 0x5D25U, 0x5D24U, 0x5E22U, 0x5F21U, 0x5F1FU, 0x601DU, 0x601CU, 0x611AU, 0x6118U, 0x6116U, 0x6215U, 0x6213U,
    0x6211U, 0x6310U, 0x630EU, 0x630CU, 0x630AU, 0x6409U, 0x6407U, 0x6405U, 0x6403U, 0x6402U, 0x6400U
  };
  return angle_to_xy[angle];
}

} // namespace vgx
