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
// \brief Serial COM port communication class
//
// This class abstracts the Windows serial COM port.
// It should be as close as possible to the MS standard implementation of
// overlapped IO handling.
// All kind of serial ports (RS232, USB, virtual etc.) are supported.
// A worker thread for overlapped/asynchronous communication is used.
//
// \todo Add functions for interbyte timing etc.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_COM_SERIAL_H_
#define _DECOM_COM_SERIAL_H_

#include <windows.h>
#include <process.h>
#include <vector>
#include <sstream>

#include "../../../com.h"

/////////////////////////////////////////////////////////////////////

// defines the receive buffer size, but the value is likely arbitrary
#ifndef DECOM_COM_SERIAL_RX_BUFSIZE
#define DECOM_COM_SERIAL_RX_BUFSIZE   32768U
#endif

/////////////////////////////////////////////////////////////////////


namespace decom {
namespace com {


class serial : public communicator
{
public:
  // params
  typedef enum tag_stopbit_type {
    stopbit_0 = 0,              // 0   stopbits (uncommon)
    stopbit_05,                 // 0.5 stopbits
    stopbit_1,                  // 1   stopbit
    stopbit_15,                 // 1.5 stopbits
    stopbit_2                   // 2   stopbits
  } stopbit_type;

  typedef enum tag_parity_type {
    parity_none = 0,            // no parity
    parity_odd,                 // odd parity bit
    parity_even,                // even parity bit
    parity_mark,                // mark parity bit
    parity_space                // space parity bit
  } parity_type;

  typedef enum tag_flowctrl_type {
    flowctrl_none = 0,          // no flow control
    flowctrl_rts_cts,           // RTS/CTS flow control
    flowctrl_dtr_dsr,           // DTR/DSR flow control
    flowctrl_xon_xoff           // XON/XOFF flow control
  } flowctrl_type;

  
public:
  /**
   * Param ctor, sets the default port parameters
   * \param baudrate The baudrate given in [baud]
   * \param databits Number of databits, normally and default is 8
   * \param parity Parity, see defines
   * \param stopbits Number of stopbits, see defines
   * \param handshake Handshake options, see defines
   * \param name Layer name
   */
  serial(std::uint32_t baudrate,
         std::uint8_t  databits  = 8U,
         parity_type   parity    = parity_none,
         stopbit_type  stopbits  = stopbit_1,
         flowctrl_type handshake = flowctrl_none,
         const char* name        = "com_serial"
        )
    : communicator(name)   // it's VERY IMPORTANT to call the base class ctor HERE!!!
    , port_(0U)
    , baudrate_(baudrate)
    , databits_(databits)
    , parity_(parity)
    , stopbits_(stopbits)
    , handshake_(handshake)
    , com_handle_(INVALID_HANDLE_VALUE)
    , thread_handle_(INVALID_HANDLE_VALUE)
    , tx_busy_(false)
    , rx_busy_(false)
  {
    // create events
    events_[ev_terminate] = ::CreateEvent(NULL, TRUE, FALSE, NULL);    // manual reset event
    events_[ev_transmit]  = ::CreateEvent(NULL, TRUE, FALSE, NULL);    // manual reset event
    events_[ev_receive]   = ::CreateEvent(NULL, TRUE, FALSE, NULL);    // manual reset event
  }


  /**
   * default dtor
   */
  ~serial()
  {
    close();

    // close handles
    (void)::CloseHandle(events_[ev_terminate]);
    (void)::CloseHandle(events_[ev_transmit]);
    (void)::CloseHandle(events_[ev_receive]);
  }


