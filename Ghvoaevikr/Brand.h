#pragma once

#include "Script.h"

class Brand : public IScript {
private:
	Champion champion;

	virtual void OnKeyDown(int vKey);

	virtual void OnKeyUp(int vKey);

	virtual void Harass();

public:
	Brand() : champion(Champion::Brand) {

	}
};