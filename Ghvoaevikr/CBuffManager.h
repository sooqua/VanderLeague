#pragma once

#include "Utils.h"
#include "CBuffEntry.h"

class CBuffManager {
public:
	DWORD GetStart();
	DWORD GetEnd();

	CBuffEntry* GetBuffEntryByName(char* BuffName);

	bool IsImmobile(int Type);

	bool isPartOf(char* w1, char* w2);

	bool IsPoisoned();

	bool IsImmobile();
};