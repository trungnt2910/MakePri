#include "StdAfx.h"

#include <ClientProfileBase.h>

namespace Microsoft::Resources
{

bool MrmProfile::IsPackageLoadAllowedDuringStartup(const int packageIndex) const
{
    (void)packageIndex;
    return true;
}

bool MrmProfile::GetIsChangeNotificationSupported(const wchar_t* const resourceMapName) const
{
    (void)resourceMapName;
    return false;
}

int MrmProfile::GetNumPackages() const { return 0; }

HRESULT
MrmProfile::ChooseDefaultProfile(
    const ProfileType type,
    const MrmPlatformVersionInternal platformVersion,
    const wchar_t* const packageFamilyName,
    const wchar_t* const packageFullName,
    const wchar_t* const applicationId,
    MrmProfile** const result)
{
    static_cast<void>(type);
    static_cast<void>(packageFamilyName);
    static_cast<void>(packageFullName);
    static_cast<void>(applicationId);

    *result = nullptr;
    DownlevelClientProfile* profile = nullptr;
    const HRESULT operationResult = DownlevelClientProfile::CreateInstance(platformVersion, &profile);
    if (SUCCEEDED(operationResult))
    {
        *result = profile;
    }
    return operationResult;
}

bool MrmProfile::IsLoadPriFileAllowed(const wchar_t* const fileName, const MRMPROFILE_PHASE phase)
{
    (void)fileName;
    (void)phase;
    return true;
}

HRESULT MrmProfile::GetPackageInfo(
    const int packageIndex,
    StringResult* const packageFamilyName,
    StringResult* const packageFullName,
    StringResult* const packageRoot) const
{
    (void)packageIndex;
    if (packageFamilyName != nullptr)
    {
        RETURN_IF_FAILED(packageFamilyName->SetRef(nullptr));
    }
    if (packageFullName != nullptr)
    {
        RETURN_IF_FAILED(packageFullName->SetRef(nullptr));
    }
    if (packageRoot != nullptr)
    {
        RETURN_IF_FAILED(packageRoot->SetRef(nullptr));
    }
    return HRESULT_FROM_WIN32(ERROR_RANGE_NOT_FOUND);
}

HRESULT MrmProfile::GetPackageRootFolder(const wchar_t* const packageFullName, StringResult* const result)
{
    (void)packageFullName;
    RETURN_IF_FAILED(result->SetRef(nullptr));
    return HRESULT_FROM_WIN32(ERROR_MRM_PACKAGE_NOT_FOUND);
}

HRESULT MrmProfile::GetMainPriFilePathForResourceMapName(
    const wchar_t* const resourceMapName,
    StringResult* const result,
    bool* const isCurrent)
{
    (void)resourceMapName;
    if (isCurrent != nullptr)
    {
        *isCurrent = false;
    }
    RETURN_IF_FAILED(result->SetRef(nullptr));
    return HRESULT_FROM_WIN32(ERROR_MRM_PACKAGE_NOT_FOUND);
}

bool MrmProfile::TryGetPackageForFile(const wchar_t* const fileName, int* const packageIndex)
{
    int matchingPackage = -1;
    StringResult packageRoot;
    const int numPackages = GetNumPackages();
    for (int index = 0; index < numPackages; ++index)
    {
        if (SUCCEEDED(GetPackageInfo(index, nullptr, nullptr, &packageRoot)) &&
            DefString_IsPrefixWithOptions(packageRoot.GetRef(), fileName, DefCompare_Default) &&
            fileName[std::wcslen(packageRoot.GetRef())] == L'\\')
        {
            matchingPackage = index;
            break;
        }
    }
    if (packageIndex != nullptr)
    {
        *packageIndex = matchingPackage;
    }
    return matchingPackage >= 0;
}

HRESULT MrmProfile::GetPackageFullNameForFile(const int packageIndex, const IRawResourceMap* const resourceMap, StringResult* const result)
{
    (void)packageIndex;
    (void)resourceMap;
    (void)result;
    return S_OK;
}

HRESULT MrmProfile::GetObjectSharedState(MrtSharedObjectState* const result) const
{
    *result = Instance;
    return S_OK;
}

HRESULT MrmProfile::GetTargetPlatformVersionForFileMagic(const DEFFILE_MAGIC& magic, MrmPlatformVersionInternal* const result)
{
    if (magic.ullMagic == gWin8PriFileMagic.ullMagic)
    {
        *result = MrmPlatformVersionInternal::WindowsClient8;
        return S_OK;
    }
    if (magic.ullMagic == gWinBluePriFileMagic.ullMagic)
    {
        *result = MrmPlatformVersionInternal::WindowsClientBlue;
        return S_OK;
    }
    if (magic.ullMagic == gWindowsPhoneBluePriFileMagic.ullMagic)
    {
        *result = MrmPlatformVersionInternal::WindowsPhoneBlue;
        return S_OK;
    }
    if (magic.ullMagic == gUniversalPriFileMagic.ullMagic)
    {
        *result = MrmPlatformVersionInternal::WindowsCore;
        return S_OK;
    }
    if (magic.ullMagic == gUniversalRS4PriFileMagic.ullMagic)
    {
        *result = MrmPlatformVersionInternal::WindowsCoreRS4;
        return S_OK;
    }
    if (magic.ullMagic == gUniversalVNextPriFileMagic.ullMagic)
    {
        *result = MrmPlatformVersionInternal::WindowsCoreVNext;
        return S_OK;
    }
    return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_PRI_FILE);
}

