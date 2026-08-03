#if __has_include_next(<processenv.h>)
#include_next <processenv.h>
#else

#pragma once

#include <minwindef.h>

extern "C"
{
    DWORD WINAPI GetEnvironmentVariableW(LPCWSTR name, LPWSTR buffer, DWORD size);
    DWORD WINAPI ExpandEnvironmentStringsW(LPCWSTR source, LPWSTR destination, DWORD size);
}

#ifdef UNICODE
#define ExpandEnvironmentStrings ExpandEnvironmentStringsW
#endif

#endif // __has_include_next(<processenv.h>)
