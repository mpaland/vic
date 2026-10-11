///////////////////////////////////////////////////////////////////////////////
// \brief Micro benchmark of the gpr primitives
// Measures the CPU time per primitive on a RAM head (memory head, no native
// line/box functions, so every pixel is a pixel_set) and the number of head
// commands a serial display would receive. Not a unit test, the executable
// vic_bench prints a table.
///////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <cstdio>

#include "dc.h"
#include "head/memory.h"
#include "io.h"

namespace {

typedef vic::head::memory<240, 160> head_type;

// head which counts commands and does nothing else (measures the gpr overhead only)
class null_head : public vic::drv
{
public:
  std::size_t pixels, lines, boxes;
  null_head() : drv(240U, 160U, 240U, 160U), pixels(0U), lines(0U), boxes(0U) { }
  virtual void init() { }
  virtual void shutdown() { }
  virtual const char* version() const { return "null"; }
  virtual bool is_graphic() const { return true; }
protected:
  virtual void pixel_set(vic::vertex_type, vic::color::value_type) { ++pixels; }
  virtual vic::color::value_type pixel_get(vic::vertex_type) { return vic::color::black; }
  virtual void line_horz(vic::vertex_type, vic::vertex_type, vic::color::value_type) { ++lines; }
  virtual void line_vert(vic::vertex_type, vic::vertex_type, vic::color::value_type) { ++lines; }
  virtual void box(const vic::rect_type&, vic::color::value_type) { ++boxes; }
};

template<typename F>
void bench(const char* name, F f)
{
  const int runs = 2000;

  // CPU time on the memory head (every pixel is written to RAM)
  head_type mem;
  vic::dc dm(mem);
  dm.set_color(vic::color::white);
  const std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
  for (int i = 0; i < runs; ++i) {
    f(dm);
  }
  const std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
  const double us = std::chrono::duration<double, std::micro>(t1 - t0).count() / runs;

  // commands on a head with native line/box functions
  null_head nh;
  vic::dc dn(nh);
  dn.set_color(vic::color::white);
  f(dn);

  // pixels drawn
  std::size_t px = 0U;
  for (std::int16_t y = 0; y < 160; ++y) {
    for (std::int16_t x = 0; x < 240; ++x) {
      px += (mem.pixel({ x, y }) == vic::color::white) ? 1U : 0U;
    }
  }

  std::printf("%-28s %8.1f us  %7u px  %6.1f ns/px   commands to a native head: %5u pixel %4u line %3u box\n",
              name, us, static_cast<unsigned>(px), px ? us * 1000.0 / px : 0.0,
              static_cast<unsigned>(nh.pixels), static_cast<unsigned>(nh.lines), static_cast<unsigned>(nh.boxes));
}

} // namespace


// io stub
namespace vic { namespace io {
void delay(std::uint32_t) { }
void init(handle_type) { }
bool write(handle_type, std::uint32_t, const std::uint8_t*, std::size_t, std::uint8_t*, std::size_t, std::uint32_t) { return true; }
bool read(handle_type, std::uint32_t, std::uint8_t*, std::size_t& len, std::uint32_t) { len = 0U; return false; }
} }


int main()
{
  struct ops {
    static void line_shallow(vic::dc& d)  { d.line({ 5, 10 }, { 230, 60 }); }
    static void line_steep(vic::dc& d)    { d.line({ 20, 5 }, { 60, 150 }); }
    static void line_diag(vic::dc& d)     { d.line({ 5, 5 }, { 155, 155 }); }
    static void line_horz(vic::dc& d)     { d.line({ 5, 80 }, { 230, 80 }); }
    static void box(vic::dc& d)           { d.box({ 20, 20, 220, 140 }); }
    static void box_rounded(vic::dc& d)   { d.box(vic::rect_type{ 20, 20, 220, 140 }, 12U); }
    static void rectangle(vic::dc& d)     { d.rectangle({ 20, 20, 220, 140 }); }
    static void circle(vic::dc& d)        { d.circle({ 120, 80 }, 60U); }
    static void disc(vic::dc& d)          { d.disc({ 120, 80 }, 60U); }
    static void sector(vic::dc& d)        { d.sector({ 120, 80 }, 20U, 60U, 30U, 300U); }
    static void triangle(vic::dc& d)      { d.triangle_solid({ 10, 150 }, { 120, 10 }, { 230, 150 }); }
  };

  std::printf("vic gpr micro benchmark, %s, 240 x 160 memory head\n\n", VIC_HEAD_MEMORY_VERSION);
  bench("line shallow (225 px)", &ops::line_shallow);
  bench("line steep (145 px)",   &ops::line_steep);
  bench("line diagonal (150 px)",&ops::line_diag);
  bench("line horizontal",       &ops::line_horz);
  bench("box 200x120",           &ops::box);
  bench("box rounded r12",       &ops::box_rounded);
  bench("rectangle 200x120",     &ops::rectangle);
  bench("circle r60",            &ops::circle);
  bench("disc r60",              &ops::disc);
  bench("sector 20..60 30..300", &ops::sector);
  bench("triangle solid",        &ops::triangle);
  return 0;
}
