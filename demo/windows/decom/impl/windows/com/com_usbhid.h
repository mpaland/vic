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
// \brief Windows USB HID device communication class
//
// This class is used to communicate with USB HID devices
//
// Example usage:
// decom::com::usbhid usb;
// decom::dev::generic gen(&usb);
// std::vector<decom::com::usbhid::hid_device_info_type> dev_info = usb.enumerate();  // enum all devices
// gen.open(dev_info[0].path, 0);  // open first device (for demo)
// gen.write(std::string("12345678901234567890"));
// gen.close();
//
// YOU HAVE TO LINK 'setupapi.lib' for USB usage. Mostly as FIRST library!
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_COM_USBHID_H_
#define _DECOM_COM_USBHID_H_

#include <windows.h>
#include <process.h>
#include <setupapi.h>   // YOU HAVE TO LINK 'setupapi.lib'. Mostly as first library.
#include <winioctl.h>
#include <atlconv.h>    // conversion macros
#include <vector>

#include "../../../com.h"

/////////////////////////////////////////////////////////////////////

// defines the receive buffer size, but the value is likely arbitrary
#ifndef DECOM_COM_USBHID_RX_BUFSIZE
#define DECOM_COM_USBHID_RX_BUFSIZE   32768U
#endif

/////////////////////////////////////////////////////////////////////

namespace decom {
namespace com {


// define HID header structures and functions of DDK
typedef struct _HIDD_ATTRIBUTES {
  ULONG  Size;
  USHORT VendorID;
  USHORT ProductID;
  USHORT VersionNumber;
} HIDD_ATTRIBUTES, *PHIDD_ATTRIBUTES;

typedef USHORT USAGE;
typedef struct _HIDP_CAPS {
  USAGE  Usage;
  USAGE  UsagePage;
  USHORT InputReportByteLength;
  USHORT OutputReportByteLength;
  USHORT FeatureReportByteLength;
  USHORT Reserved[17];
  USHORT fields_not_used_by_hidapi[10];
} HIDP_CAPS, *PHIDP_CAPS;
typedef void* PHIDP_PREPARSED_DATA;

#ifndef _NTDEF_
typedef _Return_type_success_(return >= 0) LONG NTSTATUS;
typedef NTSTATUS *PNTSTATUS;
#endif

#define HIDP_STATUS_SUCCESS 0x110000

typedef BOOLEAN(__stdcall *HidD_GetHidGuid_)(LPGUID HidGuid);
typedef BOOLEAN(__stdcall *HidD_GetAttributes_)(HANDLE device, PHIDD_ATTRIBUTES attrib);
typedef BOOLEAN(__stdcall *HidD_GetSerialNumberString_)(HANDLE device, PVOID buffer, ULONG buffer_len);
typedef BOOLEAN(__stdcall *HidD_GetManufacturerString_)(HANDLE handle, PVOID buffer, ULONG buffer_len);
typedef BOOLEAN(__stdcall *HidD_GetProductString_)(HANDLE handle, PVOID buffer, ULONG buffer_len);
typedef BOOLEAN(__stdcall *HidD_SetFeature_)(HANDLE handle, PVOID data, ULONG length);
typedef BOOLEAN(__stdcall *HidD_GetFeature_)(HANDLE handle, PVOID data, ULONG length);
typedef BOOLEAN(__stdcall *HidD_GetIndexedString_)(HANDLE handle, ULONG string_index, PVOID buffer, ULONG buffer_len);
typedef BOOLEAN(__stdcall *HidD_GetPreparsedData_)(HANDLE handle, PHIDP_PREPARSED_DATA *preparsed_data);
typedef BOOLEAN(__stdcall *HidD_FreePreparsedData_)(PHIDP_PREPARSED_DATA preparsed_data);
typedef NTSTATUS(__stdcall *HidP_GetCaps_)(PHIDP_PREPARSED_DATA preparsed_data, HIDP_CAPS *caps);
typedef BOOLEAN(__stdcall *HidD_SetNumInputBuffers_)(HANDLE handle, ULONG number_buffers);

static HidD_GetHidGuid_ HidD_GetHidGuid;
static HidD_GetAttributes_ HidD_GetAttributes;
static HidD_GetSerialNumberString_ HidD_GetSerialNumberString;
static HidD_GetManufacturerString_ HidD_GetManufacturerString;
static HidD_GetProductString_ HidD_GetProductString;
static HidD_SetFeature_ HidD_SetFeature;
static HidD_GetFeature_ HidD_GetFeature;
static HidD_GetIndexedString_ HidD_GetIndexedString;
static HidD_GetPreparsedData_ HidD_GetPreparsedData;
static HidD_FreePreparsedData_ HidD_FreePreparsedData;
static HidP_GetCaps_ HidP_GetCaps;
static HidD_SetNumInputBuffers_ HidD_SetNumInputBuffers;


class usbhid : public communicator
{
public:
  /**
   * Normal com ctor
   * \param name Layer name
   */
  usbhid(const char* name = "com_usbhid")
    : communicator(name)   // it's VERY IMPORTANT to call the base class ctor HERE!!!
    , thread_handle_(INVALID_HANDLE_VALUE)
    , device_handle_(INVALID_HANDLE_VALUE)
    , tx_busy_(false)
    , rx_busy_(false)
    , tx_offset_(0U)

