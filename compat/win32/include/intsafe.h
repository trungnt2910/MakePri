#if __has_include_next(<intsafe.h>)
#include_next <intsafe.h>

#ifndef SIZE_T_MAX
#define SIZE_T_MAX static_cast<SIZE_T>(-1)
#endif

#else

#pragma once

#include <limits>

#include <minwindef.h>
#include <winerror.h>

#define SIZE_T_MAX static_cast<SIZE_T>(-1)

inline HRESULT SizeTToInt(const size_t value, int* const result)
{
    if (result == nullptr || value > static_cast<size_t>((std::numeric_limits<int>::max)()))
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
    *result = static_cast<int>(value);
    return S_OK;
}

inline HRESULT SizeTToUInt(const size_t value, UINT* const result)
{
    if (result == nullptr || value > static_cast<size_t>((std::numeric_limits<UINT>::max)()))
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
    *result = static_cast<UINT>(value);
    return S_OK;
}

inline HRESULT SizeTToUInt16(const size_t value, UINT16* const result)
{
    if (result == nullptr || value > static_cast<size_t>((std::numeric_limits<UINT16>::max)()))
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
    *result = static_cast<UINT16>(value);
    return S_OK;
}

inline HRESULT IntToUInt16(const int value, UINT16* const result)
{
    if (result == nullptr || value < 0 || value > static_cast<int>((std::numeric_limits<UINT16>::max)()))
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
    *result = static_cast<UINT16>(value);
    return S_OK;
}

inline HRESULT IntToUShort(const int value, USHORT* const result)
{
    if (result == nullptr || value < 0 || value > static_cast<int>((std::numeric_limits<USHORT>::max)()))
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
    *result = static_cast<USHORT>(value);
    return S_OK;
}
inline HRESULT SizeTMult(size_t left, size_t right, size_t* result)
{
    if (result == nullptr || (right != 0 && left > (std::numeric_limits<size_t>::max)() / right))
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
    *result = left * right;
    return S_OK;
}

inline HRESULT SizeTAdd(size_t left, size_t right, size_t* result)
{
    if (result == nullptr || left > (std::numeric_limits<size_t>::max)() - right)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
    *result = left + right;
    return S_OK;
}

inline HRESULT DWordAdd(DWORD left, DWORD right, DWORD* result)
{
    if (result == nullptr || left > (std::numeric_limits<DWORD>::max)() - right)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
    *result = left + right;
    return S_OK;
}

#endif
