#pragma once

#include "Vector.h"
#include "CSpellData.h"

class CSpellCastInfo {
public:
	short GetTargetIndex();

	Vector GetSpellEndPos();

	Vector GetSpellCastPos();

	float GetCastDelay();

	char* GetSpellName();

	CSpellData* GetSpellData();

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

	float GetDelayCastOffsetPercent();

	float GetDelayTotalTimePercent();

	int GetMaxAmmo();
	int GetAmmoUsed();
	float GetAmmoRechargeTime();

	float GetMissileSpeed();
};