  /**
   * Called by upper layer to open this layer
   * \param address The port to open, allowed are 'COM1', 'COM2' etc.
   * \param id Unused
   * \return true if open is successful
   */
  virtual bool open(const char* address = "", eid const& id = eid_any)
  {
    (void)id;   // unused

    // check that upper protocol/device exists
    if (!upper_) {
      return false;
    }

    // assemble port address
    if (!address || !(*address)) {
      // invalid address
      return false;
    }
    std::stringstream port;
    port << "\\\\.\\" << address;   // extend to complete port address (like '\\.\COM1')

    close();                        // just in case layer was already open

    // call CreateFile to open the COM port
    com_handle_ = ::CreateFileA(
      port.str().c_str(),
      GENERIC_READ | GENERIC_WRITE,
      0U,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_OVERLAPPED,
      NULL
    );
    if (com_handle_ == INVALID_HANDLE_VALUE) {
      DECOM_LOG_ERROR("Error opening port " << port.str().c_str());
      return false;
    }

    DECOM_LOG_INFO("Opened serial port ") << address;

    // set params
    if (!set_param(baudrate_, databits_, parity_, stopbits_, handshake_)) {
      // error in set params
      DECOM_LOG_ERROR("Error setting params");
      close();
      return false;
    }

    // set timeouts
    COMMTIMEOUTS cto;
    cto.ReadIntervalTimeout         = MAXDWORD;
    cto.ReadTotalTimeoutMultiplier  = MAXDWORD;
    cto.ReadTotalTimeoutConstant    = MAXDWORD - 1;
    cto.WriteTotalTimeoutMultiplier = 0U;
    cto.WriteTotalTimeoutConstant   = 0U;
    if (!::SetCommTimeouts(com_handle_, &cto)) {
      DECOM_LOG_ERROR("Error setting timeouts");
      // error in set timeouts
      close();
      return false;
    }

    tx_busy_ = false;

    ///////////////////// create receive thread ////////////////////////////

    thread_handle_ = reinterpret_cast<HANDLE>(::_beginthreadex(
      NULL,
      0U,
      reinterpret_cast<unsigned(__stdcall *)(void*)>(&worker_thread),
      reinterpret_cast<void*>(this),
      CREATE_SUSPENDED,
      NULL
    ));
    if (!thread_handle_)
    {
      // error to create thread - close all
      DECOM_LOG_ERROR("Error creating thread");
      close();
      return false;
    }

    // run at higher priority for short latency time
    (void)::SetThreadPriority(thread_handle_, THREAD_PRIORITY_HIGHEST);
    (void)::ResumeThread(thread_handle_);

    // send port open indication
    communicator::indication(connected);

    return true;
  }


  /**
   * Called by upper layer to close this layer
   * \param id Unused
   */
  virtual void close(eid const& id = eid_any)
  {
    (void)id;   // unused

    // stop the receive thread
    if (thread_handle_ != INVALID_HANDLE_VALUE)
    {
      (void)::SetEvent(events_[ev_terminate]);
      (void)::WaitForSingleObject(thread_handle_, 3000U);   // wait 3 sec for thread termination
      (void)::CloseHandle(thread_handle_);
      thread_handle_ = INVALID_HANDLE_VALUE;
    }

    // close the comm port
    if (com_handle_ != INVALID_HANDLE_VALUE) {
      (void)purge();
      // final close
      (void)::CloseHandle(com_handle_);
      com_handle_ = INVALID_HANDLE_VALUE;

      DECOM_LOG_INFO("Closed serial port");

      // port closed indication
      communicator::indication(disconnected);
    }
  }


  /**
   * Called by upper layer to send data to COM port
   * \param data The message to send
   * \param id The endpoint identifier, unused here
   * \param more unused
   * \return true if Send is successful
   */
  virtual bool send(msg& data, eid const& id = eid_any, bool more = false)
  {
    (void)id; (void)more;   // unused

    // validate port
    if (!is_open()) {
      DECOM_LOG_ERROR("Sending failed: port is not open");
      return false;
    }

    // return false if an overlapped transfer is in progress, means no new data can be accepted
    // this is mostly the case when the upper layer didn't wait for tx_done indication
    if (tx_busy_) {
      DECOM_LOG_WARN("Transmission already in progress, data not accepted");
      return false;
    }

    tx_busy_ = true;

    // convert msg to vector - linear array is needed by WriteFile
    tx_buf_.clear();
    tx_buf_.reserve(data.size());
    tx_buf_.insert(tx_buf_.begin(), data.begin(), data.end());

    DWORD bytes_written = 0U;

    // init overlapped struct
    memset(&tx_overlapped_, 0, sizeof(tx_overlapped_));
    tx_overlapped_.hEvent = events_[ev_transmit];

    if (::WriteFile(com_handle_, static_cast<LPCVOID>(&tx_buf_[0]), static_cast<DWORD>(tx_buf_.size()), &bytes_written, &tx_overlapped_)) {
      // okay - byte written
      if (bytes_written != tx_buf_.size()) {
        // error - size mismatch, data not written
        tx_busy_ = false;
        return false;
      }
    }
    else {
      if (::GetLastError() != ERROR_IO_PENDING) {
        // sending error - this should never happen - check it!
        tx_busy_ = false;
        DECOM_LOG_CRIT("Sending error - should not happen, check it!");
        return false;
      }
    }

    // sending in progress now, indication is handled in worker thread
    return true;
  }


