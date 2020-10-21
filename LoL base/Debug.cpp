#include "Debug.h"

#include "Engine.h"
#include <stdio.h>
#include <stdarg.h>

void debug::Printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
#ifdef _DEBUG
    vprintf(format, args);
#else
    char buffer[1024];
    vsprintf_s(buffer, format, args);
    Engine::PrintChat(buffer);
#endif
    va_end(args);
}