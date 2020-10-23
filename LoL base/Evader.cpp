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

bool isLeft(Vector a, Vector b, Vector c) {
	return ((b.X - a.X) * (c.Y - a.Y) - (b.Y - a.Y) * (c.X - a.X)) > 0;
}

bool CEvader::drawEvent() {
	bool noAction = false;
	CObject object;
	CObject* pObject = object.GetFirstObject();

	while (pObject)
	{
		if (pObject->IsMissile())
		{
			auto objCaster = Engine::GetObjectByID(pObject->GetMissileSourceIndex());
			auto localObj = Engine::GetLocalObject();

			if (objCaster->IsHero() && objCaster->IsEnemyTo(localObj) && !stristr(pObject->GetName(), "basic")) {
				Vector start_pos = pObject->GetMissileStartPos();
				Vector start_pos_w2s;
				Functions.WorldToScreen(&start_pos, &start_pos_w2s);
				Vector end_pos = pObject->GetMissileEndPos();
				Vector end_pos_w2s;
				Functions.WorldToScreen(&end_pos, &end_pos_w2s);
				auto localObjPos = localObj->GetPos();
				Vector localObjPos_w2s;
				Functions.WorldToScreen(&localObjPos, &localObjPos_w2s);

				static std::map<CObject*, bool> isDodged;

				if (prediction.PointOnLineSegment(
					D3DXVECTOR2(start_pos_w2s.X, start_pos_w2s.Y),
					D3DXVECTOR2(end_pos_w2s.X, end_pos_w2s.Y),
					D3DXVECTOR2(localObjPos_w2s.X, localObjPos_w2s.Y), static_cast<double>(localObj->GetBoundingRadius())))
				{
					noAction = true;

					render.draw_line(start_pos_w2s.X, start_pos_w2s.Y, end_pos_w2s.X, end_pos_w2s.Y, ImColor(255, 255, 255), 5.0f);

					Vector direction = end_pos - start_pos;

					std::vector<Vector> points;

					Vector pos3 = end_pos + Vector(direction.Z * -1.0f, direction.Y, direction.X * 1.0f);
					Vector pos4 = end_pos + Vector(direction.Z * 1.0f, direction.Y, direction.X * -1.0f);

					Vector direction2 = pos3 - pos4;

					auto br = localObj->GetBoundingRadius() * 2;
					direction2.X = direction2.X > br ? br : direction2.X;
					direction2.X = direction2.X < -br ? -br : direction2.X;
					direction2.Y = direction2.Y > br ? br : direction2.Y;
					direction2.Y = direction2.Y < -br ? -br : direction2.Y;
					direction2.Z = direction2.Z > br ? br : direction2.Z;
					direction2.Z = direction2.Z < -br ? -br : direction2.Z;

					Vector direction3;
					direction3.X = -direction2.X;
					direction3.Y = -direction2.Y;
					direction3.Z = -direction2.Z;
					auto bIsLeft = isLeft(start_pos_w2s, end_pos_w2s, localObjPos_w2s);
					Vector direction4 = bIsLeft ? direction3 : direction2;

					auto isDodgedIt = isDodged.find(pObject);
					if (isDodgedIt == isDodged.end() || !isDodgedIt->second) {
						Vector evadePos = localObjPos + direction4;
						Vector evadePos_w2s;
						Functions.WorldToScreen(&evadePos, &evadePos_w2s);
						render.draw_line(localObjPos_w2s.X, localObjPos_w2s.Y, evadePos_w2s.X, evadePos_w2s.Y, ImColor(0, 255, 0), 5.0f);

						if (useAutokey) {
							std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(
								std::chrono::system_clock::now().time_since_epoch());
							if (now >= m_lastMoveClickTime + std::chrono::milliseconds(100)) {
								Autokey::MoveMouse(evadePos);
								Autokey::Click();
								m_lastMoveClickTime = now;
							}
						}
						else {
							if ((int)(Engine::GetGameTime() * 1000) >= m_nLastMoveCmdTick + 60) {
								Engine::MoveTo(&evadePos);
								m_nLastMoveCmdTick = (int)(Engine::GetGameTime() * 1000);
							}
						}

						isDodged.insert(std::make_pair(pObject, true));
					}
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