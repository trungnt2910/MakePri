#if __has_include_next(<urlmon.h>)
#include_next <urlmon.h>
#else

#pragma once

#include <oleauto.h>

#define Uri_CREATE_ALLOW_IMPLICIT_FILE_SCHEME 0x4
#define Uri_CREATE_NO_DECODE_EXTRA_INFO 0x80
#define Uri_CREATE_CANONICALIZE 0x100
#define Uri_CREATE_CRACK_UNKNOWN_SCHEMES 0x200
#define Uri_CREATE_PRE_PROCESS_HTML_URI 0x800
#define Uri_CREATE_NO_IE_SETTINGS 0x4000

struct IUri : IUnknown
{
    virtual HRESULT GetSchemeName(BSTR* value) = 0;
    virtual HRESULT GetPath(BSTR* value) = 0;
};

extern "C" HRESULT WINAPI CreateUri(LPCWSTR uri, DWORD flags, DWORD_PTR reserved, IUri** result);

#endif // __has_include_next(<urlmon.h>)
