#pragma once

#include "CObject.h"

#include <vector>
#include <map>
#include <Windows.h>

static class CycleManager {
	static std::vector<CObject*> m_pObjects;
	static POINT m_PreviousMousePos;
	static bool m_bResetMouseAtNextCycle;
	static bool m_bBlockAllActions;
	static std::map<int, bool> m_keyDownWasPrevented;

public:
	static void NewCycle();
	static std::vector<CObject*> GetObjects();
	static void SetBlockAllActions(bool value);
	static bool GetBlockAllActions();
	static void SetPreviousMousePos(POINT pos);
	static POINT GetPreviousMousePos();
	static void ResetMouseAtNextCycle();

	static void SetKeyDownWasPrevented(int key, bool value);
	static bool GetKeyDownWasPrevented(int key);
};