#include "ScriptUtils.h"

#include "Morgana.h"
#include "Brand.h"
#include "Caitlyn.h"

#include <string>

IScript* ScriptUtils::GetScriptByChampionName(char* championName)
{
	if (strcmp(championName, "Morgana") == 0) {
		return new Morgana();
	}
	else if (strcmp(championName, "Brand") == 0) {
		return new Brand();
	}
	else if (strcmp(championName, "Caitlyn") == 0) {
		return new Caitlyn();
	}

	return nullptr;
}
