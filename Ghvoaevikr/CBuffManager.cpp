#include "CBuffManager.h"

DWORD CBuffManager::GetStart()
{
	return *(DWORD*)(this + 0x10);
}

DWORD CBuffManager::GetEnd()
{
	return *(DWORD*)(this + 0x14);
}

CBuffEntry* CBuffManager::GetBuffEntryByName(char* BuffName) {
	int i = -1;
	for (DWORD pBuffPtr = this->GetStart(); pBuffPtr != this->GetEnd(); pBuffPtr += 0x8)
	{
		auto pBuff = *(CBuffEntry**)pBuffPtr;
		i++;
		if (!pBuff) continue;
		if (!pBuff->IsValid()) continue;
		if (pBuff->IsAlive()) {
			if (stristr(pBuff->GetBuffName(), BuffName)) {
				return pBuff;
			}
		}

	}
	return NULL;
}

bool CBuffManager::IsImmobile(int Type) {
	int i = -1;
	for (DWORD pBuffPtr = this->GetStart(); pBuffPtr != this->GetEnd(); pBuffPtr += 0x8)
	{
		auto pBuff = *(CBuffEntry**)pBuffPtr;
		i++;
		if (!pBuff) continue;
		if (!pBuff->IsValid()) continue;
		if (pBuff->IsAlive()) {
			if (pBuff->GetBuffType() == Type)
				return true;
		}

	}
	return false;
}

bool CBuffManager::isPartOf(char* w1, char* w2)
{
	int i = 0;
	int j = 0;


	while (w1[i] != '\0') {
		if (w1[i] == w2[j])
		{
			while (w1[i] == w2[j] && w2[j] != '\0')
			{
				j++;
				i++;
			}
			if (w2[j] == '\0') {
				return true;
			}
			j = 0;
		}
		i++;
	}
	return false;
}

bool CBuffManager::IsPoisoned() {
	int i = -1;
	for (DWORD pBuffPtr = this->GetStart(); pBuffPtr != this->GetEnd(); pBuffPtr += 0x8)
	{
		auto pBuff = *(CBuffEntry**)pBuffPtr;
		i++;
		if (!pBuff) continue;
		if (!pBuff->IsValid()) continue;
		if (pBuff->IsAlive()) {
			if (isPartOf(pBuff->GetBuffName(), "oison"))
				return true;
		}

	}
	return false;
}

bool CBuffManager::IsImmobile() {
	if (this->IsImmobile(22) || IsImmobile(8) || IsImmobile(28) || IsImmobile(29) || IsImmobile(5) || IsImmobile(11)) {
		return true;
	}
	return false;
}
