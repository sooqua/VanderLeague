#pragma once
#include "Offsets.h"
#include "Utils.h"

#include <Windows.h>
#include "Vector.h"
#include <vector>

class AIManager
{
public:
	DWORD getStart();

	DWORD getEnd();

	Vector GetVelocity();

	bool IsMoving();

	std::vector<Vector*> getPathList();
};
