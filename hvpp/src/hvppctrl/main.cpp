#include <cstdio>
#include <cstdint>
#include <windows.h>

#include "Hvpp.h"

#include "../hvpp/hvpp/lib/ioctl.h"

typedef struct ioctl_write_protected_memory_request_t {
  uint32_t src;
  uint32_t dst;
  uint32_t size;
} ioctl_write_protected_memory_request;

using ioctl_write_protected_memory_t = ioctl_read_write_t<1, sizeof(ioctl_write_protected_memory_request_t)>;

#define PAGE_SIZE       4096
#define PAGE_ALIGN(Va)  ((PVOID)((ULONG_PTR)(Va) & ~(PAGE_SIZE - 1)))

using pfnTestFunc = void(*)();

Hvpp testHvpp;
Hvpp test2Hvpp;

void TestFunc() {
  MessageBoxA(NULL, "Test", "", MB_OK);
}
void TestFuncHooked() {
  MessageBoxA(NULL, "Test Hooked!", "", MB_OK);
  ((pfnTestFunc)(testHvpp.GetOriginalAddress()))();
}

void TestFunc2() {
  MessageBoxA(NULL, "Test 2", "", MB_OK);
}
void TestFunc2Hooked() {
  MessageBoxA(NULL, "Test 2 Hooked!", "", MB_OK);
  ((pfnTestFunc)(test2Hvpp.GetOriginalAddress()))();
}


void TestHook()
{
  testHvpp.Init(TestFunc, TestFuncHooked);
  test2Hvpp.Init(TestFunc2, TestFunc2Hooked);

  //
  // No hooks, no hiding - call the original function.
  //
  TestFunc();
  TestFunc2();
  testHvpp.Disassemble();
  test2Hvpp.Disassemble();
  printf("\n");

  //
  // Hook the function and call it.  If the hooking was successful,
  // HookCallCount should be already incremented.
  //
  printf("Hook:\n");
  if (!ColdHook_Service::ServiceGlobalInit(nullptr)) {
    printf("Could not initialize ColdHook global service\n");
  }
  testHvpp.Hook();
  test2Hvpp.Hook();
  TestFunc();
  TestFunc2();
  testHvpp.Disassemble();
  test2Hvpp.Disassemble();
  printf("\n");

  //
  // Hide the hook and instruct the hypervisor to return memory of
  // "OriginalFunctionBackupAligned" when read    is requested and
  // "OriginalFunctionAligned"       when execute is requested.
  //
  // The output of Disassemble() should be same as when we called it
  // without any hooks, but the call of the function should be detoured
  // to our hook function - therefore HookCallCount should be incremented
  // again.
  //
  printf("Hide:\n");
  testHvpp.Hide();
  test2Hvpp.Hide();
  TestFunc();
  TestFunc2();
  testHvpp.Disassemble();
  test2Hvpp.Disassemble();
  printf("\n");

  //
  // Unhide the hook and instruct the hypervisor to return original
  // pages for both read and execute.  The output of Disassemble()
  // should be same as when we called it after hooking and the call
  // should be still detoured to our hook function.  HookCallCount
  // should be incremented again.
  //
  printf("Unhide:\n");
  Hvpp::GlobalUnhide();
  TestFunc();
  TestFunc2();
  testHvpp.Disassemble();
  test2Hvpp.Disassemble();
  printf("\n");

  //
  // Unhook the function.  Disassemble() should be same as when we
  // called it without any hooks and the call of the function should
  // call the original function - therefore HookCallCount shouldn't
  // be incremented this time.
  //
  printf("Unhook:\n");
  ColdHook_Service::ServiceGlobalShutDown(true, nullptr);
  TestFunc();
  testHvpp.Disassemble();
  test2Hvpp.Disassemble();
  printf("\n");
}

void TestIoControl()
{
  HANDLE DeviceHandle;

  DeviceHandle = CreateFile(TEXT("\\\\.\\hvpp"),
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL);

  if (DeviceHandle == INVALID_HANDLE_VALUE)
  {
    printf("Error while opening 'hvpp' device!\n");
    return;
  }

  //
  // Issue IOCTL call to the driver.
  // When kernel debugger is attached, this IOCTL will instruct
  // the hypervisor to set one-time breakpoint when IN/OUT
  // instruction from/to port 0x64 (keyboard I/O port) is executed.
  //
  // See hvpp/device_custom.cpp.
  //

  const char* test = "1234567890";
  char buffer[11];

  ioctl_write_protected_memory_request req = { 0 };
  req.src = (uint32_t)test;
  req.dst = (uint32_t)(&buffer);
  req.size = sizeof(buffer);

  DWORD BytesReturned;
  DeviceIoControl(
    DeviceHandle,
    ioctl_write_protected_memory_t::code,
    &req,
    sizeof(req),
    &req,
    sizeof(req),
    &BytesReturned,
    NULL);

  CloseHandle(DeviceHandle);

  printf("IOCTL return value: %s (size: %u)\n", buffer, BytesReturned);
}

int main()
{
  //TestCpuid();
  //TestHook();
  TestIoControl();

  return 0;
}

