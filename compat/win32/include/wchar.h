#pragma once

#include_next <wchar.h>

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <corecrt.h>

#ifndef _TRUNCATE
#define _TRUNCATE static_cast<size_t>(-1)
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    int _wtoi(const wchar_t* value);
    double _wtof(const wchar_t* value);
    int _snwprintf_s(wchar_t* buffer, size_t size, size_t count, const wchar_t* format, ...);
    int vswprintf_s(wchar_t* buffer, size_t size, const wchar_t* format, va_list arguments);
    int vwprintf_s(const wchar_t* format, va_list arguments);
    int wprintf_s(const wchar_t* format, ...);
    int swscanf_s(const wchar_t* buffer, const wchar_t* format, ...);
    int wcscpy_s(wchar_t* destination, size_t destinationSize, const wchar_t* source);
    int wcscat_s(wchar_t* destination, size_t destinationSize, const wchar_t* source);
    int wcsncpy_s(wchar_t* destination, size_t destinationSize, const wchar_t* source, size_t count);
    int _ui64tow_s(uint64_t value, wchar_t* buffer, size_t size, int radix);
    int _ultow_s(unsigned long value, wchar_t* buffer, size_t size, int radix);
    int _vscwprintf_l(const wchar_t* format, _locale_t locale, va_list arguments);
    int _wcsnicmp(const wchar_t* left, const wchar_t* right, size_t count);
#ifdef __cplusplus
}
#endif
