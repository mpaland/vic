///////////////////////////////////////////////////////////////////////////////
// \brief vic::io stub for unit tests, see io_stub.h
///////////////////////////////////////////////////////////////////////////////

#include "io_stub.h"
#include "io.h"

#include <cstring>


namespace {
  std::uint8_t  g_written[io_stub::capacity];
  std::size_t   g_written_size = 0U;
  std::size_t   g_write_calls  = 0U;
  std::size_t   g_read_calls   = 0U;
  std::size_t   g_init_calls   = 0U;
  std::uint32_t g_delay_total  = 0U;
  std::uint8_t  g_read_data[256];
  std::size_t   g_read_size    = 0U;
}


namespace io_stub {

const std::uint8_t* written()      { return g_written; }
std::size_t         written_size() { return g_written_size; }
std::size_t         write_calls()  { return g_write_calls; }
std::size_t         read_calls()   { return g_read_calls; }
std::size_t         init_calls()   { return g_init_calls; }
std::uint32_t       delay_total()  { return g_delay_total; }

void clear()
{
  g_written_size = 0U;
  g_write_calls  = 0U;
  g_read_calls   = 0U;
  g_init_calls   = 0U;
  g_delay_total  = 0U;
  g_read_size    = 0U;
}

void set_read_data(const std::uint8_t* data, std::size_t length)
{
  g_read_size = length < sizeof(g_read_data) ? length : sizeof(g_read_data);
  std::memcpy(g_read_data, data, g_read_size);
}

} // namespace io_stub


namespace vic {
namespace io {

void delay(std::uint32_t time_to_delay)
{
  g_delay_total += time_to_delay;
}

void init(handle_type device_handle)
{
  (void)device_handle;
  ++g_init_calls;
}

bool write(handle_type device_handle,
           std::uint32_t option,
           const std::uint8_t* data_out, std::size_t data_out_length,
           std::uint8_t* data_in, std::size_t data_in_length,
           std::uint32_t timeout)
{
  (void)device_handle; (void)option; (void)timeout;
  ++g_write_calls;
  if (data_out && data_out_length) {
    const std::size_t n = (g_written_size + data_out_length <= io_stub::capacity) ? data_out_length : (io_stub::capacity - g_written_size);
    std::memcpy(&g_written[g_written_size], data_out, n);
    g_written_size += n;
  }
  if (data_in && data_in_length) {
    const std::size_t n = data_in_length < g_read_size ? data_in_length : g_read_size;
    std::memcpy(data_in, g_read_data, n);
    if (n < data_in_length) {
      std::memset(data_in + n, 0, data_in_length - n);
    }
  }
  return true;
}

bool read(handle_type device_handle,
          std::uint32_t option,
          std::uint8_t* data_in, std::size_t& data_in_length,
          std::uint32_t timeout)
{
  (void)device_handle; (void)option; (void)timeout;
  ++g_read_calls;
  const std::size_t n = data_in_length < g_read_size ? data_in_length : g_read_size;
  if (data_in && n) {
    std::memcpy(data_in, g_read_data, n);
  }
  data_in_length = n;
  g_read_size = 0U;    // data is consumed
  return n != 0U;
}

} // namespace io
} // namespace vic
