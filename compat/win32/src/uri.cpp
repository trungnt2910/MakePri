#include <new>
#include <string>

#include <urlmon.h>

#include "internal/com.h"
#include "internal/strings.h"

namespace
{
class Uri final : public IUri, private win32_compat::ComReferenceCounted
{
public:
    explicit Uri(const std::u16string_view uri)
    {
        const std::size_t separator = uri.find(u':');
        if (separator == std::u16string_view::npos)
        {
            scheme_ = u"file";
            path_.assign(uri);
        }
        else
        {
            scheme_.assign(uri.substr(0, separator));
            std::size_t pathStart = separator + 1;
            if (uri.substr(pathStart).starts_with(u"//"))
            {
                pathStart += 2;
            }
            path_.assign(uri.substr(pathStart));
        }
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void** const object) override
    {
        if (object == nullptr)
        {
            return E_POINTER;
        }
        *object = static_cast<IUri*>(this);
        AddRef();
        return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return AddReference(); }
    ULONG STDMETHODCALLTYPE Release() override { return ReleaseReference(); }
    HRESULT GetSchemeName(BSTR* const value) override
    {
        if (value == nullptr)
        {
            return E_POINTER;
        }
        *value = SysAllocStringLen(reinterpret_cast<const wchar_t*>(scheme_.data()), static_cast<UINT>(scheme_.size()));
        return *value == nullptr ? E_OUTOFMEMORY : S_OK;
    }
    HRESULT GetPath(BSTR* const value) override
    {
        if (value == nullptr)
        {
            return E_POINTER;
        }
        *value = SysAllocStringLen(reinterpret_cast<const wchar_t*>(path_.data()), static_cast<UINT>(path_.size()));
        return *value == nullptr ? E_OUTOFMEMORY : S_OK;
    }

private:
    std::u16string scheme_;
    std::u16string path_;
};
} // namespace

extern "C" HRESULT WINAPI CreateUri(LPCWSTR const uri, DWORD, DWORD_PTR, IUri** const result)
{
    if (uri == nullptr || result == nullptr)
    {
        return E_INVALIDARG;
    }
    *result = new (std::nothrow) Uri(win32_compat::WideView(uri));
    return *result == nullptr ? E_OUTOFMEMORY : S_OK;
}
