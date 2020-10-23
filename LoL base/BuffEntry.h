#pragma once

#include "Offsets.h"

#include <Windows.h>
#include <stdint.h>

#define IS_NULL_RETN(OBJECT, ADDR, RETN) if ((DWORD*)(OBJECT + ADDR) == nullptr) { return RETN; }

enum BuffType {
	Internal = 0,
	Aura = 1,
	CombatEnchancer = 2,
	CombatDehancer = 3,
	SpellShield = 4,
	Stun = 5,
	Invisibility = 6,
	Silence = 7,
	Taunt = 8,
	Polymorph = 9,
	Slow = 10,
	Snare = 11,
	Damage = 12,
	Heal = 13,
	Haste = 14,
	SpellImmunity = 15,
	PhysicalImmunity = 16,
	Invulnerability = 17,
	Sleepp = 18,
	NearSight = 19,
	Frenzy = 20,
	Fear = 21,
	Charm = 22,
	Poison = 23,
	Suppression = 24,
	Blind = 25,
	Counter = 26,
	Shred = 27,
	Flee = 28,
	Knockup = 29,
	Knockback = 30,
	Disarm = 31
};

class BuffEntry
{
public:
	DWORD strptr;

	BYTE getBuffType();
	bool isBuffEmpty();
	bool IsAlive();
	bool IsValid();

	float GetBuffStartTime();

	float GetBuffEndTime();

	int GetBuffCountAlt();

	float GetBuffCountFloat();

	int GetBuffCountInt();

	char* GetBuffName();
};