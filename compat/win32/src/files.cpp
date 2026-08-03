#include <algorithm>
#include <limits>
#include <memory>
#include <mutex>
#include <system_error>
#include <unordered_map>
#include <vector>

#include <fileapi.h>
#include <handleapi.h>
#include <memoryapi.h>
#include <winbase.h>
#include <winerror.h>

#include <boost/interprocess/mapped_region.hpp>

#include "internal/handles.h"
#include "internal/strings.h"

namespace
{
using namespace win32_compat;
namespace bip = boost::interprocess;

std::mutex MappedViewsMutex;
std::unordered_map<const void*, std::unique_ptr<bip::mapped_region>> MappedViews;

struct FindHandle final : HandleBase
{
    FindHandle() : HandleBase(HandleKind::Find) {}
    std::vector<std::filesystem::directory_entry> entries;
    std::size_t index = 0;
};

bool WildcardMatches(const std::u16string_view pattern, const std::u16string_view value)
{
    std::size_t patternIndex = 0;
    std::size_t valueIndex = 0;
    std::size_t star = std::u16string_view::npos;
    std::size_t retry = 0;
    while (valueIndex < value.size())
    {
        if (patternIndex < pattern.size() &&
            (pattern[patternIndex] == u'?' || FoldAscii(pattern[patternIndex]) == FoldAscii(value[valueIndex])))
        {
            ++patternIndex;
            ++valueIndex;
        }
        else if (patternIndex < pattern.size() && pattern[patternIndex] == u'*')
        {
            star = patternIndex++;
            retry = valueIndex;
        }
        else if (star != std::u16string_view::npos)
        {
            patternIndex = star + 1;
            valueIndex = ++retry;
        }
        else
        {
            return false;
        }
    }
    while (patternIndex < pattern.size() && pattern[patternIndex] == u'*')
    {
        ++patternIndex;
    }
    return patternIndex == pattern.size();
}

void FillFindData(const std::filesystem::directory_entry& entry, WIN32_FIND_DATAW* const data)
{
    std::memset(data, 0, sizeof(*data));
    std::error_code error;
    data->dwFileAttributes = entry.is_directory(error) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
    error.clear();
    if (entry.is_symlink(error))
    {
        data->dwFileAttributes |= FILE_ATTRIBUTE_REPARSE_POINT;
    }
    const std::u16string name = entry.path().filename().u16string();
    CopyWide(name.substr(0, MAX_PATH - 1), data->cFileName, MAX_PATH);
    if (!entry.is_directory(error))
    {
        const std::uintmax_t size = entry.file_size(error);
        if (!error)
        {
            data->nFileSizeLow = static_cast<DWORD>(size);
            data->nFileSizeHigh = static_cast<DWORD>(size >> 32);
        }
    }
}

DWORD Attributes(const std::filesystem::path& path)
{
    std::error_code error;
    const auto status = std::filesystem::status(path, error);
    if (error || !std::filesystem::exists(status))
    {
        SetLastError(ERROR_FILE_NOT_FOUND);
        return INVALID_FILE_ATTRIBUTES;
    }
    DWORD result = std::filesystem::is_directory(status) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
    error.clear();
    if (std::filesystem::is_symlink(std::filesystem::symlink_status(path, error)))
    {
        result |= FILE_ATTRIBUTE_REPARSE_POINT;
    }
    const std::u16string name = path.filename().u16string();
    if (!name.empty() && name[0] == u'.')
    {
        result |= FILE_ATTRIBUTE_HIDDEN;
    }
    return result;
}

bool QueryStreamSize(std::FILE* const stream, std::uintmax_t* const size)
{
    const long position = std::ftell(stream);
    if (position < 0 || std::fseek(stream, 0, SEEK_END) != 0)
    {
        return false;
    }
    const long end = std::ftell(stream);
    const bool restored = std::fseek(stream, position, SEEK_SET) == 0;
    if (end < 0 || !restored)
    {
        return false;
    }
    *size = static_cast<std::uintmax_t>(end);
    return true;
}
} // namespace

