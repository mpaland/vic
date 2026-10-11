///////////////////////////////////////////////////////////////////////////////
// \brief Tests for dc.h: shader pipe management, present locking and the
// equivalence of the head fast path with the gpr rendering path
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "helpers.h"
#include "dc.h"
#include "head/memory.h"

using namespace vic;
using vic_test::passthrough_shader;

typedef head::memory<16, 12> head_type;


TEST_CASE(dc_shader_pipe)
{
  head_type h;
  dc _dc(h);

  // the pipe initially only contains the output shader
  CHECK(!_dc.shader_is_active());
  shader::base* const output = _dc.shader_pipe();
  CHECK(output != nullptr);
  CHECK(output->next_ == nullptr);

  passthrough_shader s1, s2, s3;
  _dc.shader_register(&s1);
  CHECK(_dc.shader_is_active());
  CHECK_EQ(static_cast<const void*>(_dc.shader_pipe()), static_cast<const void*>(&s1));
  CHECK_EQ(static_cast<const void*>(s1.next_), static_cast<const void*>(output));

  // new shaders are inserted at the front
  _dc.shader_register(&s2);
  _dc.shader_register(&s3);
  CHECK_EQ(static_cast<const void*>(_dc.shader_pipe()), static_cast<const void*>(&s3));
  CHECK_EQ(static_cast<const void*>(s3.next_), static_cast<const void*>(&s2));
  CHECK_EQ(static_cast<const void*>(s2.next_), static_cast<const void*>(&s1));

  // all shaders see the pixel
  _dc.set_color(color::white);
  _dc.plot({ 1, 1 });
  CHECK_EQ(s1.count, 1U);
  CHECK_EQ(s2.count, 1U);
  CHECK_EQ(s3.count, 1U);
  CHECK_EQ(h.pixel({ 1, 1 }), color::white);

  // remove the middle one
  CHECK(_dc.shader_remove(&s2));
  CHECK_EQ(static_cast<const void*>(s3.next_), static_cast<const void*>(&s1));
  _dc.plot({ 2, 1 });
  CHECK_EQ(s2.count, 1U);
  CHECK_EQ(s1.count, 2U);

  // removing an unknown shader fails
  CHECK(!_dc.shader_remove(&s2));

  // remove the first one
  CHECK(_dc.shader_remove(&s3));
  CHECK_EQ(static_cast<const void*>(_dc.shader_pipe()), static_cast<const void*>(&s1));
  CHECK(_dc.shader_remove(&s1));
  CHECK(!_dc.shader_is_active());

  // shader_init resets the pipe
  _dc.shader_register(&s1);
  _dc.shader_init();
  CHECK(!_dc.shader_is_active());
  CHECK_EQ(static_cast<const void*>(_dc.shader_pipe()), static_cast<const void*>(output));
}


TEST_CASE(dc_present_lock)
{
  head_type h;
  dc _dc(h);
  _dc.set_color(color::white);

  _dc.plot({ 0, 0 });
  CHECK_EQ(h.present_count(), 1U);

  // nested locks: present only on the final unlock
  _dc.present_lock();
  _dc.plot({ 1, 0 });
  _dc.present_lock();
  _dc.line({ 0, 0 }, { 5, 5 });
  _dc.box({ 2, 2, 4, 4 });
  _dc.present_lock(false);
  CHECK_EQ(h.present_count(), 1U);
  _dc.present_lock(false);
  CHECK_EQ(h.present_count(), 2U);

  // unlock without lock does nothing
  _dc.present_lock(false);
  CHECK_EQ(h.present_count(), 2U);
  _dc.present();
  CHECK_EQ(h.present_count(), 3U);

  // pixel_set does not present
  _dc.pixel_set({ 3, 3 });
  _dc.pixel_set({ 3, 4 }, color::red);
  CHECK_EQ(h.present_count(), 3U);
  CHECK_EQ(h.pixel({ 3, 3 }), color::white);
  CHECK_EQ(h.pixel({ 3, 4 }), color::red);
  CHECK_EQ(_dc.pixel_get({ 3, 4 }), color::red);
}


TEST_CASE(dc_common)
{
  head_type h;
  dc _dc(h);
  CHECK_EQ(_dc.screen_width(), 16U);
  CHECK_EQ(_dc.screen_height(), 12U);
  CHECK_EQ(_dc.get_color(), color::gray);   // default color
  _dc.set_color(color::red);
  CHECK_EQ(_dc.get_color(), color::red);

  _dc.cls(color::blue);
  CHECK_EQ(h.cls_count(), 1U);
  CHECK_EQ(h.cls_color(), color::blue);
  CHECK_EQ(vic_test::count_pixels(h, color::blue), 16U * 12U);
}


