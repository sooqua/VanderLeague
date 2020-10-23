#pragma once

#include "CSpellInfo.h"

#define oSpellLevel 
#define oSpellTimer 
#define oSpellSpellInfo 

class CSpell {
public:
	union {
		DEFINE_MEMBER_0(void* Base, 0x0);
		DEFINE_MEMBER_N(int Level, 0x20);
		DEFINE_MEMBER_N(float CooldownExpire, 0x28);
		DEFINE_MEMBER_N(int FinalCooldownExpire, 0x64);
		DEFINE_MEMBER_N(int Cooldown, 0x78);
		DEFINE_MEMBER_N(int Charges, 0x58);
		DEFINE_MEMBER_N(CSpellInfo* SpellInfo, 0x134);
	};
};