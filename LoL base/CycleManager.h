#pragma once

#include "CObject.h"

#include <vector>

static class CycleManager {
	static std::vector<CObject*> m_pObjects;

public:
	static void NewCycle();
	static std::vector<CObject*> GetObjects();
};