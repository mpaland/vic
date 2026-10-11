///////////////////////////////////////////////////////////////////////////////
// \brief Tests for the rendering cost of the primitives: how many head commands
// (native line_horz/line_vert/box vs single pixel_set) reach the head.
// On serial bus displays every head command is a bus transaction, so composite
// primitives must use the native line/box functions of the head whenever
// possible (no shader active, opaque color) and must produce exactly the same
// pixels as the per pixel path.
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "helpers.h"
#include "dc.h"
#include "sprite.h"
#include "head/memory.h"

using namespace vic;
using vic_test::PAL_X;

namespace {

// memory head which counts the native commands it receives
template<std::uint16_t X, std::uint16_t Y>
class counting_head : public head::memory<X, Y>
{
public:
  std::size_t line_horz_calls;
  std::size_t line_vert_calls;
  std::size_t box_calls;
  std::size_t pixel_calls;      // pixel_set calls NOT caused by a native line/box command

  counting_head()
    : line_horz_calls(0U), line_vert_calls(0U), box_calls(0U), pixel_calls(0U), native_(0U)
  { }

  void reset()
  {
    line_horz_calls = line_vert_calls = box_calls = pixel_calls = 0U;
    this->cls();
    this->reset_counters();
  }

protected:
  virtual void pixel_set(vertex_type vertex, color::value_type color)
  {
    if (!native_) {
      ++pixel_calls;
    }
    head::memory<X, Y>::pixel_set(vertex, color);
  }

  // nested calls (e.g. the drv box fallback calling line_horz) are part of one native command
  virtual void line_horz(vertex_type v0, vertex_type v1, color::value_type color)
  {
    if (!native_) {
      ++line_horz_calls;
    }
    ++native_;
    drv::line_horz(v0, v1, color);
    --native_;
  }

  virtual void line_vert(vertex_type v0, vertex_type v1, color::value_type color)
  {
    if (!native_) {
      ++line_vert_calls;
    }
    ++native_;
    drv::line_vert(v0, v1, color);
    --native_;
  }

  virtual void box(const rect_type& rect, color::value_type color)
  {
    if (!native_) {
      ++box_calls;
    }
    ++native_;
    drv::box(rect, color);
    --native_;
  }

private:
  std::size_t native_;
};

typedef counting_head<64, 48> head_type;

// render the same primitive on the fast path and on the pipe path and compare the pixels
template<typename F>
void same_pixels(F f)
{
  head_type a;
  head::memory<64, 48> b;
  dc da(a), db(b);
  vic_test::passthrough_shader pass;
  db.shader_register(&pass);
  da.set_color(color::white);
  db.set_color(color::white);
  f(da);
  f(db);
  CHECK_SAME_BUFFER(a, b);
  CHECK_EQ(a.present_count(), b.present_count());
}

} // namespace


TEST_CASE(perf_box_is_one_command)
{
  head_type h;
  dc d(h);
  d.set_color(color::white);
  d.box({ 5, 5, 45, 25 });
  CHECK_EQ(h.box_calls, 1U);
  CHECK_EQ(h.pixel_calls, 0U);
  CHECK_EQ(h.line_horz_calls, 0U);
  CHECK_EQ(vic_test::count_pixels(h, color::white), 800U);

  // a box far outside the screen is clipped before it reaches the head
  h.reset();
  d.box({ -100, -100, 1000, 1000 });
  CHECK_EQ(h.box_calls, 1U);
  CHECK_EQ(h.oob_count(), 0U);
  CHECK_EQ(vic_test::count_pixels(h, color::white), 64U * 48U);
  h.reset();
  d.box({ 100, 100, 200, 200 });
  CHECK_EQ(h.oob_count(), 0U);
  CHECK_EQ(h.pixel_set_count(), 0U);
  CHECK_EQ(h.present_count(), 1U);
}


TEST_CASE(perf_lines_native)
{
  head_type h;
  dc d(h);
  d.set_color(color::white);

  // axis aligned lines are one native command
  d.line({ 2, 3 }, { 40, 3 });
  CHECK_EQ(h.line_horz_calls, 1U);
  CHECK_EQ(h.pixel_calls, 0U);
  d.line({ 7, 40 }, { 7, 2 });
  CHECK_EQ(h.line_vert_calls, 1U);
  CHECK_EQ(h.pixel_calls, 0U);

  // a shallow line is drawn as one horizontal run per row
  h.reset();
  d.line({ 0, 10 }, { 29, 15 });
  CHECK_EQ(h.line_horz_calls, 6U);
  CHECK_EQ(h.pixel_calls, 0U);
  CHECK_EQ(vic_test::count_pixels(h, color::white), 30U);

  // a steep line as one vertical run per column
  h.reset();
  d.line({ 20, 0 }, { 25, 29 });
  CHECK_EQ(h.line_vert_calls, 6U);
  CHECK_EQ(h.pixel_calls, 0U);

  // clipped horizontal line: nothing out of bounds reaches the head
  h.reset();
  d.line_horz({ -50, 5 }, { 500, 5 });
  CHECK_EQ(h.line_horz_calls, 1U);
  CHECK_EQ(h.oob_count(), 0U);
  CHECK_EQ(vic_test::count_pixels(h, color::white), 64U);
  h.reset();
  d.line_vert({ 5, -50 }, { 5, 500 });
  CHECK_EQ(h.oob_count(), 0U);
  CHECK_EQ(vic_test::count_pixels(h, color::white), 48U);
  h.reset();
  d.line_horz({ 0, 100 }, { 10, 100 });
  CHECK_EQ(h.pixel_set_count(), 0U);
  CHECK_EQ(h.oob_count(), 0U);
}


