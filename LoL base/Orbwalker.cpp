#include "Orbwalker.h"
#include "Engine.h"
#include "Autokey.h"
#include "CycleManager.h"

#include <vector>
#include <algorithm>
#include <chrono>

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
	}

	m_pTarget = possibleTargets.front();

	if (lastHitOnly && (pLocal->GetEffectiveDamageOnTarget(m_pTarget) < m_pTarget->GetHealth())) {
		m_pTarget = nullptr;
	}

	possibleTargets.clear();
}

void COrbWalker::drawEvent()
{
	CObject* pLocal = Engine::GetLocalObject();

	tryFindTarget();

	if (m_pTarget != nullptr) {
		float flTimeTilAttack = pLocal->GetAttackDelay();
		int nTicksTilAttack = (int)(flTimeTilAttack * 1000);
		if ((int)(Engine::GetGameTime() * 1000) >= (m_nLastAttackCmdTick + nTicksTilAttack))
		{
			if (useAutokey) {
				Autokey::MoveMouse(m_pTarget->GetPos());
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
							Autokey::Click();
							m_lastMoveClickTime = now;
						}
					}
					else {
						Engine::MoveTo(&Engine::GetMouseWorldPosition());
					}
				}
				m_nLastMoveCmdTick = (int)(Engine::GetGameTime() * 1000);
			}
		}

		auto color = createRGB(255, 255, 255);
		Functions.DrawCircle(&m_pTarget->GetPos(), m_pTarget->GetBoundingRadius(), &color, 0, 10.0f, 0, .8f);
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
						Autokey::Click();
						m_lastMoveClickTime = now;
					}
				}
				else {
					Engine::MoveTo(&Engine::GetMouseWorldPosition());
				}
			}
			m_nLastMoveCmdTick = (int)(Engine::GetGameTime() * 1000);
		}
		m_nLastAttackCmdTick = 0;
	}
}