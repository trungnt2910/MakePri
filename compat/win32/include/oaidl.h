#if __has_include_next(<oaidl.h>)
#include_next <oaidl.h>
#else

#pragma once

#include <unknwn.h>
#include <wtypes.h>

struct IDispatch : IUnknown
{};

struct IErrorInfo : IUnknown
{
    virtual HRESULT GetDescription(BSTR* description) = 0;
};

struct VARIANT
{
    VARTYPE vt;
    USHORT wReserved1;
    USHORT wReserved2;
    USHORT wReserved3;
    union
    {
        LONG lVal;
        double dblVal;
        VARIANT_BOOL boolVal;
        BSTR bstrVal;
        IUnknown* punkVal;
        IDispatch* pdispVal;
    };
};

inline constexpr GUID IID_IDispatch {0x00020400, 0x0000, 0x0000, {0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};

#endif // __has_include_next(<oaidl.h>)
