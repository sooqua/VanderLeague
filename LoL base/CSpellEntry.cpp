#include "CSpellEntry.h"

SpellData* CSpellEntry::GetSpellData() {
	auto retaddr = *(DWORD*)(this + 0x8);
	if (retaddr == NULL)
		return NULL;
	auto ret = *(DWORD*)(retaddr + 0x38);
	if (ret == NULL)
		return NULL;
	return (SpellData*)(ret);
}

bool CSpellEntry::isAutoAttack() {
	return *(bool*)((DWORD)this + 0xBC);

}

int CSpellEntry::targetID() {
	return *(int*)((DWORD)this + 0xC0);
}

Vector CSpellEntry::GetStartPos() {
	return *(Vector*)((DWORD)this + 0x80);
}

Vector CSpellEntry::GetEndPos() {
	return *(Vector*)((DWORD)this + 0x8C);
}
