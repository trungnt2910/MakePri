#if __has_include_next(<winnt.h>)
#include_next <winnt.h>

#ifndef InterlockedExchangeNoFence
#define InterlockedExchangeNoFence InterlockedExchange
#endif

#else

#pragma once

#include <cstddef>
#include <cstdint>

#include <basetsd.h>
#include <guiddef.h>

#define NTAPI
#define __fastcall
#define DECLSPEC_NORETURN [[noreturn]]
#define DECLSPEC_SELECTANY inline
#define FORCEINLINE inline
#define __forceinline inline
#define UNALIGNED

#ifndef __noop
#define __noop(...) ((void)0)
#endif

using CHAR = char;
using SHORT = std::int16_t;
using LONG = std::int32_t;
using INT = int;
using WCHAR = wchar_t;
using VOID = void;

using PVOID = void*;
using HANDLE = PVOID;
using PHANDLE = HANDLE*;

using PCHAR = CHAR*;
using PSTR = CHAR*;
using LPSTR = CHAR*;
using LPCSTR = const CHAR*;
using PCSTR = const CHAR*;
using LPCCH = const CHAR*;
using PWSTR = WCHAR*;
using PWCHAR = WCHAR*;
using LPWSTR = WCHAR*;
using PCWSTR = const WCHAR*;
using LPCWSTR = const WCHAR*;
using LPWCH = WCHAR*;
using LPCWCH = const WCHAR*;

using HRESULT = LONG;
using LONGLONG = std::int64_t;
using ULONGLONG = std::uint64_t;
using DWORDLONG = ULONGLONG;

#include <minwindef.h>

using LCID = DWORD;

#define STDMETHODCALLTYPE WINAPI
#define STDAPICALLTYPE WINAPI

using BOOLEAN = BYTE;
using PSID = PVOID;
using PSECURITY_DESCRIPTOR = PVOID;

using SECURITY_INFORMATION = DWORD;

struct ACL
{
    BYTE AclRevision;
    BYTE Sbz1;
    WORD AclSize;
    WORD AceCount;
    WORD Sbz2;
};
using PACL = ACL*;

struct ACE_HEADER
{
    BYTE AceType;
    BYTE AceFlags;
    WORD AceSize;
};

struct ACCESS_ALLOWED_ACE
{
    ACE_HEADER Header;
    DWORD Mask;
    DWORD SidStart;
};

struct SID_IDENTIFIER_AUTHORITY
{
    BYTE Value[6];
};
using PSID_IDENTIFIER_AUTHORITY = SID_IDENTIFIER_AUTHORITY*;

union LARGE_INTEGER
{
    struct
    {
        DWORD LowPart;
        LONG HighPart;
    };
    LONGLONG QuadPart;
};
using PLARGE_INTEGER = LARGE_INTEGER*;

union ULARGE_INTEGER
{
    struct
    {
        DWORD LowPart;
        DWORD HighPart;
    };
    struct
    {
        DWORD LowPart;
        DWORD HighPart;
    } u;
    ULONGLONG QuadPart;
};
using PULARGE_INTEGER = ULARGE_INTEGER*;

struct MEMORY_BASIC_INFORMATION
{
    PVOID BaseAddress;
    PVOID AllocationBase;
    DWORD AllocationProtect;
    SIZE_T RegionSize;
    DWORD State;
    DWORD Protect;
    DWORD Type;
};
using PMEMORY_BASIC_INFORMATION = MEMORY_BASIC_INFORMATION*;

struct RTL_SRWLOCK
{
    PVOID Ptr;
};
using PRTL_SRWLOCK = RTL_SRWLOCK*;
#define RTL_SRWLOCK_INIT {nullptr}

typedef struct _EXCEPTION_RECORD EXCEPTION_RECORD;
typedef EXCEPTION_RECORD* PEXCEPTION_RECORD;
typedef struct _CONTEXT CONTEXT;
typedef CONTEXT* PCONTEXT;

