///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2017-2017, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// This file is part of the vic library.
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
// \brief vic IO class
// Define this class/file in your own project and implement the four hardware
// dependent IO functions.
// Best is to take this header file, copy it and define your access functions in it.
// Alternative use a .cpp file but the compiler won't be able to inline functions very well.
// vic driver hardware access is done via these functions like io.mem_set<std::uint8_t>(0x000, 0x00);
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_IO_H_
#define _VIC_IO_H_

#include <cstdint>
#include <cstddef>


namespace vic {
namespace io {


/**
 * Delay
 * \param time_to_delay Milliseconds [ms] to delay
 */
void delay(std::uint32_t time_to_delay);


/////////////////////////////////////////////////////////////////////////////
// D E V I C E   A C C E S S

namespace dev {

  // device handle
  typedef const void* handle_type;

  /**
   * Init the interface
   * \param device_handle Logical device handle
   */
  void init(handle_type device_handle);

  /**
   * IO write/read access to device
   * \param device_handle Logical device handle
   * \param option Optional data for the device like register selection
   * \param data_out Data transmit buffer
   * \param data_out_length Data length to send
   * \param data_in Data receive buffer, may be used with bidirectional devices like SPI
   * \param data_in_length Additional input length
   * \param timeout Time in [ms] to wait for sending data, 0 = no waiting
   * \return true if successful
   */ 
  bool write(handle_type device_handle,
             std::uint32_t option,
             const std::uint8_t* data_out, std::size_t data_out_length,
             std::uint8_t* data_in, std::size_t data_in_length,
             std::uint32_t timeout = 0U);

  /**
   * IO read access from device
   * \param device_handle Logical device handle
   * \param option Optional data for the device like register selection
   * \param data_in Data receive buffer
   * \param data_in_length Maximum buffer size on input, received chars on output
   * \param timeout Time in [ms] to wait for receiving data, 0 = no waiting (fifo check)
   * \return true if successful
   */ 
  bool read(handle_type device_handle,
            std::uint32_t option,
            std::uint8_t* data_in, std::size_t& data_in_length,
            std::uint32_t timeout = 0U);

} // namespace dev


/////////////////////////////////////////////////////////////////////////////
// M E M O R Y   A C C E S S

namespace mem {

  /**
   * Direct memory access, write
   * \param address The address to write to
   * \param value The value (of type VALUE_TYPE) written to the address
   */
  template<typename VALUE_TYPE>
  void write(const void* const address, VALUE_TYPE value)
  { *static_cast<volatile VALUE_TYPE*>(address) = value; }

  /**
   * Direct memory access, read
   * \param address The address to read from
   * \return The value (of type VALUE_TYPE) read from address
   */
  template<typename VALUE_TYPE>
  VALUE_TYPE read(const void* const address)
  { return *static_cast<volatile VALUE_TYPE*>(address); }

} // namespace mem


} // namespace io
} // namespace vic

#endif  // _VIC_IO_H_
