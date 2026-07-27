#pragma once

#include <ClientProfileBase.h>

namespace Microsoft::Resources
{

class DownlevelClientProfile : public WindowsClientProfileBase
{
public:
    static HRESULT CreateInstance(MrmPlatformVersionInternal platformVersion, DownlevelClientProfile** result);

    HRESULT GetProviderForQualifier(const IEnvironment* environment, Atom qualifier, IQualifierValueProvider** result) const override;
    HRESULT GetMainPriFilePathForResourceMapName(const wchar_t* resourceMapName, StringResult* result, bool* isCurrent) override;

private:
    explicit DownlevelClientProfile(MrmPlatformVersionInternal platformVersion) : WindowsClientProfileBase(platformVersion) {}
};

} // namespace Microsoft::Resources
