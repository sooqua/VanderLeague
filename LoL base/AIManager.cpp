#include "AIManager.h"

DWORD AIManager::getStart() {
	return *(DWORD*)offsets::aimgr::oAIMGR_NavBegin;
}

DWORD AIManager::getEnd() {
	return *(DWORD*)offsets::aimgr::oAIMGR_NavEnd;
}

Vector AIManager::GetVelocity() {
	return *(Vector*)((DWORD)this + offsets::aimgr::oAIMGR_Velocity);
}

bool AIManager::IsMoving() {
	return *(bool*)((DWORD)this + offsets::aimgr::oAIMGR_IsMoving);
}

std::vector<Vector*> AIManager::getPathList() {
	std::vector<Vector*> test;
	for (DWORD pBuffPtr = this->getStart(); pBuffPtr != this->getEnd(); pBuffPtr += 0x8)
	{
		auto pBuff = *(Vector**)pBuffPtr;
		if (!pBuff) continue;
		if (pBuff) {
			test.push_back(pBuff);
		}

	}
	return test;
}