extern "C" HANDLE WINAPI
CreateFileW(LPCWSTR const name, const DWORD access, DWORD, LPSECURITY_ATTRIBUTES, const DWORD creation, DWORD, HANDLE)
{
    const std::filesystem::path path = win32_compat::ToPath(name);
    const char* mode = nullptr;
    if (creation == CREATE_ALWAYS)
    {
        mode = (access & GENERIC_READ) != 0 ? "w+b" : "wb";
    }
    else
    {
        mode = (access & GENERIC_WRITE) != 0 ? "r+b" : "rb";
    }
    std::FILE* const stream = std::fopen(path.string().c_str(), mode);
    if (stream == nullptr)
    {
        SetLastError(std::filesystem::exists(path) ? ERROR_ACCESS_DENIED : ERROR_FILE_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }
    return new (std::nothrow) win32_compat::FileHandle(stream, path);
}

extern "C" BOOL WINAPI CloseHandle(const HANDLE handle)
{
    if (handle == nullptr || handle == INVALID_HANDLE_VALUE)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    delete win32_compat::AsHandle(handle);
    return TRUE;
}

extern "C" BOOL WINAPI GetFileSizeEx(const HANDLE handle, PLARGE_INTEGER const size)
{
    auto* const file = win32_compat::AsFile(handle);
    if (file == nullptr || size == nullptr)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    std::uintmax_t fileSize {};
    if (!QueryStreamSize(file->stream, &fileSize) || fileSize > static_cast<std::uintmax_t>(std::numeric_limits<LONGLONG>::max()))
    {
        SetLastError(ERROR_INVALID_DATA);
        return FALSE;
    }
    size->QuadPart = static_cast<LONGLONG>(fileSize);
    return TRUE;
}

extern "C" DWORD WINAPI GetFileSize(const HANDLE handle, LPDWORD const highSize)
{
    LARGE_INTEGER size {};
    if (!GetFileSizeEx(handle, &size))
    {
        return INVALID_FILE_SIZE;
    }
    if (highSize != nullptr)
    {
        *highSize = static_cast<DWORD>(static_cast<ULONGLONG>(size.QuadPart) >> 32);
    }
    return static_cast<DWORD>(size.QuadPart);
}

extern "C" DWORD WINAPI SetFilePointer(const HANDLE handle, const LONG distance, LONG* const distanceHigh, const DWORD method)
{
    auto* const file = win32_compat::AsFile(handle);
    if (file == nullptr)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return INVALID_SET_FILE_POINTER;
    }
    std::int64_t offset = static_cast<std::uint32_t>(distance);
    if (distanceHigh != nullptr)
    {
        offset |= static_cast<std::int64_t>(*distanceHigh) << 32;
    }
    const int origin = method == FILE_CURRENT ? SEEK_CUR : (method == FILE_END ? SEEK_END : SEEK_SET);
    if (std::fseek(file->stream, static_cast<long>(offset), origin) != 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_SET_FILE_POINTER;
    }
    const long position = std::ftell(file->stream);
    if (distanceHigh != nullptr)
    {
        *distanceHigh = static_cast<LONG>(static_cast<std::uint64_t>(position) >> 32);
    }
    return static_cast<DWORD>(position);
}

extern "C" BOOL WINAPI ReadFile(const HANDLE handle, LPVOID const buffer, const DWORD bytes, LPDWORD const read, LPVOID)
{
    auto* const file = win32_compat::AsFile(handle);
    if (file == nullptr)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    const std::size_t count = std::fread(buffer, 1, bytes, file->stream);
    if (read != nullptr)
    {
        *read = static_cast<DWORD>(count);
    }
    return std::ferror(file->stream) == 0;
}

extern "C" BOOL WINAPI WriteFile(const HANDLE handle, LPCVOID const buffer, const DWORD bytes, LPDWORD const written, LPVOID)
{
    auto* const file = win32_compat::AsFile(handle);
    if (file == nullptr)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    const std::size_t count = std::fwrite(buffer, 1, bytes, file->stream);
    if (written != nullptr)
    {
        *written = static_cast<DWORD>(count);
    }
    return count == bytes;
}

extern "C" BOOL WINAPI FlushFileBuffers(const HANDLE handle)
{
    auto* const file = win32_compat::AsFile(handle);
    return file != nullptr && std::fflush(file->stream) == 0;
}

extern "C" HANDLE WINAPI CreateFileMappingW(const HANDLE handle, LPSECURITY_ATTRIBUTES, const DWORD protect, DWORD, DWORD, LPCWSTR)
{
    auto* const file = win32_compat::AsFile(handle);
    if (file == nullptr)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return nullptr;
    }
    try
    {
        const auto mode = (protect & PAGE_READWRITE) != 0 ? bip::read_write : bip::read_only;
        return new (std::nothrow) win32_compat::MappingHandle(file->path, mode);
    }
    catch (const bip::interprocess_exception&)
    {
        SetLastError(ERROR_INVALID_DATA);
        return nullptr;
    }
}

