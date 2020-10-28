#pragma once

#include "Utils.h"
#include "BuffEntry.h"

class BuffManager {
private:
	DWORD getStart();
	DWORD getEnd();

public:
	BuffEntry* GetBuffEntryByName(char* BuffName);

	bool IsImmobile(int Type);

	bool isPartOf(char* w1, char* w2);

	bool IsPoisoned();

	bool IsImmobile();
};