#if __has_include_next(<memoryapi.h>)
#include_next <memoryapi.h>
#else

#pragma once

#include <minwinbase.h>

#define FILE_MAP_READ 0x0004
#define FILE_MAP_WRITE SECTION_MAP_WRITE

extern "C"
{
    HANDLE WINAPI CreateFileMappingW(
        HANDLE file,
        LPSECURITY_ATTRIBUTES security,
        DWORD protect,
        DWORD maximumSizeHigh,
        DWORD maximumSizeLow,
        LPCWSTR name);
    HANDLE WINAPI OpenFileMappingW(DWORD access, BOOL inheritHandle, LPCWSTR name);
    PVOID WINAPI MapViewOfFile(HANDLE mapping, DWORD access, DWORD offsetHigh, DWORD offsetLow, SIZE_T bytes);
    BOOL WINAPI UnmapViewOfFile(LPCVOID address);
    SIZE_T WINAPI VirtualQuery(LPCVOID address, PMEMORY_BASIC_INFORMATION information, SIZE_T length);
}

#ifdef UNICODE
#define CreateFileMapping CreateFileMappingW
#define OpenFileMapping OpenFileMappingW
#endif

#endif // __has_include_next(<memoryapi.h>)
