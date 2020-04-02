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
// \brief File device class
//
// This class is used to send and receive files to and from the stack
// If the file is too big to be send in one message (which is mostly the case)
// segmentation can be used
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_DEV_FILE_H_
#define _DECOM_DEV_FILE_H_

#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>

#include "../dev.h"

/////////////////////////////////////////////////////////////////////

namespace decom {
namespace dev {


class file : public device
{
public:
  /**
   * Device ctor
   * \param lower Lower layer
   */
  file(layer* lower, const char* name = "dev_file")
    : device(lower, name)    // it's VERY IMPORTANT to call the base class ctor HERE!!!
  {
    // init
    tx_filesize_ = 0U;
    tx_filesent_ = 0U;
    tx_msgsize_  = 0U;
    tx_lineend_  = false;
    tx_err_      = false;
    rx_ignore_more_ = false;
    worker_end_  = false;
  }


  /**
   * dtor
   */
  virtual ~file()
  {
    // call close here to make sure the stack is closed before deleting device
    close();
  }


  /**
   * Called by the application to open this stack. Device is the highest layer
   * \param address The address to open
   * \param id The endpoint identifier, normally not used in open()
   * \return true if open is successful
   */
  virtual bool open(const char* address = "", eid const& id = eid_any)
  {
    // FIRST open the lower layer - opening is done DOWN-TOP in layer stack
    bool result = device::open(address, id);
    tx_eid_ = id;

    // start write worker thread
    worker_end_ = false;
    std::thread t(worker, this);
    thread_.swap(t);

    return result;
  }


  /**
   * Called by the application to close stack
   */
  virtual void close(eid const& id = eid_any)
  {
    // close files
    if (tx_file_.is_open()) {
      tx_file_.close();
    }
    if (rx_file_.is_open()) {
      rx_file_.close();
    }

    // stop worker thread
    worker_end_ = true;
    tx_ev_.set();
    thread_.join();

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
    if (rx_file_.is_open()) {
      std::ostream_iterator<std::uint8_t> it(rx_file_);
      std::copy(data.begin(), data.end(), it);
    }

    if (!rx_ignore_more_ && !more) {
      // no more packets, close file
      read_end();
    }
  }


  /**
   * Indication from lower layer
   * \param code The error code which occurred on lower layer
   * \param id The endpoint identifier
   */
  virtual void indication(status_type code, eid const& id = eid_any)
  {
    if (tx_file_.is_open() && code == tx_done) {
      // message sent, process next segment
      tx_ev_.set();
    }
    else {
      // transmission error - abort file
      tx_err_ = true;
      tx_file_.close();
    }
  }


  static void file::worker(void* arg)
  {
    file* f = static_cast<file*>(arg);

    do {
      f->tx_ev_.wait();
      f->tx_ev_.reset();
      if (f->tx_file_.is_open()) {
        // send next segment
        f->send_segment();
      }
    } while (!f->worker_end_);
  }


  ////////////////////////////////////////////////////////////////////////
  // D E V I C E   A P I

  /**
   * Open a file and send it to the lower layer
   * \param filename The filename of the file to send to the lower layer
   * \param msgsize The maximum size of one message to the lower layer
   * \param lineend True for detection of '\n' (0x0A) line endings to segment messages
   * \return true if successful
   */
  bool write(const char* filename, std::uint32_t msgsize, bool lineend = false)
  {
    // setup file and message size
    tx_msgsize_ = msgsize;
    tx_lineend_ = lineend;
    tx_filesent_ = std::ios::beg;
    tx_err_ = false;

    tx_file_.open(filename, std::ios::in | std::ios::binary);
    tx_file_ >> std::noskipws;
    if (!tx_file_.is_open()) {
      // error to open file
      return false;
    }

    // get file size
    tx_file_.seekg(0, std::ios::end);
    tx_filesize_ = tx_file_.tellg();
    tx_file_.seekg(0, std::ios::beg);

    if (!tx_filesize_) {
      // zero filesize
      tx_file_.close();
      return false;
    }

    // send first segment
    return send_segment();
  }


  /**
   * write_progress() returns the progress of the write operation
   * \return value of 0 - 100%, representing percentage of transmitted file, 0xFF if file is closed or not available
   */
  std::uint8_t write_progress() const
  {
    return tx_err_ || !tx_filesize_ ? 0xFFU : static_cast<std::uint8_t>(tx_filesent_ * 100U / tx_filesize_);
  }


  /**
   * Retrieve data from the stack and write it to a file
   * Data is written until layer is closed or read_end() is called
   * \param filename The filename of the file to create or to append data
   * \param append True to append data, false to overwrite file
   * \param ignore_more True to ignore 'more' flag in receiving messages
   * \return true if Read was successful, false in case of timeout or error
   */
  bool read(const char* filename, bool append = false, bool ignore_more = false)
  {
    rx_ignore_more_ = ignore_more;

    // open receive file
    rx_file_.open(filename, std::ios::out | std::ios::binary | (append ? std::ios::app : std::ios::trunc));

    return rx_file_.is_open();
  }


  /**
   * End an read operation. According file is closed.
   * \return true if successful
   */
  bool read_end()
  {
    if (rx_file_.is_open()) {
      rx_file_.close();
      return true;
    }

    return false;
  }


private:

  // helper function to send segments
  bool send_segment()
  {
    if (tx_file_.eof() || !tx_file_.is_open() || !tx_filesize_) {
      // file is closed, bad or end of file reached - anyway, close file and return error
      tx_file_.close();
      return false;
    }

    // read buffer
    std::size_t msgsize = std::min<std::size_t>(static_cast<std::size_t>(tx_filesize_), tx_msgsize_);
    std::vector<std::uint8_t> buffer;
    std::uint8_t c = 0;
    for (std::size_t i = 0; i++ < msgsize && (!tx_lineend_ || c != 0x0AU); ) {
      c = static_cast<std::uint8_t>(tx_file_.get());
      buffer.push_back(c);
    }

    // check if file is complete (EOF not set here)
    tx_filesent_ = tx_file_.tellg();
    bool more = tx_filesent_ != tx_filesize_;
    if (!more) {
      // yes, file is complete
      tx_file_.close();
    }

    if (!buffer.empty()) {
      msg data(buffer.begin(), buffer.end());

      // send it to lower layer
      if (!device::send(data, tx_eid_, more)) {   // if not EOF, set more flag
        // error abort - close file
        tx_err_ = true;
        tx_file_.close();
        return false;
      }
    }

    return true;
  }


  std::thread     thread_;          // worker thread object
  volatile bool   worker_end_;      // end worker thread
  decom::util::event tx_ev_;        // transmit done indication
  std::ofstream   rx_file_;         // receive file
  bool            rx_ignore_more_;  // true to ignore more flag during reception
  std::ifstream   tx_file_;         // transmit file
  decom::eid      tx_eid_;          // tx eid
  std::size_t     tx_msgsize_;      // maximum size of a message segment
  std::streamoff  tx_filesize_;     // size of transmit file
  std::streamoff  tx_filesent_;     // size of transmit file sent
  bool            tx_lineend_;      // use line ending to segment messages
  bool            tx_err_;          // transmission error on lower layer
};

} // namespace dev
} // namespace decom

#endif // _DECOM_DEV_FILE_H_
