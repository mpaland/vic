///////////////////////////////////////////////////////////////////////////////
// \brief vic::io stub for unit tests
// Records all written bytes so that head tests can inspect the emitted commands
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_TEST_IO_STUB_H_
#define _VIC_TEST_IO_STUB_H_

#include <cstdint>
#include <cstddef>

namespace io_stub {

// capacity of the record buffer
const std::size_t capacity = 16384U;

// recorded data (option byte is not stored)
const std::uint8_t* written();
std::size_t         written_size();
std::size_t         write_calls();
std::size_t         read_calls();
std::size_t         init_calls();
std::uint32_t       delay_total();

// clear the record buffer and all counters
void clear();

// make the next read calls return this data (copied), max 256 bytes
void set_read_data(const std::uint8_t* data, std::size_t length);

} // namespace io_stub

#endif  // _VIC_TEST_IO_STUB_H_
