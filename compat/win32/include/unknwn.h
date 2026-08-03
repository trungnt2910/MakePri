#if __has_include_next(<unknwn.h>)
#include_next <unknwn.h>
#else

#pragma once

#include <guiddef.h>
#include <minwindef.h>

inline constexpr GUID IID_IUnknown {0x00000000, 0x0000, 0x0000, {0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};

struct IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) = 0;
    virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
    virtual ULONG STDMETHODCALLTYPE Release() = 0;

protected:
    ~IUnknown() = default;
};

#endif // __has_include_next(<unknwn.h>)
