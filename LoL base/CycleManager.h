#pragma once

#include "CObject.h"

#include <vector>
#include <Windows.h>

static class CycleManager {
	static std::vector<CObject*> m_pObjects;
	static POINT m_PreviousMousePos;
	static bool m_bResetMouseAtNextCycle;
	static bool m_bBlockAllActions;

public:
	static void NewCycle();
	static std::vector<CObject*> GetObjects();
	static void SetBlockAllActions(bool value);
	static bool GetBlockAllActions();
	static void SetPreviousMousePos(POINT pos);
	static POINT GetPreviousMousePos();
	static void ResetMouseAtNextCycle();
};