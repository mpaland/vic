///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2011-2017, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// This file is part of the decom library.
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
// \brief Bridge device
//
// The bridge device is kind of a loopback device to link two protocols at
// the TOP of two stacks together.
// It may be used to build gateways or protocol bridges.
// Both dev_bridge devices are constructed as normal devices and MUST be linked
// together via register_bridge() function vice versa.
//
//                     Send
//       bridge1      <----->     bridge2
//       prot2                    prot4
//       prot1                    prot3
//       com1                     com2
//
//       decom::com::com1 com1()
//       decom::prot::prot1 prot1(&com1)
//       decom::prot::prot2 prot2(&prot1)
//       decom::dev::bridge bridge1(&prot2)
//
//       decom::com::com2()
//       decom::prot::prot3(&com2)
//       decom::prot::prot4(&prot3)
//       decom::dev::bridge bridge2(&prot4)
//
//       bridge1.register_bridge(&bridge2)
//       bridge2.register_bridge(&bridge1)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_DEV_BRIDGE_H_
#define _DECOM_DEV_BRIDGE_H_

#include "../dev.h"

/////////////////////////////////////////////////////////////////////

namespace decom {
namespace dev {


class bridge : public device
{
public:
  /**
   * Normal device ctor
   * \param lower Lower layer
   */
  bridge(layer* lower)
    : device(lower, "dev_bridge")  // it's VERY IMPORTANT to call the base class ctor HERE!!!
    , counter_bridge_(nullptr)
  { }


  /**
   * dtor
   */
  virtual ~bridge()
  {
    close();
  }


  /**
   * Called by the application to open both stacks
   * \param address The address to open
   * \param id The endpoint identifier to open
   * \return true if open is successful
   */
  virtual bool open(const char* address = "", eid const& id = eid_any)
  {
    if (!counter_bridge_) {
      // fatal error - counter bridge is not registered
      DECOM_LOG_EMERG("Counter bridge is not registered");
      return false;
    }

    return device::open(address, id);   // open this stack
  }


  /**
   * Called by the application to close stack
   * \param id The endpoint identifier to close
   */
  virtual void close(eid const& id = eid_any)
  {
    // FIRST close the lower layer then close this layer - closing is done down-top in layer stack
    device::close(id);                  // close this stack
  }


  /**
   * Receive function for data from lower layer
   * \param data The message to receive
   * \param id The endpoint identifier
   * \param more more flag
   */
  virtual void receive(msg& data, eid const& id = eid_any, bool more = false)
  {
    // pass the data to the other stack / counter bridge part
    if (counter_bridge_) {
      (void)counter_bridge_->device::send(data, id, more);
    }
  }


  /**
   * Indication from lower layer - unused here
   */
  virtual void indication(status_type, eid const&)
  { }


  /**
   * A complete bridge device consists of two bridges, one for each stack.
   * The bridges MUST be registered one another before usage
   * \param counter_bridge The counter bridge device
   */
  void register_bridge(bridge* counter_bridge)
  { counter_bridge_ = counter_bridge; }


private:
  bridge* counter_bridge_;      // pointer to the counter bridge part
};

} // namespace dev
} // namespace decom

#endif // _DECOM_DEV_BRIDGE_H_
