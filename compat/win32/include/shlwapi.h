#if __has_include_next(<shlwapi.h>)
#include_next <shlwapi.h>
#else

#pragma once

#include <objbase.h>

extern "C"
{
    BOOL WINAPI PathFileExistsW(LPCWSTR path);
    BOOL WINAPI PathIsRelativeW(LPCWSTR path);
    BOOL WINAPI PathIsNetworkPathW(LPCWSTR path);
    BOOL WINAPI PathRelativePathToW(LPWSTR path, LPCWSTR from, DWORD attributesFrom, LPCWSTR to, DWORD attributesTo);
    LPWSTR WINAPI PathRemoveBackslashW(LPWSTR path);
    LPWSTR WINAPI StrChrW(LPCWSTR string, WCHAR character);
    HRESULT WINAPI SHCreateStreamOnFileW(LPCWSTR file, DWORD mode, IStream** stream);
}

#endif // __has_include_next(<shlwapi.h>)
