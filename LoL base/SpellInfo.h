#pragma once

#include "Utils.h"
#include "SpellData.h"

class SpellInfo
{
public:
	SpellData* GetSpellData() {
		return (SpellData*)((DWORD)this + 0x38);
	}
};