#pragma once

#include "Utils.h"
#include "BuffEntry.h"

class BuffManager {
private:
	union {
		DEFINE_MEMBER_0(void* base, 0x0);
		DEFINE_MEMBER_N(DWORD pStart, 0x10);
		DEFINE_MEMBER_N(DWORD pEnd, 0x14);
	};

public:
	BuffEntry* GetBuffEntryByName(char* BuffName);

	bool IsImmobile(int Type);

	bool isPartOf(char* w1, char* w2);

	bool IsPoisoned();

	bool IsImmobile();
};