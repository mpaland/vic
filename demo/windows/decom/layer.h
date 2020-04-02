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
// \brief decom base class
// This class is the lowest base class of the decom library
// It represents and abstracts any layer of the OSI model.
// All other classes are derived from this class.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_LAYER_H_
#define _DECOM_LAYER_H_

#include <cstdint>
#include <cstddef>    // for size_t

// decom configuration file
#include "decom_cfg.h"

#include "msg.h"
#include "log.h"

// common utils
#include "util/util.h"
#include "util/event.h"

///////////////////////////////////////////////////////////////////////////////

// IMPL_HEADER macro for including platform specific implementations
// usage:
// #include IMPL_HEADER(util/timer.h)
#define STRINGIFY(x) #x
#define XSTRINGIFY(d) STRINGIFY(d)
#define PATH(header, platform) XSTRINGIFY(platform ## header)
#define IMPL(s) ./../impl/s/
#define IMPL_HEADER(header) PATH(header, IMPL(PLATFORM))


///////////////////////////////////////////////////////////////////////////////

namespace decom {

/**
 * endpoint identifier
 * An endpoint consists of a 16 byte address and a 32 bit port
 */
struct eid
{
  union addr_type {
    std::uint8_t  addr[16];
    std::uint32_t addr32[4];
    std::uint64_t addr64[2];
  };


  // ctor
  eid()
  {
    addr_.addr64[0] = addr_.addr64[1] = 0U;
    port_                             = 0U;
  }


  // port ctor
  eid(std::uint32_t port)
    : port_(port)
  {
    addr_.addr64[0] = addr_.addr64[1] = 0U;
  }


  // addr access
  inline addr_type const& addr() const { return addr_; }
  inline addr_type& addr() { return addr_; }


  // port access
  inline std::uint32_t port() const { return port_; }
  inline void port(std::uint32_t val) { port_ = val; }


  // is_any test
  inline bool is_any() const
  { return (port_ == 0U) && (addr_.addr64[0] == 0U) && (addr_.addr64[1] == 0U); }


  // assignment operator
  eid& operator=(eid const& id)
  {
    addr_ = id.addr();
    port_ = id.port();
    return *this;
  }


  // comparisons
  inline bool operator==(eid const& other) const
  {
    return (port_ == other.port())                &&
      (addr_.addr64[0] == other.addr().addr64[0]) &&
      (addr_.addr64[1] == other.addr().addr64[1]);
  }


  inline bool operator!=(eid const& other) const
  { return !(*this == other); }


  inline bool operator<(eid const& other) const
  {
    return (port_ < other.port())                 &&
      ((addr_.addr64[1] < other.addr().addr64[1]) ||
       (addr_.addr64[1] == other.addr().addr64[1] && addr_.addr64[0] < other.addr().addr64[0])
      );
  }


private:
  addr_type     addr_;
  std::uint32_t port_;
};


/**
 * Static definition of eid_any - used when the eid doesn't care
 */
static const eid eid_any;


// decom base class
// This class abstracts a (any) layer of the OSI model
class layer
{
protected:
  /**
   * no default ctor
   */
  layer()
  { }


public:
  /**
   * Layer name - define this for every layer in according ctor
   */
  const char* name_;


  /**
   * Communicator base class ctor - only called by communicator derived classes,
   * cause they don't have a lower layer
   */
  layer(const char* name)
    : lower_(nullptr)
    , upper_(nullptr)
    , name_(name)
  {
    // clear/init statistics
    stats_clear();
  }


  /**
   * Protocol/device base class ctor
   * This ctor binds/inserts the layer in the stack dynamically so that
   * additional protocols/devices can be added at runtime
   * \param lower Lower layer
   * \param name Layer name
   */
  layer(layer* lower, const char* name)
    : lower_(lower)
    , upper_(lower->upper_)
    , name_(name)
  {
    // insert this layer between lower and upper layer
    if (lower->upper_) {
      lower->upper_->lower_ = this;
    }
    lower->upper_ = this;

    // notify lower layer about registration of its new upper layer
    lower->upper_registered();

    // clear/init statistics
    stats_clear();
  }


