#include "Keyboard.h"

#include <Windows.h>

bool IsKeyDown(int vKey) {
	return (GetAsyncKeyState(vKey) & (1 << 15)) != 0;
}
