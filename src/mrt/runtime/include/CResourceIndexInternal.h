#pragma once

#include <cstddef>
#include <cstdint>

#include <mrm/BaseInternal.h>
#include <mrm/Results.h>

#include <windows.h>

#include <oleauto.h>
#include <urlmon.h>

namespace Microsoft::Resources::Runtime
{

class CMrtScopeUri
{
public:
    ~CMrtScopeUri();

    HRESULT SetStringPath(const wchar_t* path);
    HRESULT GetStringPath(const wchar_t** path) const;
    HRESULT GetUri(IUri** pScopeUriOut) const;

private:
    StringResult _srScopePath;
    mutable IUri* _pScopeUri {};
};

static_assert(sizeof(CMrtScopeUri) == sizeof(StringResult) + sizeof(void*));

class CMrtUriParser
{
public:
    explicit CMrtUriParser(CMrtScopeUri* scopeUri) : _pMrtScopeUri(scopeUri) {}

    ~CMrtUriParser();

    HRESULT GetStringPath(wchar_t* result, std::size_t resultLength) const;
    IUri* GetBaseIUri();
    IUri* GetIUri() const { return _pCombineOrRelativeUri; }

    static HRESULT IsMrtUriReference(const wchar_t* pszReference, bool* pbIsReference, StringResult* pResourceNameOut);

private:
    BSTR _bstrUriHost {};
    BSTR _bstrUriPath {};
    [[maybe_unused]] bool _bOwnIBaseUri {};
    CMrtScopeUri* _pMrtScopeUri;
    BSTR _bstrQuery {};
    IUri* _pCombineOrRelativeUri {};
};

static_assert(sizeof(CMrtUriParser) == 6 * sizeof(void*));

class CResourceIndexInternal
{
public:
    static HRESULT s_ConvertToPercentEncoding(const wchar_t* value, wchar_t* result, std::uint32_t* length);
};

} // namespace Microsoft::Resources::Runtime
