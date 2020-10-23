#include "stdafx.h"
#include "Prediction.h"


Prediction::Prediction()
{
}


Prediction::~Prediction()
{
}

Vector LinePrediction::Predict(CObject* target, float range, float missilespeed, float casttime) {
	float t = target->GetDistanceToMe() / missilespeed;
	t += casttime;
	Vector veloc = target->GetAIManager()->GetVelocity();
	veloc.Y = 0;
	Vector orientation = veloc.Normalize();

	//Normalize(orientation);
	if (target->GetPos().DistTo(Engine::GetLocalObject()->GetPos()) > range)
		return Vector(0, 0, 0);

	if (veloc.X == 0 && veloc.Z == 0)
	{

		return target->GetPos();
	}
	Vector facResult = Vector(orientation.X * (t + (target->GetMoveSpeed())), orientation.Y * (t + (target->GetMoveSpeed())), orientation.Z * (t + (target->GetMoveSpeed())));
	Vector result = target->GetPos() + facResult;
	if (result.DistTo(Engine::GetLocalObject()->GetPos()) > range)
		return Vector(0, 0, 0);


	return result;
}

Vector LinePrediction::Calculate(CObject* target, float range, float missilespeed, float casttime)
{
	float t = Vector((target->GetPos() - Engine::GetLocalObject()->GetPos())).length() / missilespeed;
	t += casttime;
	Vector veloc = target->GetAIManager()->GetVelocity();
	veloc.X = 0;
	Vector orientation = veloc.Normalize();//target->GetAllShield->vVelocity.Normalized();
											 //Normalize(orientation);
	if (target->GetPos().DistTo(Engine::GetLocalObject()->GetPos()) > range)
		return Vector(0, 0, 0);

	if (veloc.X == 0 && veloc.Z == 0)
	{
		//ENGINE_MSG("GetServerPosition %f %f %f\n", orientation.x, orientation.y, orientation.z);
		//Beep(2000,200);

		return target->GetPos();
	}
	Vector facResult = Vector((target->GetMoveSpeed() * t) * orientation.X, (target->GetMoveSpeed() * t) * orientation.Y, (target->GetMoveSpeed() * t) * orientation.Z);
	Vector result = target->GetPos() + facResult;


	if (result.DistTo(Engine::GetLocalObject()->GetPos()) > range)
		return Vector(0, 0, 0);



	//ENGINE_MSG("result %f %f %f : time : %f\n", orientation.x, orientation.y, orientation.z,t);

	return result;
}

Vector CirclePrediction::Calculate(CObject* target, float range, float missilespeed, float radius, float casttime)
{


	float t = 0.0f;
	t += casttime;
	Vector veloc = target->GetAIManager()->GetVelocity();
	veloc.X = 0;
	Vector orientation = veloc.Normalize();//target->GetAllShield->vVelocity.Normalized();
											 //Normalize(orientation);
	if (target->GetPos().DistTo(Engine::GetLocalObject()->GetPos()) > range)
		return Vector(0, 0, 0);

	if (veloc.X == 0 && veloc.Z == 0)
	{
		//ENGINE_MSG("GetServerPosition %f %f %f\n", orientation.x, orientation.y, orientation.z);
		//Beep(2000,200);

		return target->GetPos();
	}

	Vector facResult = Vector((target->GetMoveSpeed() * t) * orientation.X, (target->GetMoveSpeed() * t) * orientation.Y, (target->GetMoveSpeed() * t) * orientation.Z);
	Vector result = target->GetPos() + facResult;


	return result;
}