#pragma once
#include <Windows.h>
#include "Offsets.h"
#include "Vector.h"
#include "Utils.h"
#include "SpellSlot.h"

class CSpellBook {
public:

	SpellSlot* GetSpellSlotByID(int ID) {
		return *(SpellSlot**)((DWORD)this + 0x508 + (0x4 * ID));

	}

};
