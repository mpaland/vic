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
// \brief Skeleton device class
//
// This skeleton class can be used as startpoint for own device
// implementations.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_DEV_SKELETON_H_
#define _DECOM_DEV_SKELETON_H_

#include "../dev.h"


/////////////////////////////////////////////////////////////////////

namespace decom {
namespace dev {


class skeleton : public device
{
public:
  /**
   * Device ctor
   * \param lower Lower layer
   */
  skeleton(decom::layer* lower, const char* name = "dev_skeleton")
    : device(lower, name)  // it's VERY IMPORTANT to call the base class ctor HERE!!!
  {
    // initialize your stuff here
  }


  /**
   * dtor
   */
  virtual ~skeleton()
  {
    // call close() here to make sure the stack is closed before deleting device
    close();

    // unregister all used stuff in this layer here
  }


  /**
   * Called by the application to open this stack. Device is the highest layer
   * \param address The address to open
   * \param id The endpoint identifier to open
   * \return true if open is successful
   */
  virtual bool open(const char* address = "", eid const& id = eid_any)
  {
    // FIRST open the lower layer - opening is done DOWN-TOP in layer stack
    // if opening the lower layer fails, report it
    bool result = device::open(address, id);

    // open THIS layer here
    // if NOTHING must be done here, open() needn't be implemented - the base
    // class open() is used instead

    return result;
  }


  /**
   * Called by the application to close stack
   * close() can be omitted here if nothing special is done in device, base clase close is used then
   * \param id The endpoint identifier to close
   */
  virtual void close(eid const& id = eid_any)
  {
    // FIRST close this layer here
    // multiple close requests must not be a problem
    // if NOTHING must be done here, close() needn't be implemented - the base
    // class close() is used instead

    // Close the lower layer after closing THIS layer - closing is done TOP-DOWN
    // in layer stack
    device::close(id);
  }


  /**
   * Receive function for data from lower layer
   * \param data The message to receive
   * \param id The endpoint identifier
   * \param more true if message is a fragment which is followed by another msg. False if no/last fragment
   */
  virtual void receive(msg& data, eid const& id = eid_any, bool more = false)
  {
    // this function is called by the lower layer
    // process data here and store it somewhere
    (void)data; (void)id; (void)more;   // just to avoid compiler warning - do something useful with it
  }


  /**
   * Status/Error indication from lower layer
   * \param code The status code which occurred on lower layer
   * \param id The endpoint identifier
   */
  virtual void indication(status_type code, eid const& id = eid_any)
  {
    // an error occurred on the lower layer
    // process and recover this error here
    (void)code; (void)id;   // just to avoid compiler warning - do something useful with it
  }


  ////////////////////////////////////////////////////////////////////////
  // D E V I C E   A P I

  /**
   * The device should provide a write() function to send data from the
   * application to the stack - data is 'call-by-value' in generic device because data
   * is changed on lower layers
   * \param data Data (message) to send to the device
   * \return true if write() was successful
   */
  bool write(msg data)
  {
    // do something with data here (e.g. build commands)

    // send it to lower layer
    return device::send(data);
  }


  /**
   * The device should provide a read() function to retrieve data from the stack
   * \param data The data (message) received from the stack
   * \param timeout Wait for [ms] to get data
   * \return true if read() was successful, false in case of timeout or error
   */
  bool read(decom::msg& data, std::chrono::milliseconds timeout)
  {
    // check if data is available and return it
    // if no data is available wait until timeout elapsed or error indication is sent
    (void)data; (void)timeout;  // just to avoid compiler warning - do something useful with it

    // return false in case of error or timeout
    return true;
  }


  ////////////////////////////////////////////////////////////////////////
  // L A Y E R   A P I
  // goes here
};

} // namespace dev
} // namespace decom

#endif //_DECOM_DEV_SKELETON_H_
