#include "Evader.h"

#include "Engine.h"
#include "Autokey.h"

#include <vector>
#include <algorithm>
#include <chrono>

void CEvader::drawEvent() {
	CObject object;
	CObject* pObject = object.GetFirstObject();

	while (pObject)
	{
		if (pObject->IsMissile())
		{
			auto objCaster = Engine::GetObjectByID(pObject->GetMissileSourceIndex());

			if (objCaster->IsHero() && objCaster->IsEnemyTo(Engine::GetLocalObject()) && !stristr(pObject->GetName(), "basic")) {
				Vector start_pos = pObject->GetMissileStartPos();
				Vector end_pos = pObject->GetMissileEndPos();

				Vector direction = end_pos - start_pos;

				auto localObjPos = Engine::GetLocalObject()->GetPos();
				std::vector<Vector> points;
				Vector pos1 = start_pos + Vector(direction.Z * -1.0f, direction.Y, direction.X * 1.0f);
				Vector pos2 = start_pos + Vector(direction.Z * 1.0f, direction.Y, direction.X * -1.0f);
				Vector pos3 = end_pos + Vector(direction.Z * -1.0f, direction.Y, direction.X * 1.0f);
				Vector pos4 = end_pos + Vector(direction.Z * 1.0f, direction.Y, direction.X * -1.0f);
				points.push_back(pos1);
				points.push_back(pos2);
				points.push_back(pos3);
				points.push_back(pos4);
				std::sort(points.begin(), points.end(),
					[&localObjPos](Vector first, Vector second)
					{
						return (localObjPos.DistTo(first) < localObjPos.DistTo(second));
					});
				auto closestPos = points.front();
				//closestPos.X = closestPos.X * 30.f / 100.f;
				//closestPos.Y = closestPos.Y * 30.f / 100.f;
				//closestPos.Z = closestPos.Z * 30.f / 100.f;

				if (useAutokey) {
					std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::system_clock::now().time_since_epoch());
					if (now >= m_lastMoveClickTime + std::chrono::milliseconds(100)) {
						Autokey::MoveMouse(closestPos);
						Autokey::Click();
						m_lastMoveClickTime = now;
					}
				}
				else if ((int)(Engine::GetGameTime() * 1000) >= m_nLastMoveCmdTick + 60) {
					Engine::MoveTo(&closestPos);
					m_nLastMoveCmdTick = (int)(Engine::GetGameTime() * 1000);
				}
			}
		}

		pObject = object.GetNextObject(pObject);
	}
}