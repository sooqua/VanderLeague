#pragma once

#include "Script.h"

class Morgana : public IScript {
private:
	Champion champion;

	virtual void OnKeyDown(int vKey);

	virtual void OnKeyUp(int vKey);

	virtual void Harass();

public:
	Morgana() : champion(Champion::Morgana) {

	}
};