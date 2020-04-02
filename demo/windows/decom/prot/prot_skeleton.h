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
// \brief Skeleton protocol class
//
// This skeleton class can be used as startpoint for own protocol
// implementations.
// Any empty/unused function needn't be implemented, the base class
// function is used instead
///
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_PROT_SKELETON_H_
#define _DECOM_PROT_SKELETON_H_

#include "../prot.h"


/////////////////////////////////////////////////////////////////////

namespace decom {
namespace prot {


class skeleton : public protocol
{
public:
  /**
   * Protocol ctor
   * \param lower Lower layer
   * \param name Layer name
   */
  skeleton(layer* lower, const char* name = "prot_skeleton")
    : protocol(lower, name)    // it's VERY IMPORTANT to call the base class ctor HERE!!!
  {
    // initialize your stuff here
  }


  /**
   * dtor
   */
  virtual ~skeleton()
  {
    // close the layer gracefully BUT DO NOT CALL close() here, cause this
    // would close the lower layers which must remain open if layer is deleted
    // dynamically
  }


  /**
   * Called by upper layer to open this layer
   * \param address The address to open
   * \param id The endpoint identifier to open
   * \return true if open is successful
   */
  virtual bool open(const char* address = "", eid const& id = eid_any)
  {
    // for security: check that upper protocol/device exists
    if (!upper_) {
      return false;
    }

    // assign channel
    eid_ = id;

    // open the lower layer NOW - opening is done DOWN-TOP in layer stack
    // the lowest layer is opened first, the highest layer last
    // if opening the lower layer fails, report it to upper layer
    bool result = protocol::open(address, id);

    // open THIS layer HERE
    // if NOTHING must be done here, open() needn't be implemented - the base
    // class open() is used instead

    return result;
  }


  /**
   * Called by upper layer to close this layer
   * \param id The endpoint identifier to close
   */
  virtual void close(eid const& id = eid_any)
  {
    // FIRST close THIS layer HERE
    // multiple close requests must not be a problem
    // if NOTHING must be done here, close() needn't be implemented - the base
    // class close() is used instead

    // close the lower layer after closing THIS layer - closing is done TOP-DOWN in layer stack
    // the highest layer is closed first, the lowest layer last
    protocol::close(id);
  }


  /**
   * Called by upper layer to transmit data (message) to this protocol
   * \param data The message to send
   * \param id The endpoint identifier
   * \param more true if message is a fragment which is followed by another msg. False if no/last fragment
   * \return true if Send is successful
   */
  virtual bool send(msg& data, eid const& id = eid_any, bool more = false)
  {
    // do clothing of data here, add headers and trailings like CRCs etc.
    // ATTENTION: data may be modified on lower layer(s). So if data need to be
    // reused (retransmission etc.) make a copy of data here!
    // if NOTHING must be done here, send() needn't be implemented - the base
    // class send() is used instead

    // send() is MUST NOT block. Just cloth, process and/or store data here and send it
    // to the lower layer.
    // If layer is not open or ready (buffer in use, no okay indication form lower layer etc.)
    // return false - this is a hard error case - which is finally reported to app level
    // In all other cases return true.

    // finally send the message to lower layer and return result
    // ALTERNATIVELY you can call protocol::indication(tx_done, channel) to notify upper layer that data
    // is processed/stored and more data is accepted
    return protocol::send(data, id, more);
  }


  /**
   * Receive function for data from lower layer
   * \param data The message to receive
   * \param more true if message is a fragment which is followed by another msg. False if no/last fragment
   */
  virtual void receive(msg& data, eid const& id = eid_any, bool more = false)
  {
    // this function is called by the lower layer
    // checking and stripping of heading and trailing data is done here
    // if everything is correct, resulting data is passed to the upper layer
    // if NOTHING must be done here, receive() needn't be implemented - the base
    // class receive() is used instead

    // pass data to upper layer
    // how the upper layer deals with the data doesn't care this layer
    // anyway, data is changed on upper layers, so if it needs to be reused here, make a copy
    protocol::receive(data, id, more);
  }


  /**
   * Status indication from lower layer
   * \param code The status code which occurred on lower layer
   * \param id The endpoint identifier
   */
  virtual void indication(status_type code, eid const& id = eid_any)
  {
    // process status indication of the lower layer
    // if NOTHING must be done here, indication() needn't be implemented - the base
    // class indication() is used instead
    // DO NOT call send() from here, this might end in an send() - indication() - send() ... recursion!

    // if necessary or recover is not possible, inform the upper layer
    protocol::indication(code, id);
  }


  ////////////////////////////////////////////////////////////////////////
  // L A Y E R   A P I
  // goes here

private:
  eid eid_;
};

} // namespace prot
} // namespace decom

#endif // _DECOM_PROT_SKELETON_H_
