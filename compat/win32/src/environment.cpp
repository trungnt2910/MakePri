#include <cstdlib>
#include <string>

#include <fileapi.h>
#include <processenv.h>
#include <sysinfoapi.h>

#include <uni_algo/conv.h>

#include "internal/strings.h"

namespace
{
std::u16string EnvironmentValue(const std::u16string_view name)
{
    const std::string narrow = una::utf16to8<char16_t, char>(name);
    const char* const value = std::getenv(narrow.c_str());
    return value == nullptr ? std::u16string() : una::utf8to16<char, char16_t>(value);
}

DWORD CopySystemPath(const std::u16string_view path, LPWSTR const buffer, const DWORD size)
{
    if (!win32_compat::CopyWide(path, buffer, size))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return static_cast<DWORD>(path.size() + 1);
    }
    return static_cast<DWORD>(path.size());
}
} // namespace

extern "C" DWORD WINAPI GetTempPathW(const DWORD length, LPWSTR const buffer)
{
    std::error_code error;
    std::u16string path = win32_compat::ToWindowsPath(std::filesystem::temp_directory_path(error));
    if (error)
    {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return 0;
    }
    if (path.empty() || path.back() != u'\\')
    {
        path.push_back(u'\\');
    }
    return CopySystemPath(path, buffer, length);
}

extern "C" DWORD WINAPI GetSystemWindowsDirectoryW(LPWSTR const buffer, const UINT size) { return CopySystemPath(u"\\", buffer, size); }

extern "C" DWORD WINAPI GetEnvironmentVariableW(LPCWSTR const name, LPWSTR const buffer, const DWORD size)
{
    const std::u16string value = EnvironmentValue(win32_compat::WideView(name));
    if (value.empty())
    {
        SetLastError(ERROR_BAD_ENVIRONMENT);
        return 0;
    }
    if (!win32_compat::CopyWide(value, buffer, size))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return static_cast<DWORD>(value.size() + 1);
    }
    return static_cast<DWORD>(value.size());
}

extern "C" DWORD WINAPI ExpandEnvironmentStringsW(LPCWSTR const source, LPWSTR const destination, const DWORD size)
{
    const std::u16string_view input = win32_compat::WideView(source);
    std::u16string result;
    for (std::size_t index = 0; index < input.size();)
    {
        if (input[index] == u'%')
        {
            const std::size_t end = input.find(u'%', index + 1);
            if (end != std::u16string_view::npos)
            {
                const std::u16string value = EnvironmentValue(input.substr(index + 1, end - index - 1));
                if (!value.empty())
                {
                    result.append(value);
                    index = end + 1;
                    continue;
                }
            }
        }
        result.push_back(input[index++]);
    }
    if (!win32_compat::CopyWide(result, destination, size))
    {
        return static_cast<DWORD>(result.size() + 1);
    }
    return static_cast<DWORD>(result.size() + 1);
}
