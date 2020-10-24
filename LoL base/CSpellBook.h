#pragma once

#include "Offsets.h"
#include "Vector.h"
#include "Utils.h"
#include "CSpell.h"
#include "CSpellEntry.h"

#include <Windows.h>

class CSpellBook {
public:
	CSpellEntry* GetActiveSpellEntry();

	CSpell* GetSpellByID(int ID);
};