///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2020, PALANDesign Hannover, Germany
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
// \brief Sprite demo
// A little night scene which shows the sprite system of vic:
// - sheet sprites out of several sprite sheets (bina, coin, ghost, ship, star)
// - a canvas sprite drawn with gpr primitives (the beach ball)
// - sprite rotation: the ball rolls, the coins spin and the bursts rotate
// - anti aliased primitives (moon, hills and the ball are drawn with anti aliasing enabled)
// - z-order of overlapping sprites
// - semi transparent sprites which are alpha blended with the background (ghosts, burst)
// - pixel exact collision detection: the ghosts turn red (tint shader) while they overlap
// - background restoring when sprites move
// - one shot animations (the star burst) by rendering an invalid frame to hide the sprite
// The background is rendered once with gradient shaders and text, afterwards only
// the sprites touch the screen.
//
// Link this file together with windows/main.cpp (see demo/CMakeLists.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>

#include "dc.h"
#include "tc.h"
#include "sprite.h"
#include "shader/base.h"
#include "shader/gradient.h"

// fonts
#include "font/LCD_6x8.h"
#include "font/Montserrat_16_aa4.h"

// sprite sheets
#include "sprite_sheets/bina.h"
#include "sprite_sheets/coin.h"
#include "sprite_sheets/ghost.h"
#include "sprite_sheets/ship.h"
#include "sprite_sheets/star.h"


// the head is created in main.cpp
extern vic::drv& get_singlehead(void);

// output head
static vic::drv& head()
{
  return get_singlehead();
}


namespace {

// scene constants (the head is 320 x 200 pixel)
const std::int16_t SCREEN_W  = 320;
const std::int16_t SCREEN_H  = 200;
const std::int16_t GROUND_Y  = 160;     // top of the ground
const std::uint32_t FRAME_MS = 40U;     // 25 frames per second


// little integer helpers
inline std::int16_t isin(std::int32_t angle, std::int32_t amplitude)
{
  // sin(angle) * amplitude, angle in degree
  return static_cast<std::int16_t>(static_cast<std::int32_t>(vic::util::sin(static_cast<std::int16_t>(angle % 360))) * amplitude / 16384);
}

// pseudo random generator (LCG), deterministic so every run looks the same
inline std::uint32_t rnd(std::uint32_t& seed)
{
  seed = seed * 1103515245U + 12345U;
  return (seed >> 8U) & 0xFFFFFFU;
}


// collision state of a ghost
struct ghost_state {
  vic::sprite::base* other_ghost;   // the ghost we are looking for
  bool               hit;           // true if we collided with it in the last render
};


// collision handler of the ghosts: the ghosts touch bina, the ship and the ball too, but only a hit with the
// other ghost counts. Just remember it, the ghost is tinted red at its next render
void on_ghost_collision(vic::sprite::base&, vic::sprite::base& other, void* arg)
{
  ghost_state& state = *static_cast<ghost_state*>(arg);
  if (&other == state.other_ghost) {
    state.hit = true;
  }
}


// render a ghost, tinted red if it collided with the other ghost in its last render
void render_ghost(vic::sprite::canvas_sheet<576>& ghost, vic::vertex_type pos, std::uint16_t frame, ghost_state& state, vic::shader::tint& red)
{
  if (state.hit) {
    state.hit = false;      // consumed, render() below sets it again if the ghosts still overlap
    ghost.shader_register(&red);
    ghost.render(pos, frame);
    ghost.shader_remove(&red);
  }
  else {
    ghost.render(pos, frame);
  }
}

} // namespace


/**
 * Render the static background: sky gradient, stars, moon, hills, ground and the title
 */
