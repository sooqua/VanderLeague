#pragma once

#include "CObject.h"

class COrbWalker {
private:
	CObject* m_pTarget;
	int m_nLastAttackCmdTick;
	int m_nLastMoveCmdTick;
	bool lastHitOnly;
	bool autoMove;

public:
	COrbWalker::COrbWalker() : m_pTarget(nullptr), m_nLastAttackCmdTick(0), m_nLastMoveCmdTick(0),
		lastHitOnly(true), autoMove(true)
	{
	}

public:
	void COrbWalker::tryFindTarget();
	void COrbWalker::drawEvent();
};
