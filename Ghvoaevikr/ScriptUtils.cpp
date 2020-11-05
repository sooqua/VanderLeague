#include "ScriptUtils.h"

#include "Morgana.h"
#include "Brand.h"

#include <string>

IScript* GetScriptByChampionName(char*) {
}

IScript* ScriptUtils::GetScriptByChampionName(char* championName)
{
	if (strcmp(championName, "Morgana") == 0) {
		return new Morgana();
	}
	else if (strcmp(championName, "Brand") == 0) {
		return new Brand();
	}

	return nullptr;
}
