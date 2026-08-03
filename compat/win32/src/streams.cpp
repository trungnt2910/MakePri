#include <cstdint>
#include <fstream>
#include <new>
#include <utility>

#include <shlwapi.h>

#include "internal/com.h"
#include "internal/strings.h"

namespace
{
class FileStream final : public IStream, private win32_compat::ComReferenceCounted
{
public:
    explicit FileStream(std::filesystem::path path) : path_(std::move(path)) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void** const object) override
    {
        if (object == nullptr)
        {
            return E_POINTER;
        }
        *object = static_cast<IStream*>(this);
        AddRef();
        return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return AddReference(); }
    ULONG STDMETHODCALLTYPE Release() override { return ReleaseReference(); }
    HRESULT STDMETHODCALLTYPE Read(void*, ULONG, ULONG*) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE Write(const void* const data, const ULONG bytes, ULONG* const bytesWritten) override
    {
        if (data == nullptr && bytes != 0)
        {
            return STG_E_INVALIDPOINTER;
        }
        std::ofstream stream(path_, std::ios::binary | std::ios::trunc);
        stream.write(static_cast<const char*>(data), static_cast<std::streamsize>(bytes));
        if (bytesWritten != nullptr)
        {
            *bytesWritten = stream ? bytes : 0;
        }
        return stream ? S_OK : STG_E_WRITEFAULT;
    }
    HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER*) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE Commit(DWORD) override { return S_OK; }
    HRESULT STDMETHODCALLTYPE Revert() override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE Stat(STATSTG*, DWORD) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE Clone(IStream**) override { return E_NOTIMPL; }

private:
    std::filesystem::path path_;
};
} // namespace

extern "C" HRESULT WINAPI SHCreateStreamOnFileW(LPCWSTR const file, DWORD, IStream** const stream)
{
    if (file == nullptr || stream == nullptr)
    {
        return E_INVALIDARG;
    }
    *stream = new (std::nothrow) FileStream(win32_compat::ToPath(file));
    return *stream == nullptr ? E_OUTOFMEMORY : S_OK;
}