static void render_background()
{
  vic::dc _dc(head());
  _dc.anti_aliasing_enable(true);    // smooth edges for the moon and the hills
  _dc.present_lock();

  // S K Y  -  a vertical gradient from deep blue to a warm horizon
  vic::shader::gradient_vertical<3> sky = { {   0,            0, vic::color::argb(0x10U, 0x10U, 0x40U) },
                                            {   0,          100, vic::color::argb(0x50U, 0x30U, 0x70U) },
                                            {   0, GROUND_Y - 1, vic::color::argb(0xE0U, 0x70U, 0x30U) } };
  _dc.shader_register(&sky);
  _dc.set_color(vic::color::white);
  _dc.box({ 0, 0, SCREEN_W, GROUND_Y });
  _dc.shader_remove(&sky);

  // S T A R S  -  random dots with random brightness in the upper sky
  std::uint32_t seed = 4711U;
  for (unsigned n = 0U; n < 70U; ++n) {
    const std::int16_t x = static_cast<std::int16_t>(rnd(seed) % SCREEN_W);
    const std::int16_t y = static_cast<std::int16_t>(rnd(seed) % 110U);
    const std::uint8_t lum = static_cast<std::uint8_t>(100U + rnd(seed) % 156U);
    _dc.pixel_set({ x, y }, vic::color::dim(vic::color::white, lum));
  }

  // M O O N  -  a crescent: a bright disc, then a sky colored disc punched into it
  _dc.set_color(vic::color::argb(0xFFU, 0xF0U, 0xC0U));
  _dc.disc({ 262, 38 }, 16U);
  _dc.shader_register(&sky);
  _dc.set_color(vic::color::white);
  _dc.disc({ 269, 33 }, 14U);
  _dc.shader_remove(&sky);

  // H I L L S  -  two big dark discs behind the ground
  _dc.set_color(vic::color::argb(0x28U, 0x40U, 0x30U));
  _dc.disc({  60, GROUND_Y + 50 }, 80U);
  _dc.set_color(vic::color::argb(0x30U, 0x50U, 0x38U));
  _dc.disc({ 250, GROUND_Y + 70 }, 100U);

  // G R O U N D  -  with a grass stripe
  _dc.set_color(vic::color::argb(0x20U, 0x50U, 0x20U));
  _dc.box({ 0, GROUND_Y, SCREEN_W, SCREEN_H });
  _dc.set_color(vic::color::argb(0x40U, 0xA0U, 0x40U));
  _dc.box({ 0, GROUND_Y, SCREEN_W, static_cast<std::int16_t>(GROUND_Y + 3) });
  // some grass blades
  seed = 815U;
  _dc.set_color(vic::color::argb(0x60U, 0xC0U, 0x50U));
  for (unsigned n = 0U; n < 50U; ++n) {
    const std::int16_t x = static_cast<std::int16_t>(rnd(seed) % SCREEN_W);
    const std::int16_t h = static_cast<std::int16_t>(2U + rnd(seed) % 3U);
    _dc.line_vert({ x, static_cast<std::int16_t>(GROUND_Y - h) }, { x, static_cast<std::int16_t>(GROUND_Y - 1) });
  }

  // T I T L E  -  anti aliased text, colored with the \a escape
  vic::tc _tc(_dc);
  vic::font::Montserrat_16_aa4 montserrat;
  _tc.set_font(montserrat);
  _tc.set_color(vic::color::white);
  _tc.out({ 6, 2 }, (const std::uint8_t*)"VIC \aFFD040sprite\aFFFFFF demo");

  vic::font::LCD_6x8 lcd;
  _tc.set_font(lcd);
  _tc.set_color(vic::color::argb(0xA0U, 0xD0U, 0xA0U));
  _tc.out({ 6, SCREEN_H - 10 }, (const std::uint8_t*)"sheets: bina coin ghost ship star + canvas");

  _dc.present_lock(false);
}


/**
 * The demo
 */
