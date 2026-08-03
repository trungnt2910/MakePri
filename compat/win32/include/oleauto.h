#if __has_include_next(<oleauto.h>)
#include_next <oleauto.h>
#else

#pragma once

#include <oaidl.h>

extern "C"
{
    BSTR WINAPI SysAllocString(const wchar_t* source);
    BSTR WINAPI SysAllocStringLen(const wchar_t* source, UINT length);
    void WINAPI SysFreeString(BSTR string);
    UINT WINAPI SysStringLen(BSTR string);
    void WINAPI VariantInit(VARIANT* value);
    HRESULT WINAPI VariantClear(VARIANT* value);
    HRESULT WINAPI VariantChangeTypeEx(VARIANT* destination, VARIANT* source, DWORD locale, USHORT flags, VARTYPE type);
    HRESULT WINAPI GetErrorInfo(ULONG reserved, IErrorInfo** errorInfo);
}

#endif // __has_include_next(<oleauto.h>)
