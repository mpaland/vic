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
// \brief Hub class
//
// This hub is a message distribution/routing layer - one lower layer is distributed to multiple
// upper layers according to channel value.
// This is useful if multiple upper protocols / devices share one lower layer protocol / com interface
// like in TCP/IP stacks where the lower layer is IP and the upper layers are UDP, TCP, ICMP etc.
//
//         upper1   upper2   upper3
//           |        |        |
//         ------------------------
//         |       prot_hub       |
//         ------------------------
//                    |
//                  lower
//
//
// Init is done like this:
// prot_ppp ppp(&com);
// prot_hub hub(&ppp);
// prot_ipv4 upper1(&hub);
// prot_ipv6 upper2(&hub);
// prot_ipcp upper3(&hub);
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_PROT_HUB_H_
#define _DECOM_PROT_HUB_H_

#include <vector>
#include <mutex>

#include "../prot.h"


/////////////////////////////////////////////////////////////////////

namespace decom {
namespace prot {


class hub : public protocol
{
public:
  /**
   * Normal protocol ctor
   * \param lower Lower layer
   */
  hub(layer* lower, const char* name = "prot_hub")
    : protocol(lower, name)   // it's VERY IMPORTANT to call the base class ctor HERE!!!
  {
    upper_layers_.clear();    // just to be sure
  }


  /**
   * Called during stack assembly after the registration of upper layer
   */
  virtual void upper_registered()
  {
    // save upper layer, because multiple layers register
    upper_layers_type u = { upper_, eid(), true };
    upper_layers_.push_back(u);
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
    // make sure, that only one upper layer is sending at a time
    std::lock_guard<std::mutex> lock(m_);
    return protocol::send(data, id, more);
  }


  /**
   * Receive function for data from lower layer
   * \param data The message to receive
   * \param id The endpoint identifier
   * \param more true if message is a fragment which is followed by another msg. False if no/last fragment
   */
  virtual void receive(msg& data, eid const& id = eid_any, bool more = false)
  {
    // get the count of upper layers which match the eid
    // if the msg is distributed to more than one upper layer, make a real copy of data because any upper layer may strip/modify data
    std::vector<upper_layers_type>::iterator it, it1;
    std::uint32_t cnt = 0U;
    for (it = upper_layers_.begin(); it != upper_layers_.end(); ++it) {
      if ((it->eid == id || it->eid.is_any()) && it->include) {
        it1 = it;
        if (++cnt >= 2U) {
          break;
        }
      }
    }

    if (cnt < 2U) {
      // only one upper layer with according eid
      it1->layer->receive(data, id, more);
      return;
    }
    else {
      // two or more upper layers with according eid
      msg org = data;   // make a real copy of data because any upper layer may strip/modify data
      for (it = upper_layers_.begin(); it != upper_layers_.end(); ++it) {
        if ((it->eid == id || it->eid.is_any()) && it->include) {
          if (it != upper_layers_.begin()) {
            // restore data and pass fresh copy to upper layer
            data = org;
          }
          // distribute data to all upper layers which have a generic (-1) or a matching channel ID
          it->layer->receive(data, id, more);
        }
      }
    }
  }


  /**
   * Error indication from lower layer
   * \param code The error code which occurred on lower layer
   * \param id The endpoint identifier
   */
  virtual void indication(status_type code, eid const& id = eid_any)
  {
    // distribute indication to all upper layers which have a matching eid
    for (std::vector<upper_layers_type>::iterator it = upper_layers_.begin(); it != upper_layers_.end(); ++it) {
      if ((it->eid == id || it->eid.is_any()) && it->include) {
        it->layer->indication(code, id);
      }
    }
  }


  ////////////////////////////////////////////////////////////////////////
  // L A Y E R   A P I

  /**
   * Set a specific eid of the according upper layer, the layer will only receive matching eid messages.
   * As default, any upper layer receives all eids (eid_any).
   * \param layer A valid upper registered layer
   * \param id The channel ID
   * \param include true to include the eid, false to exclude the eid (e.g. all eids except this one)
   * \return true if channel was added successfully
   */
  bool set_channel(layer* upper_layer, eid const& id, bool include = true)
  {
    // find the given layer
    for (std::vector<upper_layers_type>::iterator it = upper_layers_.begin(); it != upper_layers_.end(); ++it) {
      if (it->layer == upper_layer && it->eid.is_any() && include) {
        // ANY_LAYERS found, replace ANY_LAYERS
        it->eid = id;
        it->include = include;
        return true;
      }
    }
    // not found, add new channel info
    upper_layers_type u = { upper_layer, id, include };
    upper_layers_.push_back(u);

    // upper layer not found
    return false;
  }


  ////////////////////////////////////////////////////////////////////////

private:
  typedef struct struct_upper_layers_type
  {
    layer* layer;
    eid    eid;
    bool   include;
  } upper_layers_type;

  std::vector<upper_layers_type> upper_layers_;
  std::mutex m_;
};

} // namespace prot
} // namespace decom

#endif _DECOM_PROT_HUB_H_
