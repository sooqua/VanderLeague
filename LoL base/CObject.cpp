#include "CObject.h"
#include "Hooks.h"
#include "Engine.h"
#include "Utils.h"

bool CObject::IsAlive() {
	return Functions.IsAlive(this) && this->GetHealth() > 0.0f;
}

bool CObject::IsDead() {
	return CompareObjectTypeFlags((int)ObjectTypeFlags::DeadObject);
}

bool CObject::IsInvalidObject() {
	return CompareObjectTypeFlags((int)ObjectTypeFlags::InvalidObject);
}

bool CObject::IsMinion() {
	return Functions.IsMinion(this);
}

bool CObject::IsTurret() {
	return Functions.IsTurret(this);
}

bool CObject::IsHero() {
	return CompareObjectTypeFlags((int)ObjectTypeFlags::Hero);
}

bool CObject::IsBuilding() {
	return CompareObjectTypeFlags((int)ObjectTypeFlags::Building);
}

bool CObject::IsMissile() {
	return Functions.IsMissile(this);
}

bool CObject::IsWard() {
	return this->GetMaxHealth() == 3.f;
}

bool CObject::IsTargetable() {
	return Functions.IsTargetable(this);
}

CObject* CObject::GetFirstObject()
{
	typedef CObject* (__thiscall* fnGetFirst)(void*);
	return ((fnGetFirst)(baseAddr + offsets::global::oGetFirstObject))(*(void**)(baseAddr + offsets::global::oObjManager));
}

CObject* CObject::GetNextObject(CObject* object)
{
	typedef CObject* (__thiscall* fnGetNext)(void*, CObject*);
	return ((fnGetNext)(baseAddr + offsets::global::oGetNextObject))(*(void**)(baseAddr + offsets::global::oObjManager), object);
}

short CObject::GetIndex() {
	return *(short*)((DWORD)this + offsets::cobject::oObjIndex);
}

DWORD CObject::GetNetworkID() {
	return *(DWORD*)((DWORD)this + offsets::cobject::oObjNetworkID);
}

Vector CObject::GetPos() {
	return *(Vector*)((DWORD)this + offsets::cobject::oObjPos);
}

int CObject::GetLevel() {
	return *(int*)((DWORD)this + offsets::cobject::oObjLevel);
}

float CObject::GetHealth() {
	return *(float*)((DWORD)this + offsets::cobject::oObjHealth);
}

float CObject::GetMaxHealth() {
	return *(float*)((DWORD)this + offsets::cobject::oObjMaxHealth);
}

float CObject::GetBaseAttackDamage() {
	return *(float*)((DWORD)this + offsets::cobject::oObjBaseAtk);
}

float CObject::GetBonusAttackDamage() {
	return *(float*)((DWORD)this + offsets::cobject::oObjBonusAtk);
}

float CObject::GetTotalAttackDamage() {
	return this->GetBonusAttackDamage() + this->GetBaseAttackDamage();
}

float CObject::GetArmor() {
	return *(float*)((DWORD)this + offsets::cobject::oObjArmor);
}

float CObject::GetAttackRange() {
	return *(float*)((DWORD)this + offsets::cobject::oObjAtkRange);
}

bool CObject::IsVisible() {
	return *(bool*)((DWORD)this + offsets::cobject::oObjVisibility);
}

float CObject::GetBoundingRadius()
{
	typedef float(__thiscall* OriginalFn)(PVOID);
	return CallVirtual<OriginalFn>(this, 36)(this);
}

bool CObject::IsEnemyTo(CObject* objToCheck)
{
	if (objToCheck->GetTeam() == 100 && this->GetTeam() == 200)
		return true;
	else if (objToCheck->GetTeam() == 200 && this->GetTeam() == 100)
		return true;

	return false;
}

bool CObject::IsNeutral() {
	return this->GetTeam() == 300;
}

char* CObject::GetName() {
	return GetStr((DWORD)this + offsets::cobject::oObjName);
}

char* CObject::GetChampionName() {
	return GetStr((DWORD)this + offsets::cobject::oObjChampionName);
}

int CObject::GetTeam() {
	return *(int*)((DWORD)this + offsets::cobject::oObjTeam);
}

CSpellBook* CObject::GetSpellBook() {
	return (CSpellBook*)((DWORD)this + offsets::cobject::oObjSpellBook);
}

bool CObject::IsNexus() {
	return Functions.IsNexus(this);
}

bool CObject::IsInhibitor() {
	return Functions.IsInhibitor(this);
}

bool CObject::IsTroyEnt() {
	return Functions.IsTroyEnt(this);
}

float CObject::GetAttackDelay() {
	return Functions.GetAttackDelay(this);
}

float CObject::GetAttackCastDelay() {
	return Functions.GetAttackCastDelay(this);
}

float CObject::GetEffectiveHealth() {
	return this->GetHealth() * (100.0f + this->GetArmor()) / 100.0f;
}

float CObject::GetEffectiveDamageOnTarget(CObject* target) {
	return this->GetTotalAttackDamage() * (100 / (100 + target->GetArmor()));
}

float CObject::GetDistanceToMe() {
	return this->GetPos().DistTo(Engine::GetLocalObject()->GetPos());
}

float CObject::GetTrueAttackRange() {
	return this->GetAttackRange() + this->GetBoundingRadius();
}