TEST_CASE(dc_alpha_blending)
{
  head_type h;
  dc _dc(h);
  _dc.cls(color::blue);

  // alpha blending is enabled by default: 50% red over blue
  _dc.plot({ 0, 0 }, 0x80FF0000UL);
  const color::value_type b = h.pixel({ 0, 0 });
  CHECK(color::is_opaque(b));
  CHECK(color::get_red(b) >= 127U && color::get_red(b) <= 128U);
  CHECK(color::get_blue(b) >= 127U && color::get_blue(b) <= 128U);

  // disabled: the raw color is stored
  _dc.alpha_blending_enable(false);
  _dc.plot({ 1, 0 }, 0x80FF0000UL);
  CHECK_EQ(h.pixel({ 1, 0 }), 0x80FF0000UL);

  // opaque colors are stored as is in both modes
  _dc.alpha_blending_enable(true);
  _dc.plot({ 2, 0 }, color::green);
  CHECK_EQ(h.pixel({ 2, 0 }), color::green);
}


// render with the fast (head) path and with the gpr path and compare
namespace {

struct ops {
  template<typename F>
  static void compare(F f)
  {
    head_type ha, hb;
    dc da(ha), db(hb);
    passthrough_shader pass;
    db.shader_register(&pass);   // force the gpr path
    da.set_color(color::white);
    db.set_color(color::white);
    f(da);
    f(db);
    CHECK_SAME_BUFFER(ha, hb);
    CHECK_EQ(ha.present_count(), hb.present_count());
    // the fast path clips to the screen, so it never writes out of bounds
    CHECK_EQ(ha.oob_count(), 0U);
    CHECK_EQ(ha.pixel_set_count(), hb.pixel_set_count());
  }

  static void box_inside(dc& d)   { d.box({ 3, 2, 9, 7 }); }
  static void box_full(dc& d)     { d.box({ 0, 0, 16, 12 }); }
  static void box_empty(dc& d)    { d.box({ 5, 5, 5, 9 }); }
  static void box_clip_tl(dc& d)  { d.box({ -2, -2, 3, 3 }); }
  static void box_clip_br(dc& d)  { d.box({ 14, 10, 20, 20 }); }
  static void box_vertex(dc& d)   { d.box(vertex_type{ 8, 7 }, vertex_type{ 2, 3 }); }
  static void box_pixel(dc& d)    { d.box(vertex_type{ 4, 4 }, vertex_type{ 4, 4 }); }
  static void hline(dc& d)        { d.line_horz({ 2, 3 }, { 9, 3 }); }
  static void hline_rev(dc& d)    { d.line_horz({ 9, 3 }, { 2, 8 }); }
  static void hline_clip(dc& d)   { d.line_horz({ -4, 0 }, { 30, 0 }); }
  static void vline(dc& d)        { d.line_vert({ 5, 1 }, { 5, 9 }); }
  static void vline_rev(dc& d)    { d.line_vert({ 5, 9 }, { 1, 1 }); }
  static void vline_clip(dc& d)   { d.line_vert({ 0, -4 }, { 0, 30 }); }
};

} // namespace


TEST_CASE(dc_fast_path_equals_gpr_path)
{
  ops::compare(&ops::box_inside);
  ops::compare(&ops::box_full);
  ops::compare(&ops::box_empty);
  ops::compare(&ops::box_clip_tl);
  ops::compare(&ops::box_clip_br);
  ops::compare(&ops::box_vertex);
  ops::compare(&ops::box_pixel);
  ops::compare(&ops::hline);
  ops::compare(&ops::hline_rev);
  ops::compare(&ops::hline_clip);
  ops::compare(&ops::vline);
  ops::compare(&ops::vline_rev);
  ops::compare(&ops::vline_clip);
}


TEST_CASE(dc_fast_path_only_when_opaque)
{
  // a non opaque color must take the gpr path (and get alpha blended)
  head_type h;
  dc _dc(h);
  _dc.cls(color::blue);
  _dc.set_color(0x80FF0000UL);
  _dc.box({ 0, 0, 2, 2 });
  _dc.line_horz({ 0, 3 }, { 3, 3 });
  _dc.line_vert({ 5, 0 }, { 5, 3 });
  const color::value_type b = h.pixel({ 0, 0 });
  CHECK(color::is_opaque(b));
  CHECK(color::get_red(b) >= 127U && color::get_red(b) <= 128U);
  CHECK_EQ(h.pixel({ 1, 3 }), b);
  CHECK_EQ(h.pixel({ 5, 2 }), b);
}


TEST_CASE(dc_box_vertex_wrapper)
{
  head_type h;
  dc _dc(h);
  _dc.set_color(color::white);
  // both vertices are inside the box
  _dc.box(vertex_type{ 5, 4 }, vertex_type{ 2, 1 });
  CHECK_RASTER_AT(h, vic_test::PAL_X, (vertex_type{ 2, 1 }),
    "XXXX",
    "XXXX",
    "XXXX",
    "XXXX");
  CHECK_EQ(h.present_count(), 1U);
}


