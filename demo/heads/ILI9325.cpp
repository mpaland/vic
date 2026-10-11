///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief ILI9325 TFT demo, 240 x 320 module on an 8 bit parallel bus (memory mapped)
// io layer: in memory mode the head writes the index and data registers directly
// through io::mem::write(), io::init() only sets up the bus and the reset. The SPI
// variant (Interface_Mode 0) uses io::write() option 0 = index, 1 = data.
//
///////////////////////////////////////////////////////////////////////////////

#include "head/ILI9325.h"
#include "scene.h"


// the two "registers" of the 8 bit bus, on a real target these are the external bus addresses
static std::uint8_t reg_index = 0U;
static std::uint8_t reg_data  = 0U;

static vic::head::ILI9325<240, 320, vic::drv::orientation_0, 8U> tft(reinterpret_cast<vic::io::handle_type>(0), &reg_index, &reg_data, false);


int main()
{
  demo_banner(tft, "ILI9325 2.8\" 8 bit bus, portrait (memory writes are not traced)");
  tft.init();
  demo_scene(tft);
  return 0;
}
