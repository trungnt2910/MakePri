#if __has_include_next(<sysinfoapi.h>)
#include_next <sysinfoapi.h>
#else

#pragma once

#include <minwinbase.h>

extern "C" void WINAPI GetSystemTime(LPSYSTEMTIME systemTime);
extern "C" DWORD WINAPI GetSystemWindowsDirectoryW(LPWSTR buffer, UINT size);

#ifdef UNICODE
#define GetSystemWindowsDirectory GetSystemWindowsDirectoryW
#endif

#endif // __has_include_next(<sysinfoapi.h>)
