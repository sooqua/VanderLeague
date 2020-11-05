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

	bool initialized;

public:
	void Disassemble();

	void Hook();

	void Hide();

	static void GlobalUnhide();

	void* GetOriginalAddress();

	void Init(PVOID src, PVOID dst);

	Hvpp();
	~Hvpp();
};
