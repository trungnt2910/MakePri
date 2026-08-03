#include <cstdint>

#include <libloaderapi.h>

#include <boost/dll/runtime_symbol_info.hpp>

#include "internal/strings.h"

extern "C" DWORD WINAPI GetModuleFileNameW(HMODULE, LPWSTR const buffer, const DWORD size)
{
    std::error_code error;
    const auto location = boost::dll::program_location(error);
    if (error)
    {
        SetLastError(ERROR_FILE_NOT_FOUND);
        return 0;
    }
    const std::u16string path = win32_compat::ToWindowsPath(std::filesystem::path(location.string()));
    if (!win32_compat::CopyWide(path, buffer, size))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return size;
    }
    return static_cast<DWORD>(path.size());
}

extern "C" HMODULE WINAPI LoadLibraryExW(LPCWSTR, HANDLE, DWORD) { return reinterpret_cast<HMODULE>(static_cast<std::uintptr_t>(2)); }

extern "C" HMODULE WINAPI LoadLibraryW(LPCWSTR const fileName) { return LoadLibraryExW(fileName, nullptr, 0); }

extern "C" HMODULE WINAPI GetModuleHandleW(LPCWSTR) { return reinterpret_cast<HMODULE>(static_cast<std::uintptr_t>(2)); }

extern "C" BOOL WINAPI FreeLibrary(HMODULE) { return TRUE; }

extern "C" FARPROC WINAPI GetProcAddress(HMODULE, LPCSTR)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return nullptr;
}
