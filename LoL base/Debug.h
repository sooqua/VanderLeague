#pragma once
#include <stdio.h>

namespace debug {
    void init();
    void cleanUp();
    void printConsoleOrChat(const char* format, ...);
    void flog(const char* format, ...);
};
