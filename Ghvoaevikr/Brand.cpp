#include "Brand.h"

#include "Keyboard.h"
#include "ESpellSlot.h"
#include "CObject.h"
#include "CycleManager.h"
#include "Engine.h"
#include "Prediction.h"
#include "Autokey.h"
#include "SpellPrediction.h"

#include <Windows.h>
#include <vector>

void Brand::OnKeyDown(int) {

}

void Brand::OnKeyUp(int) {

}

void Brand::Harass() {
	std::vector<CObject*> possibleTargets;

	for (auto pObject : CycleManager::GetObjects()) {
		if (pObject != nullptr && pObject->IsHero() && pObject->IsAlive() && pObject->IsEnemyTo(Engine::GetLocalObject())) {
			possibleTargets.push_back(pObject);
		}
	}

	if (possibleTargets.empty())
		return;

	if (possibleTargets.size() > 1)
	{
		std::sort(possibleTargets.begin(), possibleTargets.end(),
			[](CObject* pFirst, CObject* pSecond)
			{
				return (pFirst->GetDistanceToMe() < pSecond->GetDistanceToMe());
			});
	}

	auto target = possibleTargets.front();

	auto buffmgr = target->GetBuffMgr();

	auto ablaze = buffmgr->GetBuffEntryByName("BrandAblaze");

	if (ablaze && !prediction.IsCollisioned(Prediction::CollisionType::Minion, target->GetPos())) {
		auto vec = spellPrediction.PredictSkillshot(possibleTargets.front(), ESpellSlot::Q);
		if (!(vec == Vector(0.f, 0.f, 0.f))) {
			Autokey::MoveMouse(vec);
			CycleManager::ResetMouseAtNextCycle();
			Autokey::PressKey(SCANCODE_Q);
			CycleManager::ReleaseKeyAtNextCycle(SCANCODE_Q);
		}
	}

	//if (target->GetAIManager()->GetVelocity() == Vector(0.f, 0.f, 0.f)) {
		auto vec = spellPrediction.PredictCircular(possibleTargets.front(), ESpellSlot::W);
		if (!(vec == Vector(0.f, 0.f, 0.f))) {
			Autokey::MoveMouse(vec);
			CycleManager::ResetMouseAtNextCycle();
			Autokey::PressKey(SCANCODE_W);
			CycleManager::ReleaseKeyAtNextCycle(SCANCODE_W);
		}
	//}

	vec = spellPrediction.PredictSkillshot(possibleTargets.front(), ESpellSlot::E);
	if (!(vec == Vector(0.f, 0.f, 0.f))) {
		Autokey::MoveMouse(vec);
		CycleManager::ResetMouseAtNextCycle();
		Autokey::PressKey(SCANCODE_E);
		CycleManager::ReleaseKeyAtNextCycle(SCANCODE_E);
	}
}

bool Brand::Evade(Vector /*evadePos*/)
{
	return false;
}
