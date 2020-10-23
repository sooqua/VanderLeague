#pragma once

#include "Vector.h"
#include "CSpellData.h"

class CSpellCastInfo {
public:
	short GetTargetIndex() {
		return *(short*)((DWORD)this + 0xA8);
	}

	/*
	Vector GetSpellEndPos() {
		Vector endPos = *(Vector*)((DWORD)this + 0x80);
		endPos.Y += 100;
		return endPos;
	}
	*/

	Vector GetSpellCastPos() {
		return *(Vector*)((DWORD)this + 0x8C);
	}

	float GetCastDelay() {
		return *(float*)((DWORD)this + 0x4A8);
	}

	char* GetSpellName() {
		return GetStr((DWORD)this + 0x007C);
	}

	CSpellData* GetSpellData() {
		return *(CSpellData**)(this + 0x44);
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
		return *(float*)((DWORD)this + 0x254);
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

	float GetMissileSpeed() {
		return *(float*)((DWORD)this + 0x450);
	}
};