extern "C" HANDLE WINAPI OpenFileMappingW(DWORD, BOOL, LPCWSTR)
{
    SetLastError(ERROR_FILE_NOT_FOUND);
    return nullptr;
}

extern "C" PVOID WINAPI
MapViewOfFile(const HANDLE handle, const DWORD access, const DWORD offsetHigh, const DWORD offsetLow, const SIZE_T requestedBytes)
{
    using namespace win32_compat;
    HandleBase* const base = AsHandle(handle);
    if (base == nullptr || base->kind != HandleKind::Mapping)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return nullptr;
    }
    auto* const mapping = static_cast<MappingHandle*>(base);
    try
    {
        const auto mode = (access & FILE_MAP_WRITE) != 0 ? bip::read_write : bip::read_only;
        const std::uint64_t offset = (static_cast<std::uint64_t>(offsetHigh) << 32) | offsetLow;
        auto view = std::make_unique<bip::mapped_region>(mapping->mapping, mode, offset, requestedBytes);
        void* const address = view->get_address();
        const std::scoped_lock lock(MappedViewsMutex);
        MappedViews.emplace(address, std::move(view));
        return address;
    }
    catch (const bip::interprocess_exception&)
    {
        SetLastError(ERROR_INVALID_DATA);
        return nullptr;
    }
}

extern "C" BOOL WINAPI UnmapViewOfFile(LPCVOID const address)
{
    const std::scoped_lock lock(MappedViewsMutex);
    if (MappedViews.erase(address) == 0)
    {
        SetLastError(ERROR_INVALID_ADDRESS);
        return FALSE;
    }
    return TRUE;
}

extern "C" HANDLE WINAPI FindFirstFileW(LPCWSTR const name, LPWIN32_FIND_DATAW const data)
{
    using namespace win32_compat;
    const std::filesystem::path search = ToPath(name);
    std::filesystem::path directory = search.parent_path();
    if (directory.empty())
    {
        directory = std::filesystem::current_path();
    }
    std::u16string pattern = search.filename().u16string();
    if (pattern == u"*.*")
    {
        pattern = u"*";
    }
    auto* const find = new (std::nothrow) FindHandle();
    if (find == nullptr)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return INVALID_HANDLE_VALUE;
    }
    std::error_code error;
    if (WildcardMatches(pattern, u"."))
    {
        find->entries.emplace_back(directory / ".");
    }
    if (WildcardMatches(pattern, u".."))
    {
        find->entries.emplace_back(directory / "..");
    }
    for (const auto& entry : std::filesystem::directory_iterator(directory, error))
    {
        if (WildcardMatches(pattern, entry.path().filename().u16string()))
        {
            find->entries.push_back(entry);
        }
    }
    if (error || find->entries.empty())
    {
        delete find;
        SetLastError(error ? ERROR_PATH_NOT_FOUND : ERROR_FILE_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }
    FillFindData(find->entries.front(), data);
    return find;
}

extern "C" BOOL WINAPI FindNextFileW(const HANDLE handle, LPWIN32_FIND_DATAW const data)
{
    using namespace win32_compat;
    HandleBase* const base = AsHandle(handle);
    if (base == nullptr || base->kind != HandleKind::Find)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    auto* const find = static_cast<FindHandle*>(base);
    if (++find->index >= find->entries.size())
    {
        SetLastError(ERROR_NO_MORE_FILES);
        return FALSE;
    }
    FillFindData(find->entries[find->index], data);
    return TRUE;
}

extern "C" BOOL WINAPI FindClose(const HANDLE handle) { return CloseHandle(handle); }

extern "C" DWORD WINAPI GetFileAttributesW(LPCWSTR const name) { return Attributes(win32_compat::ToPath(name)); }