void demo()
{
  // init the head and draw the scene
  head().init();
  render_background();

  // S P R I T E S
  // the z-index defines what is in front: bursts > ghosts > ship > ball > bina > coins
  static vic::sprite::sheet<32, 48> bina(head(), &sprite_sheet_bina, 10);
  // coins and bursts are canvas sheets, so they can be rotated (the canvas needs some headroom for rotated frames)
  static vic::sprite::canvas_sheet<320> coin0(head(), &sprite_sheet_coin, 5);
  static vic::sprite::canvas_sheet<320> coin1(head(), &sprite_sheet_coin, 5);
  static vic::sprite::canvas_sheet<320> coin2(head(), &sprite_sheet_coin, 5);
  // the ghosts are canvas sheets, so a tint shader can recolor them when they collide
  static vic::sprite::canvas_sheet<576> ghost0(head(), &sprite_sheet_ghost, 30);
  static vic::sprite::canvas_sheet<576> ghost1(head(), &sprite_sheet_ghost, 31);
  static ghost_state ghost_st[2] = { { &ghost1, false }, { &ghost0, false } };
  ghost0.set_collision_handler(on_ghost_collision, &ghost_st[0]);
  ghost1.set_collision_handler(on_ghost_collision, &ghost_st[1]);
  vic::shader::tint ghost_red(vic::color::red, 200U);
  static vic::sprite::sheet<24, 16> ship(head(), &sprite_sheet_ship, 20);
  static vic::sprite::canvas_sheet<1280> burst0(head(), &sprite_sheet_star, 40);
  static vic::sprite::canvas_sheet<1280> burst1(head(), &sprite_sheet_star, 41);

  // the ball is a canvas sprite: a beach ball drawn with disc sectors, it is rotated while rolling
  static vic::sprite::canvas<256, 1> ball(head(), 15);
  {
    ball.anti_aliasing_enable(true);   // the ball gets anti aliased edges
    ball.set_color(vic::color::argb(0xE0U, 0x30U, 0x30U));
    ball.disc_sector({ 0, 0 }, 7U, 0U);
    ball.disc_sector({ 0, 0 }, 7U, 2U);
    ball.set_color(vic::color::argb(0xF0U, 0xF0U, 0xF0U));
    ball.disc_sector({ 0, 0 }, 7U, 1U);
    ball.disc_sector({ 0, 0 }, 7U, 3U);
    ball.set_color(vic::color::argb(0x60U, 0x10U, 0x10U));
    ball.circle({ 0, 0 }, 7U);
    ball.set_color(vic::color::argb(0xFFU, 0xFFU, 0xFFU, 0xA0U));    // translucent highlight
    ball.disc({ -3, -3 }, 1U);
  }

  vic::sprite::canvas_sheet<320>*  const coins[3]  = { &coin0, &coin1, &coin2 };
  vic::sprite::canvas_sheet<1280>* const bursts[2] = { &burst0, &burst1 };

  // A N I M A T I O N   S T A T E
  std::int16_t  bina_x    = 20,  bina_dir = 1;             // bina walks left and right
  std::int16_t  ship_x    = -24, ship_y_old = 40;          // the ship flies from left to right
  std::int16_t  ball_x    = 100, ball_dx = 1;              // the ball bounces
  std::int32_t  ball_y16  = 60 * 16, ball_vy16 = 0;        // ball y and velocity in 1/16 pixel
  std::int16_t  ball_angle = 0;                            // ball rotation, it rolls with its horizontal speed
  std::int16_t  burst_tick[2] = { -1, -1 };                // burst animation ticks, < 0 is idle
  vic::vertex_type burst_pos[2] = { { 0, 0 }, { 0, 0 } };
  std::uint32_t seed = 42U;

  for (std::uint32_t t = 0U; ; ++t) {

    // B I N A  -  walks along the ground, the sheet has 4 columns per direction row
    //             rows: 0 down, 1 left, 2 right, 3 up
    bina_x = static_cast<std::int16_t>(bina_x + bina_dir);
    if (bina_x >= SCREEN_W - 32 - 20) { bina_dir = -1; }
    if (bina_x <= 20)                 { bina_dir =  1; }
    const std::uint16_t bina_row   = bina_dir > 0 ? 2U : 1U;
    const std::uint16_t bina_frame = static_cast<std::uint16_t>(bina_row * 4U + (t / 6U) % 4U);
    bina.render({ bina_x, GROUND_Y - 48 }, bina_frame);

    // C O I N S  -  spin (frames), rotate around their center and bob
    for (std::int32_t i = 0; i < 3; ++i) {
      const std::int16_t x = static_cast<std::int16_t>(60 + i * 90);
      const std::int16_t y = static_cast<std::int16_t>(GROUND_Y - 30 + isin(static_cast<std::int32_t>(t) * 8 + i * 120, 4));
      const std::int16_t angle = static_cast<std::int16_t>((static_cast<std::int32_t>(t) * (4 + i * 2)) % 360);
      coins[i]->render({ x, y }, static_cast<std::uint16_t>((t / 3U + static_cast<std::uint32_t>(i) * 3U) % 8U), angle);
    }

    // G H O S T S  -  float around on Lissajous curves, semi transparent, red while they collide
    {
      const std::int16_t x0 = static_cast<std::int16_t>(150 + isin(static_cast<std::int32_t>(t) * 2, 120));
      const std::int16_t y0 = static_cast<std::int16_t>(60  + isin(static_cast<std::int32_t>(t) * 5, 25));
      render_ghost(ghost0, { x0, y0 }, static_cast<std::uint16_t>((t / 8U) % 4U), ghost_st[0], ghost_red);
      const std::int16_t x1 = static_cast<std::int16_t>(150 + isin(static_cast<std::int32_t>(t) * 3 + 180, 100));
      const std::int16_t y1 = static_cast<std::int16_t>(85  + isin(static_cast<std::int32_t>(t) * 4 + 90, 30));
      render_ghost(ghost1, { x1, y1 }, static_cast<std::uint16_t>((t / 8U + 2U) % 4U), ghost_st[1], ghost_red);
    }

    // S H I P  -  flies in a wave from left to right, banks with its vertical speed
    {
      ship_x = static_cast<std::int16_t>(ship_x + 2);
      if (ship_x > SCREEN_W) {
        ship_x = -24;
      }
      const std::int16_t ship_y = static_cast<std::int16_t>(36 + isin(static_cast<std::int32_t>(t) * 6, 18));
      const std::uint16_t ship_frame = ship_y < ship_y_old ? 1U : ship_y > ship_y_old ? 2U : 0U;
      ship_y_old = ship_y;
      ship.render({ ship_x, ship_y }, ship_frame);
    }

    // B A L L  -  gravity, bouncing on the ground and at the screen borders
    {
      ball_vy16 += 3;                                   // gravity
      ball_y16  += ball_vy16;
      if (ball_y16 > (GROUND_Y - 8) * 16) {
        ball_y16  = (GROUND_Y - 8) * 16;
        ball_vy16 = -ball_vy16 * 7 / 8;                 // damped bounce
        if (ball_vy16 > -60) {
          ball_vy16 = -170;                             // kick it up again
          // a burst where the ball hits the ground
          for (std::int32_t b = 0; b < 2; ++b) {
            if (burst_tick[b] < 0) {
              burst_tick[b] = 0;
              burst_pos[b]  = { static_cast<std::int16_t>(ball_x - 16), static_cast<std::int16_t>(GROUND_Y - 24) };
              break;
            }
          }
        }
      }
      ball_x = static_cast<std::int16_t>(ball_x + ball_dx);
      if ((ball_x > SCREEN_W - 10) || (ball_x < 10)) {
        ball_dx = static_cast<std::int16_t>(-ball_dx);
      }
      // roll: the circumference of the ball is about 44 pixel, so one pixel is about 8 degree
      ball_angle = static_cast<std::int16_t>((ball_angle + ball_dx * 8 + 360) % 360);
      ball.set_rotation(ball_angle, { 0, 0 });
      ball.render({ ball_x, static_cast<std::int16_t>(ball_y16 / 16) });
    }

    // B U R S T S  -  one shot animations: random bursts in the sky plus the ball impacts
    if ((t % 60U) == 30U) {
      for (std::int32_t b = 0; b < 2; ++b) {
        if (burst_tick[b] < 0) {
          burst_tick[b] = 0;
          burst_pos[b]  = { static_cast<std::int16_t>(20U + rnd(seed) % (SCREEN_W - 70U)), static_cast<std::int16_t>(10U + rnd(seed) % 100U) };
          break;
        }
      }
    }
    for (std::int32_t b = 0; b < 2; ++b) {
      if (burst_tick[b] >= 0) {
        const std::uint16_t frame = static_cast<std::uint16_t>(burst_tick[b] / 3);
        if (frame < sprite_sheet_star.width / sprite_sheet_star.sprite_width) {
          // the burst rotates slowly while it plays
          bursts[b]->render(burst_pos[b], frame, static_cast<std::int16_t>(burst_tick[b] * 5));
          burst_tick[b]++;
        }
        else {
          // animation done: render an invalid frame to hide the sprite and restore the background
          bursts[b]->render(burst_pos[b], 0xFFFFU);
          burst_tick[b] = -1;
        }
      }
    }

    vic::io::delay(FRAME_MS);
  }
}
