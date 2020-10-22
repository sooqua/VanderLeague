#pragma once

#include "CObject.h"

#include <chrono>

class COrbWalker {
private:
	CObject* m_pTarget;
	int m_nLastAttackCmdTick;
	int m_nLastMoveCmdTick;
	std::chrono::milliseconds m_lastMoveClickTime;
	bool lastHitOnly;
	bool autoMove;
	bool useAutokey;

public:
	COrbWalker::COrbWalker() : m_pTarget(nullptr), m_nLastAttackCmdTick(0), m_nLastMoveCmdTick(0), m_lastMoveClickTime(0),
		lastHitOnly(true), autoMove(true), useAutokey(true)
	{
	}

public:
	void COrbWalker::tryFindTarget();
	void COrbWalker::drawEvent();
};
