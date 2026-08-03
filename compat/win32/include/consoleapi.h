#if __has_include_next(<consoleapi.h>)
#include_next <consoleapi.h>
#else

#pragma once

#include <minwindef.h>

using PHANDLER_ROUTINE = BOOL(WINAPI*)(DWORD controlType);
extern "C" BOOL WINAPI SetConsoleCtrlHandler(PHANDLER_ROUTINE handler, BOOL add);

#endif // __has_include_next(<consoleapi.h>)
