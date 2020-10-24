#include "CSpell.h"

#include "Engine.h"

bool CSpell::IsSpellReady() {
	return GetLevel() >= 1 && (Engine::GetGameTime() >= GetCooldownExpire());
}

int CSpell::GetLevel() {
	return *(int*)((DWORD)this + 0x20);
}

float CSpell::GetCooldownExpire() {
	return *(float*)((DWORD)this + 0x28);
}

float CSpell::GetFinalCooldownExpire() {
	return *(float*)((DWORD)this + 0x64);
}

float CSpell::GetCooldown() {
	return *(float*)((DWORD)this + 0x78);
}

int CSpell::GetCharges() {
	return *(int*)((DWORD)this + 0x58);
}

CSpellInfo* CSpell::GetSpellInfo() {
	return (CSpellInfo*)((DWORD)this + 0x134);
}
