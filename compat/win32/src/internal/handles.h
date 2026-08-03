#pragma once

#include <cstdio>
#include <filesystem>

#include <windows.h>

#include <boost/interprocess/file_mapping.hpp>

namespace win32_compat
{

enum class HandleKind
{
    File,
    Find,
    Mapping,
};

struct HandleBase
{
    explicit HandleBase(const HandleKind kindValue) : kind(kindValue) {}
    virtual ~HandleBase() = default;
    HandleKind kind;
};

struct FileHandle final : HandleBase
{
    FileHandle(std::FILE* const streamValue, std::filesystem::path pathValue) :
        HandleBase(HandleKind::File), stream(streamValue), path(std::move(pathValue))
    {}
    ~FileHandle() override
    {
        if (stream != nullptr)
        {
            std::fclose(stream);
        }
    }
    std::FILE* stream;
    std::filesystem::path path;
};

struct MappingHandle final : HandleBase
{
    MappingHandle(std::filesystem::path pathValue, const boost::interprocess::mode_t modeValue) :
        HandleBase(HandleKind::Mapping), path(std::move(pathValue)), mapping(path.string().c_str(), modeValue), mode(modeValue)
    {}
    std::filesystem::path path;
    boost::interprocess::file_mapping mapping;
    boost::interprocess::mode_t mode;
};

inline HandleBase* AsHandle(const HANDLE handle) { return static_cast<HandleBase*>(handle); }

inline FileHandle* AsFile(const HANDLE handle)
{
    HandleBase* const base = AsHandle(handle);
    return (base != nullptr && handle != INVALID_HANDLE_VALUE && base->kind == HandleKind::File) ? static_cast<FileHandle*>(base) : nullptr;
}

} // namespace win32_compat
