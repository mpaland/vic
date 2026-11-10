///////////////////////////////////////////////////////////////////////////////
// \brief Consumer of the installed vic package: draws on the memory head
///////////////////////////////////////////////////////////////////////////////

#include "dc.h"
#include "head/memory.h"

int main()
{
  vic::head::memory<16, 8> head;
  vic::dc dc(head);
  dc.set_color(vic::color::white);
  dc.line({ 1, 1 }, { 14, 6 });
  return (head.pixel({ 1, 1 }) == vic::color::white) && (head.pixel({ 14, 6 }) == vic::color::white) ? 0 : 1;
}