#define __TEXT(value) L##value
#define TEXT(value) __TEXT(value)
#define ARRAYSIZE(array) (sizeof(array) / sizeof((array)[0]))
#define FIELD_OFFSET(type, field) static_cast<LONG>(offsetof(type, field))
#define CONTAINING_RECORD(address, type, field) reinterpret_cast<type*>(reinterpret_cast<BYTE*>(address) - offsetof(type, field))
#define UNREFERENCED_PARAMETER(value) ((void)(value))

#define DEFINE_ENUM_FLAG_OPERATORS(enumType) \
    constexpr enumType operator|(enumType left, enumType right) noexcept \
    { \
        return static_cast<enumType>(static_cast<unsigned>(left) | static_cast<unsigned>(right)); \
    } \
    constexpr enumType operator&(enumType left, enumType right) noexcept \
    { \
        return static_cast<enumType>(static_cast<unsigned>(left) & static_cast<unsigned>(right)); \
    } \
    constexpr enumType operator^(enumType left, enumType right) noexcept \
    { \
        return static_cast<enumType>(static_cast<unsigned>(left) ^ static_cast<unsigned>(right)); \
    } \
    constexpr enumType operator~(enumType value) noexcept { return static_cast<enumType>(~static_cast<unsigned>(value)); } \
    inline enumType& operator|=(enumType& left, enumType right) noexcept { return left = left | right; } \
    inline enumType& operator&=(enumType& left, enumType right) noexcept { return left = left & right; } \
    inline enumType& operator^=(enumType& left, enumType right) noexcept { return left = left ^ right; }

#define HEAP_ZERO_MEMORY 0x00000008
#define GENERIC_READ 0x80000000UL
#define GENERIC_WRITE 0x40000000UL
#define FILE_SHARE_READ 0x00000001
#define FILE_SHARE_DELETE 0x00000004
#define FILE_ATTRIBUTE_HIDDEN 0x00000002
#define FILE_ATTRIBUTE_SYSTEM 0x00000004
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define FILE_ATTRIBUTE_REPARSE_POINT 0x00000400
#define FILE_GENERIC_READ 0x00120089
#define PAGE_READONLY 0x02
#define PAGE_READWRITE 0x04
#define SECTION_MAP_WRITE 0x0002
#define LOCALE_NAME_MAX_LENGTH 85
#define LOCALE_USER_DEFAULT 0x0400

#define DACL_SECURITY_INFORMATION 0x00000004
#define ACCESS_ALLOWED_ACE_TYPE 0x00
#define CONTAINER_INHERIT_ACE 0x02
#define OBJECT_INHERIT_ACE 0x01
#define ACL_REVISION 2
#define SECURITY_APP_PACKAGE_AUTHORITY {{0, 0, 0, 0, 0, 15}}
#define SECURITY_BUILTIN_APP_PACKAGE_RID_COUNT 2
#define SECURITY_APP_PACKAGE_BASE_RID 2
#define SECURITY_BUILTIN_PACKAGE_ANY_PACKAGE 1

#define IO_REPARSE_TAG_MOUNT_POINT 0xA0000003L
#define IO_REPARSE_TAG_DFS 0x8000000AL
#define IO_REPARSE_TAG_SYMLINK 0xA000000CL

#define FAST_FAIL_FATAL_APP_EXIT 7
#define ERROR_SEVERITY_ERROR 0xC0000000

#define LANG_NEUTRAL 0
#define LANG_ENGLISH 0x09
#define SUBLANG_DEFAULT 1
#define SUBLANG_ENGLISH_US 0x01
#define MAKELANGID(primary, sublanguage) static_cast<WORD>(((sublanguage) << 10) | (primary))

extern "C"
{
    LONG WINAPI InterlockedIncrement(volatile LONG* value);
    LONG WINAPI InterlockedDecrement(volatile LONG* value);
    LONG WINAPI InterlockedExchange(volatile LONG* target, LONG value);
    LONG _InterlockedExchange(volatile LONG* target, LONG value);
    PVOID WINAPI InterlockedCompareExchangePointer(PVOID volatile* destination, PVOID exchange, PVOID comparand);
    PVOID WINAPI InterlockedExchangePointer(PVOID volatile* destination, PVOID value);
}

#endif // __has_include_next(<winnt.h>)
