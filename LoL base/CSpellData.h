#pragma once

#include "Offsets.h"
#include "Utils.h"

#include <Windows.h>

class CSpellData
{
public:
	float GetSpellWidth();
	char* GetMissileName();
	char* GetSpellName();
	char* GetDescription();
	float GetEffectAmount();
	float GetIncreaseDamage();
	float GetSpellDuration();
	float GetRootDuration();
	float GetIncreaseDamageBonus();
	float GetCoefficient();
	float GetCoefficient2();
	int GetMaxHighlightTargets();
	float GetCooldownTime();
	float GetMissileLineWidth();
	float GetDelayCastOffsetPercent();
	float GetDelayTotalTimePercent();
	int GetMaxAmmo();
	int GetAmmoUsed();
	float GetAmmoRechargeTime();
	float GetMissileSpeed();
	float GetMaxCooldown();
	short GetCasterIndex();
	float GetSpellRange();
	float GetSpellRadius();
};