  ////////////////////////////////////////////////////////////////////////
  // L A Y E R   A P I

public:

  /**
   * Set communication parameter
   * \param baudrate The baudrate given in [baud]
   * \param databits Number of databits, normally 8
   * \param parity Parity, see defines
   * \param stopbits Number of stopbits, see defines
   * \param handshake Handshake protocol, see defines
   * \return true if successful
   */
  bool set_param(std::uint32_t baudrate,
                 std::uint8_t  databits  = 8U,
                 parity_type   parity    = parity_none,
                 stopbit_type  stopbits  = stopbit_1,
                 flowctrl_type handshake = flowctrl_none)
  {
    // validate port
    if (!is_open()) {
      return false;
    }

    DCB dcb;
    memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);

    dcb.fBinary  = TRUE;                      // only binary mode
    dcb.fParity  = (parity != parity_none);   // parity
    dcb.BaudRate = (DWORD)baudrate;           // setup the baud rate
    dcb.fOutxCtsFlow = (handshake == flowctrl_rts_cts);
    dcb.fOutxDsrFlow = (handshake == flowctrl_dtr_dsr);
    dcb.fDtrControl  = (handshake == flowctrl_dtr_dsr ? DTR_CONTROL_HANDSHAKE : DTR_CONTROL_ENABLE);
    dcb.fDsrSensitivity   = FALSE;
    dcb.fTXContinueOnXoff = FALSE;
    dcb.fOutX = (handshake == flowctrl_xon_xoff);
    dcb.fInX  = (handshake == flowctrl_xon_xoff);
    dcb.fErrorChar = FALSE;
    dcb.fNull = FALSE;
    dcb.fRtsControl = (handshake == flowctrl_rts_cts ? RTS_CONTROL_HANDSHAKE : RTS_CONTROL_ENABLE);
    dcb.fAbortOnError = FALSE;
    dcb.XonLim   = 1024U;               // Xon limit
    dcb.XoffLim  = 1024U;               // Xoff limit
    dcb.ByteSize = (BYTE)databits;      // setup the data bits
    dcb.Parity   = (BYTE)parity;        // setup the parity
    dcb.StopBits = (BYTE)stopbits - 2U; // setup the stop bits
    dcb.XonChar  = 0x11U;
    dcb.XoffChar = 0x13U;
    dcb.ErrorChar = 0x00U;
    dcb.EofChar  = 0x04U;
    dcb.EvtChar  = 0x00U;

    // apply settings
    return ::SetCommState(com_handle_, &dcb) != FALSE;
  }


  /**
   * Flush all buffers, means all data in buffers is send/get to/from hardware
   * \return true if successful
   */
  bool flush(void)
  {
    // validate port
    if (!is_open()) {
      return false;
    }

    DECOM_LOG_INFO("Flushing COM port");

    return ::FlushFileBuffers(com_handle_) != FALSE;
  }


  /**
   * Purge all buffers
   * \param rx Clears the receive buffer (if the device has one)
   * \param tx Clears the transmit buffer (if the device has one)
   * \return true if successful
   */
  bool purge(bool rx = true, bool tx = true)
  {
    // validate port
    if (!is_open()) {
      return false;
    }

    DECOM_LOG_INFO("Purging COM port");

    return ::PurgeComm(com_handle_, (rx ? (PURGE_RXABORT | PURGE_RXCLEAR) : 0U) |
                                    (tx ? (PURGE_TXABORT | PURGE_TXCLEAR) : 0U)
                      ) != FALSE;
  }

/////////////////////////////////////////////////////////////////////////////////////

private:
  /**
   * Check if the port is open
   * \return true if the COM port is open
   */
  inline bool is_open() const
  { return com_handle_ != INVALID_HANDLE_VALUE; }


