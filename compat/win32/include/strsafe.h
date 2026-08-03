#if __has_include_next(<strsafe.h>)
#include_next <strsafe.h>
#else

#pragma once

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cwchar>

#include <minwindef.h>
#include <winerror.h>

#define STRSAFE_MAX_CCH 2147483647
#define STRSAFE_E_INSUFFICIENT_BUFFER static_cast<HRESULT>(0x8007007AUL)
#define STRSAFE_E_INVALID_PARAMETER static_cast<HRESULT>(0x80070057UL)

inline HRESULT StringCchLengthW(const wchar_t* source, size_t maximum, size_t* length)
{
    if ((source == nullptr) || (length == nullptr))
    {
        return E_INVALIDARG;
    }
    size_t actual = 0;
    while (actual < maximum && source[actual] != L'\0')
    {
        ++actual;
    }
    if (actual == maximum)
    {
        *length = 0;
        return STRSAFE_E_INVALID_PARAMETER;
    }
    *length = actual;
    return S_OK;
}

inline HRESULT StringCchCopyW(wchar_t* destination, size_t capacity, const wchar_t* source)
{
    if ((destination == nullptr) || (capacity == 0) || (source == nullptr))
    {
        return E_INVALIDARG;
    }
    const size_t length = std::wcslen(source);
    if (length >= capacity)
    {
        destination[0] = L'\0';
        return STRSAFE_E_INSUFFICIENT_BUFFER;
    }
    std::wmemcpy(destination, source, length + 1);
    return S_OK;
}

inline HRESULT StringCchCopyA(char* destination, size_t capacity, const char* source)
{
    if ((destination == nullptr) || (capacity == 0) || (source == nullptr))
    {
        return E_INVALIDARG;
    }
    const size_t length = std::strlen(source);
    if (length >= capacity)
    {
        destination[0] = '\0';
        return STRSAFE_E_INSUFFICIENT_BUFFER;
    }
    std::memcpy(destination, source, length + 1);
    return S_OK;
}

inline HRESULT StringCchCopyNW(wchar_t* destination, size_t capacity, const wchar_t* source, size_t count)
{
    if ((destination == nullptr) || (capacity == 0) || (source == nullptr))
    {
        return E_INVALIDARG;
    }
    size_t length = 0;
    while (length < count && source[length] != L'\0')
    {
        ++length;
    }
    if (length >= capacity)
    {
        destination[0] = L'\0';
        return STRSAFE_E_INSUFFICIENT_BUFFER;
    }
    std::wmemcpy(destination, source, length);
    destination[length] = L'\0';
    return S_OK;
}

inline HRESULT StringCchCatW(wchar_t* destination, size_t capacity, const wchar_t* source)
{
    size_t length = 0;
    const HRESULT result = StringCchLengthW(destination, capacity, &length);
    return FAILED(result) ? result : StringCchCopyW(destination + length, capacity - length, source);
}

inline HRESULT StringCchCatExW(
    wchar_t* destination,
    size_t capacity,
    const wchar_t* source,
    wchar_t** destinationEnd,
    size_t* remaining,
    DWORD)
{
    const HRESULT result = StringCchCatW(destination, capacity, source);
    if (SUCCEEDED(result))
    {
        const size_t length = std::wcslen(destination);
        if (destinationEnd != nullptr)
        {
            *destinationEnd = destination + length;
        }
        if (remaining != nullptr)
        {
            *remaining = capacity - length;
        }
    }
    return result;
}

inline HRESULT StringCchVPrintfW(wchar_t* destination, size_t capacity, const wchar_t* format, va_list arguments)
{
    if ((destination == nullptr) || (capacity == 0) || (format == nullptr))
    {
        return E_INVALIDARG;
    }
    const int count = std::vswprintf(destination, capacity, format, arguments);
    if ((count < 0) || (static_cast<size_t>(count) >= capacity))
    {
        destination[0] = L'\0';
        return STRSAFE_E_INSUFFICIENT_BUFFER;
    }
    return S_OK;
}

inline HRESULT StringCchVPrintfA(char* destination, size_t capacity, const char* format, va_list arguments)
{
    if ((destination == nullptr) || (capacity == 0) || (format == nullptr))
    {
        return E_INVALIDARG;
    }
    const int count = std::vsnprintf(destination, capacity, format, arguments);
    if ((count < 0) || (static_cast<size_t>(count) >= capacity))
    {
        destination[0] = '\0';
        return STRSAFE_E_INSUFFICIENT_BUFFER;
    }
    return S_OK;
}

inline HRESULT StringCchPrintfW(wchar_t* destination, size_t capacity, const wchar_t* format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    const HRESULT result = StringCchVPrintfW(destination, capacity, format, arguments);
    va_end(arguments);
    return result;
}

inline HRESULT StringCchPrintfA(char* destination, size_t capacity, const char* format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    const HRESULT result = StringCchVPrintfA(destination, capacity, format, arguments);
    va_end(arguments);
    return result;
}

#ifdef UNICODE
#define StringCchCat StringCchCatW
#define StringCchCopy StringCchCopyW
#define StringCchPrintf StringCchPrintfW
#else
#define StringCchCat StringCchCatA
#define StringCchCopy StringCchCopyA
#define StringCchPrintf StringCchPrintfA
#endif

#endif // __has_include_next(<strsafe.h>)
