#include "Debug.h"
#include "Engine.h"

#include <stdio.h>
#include <stdarg.h>

namespace debug {
	FILE* logFile;


#pragma warning(push)
#pragma warning(disable: 4996)
	void debug::init() {
		debug::logFile = fopen("C:\\dev\\Projects\\Ghvoaevikr\\x64\\Release\\log.log", "a+");
	}
#pragma warning(pop)

	void debug::cleanUp() {
		fclose(debug::logFile);
	}

	void debug::printConsoleOrChat(const char* format, ...)
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

	void debug::flog(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		if (debug::logFile != NULL) {
			vfprintf(debug::logFile, format, args);
			fflush(debug::logFile);
		}
		va_end(args);
	}
};