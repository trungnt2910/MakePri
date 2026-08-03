#if __has_include_next(<minwinbase.h>)
#include_next <minwinbase.h>
#else

#pragma once

#include <cstring>

#include <minwindef.h>

#define MoveMemory(destination, source, length) std::memmove((destination), (source), (length))
#define CopyMemory(destination, source, length) std::memcpy((destination), (source), (length))
#define ZeroMemory(destination, length) std::memset((destination), 0, (length))

#define LMEM_ZEROINIT 0x0040

struct SECURITY_ATTRIBUTES
{
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
};
using LPSECURITY_ATTRIBUTES = SECURITY_ATTRIBUTES*;
using PSECURITY_ATTRIBUTES = SECURITY_ATTRIBUTES*;

struct SYSTEMTIME
{
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
};
using PSYSTEMTIME = SYSTEMTIME*;
using LPSYSTEMTIME = SYSTEMTIME*;

struct WIN32_FIND_DATAW
{
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    WCHAR cFileName[MAX_PATH];
    WCHAR cAlternateFileName[14];
};
using LPWIN32_FIND_DATAW = WIN32_FIND_DATAW*;

enum GET_FILEEX_INFO_LEVELS
{
    GetFileExInfoStandard,
    GetFileExMaxInfoLevel,
};

#ifdef UNICODE
using WIN32_FIND_DATA = WIN32_FIND_DATAW;
using LPWIN32_FIND_DATA = LPWIN32_FIND_DATAW;
#endif

#endif // __has_include_next(<minwinbase.h>)
