#if __has_include_next(<minwindef.h>)
#include_next <minwindef.h>
#else

#pragma once

#include <cstdint>

#include <basetsd.h>
#include <winapifamily.h>

using ULONG = std::uint32_t;
using PULONG = ULONG*;
using USHORT = std::uint16_t;
using UCHAR = std::uint8_t;

#define MAX_PATH 260
#define NULL 0
#define FALSE 0
#define TRUE 1

#define CALLBACK
#define WINAPI
#define WINAPIV
#define APIENTRY WINAPI

using BOOL = int;
using PBOOL = BOOL*;
using LPBOOL = BOOL*;
using BYTE = std::uint8_t;
using PBYTE = BYTE*;
using LPBYTE = BYTE*;
using WORD = std::uint16_t;
using PWORD = WORD*;
using DWORD = std::uint32_t;
using PDWORD = DWORD*;
using LPDWORD = DWORD*;
using LPVOID = void*;
using LPCVOID = const void*;

#include <winnt.h>

using UINT = unsigned int;
using PUINT = UINT*;
using HGLOBAL = HANDLE;
using HLOCAL = HANDLE;
using HMODULE = HANDLE;
using HINSTANCE = HANDLE;
using HKEY = HANDLE;
using HRSRC = HANDLE;
using HFILE = int;
using FARPROC = void (*)();
using LPHANDLE = HANDLE*;

struct FILETIME
{
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
};

#define MAKELONG(low, high) static_cast<LONG>((static_cast<WORD>(low)) | (static_cast<DWORD>(static_cast<WORD>(high)) << 16))
#define LOWORD(value) static_cast<WORD>(static_cast<DWORD_PTR>(value) & 0xffff)
#define HIWORD(value) static_cast<WORD>((static_cast<DWORD_PTR>(value) >> 16) & 0xffff)

#endif // __has_include_next(<minwindef.h>)
