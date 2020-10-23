#pragma once
#include <Windows.h>
#include "Offsets.h"
#include "Vector.h"
#include "Utils.h"
#include "CSpellBook.h"
#include "CSpellCastInfo.h"

enum class ObjectTypeFlags {
	GameObject = (1 << 0),  //0x1
	NeutralCamp = (1 << 1),  //0x2
	DeadObject = (1 << 4),  //0x10
	InvalidObject = (1 << 5),  //0x20
	AIBaseCommon = (1 << 7),  //0x80
	AttackableUnit = (1 << 9),  //0x200
	AI = (1 << 10), //0x400
	Minion = (1 << 11), //0x800
	Hero = (1 << 12), //0x1000
	Turret = (1 << 13), //0x2000
	Unknown0 = (1 << 14), //0x4000
	Missile = (1 << 15), //0x8000
	Unknown1 = (1 << 16), //0x10000
	Building = (1 << 17), //0x20000
	Unknown2 = (1 << 18), //0x40000
};

class CObject {
public:
	bool IsTurret();
	bool IsMinion();
	bool IsAlive();
	bool IsDead();
	bool IsInvalidObject();
	bool IsHero();
	bool IsBuilding();
	bool IsMissile();
	bool IsWard();
	bool IsNexus();
	bool IsInhibitor();
	bool IsTroyEnt();
	bool IsTargetable();
	bool IsNeutral();
	bool IsVisible();
	bool IsEnemyTo(CObject* objToCheck);

	bool CObject::CompareObjectTypeFlags(int objectTypeFlag)
	{
		unsigned __int8* v2; // edi
		unsigned int v3; // edx
		unsigned int v4; // esi
		int* v5; // ecx
		int v6; // eax
		unsigned __int8 v7; // al
		unsigned int v8; // eax
		unsigned __int8* v9; // edx
		char v10; // cl
		int objectId; // [esp+8h] [ebp-4h]

		v2 = (unsigned __int8*)this;
		v3 = 0;
		v4 = *(BYTE*)(v2 + 81);

		objectId = *(DWORD*)&v2[4 * v2[88] + 92];
		if (v4)
		{
			v5 = (int*)(this + 84);
			do
			{
				v6 = *v5;
				++v5;
				*(&objectId + v3) ^= ~v6;
				++v3;
			} while (v3 < v4);
		}
		v7 = v2[82];
		if (v7)
		{
			v8 = 4 - v7;
			if (v8 < 4)
			{
				v9 = &v2[v8 + 84];
				do
				{
					v10 = *v9++;
					*((BYTE*)&objectId + v8++) ^= ~v10;
				} while (v8 < 4);
			}
		}

		return (objectId & objectTypeFlag) != 0;
	}
	CObject* GetFirstObject();
	CObject* GetNextObject(CObject* object);

	short GetIndex();
	DWORD GetNetworkID();
	Vector GetPos();
	int GetLevel();
	float GetHealth();
	float GetMaxHealth();
	float GetBaseAttackDamage();
	float GetBonusAttackDamage();
	float GetTotalAttackDamage();
	float GetArmor();
	float GetAttackRange();
	float GetBoundingRadius();
	char* GetName();
	char* GetChampionName();
	int GetTeam();
	CSpellBook* GetSpellBook();
	float GetAttackDelay();
	float GetAttackCastDelay();
	float GetEffectiveHealth();
	float GetEffectiveDamageOnTarget(CObject* target);
	float GetDistanceToMe();
	float GetTrueAttackRange();

	// Spell related
	CSpellCastInfo* GetSpellCastInfo();
	Vector GetMissileEndPos();
	Vector GetMissileStartPos();
	float GetSpellSpeed();
	short GetMissileSourceIndex();
	short GetMissileTargetIndex();
};
