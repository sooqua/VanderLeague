#include "CSpellCastInfo.h"

short CSpellCastInfo::GetTargetIndex() {
	return *(short*)((DWORD)this + 0xA8);
}

Vector CSpellCastInfo::GetSpellEndPos() {
	Vector endPos = *(Vector*)((DWORD)this + 0x80);
	endPos.Y += 100;
	return endPos;
}

Vector CSpellCastInfo::GetSpellCastPos() {
	return *(Vector*)((DWORD)this + 0x8C);
}

float CSpellCastInfo::GetCastDelay() {
	return *(float*)((DWORD)this + 0x4A8);
}

char* CSpellCastInfo::GetSpellName() {
	return GetStr((DWORD)this + 0x007C);
}

CSpellData* CSpellCastInfo::GetSpellData() {
	return *(CSpellData**)(this + 0x44);
}

char* CSpellCastInfo::GetDescription() {
	return GetStr((DWORD)this + 0x0088);
}

float CSpellCastInfo::GetEffectAmount() {
	return *(float*)((DWORD)this + 0xD0);
}

float CSpellCastInfo::GetIncreaseDamage() {
	return *(float*)((DWORD)this + 0xEC);
}

float CSpellCastInfo::GetSpellDuration() {
	return *(float*)((DWORD)this + 0x108);
}

float CSpellCastInfo::GetRootDuration() {
	return *(float*)((DWORD)this + 0x15C);
}

float CSpellCastInfo::GetIncreaseDamageBonus() {
	return *(float*)((DWORD)this + 0x178);
}

float CSpellCastInfo::GetCoefficient() {
	return *(float*)((DWORD)this + 0x200);
}

float CSpellCastInfo::GetCoefficient2() {
	return *(float*)((DWORD)this + 0x204);
}

int CSpellCastInfo::GetMaxHighlightTargets() {
	return *(int*)((DWORD)this + 0x208);
}

float CSpellCastInfo::GetCooldownTime() {
	return *(float*)((DWORD)this + 0x254);
}

float CSpellCastInfo::GetDelayCastOffsetPercent() {
	return *(float*)((DWORD)this + 0x29C);
}

float CSpellCastInfo::GetDelayTotalTimePercent() {
	return *(float*)((DWORD)this + 0x2A0);
}

int CSpellCastInfo::GetMaxAmmo() {
	return *(int*)((DWORD)this + 0x31C);
}

int CSpellCastInfo::GetAmmoUsed() {
	return *(int*)((DWORD)this + 0x338);
}

float CSpellCastInfo::GetAmmoRechargeTime() {
	return *(float*)((DWORD)this + 0x354);
}

float CSpellCastInfo::GetMissileSpeed() {
	return *(float*)((DWORD)this + 0x450);
}
