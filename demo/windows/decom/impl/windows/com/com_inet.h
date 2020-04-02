///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2013-2017, PALANDesign Hannover, Germany
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
// \brief Internet communication class
//
// This class is used for client or server TCP or UDP connections over IP
// The Winsock2 API is used.
// You have to link against "Ws2_32.lib"
//
// TCP server (multi connections):
// com_inet (true, true)
// open("'localhost' or local ip : listen_port", eid_any)  --> indication: eid is (client addr/port)
// receive(data, eid (client addr/port))
// send(data, eid (client addr/port))
//
// UDP server (multi connections):
// com_inet (false, true)
// open("'localhost' or local ip : listen_port", eid_any)  --> indication: eid is (client addr/port)
// receive(data, eid (client addr/port))
// send(data, eid(client addr/port))
//
// TCP client (one connection):
// open("host:port", eid_any)                              --> indication: eid is eid_any 
// receive(data, eid_any)
// send(data, eid_any)
//
// UDP client (one connection)
// open("host:port", eid_any)                              --> indication: eid is eid_any
// receive(data, eid_any)
// send(data, eid_any)
//
// eid IP addr is stored in network format
// eid IP port is stored in host format
//
// \todo - Implementation of ::AcceptEx to be more robust against DOS and high server load
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_COM_INET_H_
#define _DECOM_COM_INET_H_

#define _WINSOCKAPI_    // stops windows.h including old winsock.h

#include <Windows.h>
#include <WinSock2.h>

#include <vector>
#include <map>
#include <sstream>
#include <thread>
#include <mutex>

#include <Ws2tcpip.h>
#include <string.h>

#include "../../../com.h"


// defines the size of the receive buffer
#define COM_INET_RX_BUFFER_SIZE         8192U

// defines the number of threads per processor in server mode, 2 is a good default
#define COM_INET_THREADS_PER_PROCESSOR  2U

/////////////////////////////////////////////////////////////////////

namespace decom {
namespace com {


class inet : public communicator
{
public:
  /**
   * Normal ctor
   * \param tcp true for TCP protocol, false for UDP protocol
   * \param server TCP: true for listening server, false for connecting client. UDP: true for send/revc, false for send only
   * \param ipv6 true for IPv6 protocol, false for IPv4, default is IPv4
   * \param name Layer name
   */
  inet(bool tcp, bool server = false, bool ipv6 = false, const char* name = "com_inet")
    : communicator(name)   // it's VERY IMPORTANT to call the base class ctor HERE!!!
    , use_tcp_(tcp)
    , use_ipv6_(ipv6)
    , server_(server)
  {
    socket_        = INVALID_SOCKET;
    accept_thread_ = nullptr;

    // start socket
    WSADATA wsa;
    if (::WSAStartup(MAKEWORD(2,2), &wsa)) {   // version 2.2 is used - this is supported down to Windows 95
      DECOM_LOG_EMERG("WSA startup error");
      return; // error
    }
    if (wsa.wVersion != MAKEWORD(2,2)) {
      DECOM_LOG_EMERG("WSA version not supported");
      return; // version not supported
    }

    // create I/O completion port
    completion_port_ = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (completion_port_ == NULL) {
      // error creation completion port - abort
      DECOM_LOG_EMERG("Creating completion port failed");
      return;
    }

    if (server_) {
      // create server worker threads
      SYSTEM_INFO system_info;
      ::GetSystemInfo(&system_info);    // determine how many processors are on the system
      DECOM_LOG_INFO("Detected " << (int)system_info.dwNumberOfProcessors << " cores, creating " << (int)system_info.dwNumberOfProcessors * COM_INET_THREADS_PER_PROCESSOR << " worker threads");
      for (DWORD i = 0; i < system_info.dwNumberOfProcessors * COM_INET_THREADS_PER_PROCESSOR; i++) {
        worker_threads_.push_back(new std::thread(&inet::worker_thread, this));
        //DECOM_LOG_DEBUG("Created worker thread ") << ss.str();
      }
    }
    else {
      // create client worker thread
      worker_threads_.push_back(new std::thread(&inet::worker_thread, this));
      //DECOM_LOG_DEBUG("Created worker thread ") << worker_threads_.back()->get_id();
    }
  }


