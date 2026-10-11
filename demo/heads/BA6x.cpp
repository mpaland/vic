///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2026, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT), see io_trace.cpp
//
// \brief Wincor BA63 customer display demo (2 x 20 characters, USB or RS232)
// io layer: io::write() sends the framed command (02h 00h length data), io::read()
// returns the 4 byte status answer (04h ...). demo/BA63-single.cpp shows a real USB
// HID io layer with decom.
//
///////////////////////////////////////////////////////////////////////////////

#include "head/BA6x.h"
#include "scene.h"


static vic::head::BA6x<vic::head::BA63> display(reinterpret_cast<vic::io::handle_type>(0));


int main()
{
  demo_banner(display, "BA63 2 x 20");
  display.init();
  demo_text(display);
  return 0;
}
