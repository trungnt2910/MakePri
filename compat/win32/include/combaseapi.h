#if __has_include_next(<combaseapi.h>)
#include_next <combaseapi.h>
#else

#pragma once

#include <objidlbase.h>
#include <wtypes.h>

#define CLSCTX_ALL (CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER)
#define IID_PPV_ARGS(pointer) IID_IUnknown, reinterpret_cast<void**>(pointer)

extern "C"
{
    LPVOID WINAPI CoTaskMemAlloc(SIZE_T bytes);
    void WINAPI CoTaskMemFree(LPVOID memory);
    HRESULT WINAPI CoInitializeEx(LPVOID reserved, DWORD concurrencyModel);
    void WINAPI CoUninitialize();
    HRESULT WINAPI CoCreateInstance(REFCLSID classId, IUnknown* outer, DWORD context, REFIID interfaceId, LPVOID* object);
}

#endif // __has_include_next(<combaseapi.h>)
