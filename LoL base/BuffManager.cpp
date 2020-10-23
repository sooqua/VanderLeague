#include "BuffManager.h"

BuffEntry* BuffManager::GetBuffEntryByName(char* BuffName) {
	int i = -1;
	for (DWORD pBuffPtr = this->pStart; pBuffPtr != this->pEnd; pBuffPtr += 0x8)
	{
		auto pBuff = *(BuffEntry**)pBuffPtr;
		i++;
		if (!pBuff) continue;
		if (!pBuff->IsValid()) continue;
		if (pBuff->IsAlive()) {
			if (!strcmp(pBuff->GetBuffName(), BuffName)) {
				return pBuff;
			}
		}

	}
	return NULL;
}

bool BuffManager::IsImmobile(int Type) {
	int i = -1;
	for (DWORD pBuffPtr = this->pStart; pBuffPtr != this->pEnd; pBuffPtr += 0x8)
	{
		auto pBuff = *(BuffEntry**)pBuffPtr;
		i++;
		if (!pBuff) continue;
		if (!pBuff->IsValid()) continue;
		if (pBuff->IsAlive()) {
			if (pBuff->getBuffType() == Type)
				return true;
		}

	}
	return false;
}

bool BuffManager::isPartOf(char* w1, char* w2)
{
	int i = 0;
	int j = 0;


	while (w1[i] != '\0') {
		if (w1[i] == w2[j])
		{
			int init = i;
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

bool BuffManager::IsPoisoned() {
	int i = -1;
	for (DWORD pBuffPtr = this->pStart; pBuffPtr != this->pEnd; pBuffPtr += 0x8)
	{
		auto pBuff = *(BuffEntry**)pBuffPtr;
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

bool BuffManager::IsImmobile() {
	if (this->IsImmobile(22) || IsImmobile(8) || IsImmobile(28) || IsImmobile(29) || IsImmobile(5) || IsImmobile(11)) {
		return true;
	}
	return false;
}
