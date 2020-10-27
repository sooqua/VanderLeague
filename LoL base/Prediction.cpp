#include "Prediction.h"


Prediction::Prediction()
{
}


Prediction::~Prediction()
{
}

Vector LinePrediction::Calculate(CObject* target, float range, float missilespeed, float casttime)
{
	float t = Vector(target->GetPos() - Engine::GetLocalObject()->GetPos()).length() / missilespeed;
	t += casttime;
	auto aim = target->GetAIManager();
	auto veloc = aim->GetVelocity();
	veloc.Y = 0.f;
	Vector orientation = veloc.Normalize();

	if (!(target && target->GetHealth() > 0.f && target->GetDistanceToMe() < range)) {
		return Vector(0.f, 0.f, 0.f);
	}

	if (veloc.X == 0.f && veloc.Z == 0.f)
	{
		return target->GetPos();
	}

	Vector result = target->GetPos() + (orientation * target->GetMoveSpeed() * t);

	if (result.DistTo(Engine::GetLocalObject()->GetPos()) > range) {
		return Vector(0.f, 0.f, 0.f);
	}

	return result;
}

Vector CirclePrediction::Calculate(CObject* target, float range, float casttime)
{
	auto aim = target->GetAIManager();
	auto veloc = aim->GetVelocity();
	veloc.Y = 0.f;
	Vector orientation = veloc.Normalize(); //target->GetAllShield->vVelocity.Normalized();

	if (!(target && target->GetHealth() > 0.f && target->GetDistanceToMe() < range)) {
		return Vector(0.f, 0.f, 0.f);
	}

	if (veloc.X == 0.f && veloc.Z == 0.f)
	{
		return target->GetPos();
	}

	Vector result = target->GetPos() + (orientation * target->GetMoveSpeed() * casttime);

	return result;
}