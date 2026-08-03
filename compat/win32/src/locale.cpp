#include <algorithm>
#include <string>

#include <stringapiset.h>
#include <winnls.h>

#include "internal/strings.h"

namespace
{
std::u16string_view CountedWide(const LPCWCH source, const int length, bool& includesNull)
{
    includesNull = length < 0;
    if (length < 0)
    {
        return win32_compat::WideView(source);
    }
    return {reinterpret_cast<const char16_t*>(source), static_cast<std::size_t>(length)};
}

int Compare(const std::u16string_view left, const std::u16string_view right, const bool ignoreCase)
{
    const std::size_t count = std::min(left.size(), right.size());
    for (std::size_t index = 0; index < count; ++index)
    {
        char16_t leftValue = left[index];
        char16_t rightValue = right[index];
        if (ignoreCase)
        {
            leftValue = win32_compat::FoldAscii(leftValue);
            rightValue = win32_compat::FoldAscii(rightValue);
        }
        if (leftValue != rightValue)
        {
            return leftValue < rightValue ? CSTR_LESS_THAN : CSTR_GREATER_THAN;
        }
    }
    return left.size() < right.size() ? CSTR_LESS_THAN : (left.size() > right.size() ? CSTR_GREATER_THAN : CSTR_EQUAL);
}
} // namespace

extern "C" int WINAPI
CompareStringOrdinal(LPCWCH const left, const int leftLength, LPCWCH const right, const int rightLength, const BOOL ignoreCase)
{
    bool unused = false;
    return Compare(CountedWide(left, leftLength, unused), CountedWide(right, rightLength, unused), ignoreCase != FALSE);
}

extern "C" int WINAPI
CompareStringW(LCID, const DWORD flags, LPCWSTR const left, const int leftLength, LPCWSTR const right, const int rightLength)
{
    bool unused = false;
    return Compare(CountedWide(left, leftLength, unused), CountedWide(right, rightLength, unused), (flags & NORM_IGNORECASE) != 0);
}

extern "C" int WINAPI LCMapStringEx(
    LPCWSTR,
    const DWORD flags,
    LPCWSTR const source,
    const int sourceLength,
    LPWSTR const destination,
    const int destinationLength,
    LPVOID,
    LPVOID,
    LONG_PTR)
{
    bool includesNull = false;
    const std::u16string_view input = CountedWide(source, sourceLength, includesNull);
    const std::size_t required = input.size() + (includesNull ? 1 : 0);
    if (destination == nullptr || destinationLength == 0)
        return static_cast<int>(required);
    if (static_cast<std::size_t>(destinationLength) < required)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return 0;
    }
    auto* const output = reinterpret_cast<char16_t*>(destination);
    for (std::size_t index = 0; index < input.size(); ++index)
    {
        char16_t value = input[index];
        if ((flags & LCMAP_LOWERCASE) != 0)
            value = win32_compat::FoldAscii(value);
        else if ((flags & LCMAP_UPPERCASE) != 0 && value >= u'a' && value <= u'z')
            value -= u'a' - u'A';
        output[index] = value;
    }
    if (includesNull)
        output[input.size()] = u'\0';
    return static_cast<int>(required);
}

extern "C" BOOL WINAPI SetThreadPreferredUILanguages(DWORD, LPCWSTR, PULONG const languageCount)
{
    if (languageCount != nullptr)
        *languageCount = 1;
    return TRUE;
}

extern "C" BOOL WINAPI GetThreadPreferredUILanguages(DWORD, PULONG const languageCount, PWSTR const buffer, PULONG const bufferLength)
{
    constexpr std::u16string_view Languages(u"en-US\0", 6);
    if (languageCount != nullptr)
        *languageCount = 1;
    if (bufferLength == nullptr)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    if (buffer == nullptr || *bufferLength < Languages.size() + 1)
    {
        *bufferLength = static_cast<ULONG>(Languages.size() + 1);
        return buffer == nullptr ? TRUE : FALSE;
    }
    auto* const output = reinterpret_cast<char16_t*>(buffer);
    std::copy(Languages.begin(), Languages.end(), output);
    output[Languages.size()] = u'\0';
    *bufferLength = static_cast<ULONG>(Languages.size() + 1);
    return TRUE;
}
