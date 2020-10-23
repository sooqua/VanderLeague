#include "CycleManager.h"

#include "CObject.h"

#include <vector>

std::vector<CObject*> CycleManager::m_pObjects = *new std::vector<CObject*>();

void CycleManager::NewCycle() {
	m_pObjects.clear();

	CObject object;
	CObject* pObject = object.GetFirstObject();

	while (pObject)
	{
		m_pObjects.push_back(pObject);
		pObject = object.GetNextObject(pObject);
	}
}

std::vector<CObject*> CycleManager::GetObjects() {
	return m_pObjects;
}
