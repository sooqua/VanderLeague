#include "CConsole.h"

#include <windows.h>
#include <conio.h>
#include <FCNTL.H>
#include <io.h>
#include <iostream>

static BOOL WINAPI MyConsoleCtrlHandler(DWORD dwCtrlEvent) { return dwCtrlEvent == CTRL_C_EVENT; }

CConsole::CConsole() : m_OwnConsole(false) {
    //Attempt to attach to parent
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
        //Try to create a new window, if allowed
        FreeConsole();
        if (!AllocConsole())
            return;
    }

    SetConsoleTitle(TEXT("Debug Console"));
    SetConsoleCtrlHandler(MyConsoleCtrlHandler, TRUE);
    RemoveMenu(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_BYCOMMAND);

    // std::cout, std::clog, std::cerr, std::cin
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
    freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
    std::cin.clear();

    // std::wcout, std::wclog, std::wcerr, std::wcin
    HANDLE hConOut = CreateFile(TEXT("CONOUT$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hConIn = CreateFile(TEXT("CONIN$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    SetStdHandle(STD_OUTPUT_HANDLE, hConOut);
    SetStdHandle(STD_ERROR_HANDLE, hConOut);
    SetStdHandle(STD_INPUT_HANDLE, hConIn);
    std::wcout.clear();
    std::wclog.clear();
    std::wcerr.clear();
    std::wcin.clear();

    m_OwnConsole = true;
}

CConsole::~CConsole() {
    if (m_OwnConsole) {freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        SetConsoleCtrlHandler(MyConsoleCtrlHandler, FALSE);
        FreeConsole();
    }
}