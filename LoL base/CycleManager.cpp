#include "CycleManager.h"

#include "CObject.h"
#include "Autokey.h"

#include <vector>

std::vector<CObject*> CycleManager::m_pObjects = *new std::vector<CObject*>();
POINT CycleManager::m_PreviousMousePos = *new POINT();
bool CycleManager::m_bResetMouseAtNextCycle;
bool CycleManager::m_bBlockAllActions;

void CycleManager::NewCycle() {
	m_pObjects.clear();

	CObject object;
	CObject* pObject = object.GetFirstObject();

	while (pObject)
	{
		m_pObjects.push_back(pObject);
		pObject = object.GetNextObject(pObject);
	}

	if (m_bResetMouseAtNextCycle) {
		Autokey::ResetMouse(m_PreviousMousePos.x, m_PreviousMousePos.y);
		m_bResetMouseAtNextCycle = false;
	}

	m_bBlockAllActions = false;
}

std::vector<CObject*> CycleManager::GetObjects() {
	return m_pObjects;
}

void CycleManager::SetBlockAllActions(bool value)
{
	m_bBlockAllActions = value;
}

bool CycleManager::GetBlockAllActions()
{
	return m_bBlockAllActions;
}

void CycleManager::SetPreviousMousePos(POINT pos)
{
	m_PreviousMousePos = pos;
}

POINT CycleManager::GetPreviousMousePos()
{
	return m_PreviousMousePos;
}

void CycleManager::ResetMouseAtNextCycle()
{
	m_bResetMouseAtNextCycle = true;
}
