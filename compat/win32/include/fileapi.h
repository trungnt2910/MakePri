#if __has_include_next(<fileapi.h>)
#include_next <fileapi.h>
#else

#pragma once

#include <minwinbase.h>

#define INVALID_FILE_ATTRIBUTES static_cast<DWORD>(-1)
#define INVALID_FILE_SIZE static_cast<DWORD>(-1)
#define INVALID_SET_FILE_POINTER static_cast<DWORD>(-1)

#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3

struct WIN32_FILE_ATTRIBUTE_DATA
{
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
};

extern "C"
{
    LONG WINAPI CompareFileTime(const FILETIME* left, const FILETIME* right);
    BOOL WINAPI FindClose(HANDLE handle);
    HANDLE WINAPI CreateFileW(
        LPCWSTR name,
        DWORD access,
        DWORD sharing,
        LPSECURITY_ATTRIBUTES security,
        DWORD creation,
        DWORD attributes,
        HANDLE templateFile);
    BOOL WINAPI GetFileSizeEx(HANDLE file, PLARGE_INTEGER size);
    DWORD WINAPI GetFileSize(HANDLE file, LPDWORD highSize);
    DWORD WINAPI SetFilePointer(HANDLE file, LONG distance, LONG* distanceHigh, DWORD method);
    BOOL WINAPI ReadFile(HANDLE file, LPVOID buffer, DWORD bytes, LPDWORD read, LPVOID overlapped);
    BOOL WINAPI WriteFile(HANDLE file, LPCVOID buffer, DWORD bytes, LPDWORD written, LPVOID overlapped);
    BOOL WINAPI FlushFileBuffers(HANDLE file);
    HANDLE WINAPI FindFirstFileW(LPCWSTR name, LPWIN32_FIND_DATAW data);
    BOOL WINAPI FindNextFileW(HANDLE find, LPWIN32_FIND_DATAW data);
    DWORD WINAPI GetFileAttributesW(LPCWSTR name);
    BOOL WINAPI GetFileAttributesExW(LPCWSTR name, GET_FILEEX_INFO_LEVELS level, LPVOID information);
    BOOL WINAPI DeleteFileW(LPCWSTR name);
    BOOL WINAPI RemoveDirectoryW(LPCWSTR name);
    BOOL WINAPI CreateDirectoryW(LPCWSTR name, LPSECURITY_ATTRIBUTES security);
    DWORD WINAPI GetTempPathW(DWORD length, LPWSTR buffer);
    DWORD WINAPI GetFullPathNameW(LPCWSTR path, DWORD length, LPWSTR buffer, LPWSTR* filePart);
    DWORD WINAPI GetFinalPathNameByHandleW(HANDLE file, LPWSTR path, DWORD size, DWORD flags);
    UINT WINAPI GetDriveTypeW(LPCWSTR rootPathName);
}

#ifdef UNICODE
#define CreateFile CreateFileW
#define FindFirstFile FindFirstFileW
#define FindNextFile FindNextFileW
#define GetFileAttributes GetFileAttributesW
#define GetFileAttributesEx GetFileAttributesExW
#define DeleteFile DeleteFileW
#define RemoveDirectory RemoveDirectoryW
#define CreateDirectory CreateDirectoryW
#define GetTempPath GetTempPathW
#endif

#endif // __has_include_next(<fileapi.h>)
