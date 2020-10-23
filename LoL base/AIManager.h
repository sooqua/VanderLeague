#pragma once
#include "Offsets.h"
#include "Utils.h"

#include <Windows.h>
#include "Vector.h"
#include <vector>

class AIManager
{
public:

	DWORD getStart() {
		return *(DWORD*)offsets::aimgr::oAIMGR_NavBegin;
	}

	DWORD getEnd() {
		return *(DWORD*)offsets::aimgr::oAIMGR_NavEnd;
	}

	Vector GetVelocity() {
		return *(Vector*)((DWORD)this + offsets::aimgr::oAIMGR_Velocity);
	}

	bool IsMoving() {
		return *(bool*)((DWORD)this + offsets::aimgr::oAIMGR_IsMoving);
	}

	std::vector<Vector*> getPathList() {
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
};