  /**
   * dtor
   * The dtor unbinds the layer so that a layer can be dynamically removed from the stack
   */
  virtual ~layer()
  {
    // unbind layer
    if (lower_) {
      lower_->upper_ = upper_;
    }
    if (upper_) {
      upper_->lower_ = lower_;
    }
  }


  /**
   * Called by upper layer to open this layer
   * \param address The address to open
   * \param id The endpoint identifier
   * \return true if open is successful
   */
  virtual bool open(const char* address = "", eid const& id = eid_any)
  { return lower_->open(address, id); }


  /**
   * Called by upper layer to close this layer
   * \param id The endpoint identifier
   */
  virtual void close(eid const& id = eid_any)
  { lower_->close(id); }


  /**
   * Called by upper layer to transmit data (message) to lower layer
   * ATTENTION: call by ref - so data may be modified on lower layer!
   * \param data The message to send
   * \param id The endpoint identifier
   * \param more true if message is a fragment which is followed by another msg. False if no/last fragment
   *        |   1   |   2   |   3   |         |   1   |   2   |   3   |
   *          more    more    last              more    more    last
   * \return true if Send is successful
   */
  virtual bool send(msg& data, eid const& id = eid_any, bool more = false)
  {
    if (lower_->send(data, id, more)) {
      stats_out(data);
      return true;
    }
    else {
      stats_error_out();
      return false;
    }
  }


  /**
   * Receive function for data from lower layer
   * ATTENTION: call by ref - so data may be modified on upper layer!
   * \param data The message to receive
   * \param id The endpoint identifier
   * \param more true if message is a fragment which is followed by another msg. False if no/last fragment
   */
  virtual void receive(msg& data, eid const& id = eid_any, bool more = false)
  {
    stats_in(data);
    upper_->receive(data, id, more);
  }


  // Status indication codes from lower to upper layer
  typedef enum tag_status_type {
    connected,      // connection established, layer can be used for communication now
    disconnected,   // interface, line, post etc. gone or connection ended
    tx_done,        // data completely transmitted, next data is acceptable by lower layer
    tx_error,       // unrecoverable transmission error, transmission is aborted
    tx_timeout,     // timeout error, upper layer may resend last data
    tx_overrun,     // transmitter overrun error - should not happen, upper layer (application) must take care
    rx_error,       // receive error
    rx_timeout,     // timeout error
    rx_overrun      // receiver overrun error
  } status_type;


  /**
   * Status/Error indication from lower layer
   * \param code The status code which occurred on lower layer
   * \param id The endpoint identifier
   */
  virtual void indication(status_type code, eid const& id = eid_any)
  { if (upper_) upper_->indication(code, id); }


  /**
   * Called during stack assembly after the registration of the upper layer
   * May be used to do additional initialization when the upper layer is known
   */
  virtual void upper_registered()
  { }


protected:
  layer* lower_;   // pointer to lower layer
  layer* upper_;   // pointer to upper layer


  // layer statistics
#ifdef DECOM_STATS
public:
  typedef struct tag_statistic_type {
    std::uint64_t bytes_in;
    std::uint64_t bytes_out;
    std::uint32_t packets_in;
    std::uint32_t packets_out;
    std::uint32_t errors_in;
    std::uint32_t errors_out;
  } statistic_type;

  statistic_type statistic_;

protected:
  // enable statistics
  inline void stats_in (msg const& data) { statistic_.bytes_in  += data.size(); statistic_.packets_in++;  }
  inline void stats_out(msg const& data) { statistic_.bytes_out += data.size(); statistic_.packets_out++; }
  inline void stats_error_in()  { statistic_.errors_in++;  }
  inline void stats_error_out() { statistic_.errors_out++; }
  inline void stats_clear() { statistic_.bytes_in = statistic_.bytes_out = statistic_.packets_in = statistic_.packets_out = statistic_.errors_in = statistic_.errors_out = 0U; }
#else
  // disable statistics
  inline void stats_in (msg const&) { }
  inline void stats_out(msg const&) { }
  inline void stats_error_in()  { }
  inline void stats_error_out() { }
  inline void stats_clear() { }
#endif // DECOM_STATS
};

} // namespace decom

#endif // _DECOM_LAYER_H_
