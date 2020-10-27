#pragma once

#include "Vector.h"

class Autokey {
public:
	static void ResetMouse(int x, int y);
	static UINT PressKey(WORD scanCode);
	static UINT ReleaseKey(WORD scanCode);
	static void Click();
	static void ClickThere(Vector pos, int returnAfterMs = 25);
	static void MoveMouse(Vector pos);
};