  {
    lib_handle_ = ::LoadLibraryA("hid.dll");
    if (lib_handle_) {
#define RESOLVE(x) x = (x##_)GetProcAddress(lib_handle_, #x);
      RESOLVE(HidD_GetHidGuid);
      RESOLVE(HidD_GetAttributes);
      RESOLVE(HidD_GetSerialNumberString);
      RESOLVE(HidD_GetManufacturerString);
      RESOLVE(HidD_GetProductString);
      RESOLVE(HidD_SetFeature);
      RESOLVE(HidD_GetFeature);
      RESOLVE(HidD_GetIndexedString);
      RESOLVE(HidD_GetPreparsedData);
      RESOLVE(HidD_FreePreparsedData);
      RESOLVE(HidP_GetCaps);
      RESOLVE(HidD_SetNumInputBuffers);
#undef RESOLVE
    }

    // create events
    events_[ev_terminate] = ::CreateEvent(NULL, TRUE, FALSE, NULL);    // manual reset event
    events_[ev_transmit]  = ::CreateEvent(NULL, TRUE, FALSE, NULL);    // manual reset event
    events_[ev_receive]   = ::CreateEvent(NULL, TRUE, FALSE, NULL);    // manual reset event
  }


  /**
   * dtor
   */
  ~usbhid()
  {
    // close device
    close();

    // close handles
    (void)::CloseHandle(events_[ev_terminate]);
    (void)::CloseHandle(events_[ev_transmit]);
    (void)::CloseHandle(events_[ev_receive]);

    if (lib_handle_) {
      (void)::FreeLibrary(lib_handle_);
    }
  }


