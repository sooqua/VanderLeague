#pragma once
#include <hvpp/lib/device.h>
#include <hvpp/vmexit/vmexit_dbgbreak.h>

#include <cstdint>

typedef struct ioctl_write_protected_memory_request_t {
  uint32_t src;
  uint32_t dst;
  uint32_t size;
} ioctl_write_protected_memory_request;

using ioctl_write_protected_memory_t = ioctl_read_write_t<1, sizeof(ioctl_write_protected_memory_request_t)>;

class device_custom
  : public device
{
  public:
    const char* name() const noexcept override { return "hvpp"; }

    auto handler() noexcept -> hvpp::vmexit_dbgbreak_handler&;
    void handler(hvpp::vmexit_dbgbreak_handler& handler_instance) noexcept;

    error_code_t on_ioctl(void* buffer, size_t buffer_size, uint32_t code) noexcept override;

  private:
    error_code_t ioctl_enable_io_debugbreak(void* buffer, size_t buffer_size);

    hvpp::vmexit_dbgbreak_handler* handler_ = nullptr;
};
