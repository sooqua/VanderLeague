#pragma once

#include <Windows.h>

#include "ColdHook.h"

class Hvpp {
	PVOID OriginalFunction;
	PVOID OriginalFunctionAligned;
	PVOID OriginalFunctionBackup;
	PVOID OriginalFunctionBackupAligned;
	PVOID FunctionToHook;
	Hook_Info* hookInfo;

public:
	void Hook();

	void Hide();

	void Unhide();

	void* GetOriginalAddress();

	Hvpp(PVOID src, PVOID dst);
};