  /**
   * dtor
   */
  virtual ~inet()
  {
    close();

    // trigger all worker threads out of waiting
    for (std::vector<std::thread*>::const_iterator it = worker_threads_.begin(); it != worker_threads_.end(); ++it) {
      (void)::PostQueuedCompletionStatus(completion_port_, 0U, (ULONG_PTR)NULL, NULL);
    }
    // AFTER that, terminate all worker threads
    for (std::vector<std::thread*>::const_iterator it = worker_threads_.begin(); it != worker_threads_.end(); ++it) {
      //DECOM_LOG_DEBUG("Joining worker thread " << (*it)->get_id());
      (*it)->join();
      delete (*it);
    }

    // close completion port
    (void)::CloseHandle(completion_port_);

    // cleanup
    (void)::WSACleanup();
  }


  /**
   * Called by upper layer to open this layer
   * \param address The address to open in the form of "host:port", "IP:port" (IPv4) or "[IP]:port" (IPv6)
   *                Server mode: Defines the local interface and listen port
   *                Client mode: Defines the client host and port, the local interface may be set via set_source_address
   * \param id Unused
   * \return true if open is successful
   */
  virtual bool open(const char* address = "", eid const& id = eid_any)
  {
    (void)id;

    // for security: check that upper protocol/device exists
    if (!upper_) {
      return false;
    }

    // already open?
    if (socket_ != INVALID_SOCKET) {
      DECOM_LOG_WARN("Socket already open");
      return false;
    }

    // resolve address
    PADDRINFOA result = resolve_address(address);
    if (!result) {
      return false;
    }

    // create the socket
    socket_ = ::WSASocket(result->ai_family, result->ai_socktype, result->ai_protocol, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (socket_ == INVALID_SOCKET) {
      // error
      DECOM_LOG_CRIT("Socket creation error");
      return false;
    }

    if (server_) {
      // S E R V E R

      // bind socket
      if (::bind(socket_, result->ai_addr, result->ai_addrlen)) {
        DECOM_LOG_ERROR("Socket bind() failed with error " << ::WSAGetLastError());
        return false;
      }

      if (use_tcp_) {
        // TCP SERVER

        // set listen socket
        if (::listen(socket_, SOMAXCONN)) {
          DECOM_LOG_ERROR("Socket listen() failed with error " << ::WSAGetLastError());
          return false;
        }

        // create accept thread
        accept_thread_ = new std::thread(&inet::accept_thread, this);
        //DECOM_LOG_DEBUG("Created accept thread " << accept_thread_->get_id());
      }
      else {
        // UDP SERVER

        client_context_type* client_context = new client_context_type;
        memset(&client_context->recv.overlapped, 0, sizeof(client_context->recv.overlapped));
        client_context->recv.wsa_buffer.buf = client_context->recv.buffer;
        client_context->recv.wsa_buffer.len = sizeof(client_context->recv.buffer);
        client_context->recv.send = false;
        memset(&client_context->send.overlapped, 0, sizeof(client_context->send.overlapped));
        client_context->send.wsa_buffer.buf = client_context->send.buffer;
        client_context->send.wsa_buffer.len = 0;
        client_context->send.send = true;
        client_context->socket = socket_;
        client_context->id = eid_any;
        {
          std::lock_guard<std::mutex> lock(client_contexts_mutex_);
          client_contexts_[eid_any] = client_context;
        }

        // associate the accept socket with IOCP
        if (::CreateIoCompletionPort((HANDLE)socket_, completion_port_, (ULONG_PTR)client_context, 0) == NULL) {
          // error
          DECOM_LOG_ERROR("Creating completion port failed");
        }

        // trigger initial receive
        DWORD flags = 0;
        DWORD recv_bytes = 0;
        client_context->recv.from_len = sizeof(client_context->recv.from_addr);
        if (::WSARecvFrom(socket_, &client_context->recv.wsa_buffer, 1U, &recv_bytes, &flags, (SOCKADDR*)&client_context->recv.from_addr, &client_context->recv.from_len, &client_context->recv.overlapped, NULL) == SOCKET_ERROR) {
          if (::WSAGetLastError() != WSA_IO_PENDING) {
            DECOM_LOG_ERROR("Initial receive failed");
          }
        }
      }
    }
    else {
      // C L I E N T

      // bind non default source address/port if given
      if (!source_addr_.empty()) {
        // source address is given, resolve it
        PADDRINFOA addr_src = resolve_address(source_addr_.c_str());
        if (addr_src) {
          // bind socket
          if (::bind(socket_, addr_src->ai_addr, addr_src->ai_addrlen)) {
            DECOM_LOG_WARN("Source bind() failed with error " << ::WSAGetLastError());
          }
        }
      }

      // connect socket
      if (::connect(socket_, result->ai_addr, result->ai_addrlen)) {
        DECOM_LOG_WARN("Socket connect() failed with error " << ::WSAGetLastError());
        // close socket
        (void)::closesocket(socket_);
        socket_ = INVALID_SOCKET;
        return false;
      }

      // register context
      client_context_type* client_context = new client_context_type;
      memset(&client_context->recv.overlapped, 0, sizeof(client_context->recv.overlapped));
      client_context->recv.wsa_buffer.buf = client_context->recv.buffer;
      client_context->recv.wsa_buffer.len = sizeof(client_context->recv.buffer);
      client_context->recv.send = false;
      memset(&client_context->send.overlapped, 0, sizeof(client_context->send.overlapped));
      client_context->send.wsa_buffer.buf = client_context->send.buffer;
      client_context->send.wsa_buffer.len = 0;
      client_context->send.send = true;
      client_context->socket = socket_;
      client_context->id = eid_any;
      {
        std::lock_guard<std::mutex> lock(client_contexts_mutex_);
        client_contexts_[eid_any] = client_context;
      }

      // associate the socket with IOCP
      if (::CreateIoCompletionPort((HANDLE)socket_, completion_port_, (ULONG_PTR)client_context, 0) == NULL) {
        // error
        DECOM_LOG_ERROR("Creating completion port failed");
        // close socket
        (void)::closesocket(socket_);
        socket_ = INVALID_SOCKET;
        return false;
      }

      // notify upper layer
      communicator::indication(connected, eid_any);

      // trigger initial receive
      DWORD flags = 0;
      DWORD recv_bytes = 0;
      if (::WSARecv(socket_, &client_context->recv.wsa_buffer, 1U, &recv_bytes, &flags, &(client_context->recv.overlapped), NULL) == SOCKET_ERROR) {
        if (::WSAGetLastError() != WSA_IO_PENDING) {
          DECOM_LOG_ERROR("Initial receive failed");
        }
      }
    }

    return true;
  }


  /**
   * Called by upper layer to close this layer
   * \param id Unused (all open eids are closed)
   */
  virtual void close(eid const& id = eid_any)
  {
    (void)id;   // unused

    if (socket_ == INVALID_SOCKET) {
      // not open / already closed
      return;
    }

    // shutdown and close the main socket
    DECOM_LOG_DEBUG("Shudown and closing socket");
    (void)::shutdown(socket_, SD_BOTH);
    (void)::closesocket(socket_);
    socket_ = INVALID_SOCKET;

    // delete all client sockets and contexts
    if (server_) {
      for (client_contexts_type::const_iterator it = client_contexts_.begin(); it != client_contexts_.end(); ++it) {
        // notify upper layer
        (void)::shutdown(it->second->socket, SD_BOTH);
        (void)::closesocket(it->second->socket);
        // closed indication in worker thread
      }
    }

    // wait for accept thread termination
    if (server_ && accept_thread_) {
      //DECOM_LOG_DEBUG("Joining accept thread " << accept_thread_->get_id());
      accept_thread_->join();
      delete accept_thread_;
      accept_thread_ = nullptr;
    }
  }


  /**
   * Called by upper layer to transmit data to the internet
   * \param data The message to send
   * \param id The endpoint identifier, use eid_any for default socket
   * \param more true if message is a fragment - mostly unused on this layer
   * \return true if Send is successful
   */
  virtual bool send(msg& data, eid const& id = eid_any, bool more = false)
  {
    (void)more;   // unused

    if (socket_ == INVALID_SOCKET) {
      // not open / already closed
      DECOM_LOG_ERROR("Sending failed: socket is not open");
      return false;
    }

    // find according client context if TCP server
    client_contexts_type::const_iterator context = client_contexts_.find(use_tcp_ && server_ ? id : eid_any);
    if (context == client_contexts_.end()) {
      // channel not found
      DECOM_LOG_WARN("Sending eid ") << format_eid(id).str().c_str() << " not found";
      return false;
    }

    // return false if an overlapped transfer is in progress, means no new data can be accepted
    // this is mostly the case when the upper layer didn't wait for tx_done indication
    if (context->second->send.wsa_buffer.len != 0U) {
      DECOM_LOG_WARN("Transmission already in progress");
      return false;
    }

    // init data buffer
    data.get((std::uint8_t*)context->second->send.wsa_buffer.buf, COM_INET_RX_BUFFER_SIZE);
    context->second->send.wsa_buffer.len = data.size();

    DWORD send_bytes = 0U;
    int err;
    if (!use_tcp_ && server_) {
      // UDP server
      SOCKADDR_STORAGE addr;
      if (use_ipv6_) {
        addr.ss_family = AF_INET6;
        ((SOCKADDR_IN6*)&addr)->sin6_port = ::htons(static_cast<std::uint16_t>(id.port()));
        memcpy(&((SOCKADDR_IN6*)&addr)->sin6_addr, &id.addr(), 16U);
      }
      else {
        addr.ss_family = AF_INET;
        ((SOCKADDR_IN*)&addr)->sin_port = ::htons(static_cast<std::uint16_t>(id.port()));
        memcpy(&((SOCKADDR_IN*)&addr)->sin_addr, &id.addr(), 4U);
      }
      err = ::WSASendTo(context->second->socket, &context->second->send.wsa_buffer, 1U, &send_bytes, 0U, (SOCKADDR*)&addr, sizeof(SOCKADDR_STORAGE), &context->second->send.overlapped, NULL);
    }
    else {
      // TCP server or client
      err = ::WSASend(context->second->socket, &context->second->send.wsa_buffer, 1U, &send_bytes, 0U, &context->second->send.overlapped, NULL);
    }
    if (err == SOCKET_ERROR && ::WSAGetLastError() != WSA_IO_PENDING) {
      DECOM_LOG_ERROR("Sending failure, eid ") << format_eid(id).str().c_str();
      // tx error indication
      communicator::indication(tx_error, id);
      return false;
    }

    return true;
  }


  ////////////////////////////////////////////////////////////////////////
  // C O M M U N I C A T O R   A P I

  /**
   * TCP/UDP CLIENT mode: Set the client (source) address/port in and define other address/port than default.
   * Set this BEFORE calling open()!
   * \param address The source address in the form of "host:port", "IP:port" (IPv4) or "[IP]:port" (IPv6)
   */
  void set_source_address(const char* address = "")
  {
    // already open?
    if (socket_ != INVALID_SOCKET) {
      DECOM_LOG_ERROR("Socket already open, source address can't be changed anymore");
    }

    source_addr_ = address;
  }


  ////////////////////////////////////////////////////////////////////////


  // accept thread for TCP server connections
  static void accept_thread(void* arg)
  {
    inet* i = static_cast<inet*>(arg);

    for (;;) {
      SOCKADDR_STORAGE client_addr;
      int client_addr_len = sizeof(client_addr);
      SOCKET accept_socket = ::WSAAccept(i->socket_, (SOCKADDR*)&client_addr, &client_addr_len, NULL, NULL);
      if (accept_socket == INVALID_SOCKET) {
        if (::WSAGetLastError() == WSAEINTR) {
          // shutdown
          DECOM_LOG_DEBUG2("Shutdown accept thread", i->name_);
          break;
        }
        else {
          // failure
          DECOM_LOG_ERROR2("Accepting socket failed with error " << WSAGetLastError(), i->name_);
        }
      }

      // report client
      DECOM_LOG_INFO2("Client connected from " << i->address_to_string(&client_addr).str().c_str(), i->name_);

      // register client context
      client_context_type* client_context = new client_context_type;
      memset(&client_context->recv.overlapped, 0, sizeof(client_context->recv.overlapped));
      client_context->recv.wsa_buffer.buf = client_context->recv.buffer;
      client_context->recv.wsa_buffer.len = sizeof(client_context->recv.buffer);
      client_context->recv.send = false;
      memset(&client_context->send.overlapped, 0, sizeof(client_context->send.overlapped));
      client_context->send.wsa_buffer.buf = client_context->send.buffer;
      client_context->send.wsa_buffer.len = 0;
      client_context->send.send = true;
      client_context->socket = accept_socket;
      // set client context eid
      std::uint16_t port = ::ntohs(client_addr.ss_family == AF_INET ? ((SOCKADDR_IN*)&client_addr)->sin_port : ((SOCKADDR_IN6*)&client_addr)->sin6_port);
      client_context->id.port(port);
      memcpy((void*)client_context->id.addr().addr, (client_addr.ss_family == AF_INET) ? (void*)&((SOCKADDR_IN*)&client_addr)->sin_addr : (void*)&((SOCKADDR_IN6*)&client_addr)->sin6_addr, (client_addr.ss_family == AF_INET) ? 4U : 16U);
      {
        std::lock_guard<std::mutex> lock(i->client_contexts_mutex_);
        i->client_contexts_[client_context->id] = client_context;
      }

      // associate the accept socket with IOCP
      if (::CreateIoCompletionPort((HANDLE)accept_socket, i->completion_port_, (ULONG_PTR)client_context, 0) == NULL) {
        // error
        DECOM_LOG_ERROR2("Creating completion port failed", i->name_);
      }

      // notify upper layer
      i->communicator::indication(connected, client_context->id);

      // trigger initial receive
      DWORD flags = 0;
      DWORD recv_bytes = 0;
      if (::WSARecv(accept_socket, &client_context->recv.wsa_buffer, 1U, &recv_bytes, &flags, &(client_context->recv.overlapped), NULL) == SOCKET_ERROR) {
        if (::WSAGetLastError() != WSA_IO_PENDING) {
          DECOM_LOG_ERROR2("Initial receive failed", i->name_);
        }
      }
    }

    DECOM_LOG_DEBUG2("Terminating accept thread", i->name_);
  }


  static void worker_thread(void* arg)
  {
    inet* i = static_cast<inet*>(arg);

    for (;;) {
      DWORD bytes_transferred = 0U;
      OVERLAPPED* overlapped;
      client_context_type* client_context;

      // actual context is given as completion key
      if (::GetQueuedCompletionStatus(i->completion_port_, &bytes_transferred, (PULONG_PTR)&client_context, &overlapped, INFINITE))
      {
        if (!client_context) {
          // shutdown
          DECOM_LOG_DEBUG2("Shutdown worker thread", i->name_);
          break;
        }

        if (bytes_transferred == 0U) {
          // socket has been closed
          DECOM_LOG_INFO2("Socket has been closed by peer", i->name_);
          // closed indication
          i->communicator::indication(disconnected, client_context->id);
          //remove/delete client_context
          {
            std::lock_guard<std::mutex> lock(i->client_contexts_mutex_);
            i->client_contexts_.erase(client_context->id);
          }
          continue;
        }

        if (overlapped) {
          io_data_type* io_data = CONTAINING_RECORD(overlapped, io_data_type, overlapped);
          if (io_data->send) {
            // sending bytes
            if (bytes_transferred != io_data->wsa_buffer.len) {
              // sending not completed
              client_context->send.wsa_buffer.buf += bytes_transferred;
              client_context->send.wsa_buffer.len -= bytes_transferred;
              if (::WSASend(client_context->socket, &client_context->send.wsa_buffer, 1U, &bytes_transferred, 0U, &client_context->send.overlapped, NULL) == SOCKET_ERROR) {
                if (::WSAGetLastError() != WSA_IO_PENDING) {
                  DECOM_LOG_ERROR2("Sending failure", i->name_);
                  // tx error indication
                  i->communicator::indication(tx_error, client_context->id);
                }
              }
            }
            else {
              io_data->wsa_buffer.len = 0U;
              // tx done indication
              i->communicator::indication(tx_done, client_context->id);
            }
          }
          else {
            // bytes received, pass data to upper layer
            decom::msg data(client_context->recv.wsa_buffer.buf, client_context->recv.wsa_buffer.buf + bytes_transferred);
            decom::eid id;
            id.port(::ntohs(client_context->recv.from_addr.ss_family == AF_INET ? ((SOCKADDR_IN*)&client_context->recv.from_addr)->sin_port : ((SOCKADDR_IN6*)&client_context->recv.from_addr)->sin6_port));
            memcpy((void*)id.addr().addr, (client_context->recv.from_addr.ss_family == AF_INET) ? (void*)&((SOCKADDR_IN*)&client_context->recv.from_addr)->sin_addr : (void*)&((SOCKADDR_IN6*)&client_context->recv.from_addr)->sin6_addr, (client_context->recv.from_addr.ss_family == AF_INET) ? 4U : 16U);
            i->communicator::receive(data, (!i->use_tcp_ && i->server_) ? id : client_context->id);  // use received address as eid if UDP server
            // continue receiving
            DWORD flags = 0U;
            client_context->recv.from_len = sizeof(client_context->recv.from_addr);
            if (::WSARecvFrom(client_context->socket, &client_context->recv.wsa_buffer, 1U, &bytes_transferred, &flags, (SOCKADDR*)&client_context->recv.from_addr, &client_context->recv.from_len, &client_context->recv.overlapped, NULL) == SOCKET_ERROR) {
              if (::WSAGetLastError() != WSA_IO_PENDING) {
                DECOM_LOG_ERROR2("Receive failed", i->name_);
                // rx error indication
                i->communicator::indication(rx_error, (!i->use_tcp_ && i->server_) ? id : client_context->id);
              }
            }
          }
        }
      }
      else {
        // socket has been closed
        DECOM_LOG_INFO2("Client disconnected", i->name_);
        // closed indication
        i->communicator::indication(disconnected, client_context->id);
        //remove/delete client_context
        std::lock_guard<std::mutex> lock(i->client_contexts_mutex_);
        i->client_contexts_.erase(client_context->id.port());
      }
    }

    DECOM_LOG_DEBUG2("Terminating worker thread", i->name_);
  }


/////////////////////////////////////////////////////////////////////////////
// H E L P E R
private:

  std::stringstream format_eid(eid const& id) const
  {
    std::stringstream eid_str;
    if (id.is_any()) {
      eid_str << "ANY";
    }
    else {
      eid_str << std::hex << id.addr().addr32[0] << "."
              << std::hex << id.addr().addr32[1] << "."
              << std::hex << id.addr().addr32[2] << "."
              << std::hex << id.addr().addr32[3] << ":"
              << std::dec << id.port();
    }
    return eid_str;
  }


  std::stringstream address_to_string(SOCKADDR_STORAGE* addr) const
  {
    char ip[128];
    DWORD ip_len = sizeof(ip);
    (void)::WSAAddressToStringA((SOCKADDR*)addr, sizeof(SOCKADDR_STORAGE), NULL, ip, &ip_len);
    std::stringstream str(ip);
    return str;
  }


  PADDRINFOA inet::resolve_address(const char* address) const
  {
    // extract host and port from address
    std::string host(address);
    std::string port(address);

    // IPv6 IP address given?
    std::string::size_type c = host.rfind("]:");
    if (c != std::string::npos) {
      // yes cut off port
      host.erase(c, host.length() - c);
      // remove start bracket
      if (host.find("[") != std::string::npos) {
        host.erase(host.begin() + host.find("["));
      }
      // cut off address
      port.erase(0, c + 2U);
    }
    else {
      c = host.rfind(":");
      if (c != std::string::npos) {
        // cut off port
        host.erase(c, host.length() - c);
        // cut off address
        port.erase(0, c + 1U);
      }
      else {
        port.clear();
      }
    }

    // resolve address
    PADDRINFOA result = NULL;
    ADDRINFOA hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = use_ipv6_ ? AF_INET6 : AF_INET;         // use IPv4 or IPv6
    hints.ai_socktype = use_tcp_ ? SOCK_STREAM : SOCK_DGRAM;    // TCP or UDP
    hints.ai_protocol = use_tcp_ ? IPPROTO_TCP : IPPROTO_UDP;   // TCP or UDP
    hints.ai_flags    = 0;                                      // no flags
    int err = ::getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
    if (err) {
      DECOM_LOG_ERROR("Address " << address << " can't be resolved, getaddrinfo() failed with error " << ::WSAGetLastError());
      return nullptr;
    }
    DECOM_LOG_INFO("Address resolved to ") << address_to_string((SOCKADDR_STORAGE*)result->ai_addr).str().c_str();

    return result;
  }


private:
  std::vector<std::thread*> worker_threads_;    // pool of worker threads
  std::thread* accept_thread_;                  // accept thread

  bool        use_tcp_;             // use TCP or UDP
  bool        use_ipv6_;            // use IPv4 or IPv6
  bool        server_;              // server or client
  SOCKET      socket_;              // socket
  HANDLE      completion_port_;     // completion port
  std::string source_addr_;         // source addr

  typedef struct tag_io_data_type {
    OVERLAPPED        overlapped;
    WSABUF            wsa_buffer;
    char              buffer[COM_INET_RX_BUFFER_SIZE];
    SOCKADDR_STORAGE  from_addr;
    int               from_len;
    bool              send;
  } io_data_type;

  typedef struct tag_client_context_type {
    io_data_type recv;
    io_data_type send;
    SOCKET       socket;    // associated accept socket
    eid          id;        // eid of the socket, here: address = IP, port = port
  } client_context_type;

  typedef std::map<eid, client_context_type*> client_contexts_type;

  client_contexts_type client_contexts_;
  std::mutex           client_contexts_mutex_;
};

} // namespace com
} // namespace decom

#endif  // _DECOM_COM_INET_H_
