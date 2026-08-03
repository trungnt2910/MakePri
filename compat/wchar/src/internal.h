#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <string_view>

namespace wchar_compat
{

inline std::u16string_view View(const char16_t* const value)
{
    if (value == nullptr)
    {
        return {};
    }
    std::size_t length = 0;
    while (value[length] != u'\0')
    {
        ++length;
    }
    return {value, length};
}

inline int CopyResult(char16_t* const buffer, const std::size_t size, const std::u16string_view value)
{
    if (buffer == nullptr || size == 0)
    {
        return -1;
    }
    const std::size_t copied = std::min(value.size(), size - 1);
    std::copy_n(value.data(), copied, buffer);
    buffer[copied] = u'\0';
    return copied == value.size() ? static_cast<int>(copied) : -1;
}

int WriteWide(std::FILE* stream, std::u16string_view value);

} // namespace wchar_compat
