#if __has_include_next(<winbase.h>)
#include_next <winbase.h>
#else

#pragma once

#include <cstdarg>
#include <cstring>

#include <errhandlingapi.h>
#include <fileapi.h>
#include <handleapi.h>
#include <heapapi.h>
#include <libloaderapi.h>
#include <memoryapi.h>
#include <processenv.h>
#include <processthreadsapi.h>
#include <securitybaseapi.h>
#include <stringapiset.h>
#include <synchapi.h>
#include <sysinfoapi.h>
#include <timezoneapi.h>

#define FILE_BEGIN 0
#define FILE_CURRENT 1
#define FILE_END 2

#define DRIVE_UNKNOWN 0
#define DRIVE_NO_ROOT_DIR 1
#define DRIVE_FIXED 3

#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100
#define FORMAT_MESSAGE_IGNORE_INSERTS 0x00000200
#define FORMAT_MESSAGE_FROM_STRING 0x00000400
#define FORMAT_MESSAGE_FROM_SYSTEM 0x00001000
#define FORMAT_MESSAGE_ARGUMENT_ARRAY 0x00002000

#define SecureZeroMemory(destination, length) std::memset((destination), 0, (length))

extern "C"
{
    HLOCAL WINAPI LocalAlloc(UINT flags, SIZE_T bytes);
    HLOCAL WINAPI LocalFree(HLOCAL memory);
    SIZE_T WINAPI LocalSize(HLOCAL memory);
    BOOL WINAPI CopyFileExW(LPCWSTR existingFile, LPCWSTR newFile, LPVOID progressRoutine, LPVOID data, LPBOOL cancel, DWORD flags);
    DWORD WINAPI
    FormatMessageW(DWORD flags, LPCVOID source, DWORD messageId, DWORD languageId, LPWSTR buffer, DWORD size, va_list* arguments);
}

#ifdef UNICODE
#define FormatMessage FormatMessageW
#endif

#endif // __has_include_next(<winbase.h>)