  /**
   * Called by upper layer to open this layer
   * \param address Full path of the HID device to open. Path is given in hid_device_info structure of enumerate()
   * \param id Unused
   * \return true if open is successful
   */
  bool open(const char* address = "", eid const& id = eid_any)
  {
    (void)id;

    // for security: check that upper protocol/device exists
    if (!upper_) {
      return false;
    }

    // library must be loaded
    if (!lib_handle_) {
      return false;
    }

    // open device
    device_handle_ = open_device(address);
    if (device_handle_ == INVALID_HANDLE_VALUE) {
      // error opening device
      return false;
    }

    // set the input report buffer size to 64 reports
    if (!HidD_SetNumInputBuffers(device_handle_, 64)) {
      DECOM_LOG_ERROR("Cannot set input buffers");
      return false;
    }

    // setup device info
    device_info_ = get_device_info(device_handle_);
    device_info_.path = address;

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
  void close(eid const& id = eid_any)
  {
    (void)id;

    // stop the receive thread
    if (thread_handle_ != INVALID_HANDLE_VALUE)
    {
      (void)::SetEvent(events_[ev_terminate]);
      (void)::WaitForSingleObject(thread_handle_, 3000U);   // wait 3 sec for thread termination
      (void)::CloseHandle(thread_handle_);
      thread_handle_ = INVALID_HANDLE_VALUE;
    }

    // close device
    if (device_handle_ != INVALID_HANDLE_VALUE) {
      (void)::CloseHandle(device_handle_);
      device_handle_ = INVALID_HANDLE_VALUE;
    }

    // port closed indication
    communicator::indication(disconnected);
  }


  /**
   * Called by upper layer to transmit data to this communication endpoint / physical layer
   * \param data Data to send
   * \param id eid.port is used as report ID (default is 0)
   * \param more unused
   * \return true if Send is successful
   */
  bool send(msg& data, eid const& id = eid_any, bool more = false)
  {
    (void)id; (void)more;

    // validate device
    if (!is_open()) {
      DECOM_LOG_ERROR("Sending failed: device is not open");
      return false;
    }

    // if an overlapped transfer is in progress, no new data can be accepted
    if (tx_busy_) {
      // abort - did you wait for tx_done indication?
      DECOM_LOG_WARN("Transmission already in progress, data not accepted");
      return false;
    }

    tx_busy_   = true;
    tx_msg_ = data;     // store a cheap copy
    tx_eid_    = id;      // store the id
    tx_offset_ = 0U;

    // start transmission
    return send_segment();
  }


private:

  bool usbhid::send_segment()
  {
    if (tx_offset_ < tx_msg_.size())
    {
      // convert msg to vector - linear array is needed by WriteFile
      tx_buf_.clear();
      tx_buf_.resize(device_info_.output_report_length + 1U);
      tx_buf_[0] = static_cast<std::uint8_t>(tx_eid_.port());                         // byte 0 is the report id
      tx_msg_.get(&tx_buf_[1], device_info_.output_report_length - 1U, tx_offset_);   // copy msg

      tx_offset_ += device_info_.output_report_length - 1U;

      DWORD bytes_written = 0U;

      // init overlapped struct
      memset(&tx_overlapped_, 0, sizeof(tx_overlapped_));
      tx_overlapped_.hEvent = events_[ev_transmit];

      if (::WriteFile(device_handle_, static_cast<LPCVOID>(&tx_buf_[0]), static_cast<DWORD>(tx_buf_.size()), &bytes_written, &tx_overlapped_)) {
        if (bytes_written != tx_buf_.size()) {
          // error - bytes not written
          tx_busy_ = false;
          return false;
        }
      }
      else {
        if (::GetLastError() != ERROR_IO_PENDING) {
          // sending error - wrong report id, device not ready etc.
          tx_busy_ = false;
          DECOM_LOG_CRIT("Sending error - should not happen, check it!");
          return false;
        }
      }
    }

    // nothing to send or sending in progress now, indication is handled in worker thread
    return true;
  }

  ////////////////////////////////////////////////////////////////////////
  // L A Y E R   A P I

public:

  /**
   * HID device info structure
   */
  typedef struct tag_hid_device_info_type {
    std::string   path;                 // platform specific device path - used for open()
    std::uint16_t vendor_id;            // vendor ID
    std::uint16_t product_id;           // product ID
    std::string   serial_number;        // serial number
    std::uint16_t release_number;       // release (version) number in BCD
    std::string   manufacturer_string;  // Manufacturer String
    std::string   product_string;       // Product string
    std::uint16_t usage_page;           // Usage Page for this Device/Interface
    std::uint16_t usage;                // Usage for this Device/Interface
    std::uint16_t output_report_length; // OutputReportByteLength
    std::uint16_t input_report_length;  // InputReportByteLength
    std::int32_t  interface_number;     // USB interface which represents this logical device
  } hid_device_info_type;


  /**
   * Enumerate USB HID devices.
   * This function returns a list of all the HID devices attached to the system
   * which match the vendor_id and product_id.
   * If vendor_id and product_id are both set to 0, then all HID devices are returned.
   * \param vendor_id The Vendor ID (VID) of the device to match
   * \param product_id The Product ID (PID) of the device to match
   * \return Returns a vector of available/matching devices
   */
  std::vector<hid_device_info_type> enumerate(std::uint16_t vendor_id = 0U, std::uint16_t product_id = 0U)
  {
    std::vector<hid_device_info_type> result;
   hid_device_info_type device_info;

    // windows objects for interacting with the driver
    SP_DEVINFO_DATA devinfo_data;
    SP_DEVICE_INTERFACE_DATA device_interface_data;
    SP_DEVICE_INTERFACE_DETAIL_DATA_A* device_interface_detail_data = NULL;
    HDEVINFO device_info_set = INVALID_HANDLE_VALUE;
    DWORD member_index = 0;

    // init windows objects
    memset(&devinfo_data, 0, sizeof(devinfo_data));
    devinfo_data.cbSize = sizeof(devinfo_data);
    memset(&device_interface_data, 0, sizeof(device_interface_data));
    device_interface_data.cbSize = sizeof(device_interface_data);

    // library must be loaded
    if (!lib_handle_) {
      return result;  // return empty vector
    }

    // get info of all devices belonging to the HID class
    GUID guid;
    HidD_GetHidGuid(&guid);
    device_info_set = ::SetupDiGetClassDevsA(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (device_info_set == INVALID_HANDLE_VALUE) {
      // abort
      return result;
    }

    // iterate devices in the HID class
    for (;;) {
      if (!::SetupDiEnumDeviceInterfaces(
        device_info_set,
        NULL,
        &guid,
        member_index++,
        &device_interface_data)) {
        // no more devices
        break;
      }

      // get required size
      DWORD required_size = 0;
      if ((!::SetupDiGetDeviceInterfaceDetailA(
        device_info_set,
        &device_interface_data,
        NULL,
        0U,
        &required_size,
        NULL
      ) && ::GetLastError() != ERROR_INSUFFICIENT_BUFFER) || !required_size) {
        // no size given
        continue;
      }
      device_interface_detail_data = (SP_DEVICE_INTERFACE_DETAIL_DATA_A*)new std::uint8_t[required_size > sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A) ? required_size : sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A)];
      device_interface_detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);

      // get detailed data for this device. The detail data gives us
      // device path is passed into CreateFile() to get a handle to the device
      if (!::SetupDiGetDeviceInterfaceDetailA(
        device_info_set,
        &device_interface_data,
        device_interface_detail_data,
        required_size,
        NULL,
        &devinfo_data)
        ) {
        // register_error - skip this device
        delete[] device_interface_detail_data;
        continue;
      }

      char property_buffer[256];
      bool driver_bound = false;
      for (DWORD i = 0U;; i++) {
        if (!::SetupDiEnumDeviceInfo(device_info_set, i, &devinfo_data)) {
          break;
        }
        if (!::SetupDiGetDeviceRegistryPropertyA(device_info_set, &devinfo_data, SPDRP_CLASS, NULL, (PBYTE)property_buffer, sizeof(property_buffer), NULL)) {
          break;
        }
        if ((strcmp(property_buffer, "HIDClass") == 0) && ::SetupDiGetDeviceRegistryPropertyA(device_info_set, &devinfo_data, SPDRP_DRIVER, NULL, (PBYTE)property_buffer, sizeof(property_buffer), NULL)) {
          // okay, driver is bound
          driver_bound = true;
          break;
        }
      }
      if (!driver_bound) {
        // no driver bound - skip this device
        delete[] device_interface_detail_data;
        continue;
      }

      (void)::SetupDiGetDeviceRegistryPropertyA(device_info_set, &devinfo_data, SPDRP_DEVICEDESC, NULL, (PBYTE)property_buffer, sizeof(property_buffer), NULL);

      // open a handle to the device
      HANDLE write_handle = open_device(device_interface_detail_data->DevicePath);
      if (write_handle == INVALID_HANDLE_VALUE) {
        // unable to open device
        delete[] device_interface_detail_data;
        continue;
      }

      // get device info
      device_info = get_device_info(write_handle);

      // get path
      device_info.path = device_interface_detail_data->DevicePath;

      // check the VID/PID to see if we should add this device to the enumeration list
      if ((vendor_id == 0U && product_id == 0U) ||
          (device_info.vendor_id == vendor_id && device_info.product_id == product_id))
      {
        // VID/PID match - create entry
        result.push_back(device_info);
      }

      delete[] device_interface_detail_data;
      (void)::CloseHandle(write_handle);
    }

