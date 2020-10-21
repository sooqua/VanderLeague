#include "DebugConsole.h"

using namespace std;

FILE* __fStdOut = NULL;
HANDLE __hStdOut = NULL;

void DebugConsole::Init(char* title, int width, int height, char* fname) {
	AllocConsole();
	SetConsoleTitle(title);
	__hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD co = { width, height };
	SetConsoleScreenBufferSize(__hStdOut, co);
	if (fname)
		__fStdOut = fopen(fname, "w");

}

int DebugConsole::Print(char* fmt, ...) {
	char s[3000];
	va_list argptr;
	int cnt;
	va_start(argptr, fmt);
	cnt = vsprintf(s, fmt, argptr);
	va_end(argptr);
	DWORD cCharsWritten;
	if (__hStdOut)
		WriteConsole(__hStdOut, s, strlen(s), &cCharsWritten, NULL);
	if (__fStdOut)
		fprintf(__fStdOut, s);
	return (cnt);
}
