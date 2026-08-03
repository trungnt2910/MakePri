#include <new>
#include <string>

#include <errhandlingapi.h>
#include <oleauto.h>

#include "internal/com.h"
#include "internal/errors.h"

namespace
{
thread_local DWORD LastError = ERROR_SUCCESS;

class ErrorInfo final : public IErrorInfo, private win32_compat::ComReferenceCounted
{
public:
    explicit ErrorInfo(const std::u16string_view description) : description_(description) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void** const object) override
    {
        if (object == nullptr)
        {
            return E_POINTER;
        }
        *object = static_cast<IErrorInfo*>(this);
        AddRef();
        return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return AddReference(); }
    ULONG STDMETHODCALLTYPE Release() override { return ReleaseReference(); }
    HRESULT GetDescription(BSTR* const description) override
    {
        if (description == nullptr)
        {
            return E_POINTER;
        }
        *description = SysAllocStringLen(reinterpret_cast<const wchar_t*>(description_.data()), static_cast<UINT>(description_.size()));
        return *description == nullptr ? E_OUTOFMEMORY : S_OK;
    }

private:
    std::u16string description_;
};

thread_local IErrorInfo* CurrentErrorInfo = nullptr;
} // namespace

void win32_compat::SetErrorDescription(const std::u16string_view description)
{
    if (CurrentErrorInfo != nullptr)
    {
        CurrentErrorInfo->Release();
    }
    CurrentErrorInfo = new (std::nothrow) ErrorInfo(description);
}

extern "C" DWORD WINAPI GetLastError() { return LastError; }
extern "C" void WINAPI SetLastError(const DWORD error) { LastError = error; }

extern "C" HRESULT WINAPI GetErrorInfo(ULONG, IErrorInfo** const errorInfo)
{
    if (errorInfo == nullptr)
    {
        return E_POINTER;
    }
    *errorInfo = CurrentErrorInfo;
    CurrentErrorInfo = nullptr;
    return *errorInfo == nullptr ? S_FALSE : S_OK;
}
