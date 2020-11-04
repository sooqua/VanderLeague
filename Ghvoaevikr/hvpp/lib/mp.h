#pragma once
#include <Windows.h>

BOOL
ForEachLogicalCore(
  void (*CallbackFunction)(void*),
  void* Context
  );