TEST_CASE(dc_rounded_box_accessible)
{
  // the gpr overloads with border radius must be reachable through a dc
  head_type h;
  dc _dc(h);
  _dc.set_color(color::white);
  _dc.box(rect_type{ 0, 0, 10, 6 }, 0U);
  CHECK_EQ(vic_test::count_pixels(h, color::white), 60U);
  _dc.cls();
  _dc.box(vertex_type{ 0, 0 }, vertex_type{ 9, 5 }, 0U);
  CHECK_EQ(vic_test::count_pixels(h, color::white), 60U);
  _dc.cls();
  _dc.rectangle(rect_type{ 0, 0, 10, 6 }, 0U);
  CHECK_EQ(vic_test::count_pixels(h, color::white), 28U);
}


TEST_CASE(dc_move)
{
  head_type h;
  dc _dc(h);
  _dc.set_color(color::white);
  _dc.box({ 0, 0, 2, 2 });
  h.reset_counters();

  _dc.move({ 0, 0, 2, 2 }, { 10, 8 });
  CHECK_EQ(h.pixel({ 10, 8 }), color::white);
  CHECK_EQ(h.pixel({ 11, 9 }), color::white);
  CHECK_EQ(h.pixel({ 12, 8 }), color::none);
  CHECK_EQ(h.present_count(), 1U);

  _dc.move({ 0, 0 }, { 5, 5 }, 2U, 2U);
  CHECK_EQ(h.pixel({ 5, 5 }), color::white);
  CHECK_EQ(h.pixel({ 6, 6 }), color::white);
  CHECK_EQ(h.pixel({ 7, 5 }), color::none);
  CHECK_EQ(h.present_count(), 2U);
}


TEST_CASE(dc_bg_color)
{
  head_type h;
  dc _dc(h);
  // no fallback by default, cls() is not affected by the background color
  CHECK_EQ(_dc.get_bg_color(), color::none);
  _dc.cls();
  CHECK_EQ(h.cls_color(), color::none);
  _dc.set_bg_color(color::blue);
  CHECK_EQ(_dc.get_bg_color(), color::blue);
  _dc.cls();
  CHECK_EQ(h.cls_color(), color::none);
  // the memory head reads its pixels, so the background color is not used for blending
  _dc.cls(color::red);
  _dc.plot({ 0, 0 }, 0x80FFFFFFUL);
  const color::value_type b = h.pixel({ 0, 0 });
  CHECK(color::get_red(b) >= 250U);           // white over red, not white over blue
  CHECK(color::get_blue(b) <= 128U);
}


TEST_CASE(dc_blit)
{
  static const color::value_type block[6] = { color::red, color::green, color::blue, color::white, color::red, color::green };
  head_type h;
  dc _dc(h);
  _dc.cls(color::black);

  // opaque block, no shader: one native blit
  _dc.blit({ 2, 2, 5, 4 }, block, 3U);
  CHECK_EQ(h.blit_count(), 1U);
  CHECK_EQ(h.pixel_set_count(), 0U);
  CHECK_EQ(h.present_count(), 1U);
  CHECK_EQ(h.pixel({ 2, 2 }), color::red);
  CHECK_EQ(h.pixel({ 4, 3 }), color::green);
  CHECK_EQ(vic_test::count_pixels_not(h, color::black), 6U);

  // with a shader the block takes the pipe (same pixels, no native blit)
  head_type hp;
  dc dp(hp);
  dp.cls(color::black);
  passthrough_shader pass;
  dp.shader_register(&pass);
  dp.blit({ 2, 2, 5, 4 }, block, 3U);
  CHECK_EQ(hp.blit_count(), 0U);
  CHECK_EQ(pass.count, 6U);
  CHECK_SAME_BUFFER(h, hp);

  // a translucent pixel is blended, so the block takes the pipe as well
  static const color::value_type alpha_block[2] = { 0x80FF0000UL, color::green };
  _dc.cls(color::blue);
  h.reset_counters();
  _dc.blit({ 0, 0, 2, 1 }, alpha_block, 2U);
  CHECK_EQ(h.blit_count(), 0U);
  const color::value_type b = h.pixel({ 0, 0 });
  CHECK(color::is_opaque(b));
  CHECK(color::get_red(b) >= 127U && color::get_red(b) <= 128U);
  CHECK(color::get_blue(b) >= 127U && color::get_blue(b) <= 128U);
  CHECK_EQ(h.pixel({ 1, 0 }), color::green);
  // without alpha blending the raw colors are blitted natively
  _dc.alpha_blending_enable(false);
  h.reset_counters();
  _dc.blit({ 0, 1, 2, 2 }, alpha_block, 2U);
  CHECK_EQ(h.blit_count(), 1U);
  CHECK_EQ(h.pixel({ 0, 1 }), 0x80FF0000UL);
}
