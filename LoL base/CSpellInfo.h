#pragma once

#include "CSpellData.h"
\
class CSpellInfo {
public:
	union {
		DEFINE_MEMBER_0(DWORD Base, 0x0);
		DEFINE_MEMBER_N(CSpellData* SpellData, 0x44);
	};
};