#pragma once

#include "Offsets.h"
#include "Utils.h"

#include <Windows.h>

class CSpellData
{
public:
	char* GetMissileName() {
		return GetStr((DWORD)this + 0x58);
	}

	char* GetSpellName() {
		return GetStr((DWORD)this + 0x7C);
	}

	char* GetDescription() {
		return GetStr((DWORD)this + 0x0088);
	}

	float GetEffectAmount() {
		return *(float*)((DWORD)this + 0xD0);
	}

	float GetIncreaseDamage() {
		return *(float*)((DWORD)this + 0xEC);
	}

	float GetSpellDuration() {
		return *(float*)((DWORD)this + 0x108);
	}

	float GetRootDuration() {
		return *(float*)((DWORD)this + 0x15C);
	}

	float GetIncreaseDamageBonus() {
		return *(float*)((DWORD)this + 0x178);
	}

	float GetCoefficient() {
		return *(float*)((DWORD)this + 0x200);
	}

	float GetCoefficient2() {
		return *(float*)((DWORD)this + 0x204);
	}

	int GetMaxHighlightTargets() {
		return *(int*)((DWORD)this + 0x208);
	}

	float GetCooldownTime() {
		return *(float*)((DWORD)this + 0x280);
	}

	float GetMissileLineWidth()
	{
		return  *(float*)((DWORD)this + 0x44C); // 0x458? 0x484?
	}
	
	float GetDelayCastOffsetPercent() {
		return *(float*)((DWORD)this + 0x29C);
	}

	float GetDelayTotalTimePercent() {
		return *(float*)((DWORD)this + 0x2A0);
	}

	int GetMaxAmmo() {
		return *(int*)((DWORD)this + 0x31C);
	}

	int GetAmmoUsed() {
		return *(int*)((DWORD)this + 0x338);
	}

	float GetAmmoRechargeTime() {
		return *(float*)((DWORD)this + 0x354);
	}

	float GetMissileSpeed()
	{
		return  *(float*)((DWORD)this + 0x424); // 0x450?
	}

	float GetMaxCooldown()
	{
		return  *(float*)((DWORD)this + 0x25C);
	}

	short GetCasterIndex() {
		return *(short*)((DWORD)this + offsets::cobject::oObjSourceIndex);
	}
};