HRESULT MrmProfile::GetDefaultTargetOsVersionForFileMagic(
    const DEFFILE_MAGIC& magic,
    StringResult* const platform,
    StringResult* const version)
{
    const wchar_t* platformName;
    const wchar_t* versionName;
    if (magic.ullMagic == gWin8PriFileMagic.ullMagic)
    {
        platformName = L"client";
        versionName = L"6.2.1";
    }
    else if (magic.ullMagic == gWinBluePriFileMagic.ullMagic)
    {
        platformName = L"client";
        versionName = L"6.3.0";
    }
    else if (magic.ullMagic == gWindowsPhoneBluePriFileMagic.ullMagic)
    {
        platformName = L"WindowsPhone";
        versionName = L"6.3.1";
    }
    else if (magic.ullMagic == gUniversalPriFileMagic.ullMagic)
    {
        platformName = L"universal";
        versionName = L"10.0.0";
    }
    else if (magic.ullMagic == gUniversalRS4PriFileMagic.ullMagic)
    {
        platformName = L"universal";
        versionName = L"10.0.0.5";
    }
    else if (magic.ullMagic == gUniversalVNextPriFileMagic.ullMagic)
    {
        platformName = L"universal";
        versionName = L"99.0.1";
    }
    else
    {
        return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_PRI_FILE);
    }

    if (platform != nullptr)
    {
        RETURN_IF_FAILED(platform->SetRef(platformName));
    }
    if (version != nullptr)
    {
        RETURN_IF_FAILED(version->SetRef(versionName));
    }
    return S_OK;
}

