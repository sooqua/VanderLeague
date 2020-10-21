#pragma once
#include <Windows.h>
#include "Offsets.h"
#include "Vector.h"
#include "Utils.h"
#include "SpellInfo.h"

class SpellSlot {
public:
	int GetLevel() {
		return *(int*)((DWORD)this + 0x20);
	}

	float GetTime() {
		return *(float*)((DWORD)this + 0x28);
	}

	float GetCD() {
		return *(float*)((DWORD)this + 0x18);
	}

	bool IsReady() {
		if (GetSpellInfo()->GetSpellData()->GetCooldownTime() == 0) return true;
		return false;
	}

	SpellInfo* GetSpellInfo() {
		return (SpellInfo*)((DWORD)this + 0x124);
	}

};