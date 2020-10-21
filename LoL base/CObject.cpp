#include "CObject.h"
#include "Hooks.h"
#include "Engine.h"

bool CObject::IsAlive() {
	return Functions.IsAlive(this) && this->GetHealth() > 0.0f;
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

bool CObject::IsMissile() {
	return Functions.IsMissile(this);
}

bool CObject::IsTargetable() {
	return Functions.IsTargetable(this);
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
