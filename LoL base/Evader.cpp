#include "Evader.h"

#include "Engine.h"
#include "Prediction.h"
#include "Autokey.h"
#include "ImRender.hpp"

#include "Debug.h"

#include <vector>
#include <algorithm>
#include <chrono>
#include <map>

extern Prediction prediction;

bool CEvader::drawEvent() {
	bool noAction = false;
	CObject object;
	CObject* pObject = object.GetFirstObject();

	while (pObject)
	{
		if (pObject->IsMissile())
		{
			auto objCaster = Engine::GetObjectByID(pObject->GetMissileSourceIndex());

			if (objCaster->IsHero() && objCaster->IsEnemyTo(Engine::GetLocalObject()) && !stristr(pObject->GetName(), "basic")) {
				Vector start_pos = pObject->GetMissileStartPos();
				Vector start_pos_w2s;
				Functions.WorldToScreen(&start_pos, &start_pos_w2s);
				Vector end_pos = pObject->GetMissileEndPos();
				Vector end_pos_w2s;
				Functions.WorldToScreen(&end_pos, &end_pos_w2s);
				auto localObjPos = Engine::GetLocalObject()->GetPos();
				Vector localObjPos_w2s;
				Functions.WorldToScreen(&localObjPos, &localObjPos_w2s);

				static std::map<CObject*, bool> isDodged;

				if (prediction.PointOnLineSegment(
					D3DXVECTOR2(start_pos_w2s.X, start_pos_w2s.Y),
					D3DXVECTOR2(end_pos_w2s.X, end_pos_w2s.Y),
					D3DXVECTOR2(localObjPos_w2s.X, localObjPos_w2s.Y), static_cast<double>(Engine::GetLocalObject()->GetBoundingRadius())))
				{
					noAction = true;

					render.draw_line(start_pos_w2s.X, start_pos_w2s.Y, end_pos_w2s.X, end_pos_w2s.Y, ImColor(255, 255, 255), 5.0f);

					Vector direction = end_pos - start_pos;

					std::vector<Vector> points;

					Vector pos3 = end_pos + Vector(direction.Z * -1.0f, direction.Y, direction.X * 1.0f);
					Vector pos4 = end_pos + Vector(direction.Z * 1.0f, direction.Y, direction.X * -1.0f);

					Vector pos3_w2s;
					Vector pos4_w2s;
					Functions.WorldToScreen(&pos3, &pos3_w2s);
					Functions.WorldToScreen(&pos4, &pos4_w2s);
					render.draw_line(pos4_w2s.X, pos4_w2s.Y, pos3_w2s.X, pos3_w2s.Y, ImColor(255, 0, 0), 5.0f);

					Vector direction2;
					direction2.X = pos4.X - pos3.X;
					direction2.Y = pos4.Y - pos3.Y;
					direction2.Z = pos4.Z - pos3.Z;

					debug::printConsoleOrChat("DIRECTION: %.6f %.6f %.6f", direction2.X, direction2.Y, direction2.Z);
					debug::printConsoleOrChat("pos3: %.6f %.6f %.6f", pos3.X, pos3.Y, pos3.Z);
					debug::printConsoleOrChat("pos4: %.6f %.6f %.6f", pos4.X, pos4.Y, pos4.Z);
					 
					//auto isDodgedIt = isDodged.find(pObject);
					//if (isDodgedIt == isDodged.end() || !isDodgedIt->second) {
						/*points.push_back(pos3);
						points.push_back(pos4);
						std::sort(points.begin(), points.end(),
							[&localObjPos](Vector first, Vector second)
							{
								return (localObjPos.DistTo(first) < localObjPos.DistTo(second));
							});
						auto closestPos = points.front();*/

					Vector evadePos = Engine::GetLocalObject()->GetPos() + direction2;
					Vector evadePos_w2s;
					Functions.WorldToScreen(&evadePos, &evadePos_w2s);
					render.draw_line(Engine::GetLocalObject()->GetPos().X, Engine::GetLocalObject()->GetPos().Y, evadePos_w2s.X, evadePos_w2s.Y, ImColor(255, 255, 255), 5.0f);

						if (useAutokey) {
							std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(
								std::chrono::system_clock::now().time_since_epoch());
							if (now >= m_lastMoveClickTime + std::chrono::milliseconds(100)) {
								Autokey::MoveMouse(evadePos);
								Autokey::Click();
								m_lastMoveClickTime = now;
							}
						}
						else if ((int)(Engine::GetGameTime() * 1000) >= m_nLastMoveCmdTick + 60) {
							Engine::MoveTo(&evadePos);
							m_nLastMoveCmdTick = (int)(Engine::GetGameTime() * 1000);
						}

						//isDodged.insert(std::make_pair(pObject, true));
						//debug::printConsoleOrChat("DODGING NOW");
					//}
				}
				else {
					isDodged.clear();
				}
			}
		}

		pObject = object.GetNextObject(pObject);
	}

	return noAction;
}