    // close device info handle
    (void)::SetupDiDestroyDeviceInfoList(device_info_set);

    return result;
  }


  /**
   * Send a feature report to the device
   * \param report_id Report ID
   * \param data Report data
   * \return true if successful
   */
  bool send_feature_report(std::uint8_t report_id, msg const& data)
  {
    tmp_buf_.clear();
    tmp_buf_.push_back(report_id);
    tmp_buf_.insert(tmp_buf_.end(), data.begin(), data.end());

    return HidD_SetFeature(device_handle_, (PVOID)&tmp_buf_[0], static_cast<ULONG>(tmp_buf_.size())) != FALSE;
  }


  /**
   * Get a feature report from the device
   * \param report_id Report ID
   * \param data Report data
   * \return true if successful
   */
  bool get_feature_report(std::uint8_t report_id, msg& data)
  {
    BOOL res;
    DWORD bytes_returned;
#if 0
    OVERLAPPED ol;
    memset(&ol, 0, sizeof(ol));

    tmp_buf_.clear();


    msg data(buf + 1U, buf + bytes_read);         // first byte is report id


    res = DeviceIoControl(dev->device_handle, IOCTL_HID_GET_FEATURE, data, length, (PVOID)&tmp_buf_[0], static_cast<ULONG>(tmp_buf_.size()), &bytes_returned, &ol);
    if (!res) {
      if (GetLastError() != ERROR_IO_PENDING) {
        /* DeviceIoControl() failed. Return error. */
        //register_error(dev, "Send Feature Report DeviceIoControl");
        return false;
      }
    }

    /* Wait here until the write is done. This makes hid_get_feature_report() synchronous. */
    res = GetOverlappedResult(dev->device_handle, &ol, &bytes_returned, TRUE/*wait*/);
    if (!res) {
      /* The operation failed. */
      //register_error(dev, "Send Feature Report GetOverLappedResult");
      return false;
    }


    /* bytes_returned does not include the first byte which contains the
     report ID. The data buffer actually contains one more byte than
     bytes_returned. */
    data.clear();
    data.put(buf + 1U, buf + bytes_returned);
#endif
    return true;
  }


