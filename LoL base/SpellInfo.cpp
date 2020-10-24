#include "SpellInfo.h"

#include "SpellData.h"

SpellData* SpellInfo::GetSpellData() {
	return (SpellData*)((DWORD)this + 0x38);
}
