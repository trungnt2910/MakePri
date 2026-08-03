#include <algorithm>
#include <cstring>

#include "internal.h"

extern "C" std::size_t __wrap_wcslen(const char16_t* const value) { return wchar_compat::View(value).size(); }

extern "C" std::size_t __wrap_wcsnlen(const char16_t* const value, const std::size_t maximum)
{
    std::size_t length = 0;
    while (length < maximum && value[length] != u'\0')
        ++length;
    return length;
}

extern "C" int __wrap_wcscmp(const char16_t* left, const char16_t* right)
{
    while (*left != u'\0' && *left == *right)
    {
        ++left;
        ++right;
    }
    return *left < *right ? -1 : (*left > *right ? 1 : 0);
}

extern "C" int __wrap_wcsncmp(const char16_t* left, const char16_t* right, std::size_t count)
{
    while (count > 0 && *left != u'\0' && *left == *right)
    {
        ++left;
        ++right;
        --count;
    }
    return count == 0 ? 0 : (*left < *right ? -1 : (*left > *right ? 1 : 0));
}

extern "C" int __wrap_wcscasecmp(const char16_t* const left, const char16_t* const right)
{
    const std::size_t count = std::max(wchar_compat::View(left).size(), wchar_compat::View(right).size()) + 1;
    for (std::size_t index = 0; index < count; ++index)
    {
        char16_t leftValue = left[index];
        char16_t rightValue = right[index];
        if (leftValue >= u'A' && leftValue <= u'Z')
            leftValue += u'a' - u'A';
        if (rightValue >= u'A' && rightValue <= u'Z')
            rightValue += u'a' - u'A';
        if (leftValue != rightValue)
            return leftValue < rightValue ? -1 : 1;
        if (leftValue == u'\0')
            return 0;
    }
    return 0;
}

extern "C" char16_t* __wrap_wcschr(const char16_t* const value, const char16_t character)
{
    const std::u16string_view input = wchar_compat::View(value);
    const auto position = input.find(character);
    return position == std::u16string_view::npos ? nullptr : const_cast<char16_t*>(value + position);
}

extern "C" char16_t* __wrap_wcsrchr(const char16_t* const value, const char16_t character)
{
    const std::u16string_view input = wchar_compat::View(value);
    const auto position = input.rfind(character);
    return position == std::u16string_view::npos ? nullptr : const_cast<char16_t*>(value + position);
}

extern "C" char16_t* __wrap_wcsstr(const char16_t* const value, const char16_t* const searched)
{
    const auto position = wchar_compat::View(value).find(wchar_compat::View(searched));
    return position == std::u16string_view::npos ? nullptr : const_cast<char16_t*>(value + position);
}

extern "C" std::size_t __wrap_wcscspn(const char16_t* const value, const char16_t* const rejected)
{
    const auto position = wchar_compat::View(value).find_first_of(wchar_compat::View(rejected));
    return position == std::u16string_view::npos ? wchar_compat::View(value).size() : position;
}

extern "C" char16_t* __wrap_wmemchr(const char16_t* const value, const char16_t character, const std::size_t count)
{
    for (std::size_t index = 0; index < count; ++index)
    {
        if (value[index] == character)
        {
            return const_cast<char16_t*>(value + index);
        }
    }
    return nullptr;
}

extern "C" int __wrap_wmemcmp(const char16_t* const left, const char16_t* const right, const std::size_t count)
{
    return __wrap_wcsncmp(left, right, count);
}

extern "C" char16_t* __wrap_wmemcpy(char16_t* const destination, const char16_t* const source, const std::size_t count)
{
    std::memcpy(destination, source, count * sizeof(char16_t));
    return destination;
}

extern "C" char16_t* __wrap_wmemmove(char16_t* const destination, const char16_t* const source, const std::size_t count)
{
    std::memmove(destination, source, count * sizeof(char16_t));
    return destination;
}

extern "C" char16_t* __wrap_wmemset(char16_t* const destination, const char16_t value, const std::size_t count)
{
    for (std::size_t index = 0; index < count; ++index)
    {
        destination[index] = value;
    }
    return destination;
}
