#pragma once

#include <wil/resource.h>

namespace wil
{
inline const wchar_t* find_last_path_segment(const wchar_t* path) noexcept
{
    if (path == nullptr)
        return nullptr;
    const wchar_t* result = path;
    for (const wchar_t* current = path; *current != L'\0'; ++current)
    {
        if (*current == L'/' || *current == L'\\')
            result = current + 1;
    }
    return result;
}
} // namespace wil
