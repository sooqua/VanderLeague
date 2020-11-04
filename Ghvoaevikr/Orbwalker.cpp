#include "Orbwalker.h"
#include "Engine.h"
#include "Autokey.h"
#include "CycleManager.h"
#include "Script.h"

#include <vector>
#include <algorithm>
#include <chrono>

#include "Debug.h"

// AttackCastDelay = time until u can move without canceling attack
// AttackDelay = time until u can attack again

void COrbWalker::tryFindTarget()
{
	CObject* pLocal = Engine::GetLocalObject();
	if (!pLocal)
	{
		m_pTarget = nullptr;
		return;
	}

	if (m_pTarget != nullptr)
	{
		if (m_pTarget->IsAlive() && m_pTarget->IsTargetable() && m_pTarget->IsVisible() && !m_pTarget->IsInvalidObject())
		{
			if (m_pTarget->GetDistanceToMe() <= pLocal->GetAttackRange() + 2 * pLocal->GetBoundingRadius())
			{
				return;
			}
		}
		m_pTarget = nullptr;
	}

	bool heroesInRange = false;

	std::vector<CObject*> possibleTargets;
	for (auto pObject : CycleManager::GetObjects())
	{
		if (pObject != nullptr && pObject->IsEnemyTo(pLocal))
		{
			if (pObject->IsHero() || pObject->IsMinion() || pObject->IsTurret())
			{
				if (pObject->IsAlive() && pObject->IsTargetable() && pObject->IsVisible() && !pObject->IsInvalidObject() &&
					pLocal->GetPos().DistTo(pObject->GetPos()) <= pLocal->GetAttackRange() + 2 * pLocal->GetBoundingRadius())
				{
					if (pObject->IsHero())
						heroesInRange = true;
					possibleTargets.push_back(pObject);
				}
			}
		}
	}

	if (possibleTargets.empty())
		return;

	if (possibleTargets.size() > 1)
	{
		std::sort(possibleTargets.begin(), possibleTargets.end(),
			[](CObject* pFirst, CObject* pSecond)
			{
				return (pFirst->GetHealth() < pSecond->GetHealth());
			});

		if (preferHarassOverFarm && heroesInRange)
		{
			static_cast<void>(std::remove_if(possibleTargets.begin(), possibleTargets.end(),
				[](CObject* pObject)
				{
					return !(pObject->IsHero());
				}));
		}
	}

	m_pTarget = possibleTargets.front();

	auto notEnoughDmgToKill = (pLocal->GetEffectiveDamageOnTarget(m_pTarget) < m_pTarget->GetHealth());
	if ((lastHitOnly && notEnoughDmgToKill) && !(preferHarassOverFarm && heroesInRange)) {
		m_pTarget = nullptr;
	}

	possibleTargets.clear();
}

CObject* COrbWalker::GetNearestToMouseMinion() {
	auto pLocal = Engine::GetLocalObject();

	POINT mousePos;
	GetCursorPos(&mousePos);

	std::vector<CObject*> newObjects;
	for (auto pObject : CycleManager::GetObjects()) {
		if (pObject != nullptr)
		{
			if (pObject->IsEnemyTo(pLocal) && pObject->IsMinion() && pObject->IsAlive()
				&& pObject->IsTargetable() && pObject->IsVisible() && !pObject->IsInvalidObject())
			{
				Vector objPos_w2s;
				auto objPos = pObject->GetPos();
				Functions.WorldToScreen(&objPos, &objPos_w2s);
				if (calculate2dDistance(
					objPos_w2s.X,
					objPos_w2s.Y,
					static_cast<float>(mousePos.x),
					static_cast<float>(mousePos.y))
					< pObject->GetBoundingRadius()) {
					newObjects.push_back(pObject);
				}
			}
		}
	}
	if (newObjects.empty()) {
		return nullptr;
	}
	if (newObjects.size() > 1) {
		std::sort(newObjects.begin(), newObjects.end(),
			[](CObject* pFirst, CObject* pSecond)
			{
				return (pFirst->GetDistanceToMe() < pSecond->GetDistanceToMe());
			});
	}
	auto target = newObjects.front();
	return target;
}

void COrbWalker::drawEvent()
{
	if (CycleManager::GetBlockAllActions()) return;
	CObject* pLocal = Engine::GetLocalObject();

	tryFindTarget();

	if (championScript) {
		championScript->Harass();
	}

	if (m_pTarget != nullptr) {
		float flTimeTilAttack = pLocal->GetAttackDelay();
		int nTicksTilAttack = (int)(flTimeTilAttack * 1000);
		if ((int)(Engine::GetGameTime() * 1000) >= (m_nLastAttackCmdTick + nTicksTilAttack))
		{
			if (useAutokey) {
				Autokey::MoveMouse(m_pTarget->GetPos());
				CycleManager::ResetMouseAtNextCycle();
				Autokey::Click();
			}
			else {
				Engine::AttackTarget(m_pTarget);
			}

			m_nLastAttackCmdTick = (int)(Engine::GetGameTime() * 1000);
		}
		else
		{
			float flTimeToMove = pLocal->GetAttackCastDelay();
			int nTicksToMove = (int)(flTimeToMove * 1000) + 35;

			if ((int)(Engine::GetGameTime() * 1000) >= (m_nLastAttackCmdTick + nTicksToMove)
				&& (int)(Engine::GetGameTime() * 1000) >= m_nLastMoveCmdTick + 60)
			{
				if (autoMove) {
					if (useAutokey) {
						std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(
							std::chrono::system_clock::now().time_since_epoch());
						if (now >= m_lastMoveClickTime + std::chrono::milliseconds(100)) {
							auto nearestToMouseMinion = GetNearestToMouseMinion();
							if (nearestToMouseMinion == nullptr) {
								Autokey::Click();
							}
							m_lastMoveClickTime = now;
						}
					}
					else {
						auto mouseWorldPos = Engine::GetMouseWorldPosition();
						Engine::MoveTo(&mouseWorldPos);
					}
				}
				m_nLastMoveCmdTick = (int)(Engine::GetGameTime() * 1000);
			}
		}

		//auto color = createRGB(255, 255, 255);
		//auto targetPos = m_pTarget->GetPos();
		//Functions.DrawCircle(&targetPos, m_pTarget->GetBoundingRadius(), &color, 0, 10.0f, 0, .8f);
	}
	else
	{
		if ((int)(Engine::GetGameTime() * 1000) >= (m_nLastMoveCmdTick + 60))
		{
			if (autoMove) {
				if (useAutokey) {
					std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::system_clock::now().time_since_epoch());
					if (now >= m_lastMoveClickTime + std::chrono::milliseconds(100)) {
						auto nearestToMouseMinion = GetNearestToMouseMinion();
						if (nearestToMouseMinion == nullptr) {
							Autokey::Click();
						}
						m_lastMoveClickTime = now;
					}
				}
				else {
					auto mouseWorldPos = Engine::GetMouseWorldPosition();
					Engine::MoveTo(&mouseWorldPos);
				}
			}
			m_nLastMoveCmdTick = (int)(Engine::GetGameTime() * 1000);
		}
		m_nLastAttackCmdTick = 0;
	}
}