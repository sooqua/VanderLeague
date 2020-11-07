#include "device_custom.h"

#include <hvpp/lib/assert.h>
#include <hvpp/lib/debugger.h>
#include <hvpp/lib/log.h>

#include <ntifs.h>

NTSTATUS KernelMemCopy(void* dst, void* src, SIZE_T size);

auto device_custom::handler() noexcept -> hvpp::vmexit_dbgbreak_handler&
{
  return *handler_;
}

void device_custom::handler(hvpp::vmexit_dbgbreak_handler& handler_instance) noexcept
{
  handler_ = &handler_instance;
}

error_code_t device_custom::on_ioctl(void* buffer, size_t buffer_size, uint32_t code) noexcept
{
  switch (code)
  {
    case ioctl_write_protected_memory_t::code:
      return ioctl_enable_io_debugbreak(buffer, buffer_size);

    default:
      hvpp_assert(0);
      return make_error_code_t(std::errc::invalid_argument);
  }
}

error_code_t device_custom::ioctl_enable_io_debugbreak(void* buffer, size_t buffer_size)
{
  hvpp_assert(handler_);
  hvpp_assert(buffer);
  hvpp_assert(buffer_size >= ioctl_write_protected_memory_t::size);

  if (!buffer || buffer_size < ioctl_write_protected_memory_t::size)
  {
    return make_error_code_t(std::errc::invalid_argument);
  }

  ioctl_write_protected_memory_request& req = *((ioctl_write_protected_memory_request*)buffer);

  PEPROCESS target_proc;
  auto status = PsLookupProcessByProcessId(PsGetCurrentProcessId(), &target_proc);
  if (NT_SUCCESS(status)) {
    KAPC_STATE apc;
    KeStackAttachProcess(target_proc, &apc);
    KernelMemCopy((void*)req.dst, (void*)req.src, req.size);
    KeUnstackDetachProcess(&apc);
    ObfDereferenceObject(target_proc);
  }

  return {};
}

NTSTATUS KernelMemCopy(void* dst, void* src, SIZE_T size)
{
  NTSTATUS status = STATUS_SUCCESS;

  KIRQL curreirql;

  curreirql = KeGetCurrentIrql();
  KIRQL irql = curreirql;

  if (curreirql < DISPATCH_LEVEL)
  {
    irql = KeRaiseIrqlToDpcLevel();
  }
  UINT64 cr0 = __readcr0();
  cr0 &= 0xfffffffffffeffff;
  __writecr0(cr0);
  _disable();
  ProbeForRead(src, size, sizeof(UINT8));
  ProbeForWrite(dst, size, sizeof(UINT8));
  RtlCopyMemory(dst, src, size);
  cr0 = __readcr0();
  cr0 |= 0x10000;
  _enable();
  __writecr0(cr0);
  if (curreirql < DISPATCH_LEVEL)
  {
    KeLowerIrql(irql);
  }

  return status;
}
