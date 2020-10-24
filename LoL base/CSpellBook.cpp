#include "CSpellBook.h"

CSpellEntry* CSpellBook::GetActiveSpellEntry() {
	return *(CSpellEntry**)((DWORD)this + 0x24); // 0x74? 0x4A? 0x20?
}

CSpell* CSpellBook::GetSpellByID(int ID) {
	return *(CSpell**)((DWORD)this + 0x478 + (0x4 * ID));
}
