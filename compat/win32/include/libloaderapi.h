#if __has_include_next(<libloaderapi.h>)
#include_next <libloaderapi.h>
#else

#pragma once

#include <minwindef.h>

#define LOAD_LIBRARY_SEARCH_SYSTEM32 0x00000800

extern "C"
{
    DWORD WINAPI GetModuleFileNameW(HMODULE module, LPWSTR buffer, DWORD size);
    HMODULE WINAPI LoadLibraryExW(LPCWSTR fileName, HANDLE file, DWORD flags);
    HMODULE WINAPI LoadLibraryW(LPCWSTR fileName);
    HMODULE WINAPI GetModuleHandleW(LPCWSTR moduleName);
    BOOL WINAPI FreeLibrary(HMODULE module);
    FARPROC WINAPI GetProcAddress(HMODULE module, LPCSTR procedureName);
}

#endif // __has_include_next(<libloaderapi.h>)