  HANDLE open_device(const char* path)
  {
    // try to open with sharing mode turned off first. So that HID device
    // can only be opened once. This is consistent with the behavior on other platforms
    HANDLE handle = ::CreateFileA(
      path,
      GENERIC_READ | GENERIC_WRITE,
      0U,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_OVERLAPPED,
      NULL
    );

    if (handle == INVALID_HANDLE_VALUE) {
      // error opening the device. Some devices must be opened with sharing enabled (even though they are only opened once)
      handle = ::CreateFileA(
        path,
        GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL
      );
    }

    return handle;
  }


  HANDLE open_device(std::uint16_t vendor_id, std::uint16_t product_id, const char* serial_number)
  {
    std::vector<hid_device_info_type> devs = enumerate(vendor_id, product_id);
    for (std::vector<hid_device_info_type>::const_iterator it = devs.begin(); it != devs.end(); ++it) {
      if (serial_number == it->serial_number) {
        return open_device(it->path.c_str());
      }
    }

    // no device found
    return INVALID_HANDLE_VALUE;
  }


/////////////////////////////////////////////////////////////////////////////////////

private:
  /**
   * Check if the device is open
   * \return true if the device is open
   */
  inline bool is_open() const
  { return device_handle_ != INVALID_HANDLE_VALUE; }


  static void worker_thread(void* arg)
  {
    usbhid* u = static_cast<usbhid*>(arg);
    DWORD bytes_read, bytes_written;

    // validate port - MUST be open and active
    DECOM_LOG_VERIFY(u->is_open());

    for (;;)
    {
      if (!u->rx_busy_) {
        // issue read operation
        u->rx_busy_ = true;
        memset(&u->rx_overlapped_, 0, sizeof(u->rx_overlapped_));
        u->rx_overlapped_.hEvent = u->events_[ev_receive];
        if (::ReadFile(u->device_handle_, u->rx_buf_, DECOM_COM_USBHID_RX_BUFSIZE, &bytes_read, &u->rx_overlapped_)) {
          // data is available, dwBytesRead is valid
          if (bytes_read) {
            msg data(u->rx_buf_ + 1U, u->rx_buf_ + bytes_read);   // first byte is report id
            u->communicator::receive(data);
          }
          u->rx_busy_ = false;
        }
        else {
          if (::GetLastError() != ERROR_IO_PENDING) {
            // com error, port closed etc.
            DECOM_LOG_ASSERT(false);
            u->rx_busy_ = false;
          }
        }
      }

      switch (::WaitForMultipleObjects(ev_max, reinterpret_cast<::HANDLE*>(u->events_), FALSE, INFINITE))
      {
        case WAIT_OBJECT_0 + ev_terminate :
          // kill event - terminate worker thread
          _endthreadex(0);
          return;

        case WAIT_OBJECT_0 + ev_transmit :
          // tx event
          {
            BOOL res = ::GetOverlappedResult(u->device_handle_, &u->tx_overlapped_, &bytes_written, FALSE) && bytes_written;
          (void)::ResetEvent(u->events_[ev_transmit]);    // manual reset event
          if (res) {
            // okay - byte written, any more data segments to send
            if (u->tx_offset_ < u->tx_msg_.size()) {
              // send next segment
              if (!u->send_segment()) {
                // serious error - inform upper layer
                u->tx_busy_ = false;
                u->communicator::indication(tx_error, u->tx_eid_);
              }
            }
            else {
              // data complete, inform upper layer
              u->tx_busy_ = false;
              u->communicator::indication(tx_done, u->tx_eid_);
            }
          }
          else {
            // error - bytes not written
            // discard any outstanding send data
            u->tx_busy_ = false;
            u->communicator::indication(tx_error, u->tx_eid_);
          }
          }
          break;

        case WAIT_OBJECT_0 + usbhid::ev_receive :
          // rx event
          if (::GetOverlappedResult(u->device_handle_, &u->rx_overlapped_, &bytes_read, FALSE) && bytes_read) {
            // bytes received, pass data to upper layer
            msg data(u->rx_buf_ + 1U, u->rx_buf_ + bytes_read);     // first byte is report id
            u->communicator::receive(data, u->rx_buf_[0]);          // use report id as channel
          }
          (void)::ResetEvent(u->events_[ev_receive]);     // manual reset event
          u->rx_busy_ = false;
          break;

        case WAIT_TIMEOUT:
          // should never happen
          DECOM_LOG_ASSERT(false);
          break;

        default:
          break;
      }
    }
  }


