#include "StdAfx.h"

#include <CResourceIndexInternal.h>

namespace Microsoft::Resources::Runtime
{
CMrtScopeUri::~CMrtScopeUri()
{
    if (_pScopeUri != nullptr)
    {
        _pScopeUri->Release();
    }
}

HRESULT CMrtScopeUri::SetStringPath(const wchar_t* const path)
{
    // Original line: 54
    RETURN_IF_FAILED(_srScopePath.SetCopy(path));
    return S_OK;
}

HRESULT CMrtScopeUri::GetStringPath(const wchar_t** const path) const
{
    *path = _srScopePath.GetRef();
    // Original line: 65
    RETURN_HR_IF(HRESULT_FROM_WIN32(ERROR_INVALID_STATE), *path == nullptr);
    return S_OK;
}

HRESULT CMrtScopeUri::GetUri(IUri** const pScopeUriOut) const
{
    *pScopeUriOut = nullptr;
    if (_pScopeUri == nullptr)
    {
        IUri* newUri {};
        // Original line: 84
        RETURN_IF_FAILED(CreateUri(
            _srScopePath.GetRef(),
            Uri_CREATE_ALLOW_IMPLICIT_FILE_SCHEME | Uri_CREATE_CANONICALIZE | Uri_CREATE_NO_DECODE_EXTRA_INFO |
                Uri_CREATE_CRACK_UNKNOWN_SCHEMES | Uri_CREATE_PRE_PROCESS_HTML_URI | Uri_CREATE_NO_IE_SETTINGS,
            0,
            &newUri));
        void* const previousUri = InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(&_pScopeUri), newUri, nullptr);
        if (previousUri != nullptr)
        {
            newUri->Release();
        }
    }
    if (_pScopeUri != nullptr)
    {
        *pScopeUriOut = _pScopeUri;
    }
    return S_OK;
}

CMrtUriParser::~CMrtUriParser()
{
    SysFreeString(_bstrQuery);
    SysFreeString(_bstrUriPath);
    SysFreeString(_bstrUriHost);
}

HRESULT CMrtUriParser::GetStringPath(wchar_t* const result, const std::size_t resultLength) const
{
    const wchar_t* stringPath {};
    // Original line: 424
    RETURN_IF_FAILED(_pMrtScopeUri->GetStringPath(&stringPath));
    // Original line: 427
    RETURN_IF_FAILED(StringCchCopyW(result, resultLength, stringPath));
    return S_OK;
}

IUri* CMrtUriParser::GetBaseIUri()
{
    IUri* pBaseUri {};
    if (SUCCEEDED(_pMrtScopeUri->GetUri(&pBaseUri)))
    {
        return pBaseUri;
    }
    return nullptr;
}

HRESULT CMrtUriParser::IsMrtUriReference(const wchar_t* const pszReference, bool* const pbIsReference, StringResult* const pResourceNameOut)
{
    IUri* pIUri {};
    *pbIsReference = false;
    if (FAILED(CreateUri(
            pszReference,
            Uri_CREATE_ALLOW_IMPLICIT_FILE_SCHEME | Uri_CREATE_CANONICALIZE | Uri_CREATE_NO_DECODE_EXTRA_INFO |
                Uri_CREATE_CRACK_UNKNOWN_SCHEMES | Uri_CREATE_PRE_PROCESS_HTML_URI | Uri_CREATE_NO_IE_SETTINGS,
            0,
            &pIUri)))
    {
        return S_OK;
    }
    const auto releaseUri = wil::scope_exit([&] { pIUri->Release(); });

    BSTR scheme {};
    if (FAILED(pIUri->GetSchemeName(&scheme)))
    {
        return S_OK;
    }
    const auto freeScheme = wil::scope_exit([&] { SysFreeString(scheme); });
    if (scheme == nullptr || SysStringLen(scheme) != 11 || CompareStringOrdinal(scheme, -1, L"ms-resource", -1, TRUE) != CSTR_EQUAL)
    {
        return S_OK;
    }

    BSTR uriPath {};
    if (FAILED(pIUri->GetPath(&uriPath)))
    {
        return S_OK;
    }
    const auto freePath = wil::scope_exit([&] { SysFreeString(uriPath); });
    if (uriPath == nullptr || SysStringLen(uriPath) == 0)
    {
        return S_OK;
    }

    *pbIsReference = true;
    if (pResourceNameOut != nullptr)
    {
        pResourceNameOut->SetCopy(uriPath);
    }
    return S_OK;
}

HRESULT CResourceIndexInternal::s_ConvertToPercentEncoding(const wchar_t* const value, wchar_t* const result, std::uint32_t* const length)
{
    CMrtScopeUri scopeUri;
    // Original line: 1521
    RETURN_IF_FAILED(scopeUri.SetStringPath(value));
    CMrtUriParser parser(&scopeUri);
    // Original line: 1524
    RETURN_IF_FAILED(parser.GetStringPath(result, *length));
    *length = static_cast<std::uint32_t>(std::wcslen(result));
    return S_OK;
}

} // namespace Microsoft::Resources::Runtime
