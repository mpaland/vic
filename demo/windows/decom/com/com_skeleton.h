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
// \brief Skeleton communication class
//
// This skeleton class can be used as startup for own communication
// implementations which represent the lowest layer in the stack.
// Below this layer is the hardware or an OS HAL/API.
// Communicators are the lowest level.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_COM_SKELETON_H_
#define _DECOM_COM_SKELETON_H_

#include "../com.h"

/////////////////////////////////////////////////////////////////////

namespace decom {
namespace com {

class skeleton : public communicator
{
public:
  /**
   * Normal communicator ctor
   * \param name Layer name
   */
  skeleton(const char* name = "com_skeleton")
    : communicator(name)   // it's VERY IMPORTANT to call the base class ctor HERE!!!
  {
    // initialize your stuff here
  }


  /**
   * dtor
   */
  virtual ~skeleton()
  {
    // close the layer gracefully - you should/can call close() here because
    // it's the lowest layer
    close();
  }


  /**
   * Called by upper layer to open this layer/channel
   * \param address The address to open
   * \param id The endpoint identifier to open
   * \return true if open is successful
   */
  virtual bool open(const char* address = "", eid const& id = eid_any)
  {
    (void)address;    // just to avoid compiler warning - do something useful with it

    // for security: check that upper protocol/device exists
    if (!upper_) {
      return false;
    }

    // open this layer here - open() MUST be implemented here

    // send notification to upper layers when com is connected, this may be implemented in a thread as well
    communicator::indication(connected, id);

    return true;
  }


  /**
   * Called by upper layer to close this layer/channel
   * \param id The endpoint identifier to open
   */
  virtual void close(eid const& id = eid_any)
  {
    // close this layer here - close() MUST be implemented here
    // multiple close requests must not be a problem

    // send notification to upper layers that com is disconnected, this may be implemented in a thread as well
    communicator::indication(disconnected, id);
  }


  /**
   * Called by upper layer to transmit data to this communication endpoint / physical layer
   * \param data The message to send
   * \param id The endpoint identifier
   * \param more true if message is a fragment - mostly unused on this layer
   * \return true if Send is successful
   */
  virtual bool send(msg& data, eid const& id = eid_any, bool more = false)
  {
    // transmit data to physical layer and return true if this was successful
    // send() must be implemented
    (void)data; (void)id; (void)more;   // just to avoid compiler warning - do something useful with it

    // DON'T FORGET to call communicator::indication(tx_done, id) to notify the upper layer when data
    // is sent - THIS IS MANDATORY!!!
    return true;
  }


private:
  /**
   * Receive function for data from physical layer 
   */
  void receive()
  {
    // this function is called by an interrupt, a system call or out of a polling/waiting receive/worker thread
    // if data is available put it in a msg and pass it to the upper layer
    msg data;
    eid id;

    // how the upper layer deals with the data doesn't care this layer
    communicator::receive(data, id);

    // if there are any errors on THIS layer, inform the upper layer
#if (0)   // may be implemented in real layers like this
    communicator::indication(rx_error, id);
#endif
  }


  ////////////////////////////////////////////////////////////////////////
  // C O M M U N I C A T O R   A P I

  // additional communicator functions
public:

};

} // namespace com
} // namespace decom

#endif    // _DECOM_COM_SKELETON_H_