  // returns information about the device
  hid_device_info_type get_device_info(HANDLE device_handle)
  {
    hid_device_info_type device_info;

    // get the usage page and usage of the device
    PHIDP_PREPARSED_DATA pp_data = NULL;
    HIDP_CAPS caps;
    if (HidD_GetPreparsedData(device_handle, &pp_data)) {
      if (HidP_GetCaps(pp_data, &caps) == HIDP_STATUS_SUCCESS) {
        device_info.usage_page = caps.UsagePage;
        device_info.usage = caps.Usage;
        device_info.output_report_length = caps.OutputReportByteLength;
        device_info.input_report_length = caps.InputReportByteLength;
      }
      HidD_FreePreparsedData(pp_data);
    }

    // serial number
    wchar_t wstr[256];
    if (HidD_GetSerialNumberString(device_handle, wstr, sizeof(wstr))) {
      USES_CONVERSION;
      device_info.serial_number = W2A(wstr);
    }

    // manufacturer string
    if (HidD_GetManufacturerString(device_handle, wstr, sizeof(wstr))) {
      USES_CONVERSION;
      device_info.manufacturer_string = W2A(wstr);
    }

    // product string
    if (HidD_GetProductString(device_handle, wstr, sizeof(wstr))) {
      USES_CONVERSION;
      device_info.product_string = W2A(wstr);
    }

    // VID/PID
    HIDD_ATTRIBUTES attr;
    attr.Size = sizeof(attr);
    HidD_GetAttributes(device_handle, &attr);
    device_info.vendor_id = attr.VendorID;
    device_info.product_id = attr.ProductID;

    // release number
    device_info.release_number = attr.VersionNumber;

    // interface number
    // It can sometimes be parsed out of the path on Windows if a device has multiple interfaces
    // See http://msdn.microsoft.com/en-us/windows/hardware/gg487473
    // If it's not in the path, -1 is set
    device_info.interface_number = -1;
    if (device_info.path.find("&mi_") != std::string::npos) {
      std::string hex_str = device_info.path.substr(device_info.path.find("&mi_"));
      device_info.interface_number = util::hex2int<std::int32_t>(hex_str.c_str() + 4U);   // interface number is the # after "&mi_"
    }

    return device_info;
  }


  hid_device_info_type device_info_;    // info of opened device

  HMODULE         lib_handle_;          // handle to hid library
  HANDLE          device_handle_;       // handle to the opened device
  HANDLE          thread_handle_;       // worker thread handle
  OVERLAPPED      tx_overlapped_;       // tx overlapped structure
  OVERLAPPED      rx_overlapped_;       // rx overlapped structure
  decom::msg      tx_msg_;              // tx msg
  decom::eid      tx_eid_;              // tx eid (report id)
  std::size_t     tx_offset_;           // tx offset

  std::vector<std::uint8_t> tmp_buf_;   // static temp buffer
  std::vector<std::uint8_t> tx_buf_;    // static linear tx buffer
  std::uint8_t              rx_buf_[DECOM_COM_USBHID_RX_BUFSIZE];   // receive buffer

  volatile bool   tx_busy_;             // tx transfer running
  volatile bool   rx_busy_;             // rx transfer running

  enum {
    ev_terminate = 0,
    ev_transmit  = 1,
    ev_receive   = 2,
    ev_max       = 3
  };

  HANDLE events_[ev_max];   // event handles
};

} // namespace com
} // namespace decom

#endif    // _DECOM_COM_USBHID_H_
