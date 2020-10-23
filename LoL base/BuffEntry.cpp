#include "BuffEntry.h"

#include "Engine.h"

BYTE BuffEntry::getBuffType() {
	return *(BYTE*)((DWORD)this + 0x4);
}

bool BuffEntry::isBuffEmpty() {
	return strptr == 0;
}

bool BuffEntry::IsAlive() {
	auto time = Engine::GetGameTime();
	return this->GetBuffStartTime() < time && time < this->GetBuffEndTime();
}

bool BuffEntry::IsValid() {
	if (this == NULL || (DWORD)this <= 0x1000)
		return false;

	return strcmp(GetBuffName(), "NULL") && GetBuffCountAlt() > 0;
}

float BuffEntry::GetBuffStartTime() {
	return *(float*)((DWORD)this + offsets::buff::O_BUFFMGR_STARTTIME);
}

float BuffEntry::GetBuffEndTime() {
	return *(float*)((DWORD)this + offsets::buff::O_BUFFMGR_ENDTIME);
}

int BuffEntry::GetBuffCountAlt() {
	return (*(int*)((DWORD)this + 0x20) - *(int*)((DWORD)this + 0x1c)) >> 3;
}

float BuffEntry::GetBuffCountFloat() {
	return *(float*)((DWORD)this + offsets::buff::O_BUFFMGR_flBUFFCOUNT);
}

int BuffEntry::GetBuffCountInt() {
	return *(int*)((DWORD)this + offsets::buff::O_BUFFMGR_iBUFFCOUNT);
}

char* BuffEntry::GetBuffName() {
	DWORD aux = *(DWORD*)((DWORD)this + offsets::buff::O_BUFFMGR_BUFFNAME);
	if (aux == NULL)
		return "NULL";

	if (*(DWORD*)(aux + offsets::buff::O_BUFFMGR_BUFFNAME) == NULL)
		return "NULL";

	return (char*)(aux + offsets::buff::O_BUFFMGR_BUFFNAME);
}
