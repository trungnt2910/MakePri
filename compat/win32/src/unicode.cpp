#include <algorithm>
#include <cstdint>
#include <string>

#include <stringapiset.h>

#include <uni_algo/conv.h>

#include "internal/strings.h"

namespace
{
std::u16string DecodeUtf8(const std::string_view input, bool& valid)
{
    una::error error;
    std::u16string output = una::strict::utf8to16<char, char16_t>(input, error);
    valid = !error;
    return valid ? output : una::utf8to16<char, char16_t>(input);
}

std::string EncodeUtf8(const std::u16string_view input, bool& valid)
{
    una::error error;
    std::string output = una::strict::utf16to8<char16_t, char>(input, error);
    valid = !error;
    return valid ? output : una::utf16to8<char16_t, char>(input);
}

std::u16string_view CountedWide(const LPCWCH source, const int length, bool& includesNull)
{
    includesNull = length < 0;
    if (length < 0)
    {
        return win32_compat::WideView(source);
    }
    return {reinterpret_cast<const char16_t*>(source), static_cast<std::size_t>(length)};
}

} // namespace

extern "C" int WINAPI MultiByteToWideChar(
    const UINT codePage,
    const DWORD flags,
    const LPCCH source,
    const int sourceLength,
    LPWSTR const destination,
    const int destinationLength)
{
    if (source == nullptr || sourceLength == 0 || (codePage != CP_UTF8 && codePage != CP_ACP))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }
    const bool includesNull = sourceLength < 0;
    const std::size_t bytes = includesNull ? std::char_traits<char>::length(source) : static_cast<std::size_t>(sourceLength);
    bool valid = true;
    std::u16string converted;
    converted = DecodeUtf8({source, bytes}, valid);
    if (!valid && (flags & MB_ERR_INVALID_CHARS) != 0)
    {
        SetLastError(ERROR_NO_UNICODE_TRANSLATION);
        return 0;
    }
    const std::size_t required = converted.size() + (includesNull ? 1 : 0);
    if (destination == nullptr || destinationLength == 0)
    {
        return static_cast<int>(required);
    }
    if (static_cast<std::size_t>(destinationLength) < required)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return 0;
    }
    auto* const output = reinterpret_cast<char16_t*>(destination);
    std::copy(converted.begin(), converted.end(), output);
    if (includesNull)
        output[converted.size()] = u'\0';
    return static_cast<int>(required);
}

extern "C" int WINAPI WideCharToMultiByte(
    const UINT codePage,
    const DWORD flags,
    const LPCWCH source,
    const int sourceLength,
    LPSTR const destination,
    const int destinationLength,
    LPCSTR,
    LPBOOL const usedDefaultChar)
{
    if (source == nullptr || sourceLength == 0 || (codePage != CP_UTF8 && codePage != CP_ACP))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }
    bool includesNull = false;
    const std::u16string_view input = CountedWide(source, sourceLength, includesNull);
    bool valid = true;
    std::string converted;
    converted = EncodeUtf8(input, valid);
    if (usedDefaultChar != nullptr)
        *usedDefaultChar = valid ? FALSE : TRUE;
    if (!valid && (flags & WC_ERR_INVALID_CHARS) != 0)
    {
        SetLastError(ERROR_NO_UNICODE_TRANSLATION);
        return 0;
    }
    const std::size_t required = converted.size() + (includesNull ? 1 : 0);
    if (destination == nullptr || destinationLength == 0)
        return static_cast<int>(required);
    if (static_cast<std::size_t>(destinationLength) < required)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return 0;
    }
    std::copy(converted.begin(), converted.end(), destination);
    if (includesNull)
        destination[converted.size()] = '\0';
    return static_cast<int>(required);
}