HRESULT MrmProfile::GetQualifierInfoForEnvironment(
    const wchar_t* const simpleName,
    const IEnvironmentVersionInfo* const version,
    const IEnvironment* const environment,
    int* const numQualifiers,
    const wchar_t* const** const qualifierNames,
    const Atom::SmallIndex** const qualifierMappings) const
{
    *numQualifiers = 0;
    *qualifierNames = nullptr;
    *qualifierMappings = nullptr;

    if (DefString_CompareWithOptions(environment->GetUniqueName(), L"win://WinCore/1", DefCompare_CaseInsensitive) == 0)
    {
        if (DefString_CompareWithOptions(simpleName, L"Windows", DefCompare_CaseInsensitive) == 0)
        {
            *numQualifiers = 9;
            *qualifierNames = WindowsClientEnvironment::ClientQualifierNames;
            *qualifierMappings = WindowsClientEnvironment::ClientToCoreQualifierMapping;
        }
        else if (DefString_CompareWithOptions(simpleName, L"WindowsPhone", DefCompare_CaseInsensitive) == 0)
        {
            *numQualifiers = 9;
            *qualifierNames = WindowsPhoneEnvironment::PhoneQualifierNames;
            *qualifierMappings = WindowsPhoneEnvironment::PhoneToCoreQualifierMapping;
        }
        return S_OK;
    }
    if (DefString_CompareWithOptions(environment->GetUniqueName(), L"win://Windows/1", DefCompare_CaseInsensitive) == 0)
    {
        if (DefString_CompareWithOptions(simpleName, L"WinCore", DefCompare_CaseInsensitive) == 0)
        {
            *numQualifiers = 12;
            *qualifierNames = CoreEnvironment::QualifierNames;
            *qualifierMappings = WindowsClientEnvironment::CoreToClientQualifierMapping;
        }
        return S_OK;
    }
    if (DefString_CompareWithOptions(environment->GetUniqueName(), L"win://WindowsPhone/1", DefCompare_CaseInsensitive) == 0)
    {
        if (DefString_CompareWithOptions(simpleName, L"WinCore", DefCompare_CaseInsensitive) == 0)
        {
            *numQualifiers = 12;
            *qualifierNames = CoreEnvironment::QualifierNames;
            *qualifierMappings = WindowsPhoneEnvironment::CoreToPhoneQualifierMapping;
        }
        return S_OK;
    }
    return CoreProfile::GetQualifierInfoForEnvironment(simpleName, version, environment, numQualifiers, qualifierNames, qualifierMappings);
}

HRESULT MrmProfile::GetDefaultEnvironmentForFileMagic(
    const DEFFILE_MAGIC& magic,
    StringResult* const environmentName,
    EnvironmentVersionInfo* const result) const
{
    if (!IsSupportedFileMagicNumber(magic))
    {
        return E_DEF_UNSUPPORTED_VERSION;
    }

    const MRMFILE_ENVIRONMENT_VERSION_INFO* version = nullptr;
    HRESULT status;
    if ((magic.ullMagic == gUniversalPriFileMagic.ullMagic) || (magic.ullMagic == gUniversalRS4PriFileMagic.ullMagic) ||
        (magic.ullMagic == gUniversalVNextPriFileMagic.ullMagic))
    {
        status = MrmEnvironment::FindInfoForCurrentVersion(&CoreEnvironment::EnvironmentDescription, &version, nullptr, environmentName);
    }
    else if (magic.ullMagic == gWinBluePriFileMagic.ullMagic)
    {
        status = MrmEnvironment::FindInfoForVersion(
            &WindowsClientEnvironment::ClientEnvironmentDescription, 4, 0, &version, nullptr, environmentName);
    }
    else if (magic.ullMagic == gWindowsPhoneBluePriFileMagic.ullMagic)
    {
        status = MrmEnvironment::FindInfoForVersion(
            &WindowsPhoneEnvironment::PhoneEnvironmentDescription, 1, 0, &version, nullptr, environmentName);
    }
    else if (magic.ullMagic == gWin8PriFileMagic.ullMagic)
    {
        status = MrmEnvironment::FindInfoForVersion(
            &WindowsClientEnvironment::ClientEnvironmentDescription, 3, 0, &version, nullptr, environmentName);
    }
    else
    {
        return E_DEF_UNSUPPORTED_VERSION;
    }
    RETURN_IF_FAILED(status);
    if (version == nullptr)
    {
        return E_DEF_UNSUPPORTED_VERSION;
    }
    if (result != nullptr)
    {
        result->SetVersionInfo(version);
    }
    return S_OK;
}

} // namespace Microsoft::Resources
