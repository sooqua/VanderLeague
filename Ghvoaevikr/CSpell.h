#pragma once

#include "CSpellInfo.h"

class CSpell {
public:
	bool IsSpellReady();

	int GetLevel();

	float GetCooldownExpire();

	float GetFinalCooldownExpire();

	float GetCooldown();

	int GetCharges();

	CSpellInfo* GetSpellInfo();
};