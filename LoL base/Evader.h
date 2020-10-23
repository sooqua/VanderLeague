#pragma once

#include "CObject.h"

#include <chrono>
#include <map>

class CEvader {
private:
	std::map<CObject*, bool> m_isDodged;
	int m_nLastMoveCmdTick;
	std::chrono::milliseconds m_lastMoveClickTime;
	bool useAutokey;

public:
	CEvader::CEvader() : m_nLastMoveCmdTick(0), m_lastMoveClickTime(0),
		useAutokey(true)
	{
	}

public:
	bool CEvader::drawEvent();
};
