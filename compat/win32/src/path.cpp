#include <system_error>

#include <shlwapi.h>

#include "internal/strings.h"

extern "C" BOOL WINAPI PathFileExistsW(LPCWSTR const path)
{
    std::error_code error;
    const bool exists = std::filesystem::exists(win32_compat::ToPath(path), error);
    if (!exists)
    {
        SetLastError(error ? ERROR_PATH_NOT_FOUND : ERROR_FILE_NOT_FOUND);
    }
    return exists;
}

extern "C" BOOL WINAPI PathIsRelativeW(LPCWSTR const path) { return win32_compat::ToPath(path).is_relative(); }

extern "C" BOOL WINAPI PathIsNetworkPathW(LPCWSTR const path)
{
    const std::u16string_view value = win32_compat::WideView(path);
    return value.starts_with(u"\\\\");
}

extern "C" BOOL WINAPI PathRelativePathToW(LPWSTR const path, LPCWSTR const from, DWORD, LPCWSTR const to, DWORD)
{
    std::error_code error;
    std::filesystem::path fromPath = win32_compat::ToPath(from);
    if (!std::filesystem::is_directory(fromPath, error))
    {
        fromPath = fromPath.parent_path();
    }
    const std::filesystem::path relative = std::filesystem::relative(win32_compat::ToPath(to), fromPath, error);
    if (error)
    {
        SetLastError(ERROR_BAD_PATHNAME);
        return FALSE;
    }
    std::u16string result = win32_compat::ToWindowsPath(relative);
    if (!result.starts_with(u"."))
    {
        result.insert(0, u".\\");
    }
    if (!win32_compat::CopyWide(result, path, MAX_PATH))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
    return TRUE;
}

extern "C" LPWSTR WINAPI PathRemoveBackslashW(LPWSTR const path)
{
    auto* const text = reinterpret_cast<char16_t*>(path);
    std::size_t length = win32_compat::WideView(path).size();
    if (length > 0 && (text[length - 1] == u'\\' || text[length - 1] == u'/'))
    {
        text[--length] = u'\0';
    }
    return reinterpret_cast<LPWSTR>(text + length);
}
