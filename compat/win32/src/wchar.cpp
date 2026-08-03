#include <algorithm>
#include <array>
#include <cerrno>
#include <charconv>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <string>
#include <vector>

#include <wchar.h>

#include <uni_algo/conv.h>

#include "internal/strings.h"

namespace
{
int CopyWideResult(wchar_t* const buffer, const std::size_t size, const std::u16string_view value)
{
    return win32_compat::CopyWide(value, buffer, size) ? static_cast<int>(value.size()) : -1;
}
} // namespace

extern "C" int vswprintf_s(wchar_t* const buffer, const std::size_t size, const wchar_t* const format, va_list arguments)
{
    if (buffer == nullptr || size == 0 || format == nullptr)
    {
        return -1;
    }
    return std::vswprintf(buffer, size, format, arguments);
}

extern "C" int _snwprintf_s(wchar_t* const buffer, const std::size_t size, const std::size_t count, const wchar_t* const format, ...)
{
    const std::size_t limit = count == _TRUNCATE ? size : std::min(size, count + 1);
    va_list arguments;
    va_start(arguments, format);
    const int result = vswprintf_s(buffer, limit, format, arguments);
    va_end(arguments);
    return result;
}

extern "C" int _vscwprintf_l(const wchar_t* const format, _locale_t, va_list arguments)
{
    for (std::size_t size = 128; size <= (1U << 20); size *= 2)
    {
        std::vector<wchar_t> buffer(size);
        va_list copiedArguments;
        va_copy(copiedArguments, arguments);
        const int result = std::vswprintf(buffer.data(), buffer.size(), format, copiedArguments);
        va_end(copiedArguments);
        if (result >= 0)
        {
            return result;
        }
    }
    return -1;
}

extern "C" int vwprintf_s(const wchar_t* const format, va_list arguments)
{
    const int length = _vscwprintf_l(format, nullptr, arguments);
    if (length < 0)
    {
        return -1;
    }
    std::vector<wchar_t> buffer(static_cast<std::size_t>(length) + 1);
    va_list copiedArguments;
    va_copy(copiedArguments, arguments);
    const int result = std::vswprintf(buffer.data(), buffer.size(), format, copiedArguments);
    va_end(copiedArguments);
    return result < 0 ? result : std::fwprintf(stdout, L"%s", buffer.data());
}

extern "C" int wprintf_s(const wchar_t* const format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    const int result = vwprintf_s(format, arguments);
    va_end(arguments);
    return result;
}

extern "C" int _wtoi(const wchar_t* const value)
{
    const std::string narrow = una::utf16to8<char16_t, char>(win32_compat::WideView(value));
    return static_cast<int>(std::strtol(narrow.c_str(), nullptr, 10));
}

extern "C" double _wtof(const wchar_t* const value)
{
    const std::string narrow = una::utf16to8<char16_t, char>(win32_compat::WideView(value));
    return std::strtod(narrow.c_str(), nullptr);
}

extern "C" int _ui64tow_s(const std::uint64_t value, wchar_t* const buffer, const std::size_t size, const int radix)
{
    std::array<char, 96> formatted {};
    const auto [end, error] = std::to_chars(formatted.data(), formatted.data() + formatted.size(), value, radix);
    if (error != std::errc())
        return EINVAL;
    return CopyWideResult(
               buffer, size, una::utf8to16<char, char16_t>({formatted.data(), static_cast<std::size_t>(end - formatted.data())})) < 0 ?
               ERANGE :
               0;
}

extern "C" int _ultow_s(const unsigned long value, wchar_t* const buffer, const std::size_t size, const int radix)
{
    return _ui64tow_s(value, buffer, size, radix);
}

extern "C" int wcscpy_s(wchar_t* const destination, const std::size_t size, const wchar_t* const source)
{
    return CopyWideResult(destination, size, win32_compat::WideView(source)) < 0 ? ERANGE : 0;
}

extern "C" int wcscat_s(wchar_t* const destination, const std::size_t size, const wchar_t* const source)
{
    const std::size_t used = win32_compat::WideView(destination).size();
    if (used >= size)
    {
        return ERANGE;
    }
    return CopyWideResult(destination + used, size - used, win32_compat::WideView(source)) < 0 ? ERANGE : 0;
}

extern "C" int wcsncpy_s(wchar_t* const destination, const std::size_t size, const wchar_t* const source, const std::size_t count)
{
    const std::u16string_view input = win32_compat::WideView(source);
    const std::size_t copied = count == _TRUNCATE ? input.size() : std::min(input.size(), count);
    return CopyWideResult(destination, size, input.substr(0, copied)) < 0 ? ERANGE : 0;
}

extern "C" int _wcsnicmp(const wchar_t* const left, const wchar_t* const right, const std::size_t count)
{
    for (std::size_t index = 0; index < count; ++index)
    {
        const char16_t leftValue = win32_compat::FoldAscii(reinterpret_cast<const char16_t*>(left)[index]);
        const char16_t rightValue = win32_compat::FoldAscii(reinterpret_cast<const char16_t*>(right)[index]);
        if (leftValue != rightValue)
            return leftValue < rightValue ? -1 : 1;
        if (leftValue == u'\0')
            return 0;
    }
    return 0;
}
