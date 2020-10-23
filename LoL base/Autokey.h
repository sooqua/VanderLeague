#pragma once

#include "Vector.h"

static class Autokey {
public:
	static void Click();
	static void ClickThere(Vector pos, int returnAfterMs = 25);
	static void MoveMouse(Vector pos);
};