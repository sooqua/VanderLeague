#include "CSpellInfo.h"

CSpellData* CSpellInfo::GetSpellData() {
	return *(CSpellData**)((DWORD)this + 0x44);
}