extern "C" BOOL WINAPI GetFileAttributesExW(LPCWSTR const name, GET_FILEEX_INFO_LEVELS, LPVOID const information)
{
    auto* const data = static_cast<WIN32_FILE_ATTRIBUTE_DATA*>(information);
    std::memset(data, 0, sizeof(*data));
    const std::filesystem::path path = win32_compat::ToPath(name);
    data->dwFileAttributes = Attributes(path);
    if (data->dwFileAttributes == INVALID_FILE_ATTRIBUTES)
    {
        return FALSE;
    }
    if ((data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
        std::error_code error;
        const std::uintmax_t size = std::filesystem::file_size(path, error);
        if (!error)
        {
            data->nFileSizeLow = static_cast<DWORD>(size);
            data->nFileSizeHigh = static_cast<DWORD>(size >> 32);
        }
    }
    return TRUE;
}

extern "C" BOOL WINAPI DeleteFileW(LPCWSTR const name)
{
    std::error_code error;
    const bool removed = std::filesystem::remove(win32_compat::ToPath(name), error);
    if (!removed)
    {
        SetLastError(error ? ERROR_ACCESS_DENIED : ERROR_FILE_NOT_FOUND);
    }
    return removed;
}

extern "C" BOOL WINAPI RemoveDirectoryW(LPCWSTR const name) { return DeleteFileW(name); }

extern "C" BOOL WINAPI CreateDirectoryW(LPCWSTR const name, LPSECURITY_ATTRIBUTES)
{
    std::error_code error;
    const std::filesystem::path path = win32_compat::ToPath(name);
    const bool created = std::filesystem::create_directory(path, error);
    if (!created)
    {
        SetLastError(std::filesystem::exists(path) ? ERROR_ALREADY_EXISTS : ERROR_PATH_NOT_FOUND);
    }
    return created;
}

extern "C" BOOL WINAPI CopyFileExW(LPCWSTR const existingFile, LPCWSTR const newFile, LPVOID, LPVOID, LPBOOL const cancel, DWORD)
{
    if (cancel != nullptr && *cancel)
    {
        SetLastError(ERROR_INVALID_OPERATION);
        return FALSE;
    }
    std::error_code error;
    const bool copied = std::filesystem::copy_file(
        win32_compat::ToPath(existingFile), win32_compat::ToPath(newFile), std::filesystem::copy_options::overwrite_existing, error);
    if (!copied)
    {
        SetLastError(ERROR_ACCESS_DENIED);
    }
    return copied;
}

extern "C" DWORD WINAPI GetFullPathNameW(LPCWSTR const path, const DWORD length, LPWSTR const buffer, LPWSTR* const filePart)
{
    std::error_code error;
    const std::filesystem::path input = win32_compat::ToPath(path);
    std::u16string full;
    if (input.is_absolute())
    {
        full = input.lexically_normal().u16string();
    }
    else
    {
        full = std::filesystem::current_path(error).u16string();
        if (!full.empty() && full.back() != u'/' && full.back() != u'\\')
            full.push_back(u'\\');
        full.append(win32_compat::ToWindowsPath(input.lexically_normal()));
    }
    if (error)
    {
        SetLastError(ERROR_BAD_PATHNAME);
        return 0;
    }
    if (length == 0 || buffer == nullptr)
    {
        return static_cast<DWORD>(full.size() + 1);
    }
    if (!win32_compat::CopyWide(full, buffer, length))
    {
        return static_cast<DWORD>(full.size() + 1);
    }
    if (filePart != nullptr)
    {
        const std::size_t separator = full.find_last_of(u'\\');
        *filePart = buffer + (separator == std::u16string::npos ? 0 : separator + 1);
    }
    return static_cast<DWORD>(full.size());
}

extern "C" DWORD WINAPI GetFinalPathNameByHandleW(const HANDLE handle, LPWSTR const path, const DWORD size, DWORD)
{
    auto* const file = win32_compat::AsFile(handle);
    if (file == nullptr)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return 0;
    }
    std::error_code error;
    const std::u16string result = u"\\\\?\\" + win32_compat::ToWindowsPath(std::filesystem::absolute(file->path, error).lexically_normal());
    if (error)
    {
        return 0;
    }
    if (!win32_compat::CopyWide(result, path, size))
    {
        return static_cast<DWORD>(result.size() + 1);
    }
    return static_cast<DWORD>(result.size());
}

extern "C" LONG WINAPI CompareFileTime(const FILETIME* const left, const FILETIME* const right)
{
    const ULONGLONG leftValue = (static_cast<ULONGLONG>(left->dwHighDateTime) << 32) | left->dwLowDateTime;
    const ULONGLONG rightValue = (static_cast<ULONGLONG>(right->dwHighDateTime) << 32) | right->dwLowDateTime;
    return leftValue < rightValue ? -1 : (leftValue > rightValue ? 1 : 0);
}
