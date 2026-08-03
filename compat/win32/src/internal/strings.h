#pragma once

#include <algorithm>
#include <filesystem>
#include <string>
#include <string_view>

#include <windows.h>

namespace win32_compat
{

inline std::u16string_view WideView(const LPCWSTR value)
{
    if (value == nullptr)
    {
        return {};
    }

    const auto* const text = reinterpret_cast<const char16_t*>(value);
    std::size_t length = 0;
    while (text[length] != u'\0')
    {
        ++length;
    }
    return {text, length};
}

inline std::u16string ToWindowsPath(const std::filesystem::path& path)
{
    std::u16string result = path.u16string();
    for (char16_t& character : result)
    {
        if (character == u'/')
        {
            character = u'\\';
        }
    }
    return result;
}

inline std::filesystem::path ToPath(const LPCWSTR value)
{
    std::u16string text(WideView(value));
    if (text.starts_with(u"\\\\?\\"))
    {
        text.erase(0, 4);
    }
    for (char16_t& character : text)
    {
        if (character == u'\\')
        {
            character = u'/';
        }
    }
    return std::filesystem::path(text);
}

inline bool CopyWide(const std::u16string_view source, LPWSTR destination, const std::size_t capacity)
{
    if (destination == nullptr || capacity <= source.size())
    {
        return false;
    }
    auto* const output = reinterpret_cast<char16_t*>(destination);
    std::copy(source.begin(), source.end(), output);
    output[source.size()] = u'\0';
    return true;
}

inline char16_t FoldAscii(const char16_t value)
{
    return (value >= u'A' && value <= u'Z') ? static_cast<char16_t>(value + (u'a' - u'A')) : value;
}

} // namespace win32_compat
