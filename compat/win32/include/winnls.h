#if __has_include_next(<winnls.h>)
#include_next <winnls.h>
#else

#pragma once

#include <minwindef.h>

#define MUI_CONSOLE_FILTER 0x100
#define NORM_IGNORECASE 0x00000001
#define LCMAP_LOWERCASE 0x00000100
#define LCMAP_UPPERCASE 0x00000200
#define LOCALE_NAME_INVARIANT L""
#define CP_ACP 0
#define CP_UTF8 65001
#define CSTR_LESS_THAN 1
#define CSTR_EQUAL 2
#define CSTR_GREATER_THAN 3
#define MB_ERR_INVALID_CHARS 0x00000008
#define WC_ERR_INVALID_CHARS 0x00000080
#define MUI_LANGUAGE_NAME 0x00000008
#define MUI_MERGE_SYSTEM_FALLBACK 0x00000010
#define MUI_MERGE_USER_FALLBACK 0x00000020
#define MUI_UI_FALLBACK (MUI_MERGE_SYSTEM_FALLBACK | MUI_MERGE_USER_FALLBACK)

extern "C"
{
    BOOL WINAPI SetThreadPreferredUILanguages(DWORD flags, LPCWSTR languages, PULONG languageCount);
    BOOL WINAPI GetThreadPreferredUILanguages(DWORD flags, PULONG languageCount, PWSTR languagesBuffer, PULONG bufferLength);
    int WINAPI CompareStringW(LCID locale, DWORD flags, LPCWSTR left, int leftLength, LPCWSTR right, int rightLength);
    int WINAPI LCMapStringEx(
        LPCWSTR localeName,
        DWORD flags,
        LPCWSTR source,
        int sourceLength,
        LPWSTR destination,
        int destinationLength,
        LPVOID versionInformation,
        LPVOID reserved,
        LONG_PTR sortHandle);
}

#endif // __has_include_next(<winnls.h>)