TEST_CASE(perf_composite_primitives_native)
{
  head_type h;
  dc d(h);
  d.set_color(color::white);

  d.disc({ 30, 20 }, 10U);
  CHECK_EQ(h.line_horz_calls, 21U);
  CHECK_EQ(h.pixel_calls, 0U);
  CHECK_EQ(h.present_count(), 1U);

  h.reset();
  d.disc_sector({ 30, 20 }, 10U, 0U);
  CHECK_EQ(h.line_horz_calls, 11U);
  CHECK_EQ(h.pixel_calls, 0U);

  h.reset();
  d.triangle_solid({ 2, 2 }, { 50, 10 }, { 20, 40 });
  CHECK(h.line_horz_calls > 0U);
  CHECK_EQ(h.pixel_calls, 0U);
  CHECK_EQ(h.present_count(), 1U);

  h.reset();
  d.sector({ 30, 20 }, 5U, 15U, 20U, 300U);
  CHECK(h.line_horz_calls > 0U);
  CHECK_EQ(h.pixel_calls, 0U);

  h.reset();
  d.rectangle({ 5, 5, 40, 30 });
  CHECK_EQ(h.line_horz_calls, 2U);
  CHECK_EQ(h.line_vert_calls, 2U);
  CHECK_EQ(h.pixel_calls, 0U);

  h.reset();
  d.box(rect_type{ 5, 5, 40, 30 }, 4U);
  CHECK_EQ(h.box_calls, 3U);
  CHECK(h.line_horz_calls > 0U);
  CHECK_EQ(h.pixel_calls, 0U);
  CHECK_EQ(h.present_count(), 1U);

  // the outline primitives still plot pixels (no runs to exploit)
  h.reset();
  d.circle({ 30, 20 }, 10U);
  CHECK(h.pixel_calls > 0U);
  CHECK_EQ(h.line_horz_calls, 0U);
}


TEST_CASE(perf_pipe_path_bypasses_native)
{
  // with a shader active (or a non opaque color) everything goes through the pipe pixel by pixel
  head_type h;
  dc d(h);
  vic_test::passthrough_shader pass;
  d.shader_register(&pass);
  d.set_color(color::white);
  d.disc({ 30, 20 }, 10U);
  d.box({ 1, 1, 10, 10 });
  d.line({ 0, 0 }, { 40, 7 });
  CHECK_EQ(h.line_horz_calls, 0U);
  CHECK_EQ(h.line_vert_calls, 0U);
  CHECK_EQ(h.box_calls, 0U);
  CHECK(h.pixel_calls > 0U);

  d.shader_remove(&pass);
  h.reset();
  d.set_color(0x80FFFFFFUL);
  d.disc({ 30, 20 }, 10U);
  CHECK_EQ(h.line_horz_calls, 0U);
  CHECK(h.pixel_calls > 0U);
}


TEST_CASE(perf_sprites_never_use_native_commands)
{
  // drawing on a canvas must not reach the head at all, rendering uses pixel_set only
  head_type h;
  sprite::canvas<512, 1> s(h, 0);
  s.set_color(color::white);
  s.disc({ 0, 0 }, 6U);
  s.box({ -3, -3, 3, 3 });
  s.line({ -6, 0 }, { 6, 0 });
  CHECK_EQ(h.pixel_calls, 0U);
  CHECK_EQ(h.line_horz_calls, 0U);
  CHECK_EQ(h.box_calls, 0U);
  s.render({ 20, 20 });
  CHECK(h.pixel_calls > 0U);
  CHECK_EQ(h.line_horz_calls, 0U);
  CHECK_EQ(h.box_calls, 0U);
}


TEST_CASE(perf_fast_path_pixels_identical)
{
  struct ops {
    static void shallow(dc& d)   { d.line({ 1, 2 }, { 60, 11 }); }
    static void steep(dc& d)     { d.line({ 3, 1 }, { 12, 46 }); }
    static void diagonal(dc& d)  { d.line({ 0, 0 }, { 40, 40 }); }
    static void reversed(dc& d)  { d.line({ 60, 11 }, { 1, 2 }); }
    static void horz(dc& d)      { d.line({ 50, 7 }, { 3, 7 }); }
    static void vert(dc& d)      { d.line({ 9, 40 }, { 9, 4 }); }
    static void clipped(dc& d)   { d.line({ -20, -5 }, { 90, 60 }); }
    static void disc(dc& d)      { d.disc({ 30, 20 }, 15U); }
    static void sector(dc& d)    { d.sector({ 30, 20 }, 4U, 18U, 30U, 250U); }
    static void tri(dc& d)       { d.triangle_solid({ 1, 1 }, { 60, 12 }, { 25, 45 }); }
    static void rbox(dc& d)      { d.box(rect_type{ 2, 2, 50, 40 }, 7U); }
    static void rrect(dc& d)     { d.rectangle(rect_type{ 2, 2, 50, 40 }, 7U); }
    static void bigbox(dc& d)    { d.box({ -10, -10, 100, 100 }); }
    static void poly(dc& d)      { static const vertex_type v[] = { { 1, 40 }, { 20, 5 }, { 40, 30 }, { 60, 2 } }; d.polyline(v, 4U); }
  };
  same_pixels(&ops::shallow);
  same_pixels(&ops::steep);
  same_pixels(&ops::diagonal);
  same_pixels(&ops::reversed);
  same_pixels(&ops::horz);
  same_pixels(&ops::vert);
  same_pixels(&ops::clipped);
  same_pixels(&ops::disc);
  same_pixels(&ops::sector);
  same_pixels(&ops::tri);
  same_pixels(&ops::rbox);
  same_pixels(&ops::rrect);
  same_pixels(&ops::bigbox);
  same_pixels(&ops::poly);
}
