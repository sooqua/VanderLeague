#pragma once

#include "SpellData.h"
#include "Vector.h"

#include <Windows.h>

/*
v10.21
internal static class ActiveSpellEntry
{
    public const int ActiveSpellEntryInstance = 0x20;

    public const int ActiveSpellInfo = 0x8;
    public const int CastEndTime = 0x528;
    public const int CasterName = 0x20;
    public const int CastStartTime = 0x10;
    public const int Cooldown = 0x4d4;
    public const int IsSpecialAttack = 0x4E1;
    public const int ManaCost = 0x4f0;
    public const int MissileIndex = 0x14;

    public const int oActiveSpellTargetIndex = 0xc0;
    public const int oIsBasicAttack = 0xBC;
    public const int oIsSpell = 0x4dc;
    public const int SpellEndPos = 0x8c;

    public const int SpellSlot = 0x0C;
    public const int SpellStartPos = 0x80;
    public const int WindupTime = 0x4C0;
}
*/

class CSpellEntry {
public:
	SpellData* GetSpellData();

	bool isAutoAttack();
	int targetID();
	Vector GetStartPos();
	Vector GetEndPos();
};