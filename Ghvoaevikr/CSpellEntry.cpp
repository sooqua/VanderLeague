#include "CSpellEntry.h"

CSpellInfo* CSpellEntry::GetSpellInfo() {
	auto retaddr = *(DWORD*)(this + 0x8);
	if (retaddr == NULL)
		return NULL;
	return (CSpellInfo*)(retaddr);
}

bool CSpellEntry::IsBasicAttack() {
	return *(bool*)((DWORD)this + 0xBC);

}

int CSpellEntry::GetTargetIndex() {
	return *(int*)((DWORD)this + 0xc0);
}

Vector CSpellEntry::GetStartPos() {
	return *(Vector*)((DWORD)this + 0x80);
}

Vector CSpellEntry::GetEndPos() {
	return *(Vector*)((DWORD)this + 0x8C);
}
