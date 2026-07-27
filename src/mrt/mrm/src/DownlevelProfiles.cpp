#include "StdAfx.h"

#include <DownlevelProfiles.h>

namespace Microsoft::Resources
{
HRESULT DownlevelClientProfile::CreateInstance(const MrmPlatformVersionInternal platformVersion, DownlevelClientProfile** const result)
{
    *result = nullptr;
    void* const memory = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DownlevelClientProfile));
    AutoDeletePtr<DownlevelClientProfile> profile(memory != nullptr ? new (memory) DownlevelClientProfile(platformVersion) : nullptr);
    if (profile.Data() == nullptr)
    {
        // Original line: 17
        RETURN_HR(E_OUTOFMEMORY);
    }

    const HRESULT operationResult = profile.Data()->Initialize(platformVersion);
    if (FAILED(operationResult))
    {
        // Original line: 18
        RETURN_HR(operationResult);
    }

    *result = profile.Detach();
    return S_OK;
}

HRESULT DownlevelClientProfile::GetProviderForQualifier(
    const IEnvironment* const environment,
    const Atom qualifier,
    IQualifierValueProvider** const result) const
{
    static_cast<void>(environment);
    static_cast<void>(qualifier);
    const HRESULT operationResult = GenericQVProvider::CreateInstance(L"downlevel", reinterpret_cast<GenericQVProvider**>(result));
    if (FAILED(operationResult))
    {
        // Original line: 40
        RETURN_HR(operationResult);
    }
    return S_OK;
}

HRESULT DownlevelClientProfile::GetMainPriFilePathForResourceMapName(
    const wchar_t* const resourceMapName,
    StringResult* const result,
    bool* const isCurrent)
{
    static_cast<void>(resourceMapName);
    static_cast<void>(result);
    static_cast<void>(isCurrent);
    return E_FAIL;
}

} // namespace Microsoft::Resources
