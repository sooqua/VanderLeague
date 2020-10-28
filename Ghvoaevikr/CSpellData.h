#pragma once

#include "Offsets.h"
#include "Utils.h"

#include <Windows.h>

class SpellBooleans //
{
public:
	bool AmmoNotAffectedByCDR; //0x0000
	bool AmmoNotAffectedByCDR2; //0x0001
	bool CostAlwaysShownInUI; //0x0002
	bool CannotBeSuppressed; //0x0003
	bool CanCastWhileDisabled; //0x0004
	bool CanCastOrQueueWhileCasting; //0x0005
	bool CanOnlyCastWhileDisabled; //0x0006
	bool CantCancelWhileWindingUp; //0x0007
	bool CantCancelWhileChanneling; //0x0008
	bool CantCastWhileRooted; //0x0009
	bool ApplyAttackDamage; //0x000A
	bool ApplyAttackEffect; //0x000B
	bool ApplyMaterialOnHitSound; //0x000C
	bool DoesntBreakChannels; //0x000D
	bool BelongsToAvatar; //0x000E
	bool IsDisabledWhileDead; //0x000F
	bool CanOnlyCastWhileDead; //0x0010
	bool CursorChangesInGrass; //0x0011
	bool CursorChangesInTerrain; //0x0012
	bool LineMissileEndsAtTargetPoint; //0x0013
	bool SpellRevealsChampion; //0x0014
	bool LineMissileTrackUnitsAndContinues; //0x0015
	bool UseMinimapTargeting; //0x0016
	bool CastRangeUseBoundingBoxes; //0x0017
	bool MinimapIconRotation; //0x0018
	bool UseChargeChanneling; //0x0019
	bool UseChargeTargeting; //0x001A
	bool CanMoveWhileChanneling; //0x001B
	bool DisableCastBar; //0x001C
	bool ShowChannelBar; //0x001D
	bool AlwaysSnapFacing; //0x001E
	bool UseAnimatorFramerate; //0x001F
	bool HaveHitEffect; //0x0020
	bool HaveHitBone; //0x0021
	bool HaveAfterEffect; //0x0022
	bool HavePointEffect; //0x0023
	bool IsToggleSpell; //0x0024
	bool LineMissileBounces; //0x0025
	bool LineMissileUsesAccelerationForBounce; //0x0026
	bool MissileFollowsTerrainHeight; //0x0027
	bool DoNotNeedToFaceTarget; //0x0028
	bool NoWinddownIfCancelled; //0x0029
	bool IgnoreRangeCheck; //0x002A
	bool OrientRadiusTextureFromPlayer; //0x002B
	bool UseAutoattackCastTime; //0x002C
	bool IgnoreAnimContinueUntilCastFrame; //0x002D
	bool HideRangeIndicatorWhenCasting; //0x002E
	bool UpdateRotationWhenCasting; //0x002F
	bool ConsideredAsAutoAttack; //0x0030
	bool MinimapIconDisplayFlag; //0x0031
}; //Size: 0x0044

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
	float GetManaCostByLevel(unsigned int level);
};
