#include <Windows.h>

class CSPellData
{
public:
	float GetSpellWidth() {
		return *(float*)(DWORD)this + 0x44C;
	}
};