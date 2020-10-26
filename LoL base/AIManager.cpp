#include "AIManager.h"

Vector* AIManager::GetNavBegin() {
	return *(Vector**)((DWORD)this + offsets::aimgr::oAIMGR_NavBegin);
}

Vector* AIManager::GetNavEnd() {
	return (Vector*)((DWORD)this + offsets::aimgr::oAIMGR_NavEnd);
}

Vector AIManager::GetVelocity() {
	return *(Vector*)((DWORD)this + offsets::aimgr::oAIMGR_Velocity);
}

bool AIManager::IsMoving() {
	return *(bool*)((DWORD)this + offsets::aimgr::oAIMGR_IsMoving);
}

//std::vector<Vector*> AIManager::GetPathList() {
//	std::vector<Vector*> test;
//	for (DWORD pBuffPtr = this->GetNavBegin(); pBuffPtr != this->GetNavEnd(); pBuffPtr += 0x8)
//	{
//		auto pBuff = *(Vector**)pBuffPtr;
//		if (!pBuff) continue;
//		if (pBuff) {
//			test.push_back(pBuff);
//		}
//
//	}
//	return test;
//}
