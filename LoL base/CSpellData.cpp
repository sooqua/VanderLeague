#include "CSpellData.h"

#include <Windows.h>

float CSpellData::GetSpellWidth() {
	return *(float*)(DWORD)this + 0x44C;
}

char* CSpellData::GetMissileName() {
	return GetStr((DWORD)this + 0x58);
}

char* CSpellData::GetSpellName() {
	return GetStr((DWORD)this + 0x7C);
}

char* CSpellData::GetDescription() {
	return GetStr((DWORD)this + 0x0088);
}

float CSpellData::GetEffectAmount() {
	return *(float*)((DWORD)this + 0xD0);
}

float CSpellData::GetIncreaseDamage() {
	return *(float*)((DWORD)this + 0xEC);
}

float CSpellData::GetSpellDuration() {
	return *(float*)((DWORD)this + 0x108);
}

float CSpellData::GetRootDuration() {
	return *(float*)((DWORD)this + 0x15C);
}

float CSpellData::GetIncreaseDamageBonus() {
	return *(float*)((DWORD)this + 0x178);
}

float CSpellData::GetCoefficient() {
	return *(float*)((DWORD)this + 0x1D4);
}

float CSpellData::GetCoefficient2() {
	return *(float*)((DWORD)this + 0x1D8);
}

int CSpellData::GetMaxHighlightTargets() {
	return *(int*)((DWORD)this + 0x1DC);
}

float CSpellData::GetCooldownTime() {
	return *(float*)((DWORD)this + 0x280);
}

float CSpellData::GetMissileLineWidth()
{
	return  *(float*)((DWORD)this + 0x44C);
}

float CSpellData::GetDelayCastOffsetPercent() {
	return *(float*)((DWORD)this + 0x26C);
}

float CSpellData::GetDelayTotalTimePercent() {
	return *(float*)((DWORD)this + 0x2A0);
}

int CSpellData::GetMaxAmmo() {
	return *(int*)((DWORD)this + 0x31C);
}

int CSpellData::GetAmmoUsed() {
	return *(int*)((DWORD)this + 0x338);
}

float CSpellData::GetAmmoRechargeTime() {
	return *(float*)((DWORD)this + 0x354);
}

float CSpellData::GetMissileSpeed()
{
	return  *(float*)((DWORD)this + 0x424);
}

float CSpellData::GetMaxCooldown()
{
	return  *(float*)((DWORD)this + 0x25C);
}

short CSpellData::GetCasterIndex() {
	return *(short*)((DWORD)this + offsets::cobject::oObjSourceIndex);
}

float CSpellData::GetSpellRange()
{
	return *(float*)((DWORD)this + 0x398);
}

float CSpellData::GetSpellRadius()
{
	return *(float*)((DWORD)this + 0x458);
}