  // worker thread
  static void worker_thread(void* arg)
  {
    // see https://msdn.microsoft.com/en-us/library/ff802693.aspx
    // and http://support.microsoft.com/kb/156932
    // and http://www.daniweb.com/software-development/cpp/threads/8020

    serial* s = static_cast<serial*>(arg);
    DWORD bytes_read, bytes_written;

    // validate port - MUST be open and active
    DECOM_LOG_VERIFY(s->is_open());

    for (;;)
    {
      if (!s->rx_busy_) {
        // issue read operation
        s->rx_busy_ = true;
        memset(&s->rx_overlapped_, 0, sizeof(s->rx_overlapped_));
        s->rx_overlapped_.hEvent = s->events_[ev_receive];
        if (::ReadFile(s->com_handle_, s->rx_buf_, DECOM_COM_SERIAL_RX_BUFSIZE, &bytes_read, &s->rx_overlapped_)) {
          // data is available, dwBytesRead is valid
          if (bytes_read) {
            msg data(s->rx_buf_, s->rx_buf_ + bytes_read);
            s->communicator::receive(data);
          }
          s->rx_busy_ = false;
        }
        else {
          if (::GetLastError() != ERROR_IO_PENDING) {
            // com error, port closed etc.
            DECOM_LOG_ASSERT(false);
            s->rx_busy_ = false;
          }
        }
      }

      switch (::WaitForMultipleObjects(ev_max, reinterpret_cast<HANDLE*>(s->events_), FALSE, INFINITE))
      {
        case WAIT_OBJECT_0 + ev_terminate :
          // kill event - terminate worker thread
          _endthreadex(0);
          return;

        case WAIT_OBJECT_0 + ev_transmit :
          // tx event
          {
            BOOL res = ::GetOverlappedResult(s->com_handle_, &s->tx_overlapped_, &bytes_written, FALSE) && bytes_written;
          (void)::ResetEvent(s->events_[ev_transmit]);              // manual reset event
          s->tx_busy_ = false;
          s->communicator::indication(res ? tx_done : tx_error);    // inform upper layer
          }
          break;

        case WAIT_OBJECT_0 + ev_receive :
          // rx event
          if (::GetOverlappedResult(s->com_handle_, &s->rx_overlapped_, &bytes_read, FALSE) && bytes_read) {
            // bytes received, pass data to upper layer
            msg data(s->rx_buf_, s->rx_buf_ + bytes_read);
            s->communicator::receive(data);
          }
          (void)::ResetEvent(s->events_[ev_receive]);               // manual reset event
          s->rx_busy_ = false;
          break;

        case WAIT_TIMEOUT:
          // should never happen in infinite waiting
          DECOM_LOG_ASSERT(false);
          break;

        default:
          break;
      }
    }
  }


  // COM parameters
  std::uint32_t   baudrate_;
  std::uint8_t    port_;
  std::uint8_t    databits_;
  parity_type     parity_;
  stopbit_type    stopbits_;
  flowctrl_type   handshake_;

  HANDLE          com_handle_;          // COM port handle
  HANDLE          thread_handle_;       // worker thread handle
  OVERLAPPED      tx_overlapped_;       // tx overlapped structure
  OVERLAPPED      rx_overlapped_;       // rx overlapped structure
  std::vector<std::uint8_t> tx_buf_;    // static linear tx buffer
  std::uint8_t    rx_buf_[DECOM_COM_SERIAL_RX_BUFSIZE];   // receive buffer

  volatile bool   tx_busy_;             // tx transfer running
  volatile bool   rx_busy_;             // rx transfer running

  enum {
    ev_terminate = 0,
    ev_transmit  = 1,
    ev_receive   = 2,
    ev_max       = 3
  };

  HANDLE events_[ev_max];     // terminate, xmit and receive event handles
};

} // namespace com
} // namespace decom

#endif // _DECOM_COM_SERIAL_H_
