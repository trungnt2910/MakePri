#if __has_include_next(<winreg.h>)
#include_next <winreg.h>
#else

#pragma once

#include <minwindef.h>

using LSTATUS = LONG;

#define HKEY_LOCAL_MACHINE reinterpret_cast<HKEY>(static_cast<std::intptr_t>(0x80000002UL))
#define RRF_RT_REG_SZ 0x00000002
#define RRF_RT_REG_EXPAND_SZ 0x00000004

extern "C" LSTATUS WINAPI RegGetValueW(HKEY key, LPCWSTR subKey, LPCWSTR value, DWORD flags, LPDWORD type, PVOID data, LPDWORD dataSize);

#endif // __has_include_next(<winreg.h>)
