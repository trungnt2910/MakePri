#include "StdAfx.h"

#include <ClientProfileBase.h>

namespace Microsoft::Resources::CoreEnvironment
{
const ENVIRONMENT_INITIALIZER& EnvironmentInitializer = *GetEnvironmentInitializer();
const ENVIRONMENT_DESCRIPTION& EnvironmentDescription = *EnvironmentInitializer.pEnvironmentDescription;
const QUALIFIER_INFO* const QualifierInfo = EnvironmentInitializer.pQualifierInfos;
} // namespace Microsoft::Resources::CoreEnvironment

namespace Microsoft::Resources
{

const wchar_t* const WindowsClientEnvironment::ClientQualifierNames[9] {
    L"Language",
    L"Contrast",
    L"Scale",
    L"HomeRegion",
    L"TargetSize",
    L"LayoutDirection",
    L"Configuration",
    L"AlternateForm",
    L"DXFeatureLevel",
};

const std::uint16_t WindowsClientEnvironment::ClientToCoreQualifierMapping[9] {0, 1, 2, 3, 4, 5, 9, 7, 8};

const std::uint16_t WindowsClientEnvironment::CoreToClientQualifierMapping[12] {0, 1, 2, 3, 4, 5, 0xFFFF, 7, 8, 6, 0xFFFF, 0xFFFF};

const MRMFILE_ENVIRONMENT_VERSION_INFO WindowsClientEnvironment::ClientVersions[5] {
    {1, 4, 0x64D4872B, 9, 9, 3, 3, 3, 0},
    {1, 3, 0x2AEB3101, 8, 8, 3, 3, 3, 0},
    {1, 2, 0x29CB5A06, 8, 8, 2, 2, 3, 0},
    {1, 1, 0x8AE57D17, 7, 7, 2, 2, 3, 0},
    {1, 0, 0xC5872444, 3, 3, 2, 2, 3, 0},
};

const wchar_t* const WindowsClientEnvironment::ClientQualifierTypeNames[9] {
    L"LanguageList",
    L"ContrastMode",
    L"ScaleFactor",
    L"RegionId",
    L"TargetSizeValue",
    L"LayoutDirectionValue",
    L"String",
    L"String",
    L"DXFeatureLevel",
};

const QUALIFIER_DESCRIPTION WindowsClientEnvironment::ClientQualifierDescriptions[9] {
    {L"lang", L"en-US", 0, 0, 900, 700},
    {L"contrast", L"standard", 0, 1, 700, 400},
    {L"scale", L"100", 0x11, 2, 500, 200},
    {L"region", L"001", 0, 3, 950, 800},
    {L"target", L"256", 0x20, 4, 400, 300},
    {L"layoutdir", L"LTR", 0, 5, 450, 600},
    {L"config", L"", 2, 6, 1000, 900},
    {L"altform", L"", 0x20, 7, 10, 100},
    {L"dxfl", L"DX9", 0x20, 8, 150, 150},
};

const QUALIFIER_TYPE_DESCRIPTION WindowsClientEnvironment::ClientQualifierTypeDescriptions[9] {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}};

const QUALIFIER_INFO WindowsClientEnvironment::ClientQualifierInfo[] {
    {0x64D4872B, 9, ClientQualifierNames, ClientQualifierDescriptions, 9, ClientQualifierTypeNames, ClientQualifierTypeDescriptions},
    {0x2AEB3101, 8, ClientQualifierNames, ClientQualifierDescriptions, 8, ClientQualifierTypeNames, ClientQualifierTypeDescriptions},
    {0x29CB5A06, 8, ClientQualifierNames, ClientQualifierDescriptions, 8, ClientQualifierTypeNames, ClientQualifierTypeDescriptions},
    {0x8AE57D17, 7, ClientQualifierNames, ClientQualifierDescriptions, 8, ClientQualifierTypeNames, ClientQualifierTypeDescriptions},
    {0xC5872444, 3, ClientQualifierNames, ClientQualifierDescriptions, 3, ClientQualifierTypeNames, ClientQualifierTypeDescriptions},
};

const ENVIRONMENT_DESCRIPTION WindowsClientEnvironment::ClientEnvironmentDescription {L"Windows", L"win://Windows/1", 5, ClientVersions};

const ENVIRONMENT_INITIALIZER WindowsClientEnvironment::ClientEnvironmentInitializer {
    &ClientEnvironmentDescription,
    -1,
    5,
    ClientQualifierInfo};

const wchar_t* const WindowsPhoneEnvironment::PhoneQualifierNames[9] {
    L"Language",
    L"Contrast",
    L"Scale",
    L"HomeRegion",
    L"TargetSize",
    L"LayoutDirection",
    L"AlternateForm",
    L"Theme",
    L"DXFeatureLevel",
};

const std::uint16_t WindowsPhoneEnvironment::PhoneToCoreQualifierMapping[9] {0, 1, 2, 3, 4, 5, 7, 6, 8};

const std::uint16_t WindowsPhoneEnvironment::CoreToPhoneQualifierMapping[12] {0, 1, 2, 3, 4, 5, 7, 6, 8, 0xFFFF, 0xFFFF, 0xFFFF};

const MRMFILE_ENVIRONMENT_VERSION_INFO WindowsPhoneEnvironment::PhoneVersions[2] {
    {1, 1, 0x7103FD70, 9, 9, 3, 3, 3, 0},
    {1, 0, 0xBADC0FFE, 9, 9, 3, 3, 3, 0},
};

const wchar_t* const WindowsPhoneEnvironment::PhoneQualifierTypeNames[9] {
    L"LanguageList",
    L"ContrastMode",
    L"ScaleFactor",
    L"RegionId",
    L"TargetSizeValue",
    L"LayoutDirectionValue",
    L"AlternateForm",
    L"Theme",
    L"DXFeatureLevel",
};

const QUALIFIER_DESCRIPTION WindowsPhoneEnvironment::PhoneQualifierDescriptions[9] {
    {L"lang", L"en-US", 0, 0, 900, 700},
    {L"contrast", L"standard", 0x80008, 1, 700, 400},
    {L"scale", L"100", 0, 2, 500, 200},
    {L"region", L"001", 0, 3, 950, 800},
    {L"target", L"256", 0, 4, 400, 300},
    {L"layoutdir", L"LTR", 0, 5, 450, 600},
    {L"altform", L"", 0, 6, 10, 100},
    {L"theme", L"dark", 0x80008, 7, 600, 350},
    {L"dxfl", L"DX9", 0, 8, 150, 150},
};

const QUALIFIER_TYPE_DESCRIPTION WindowsPhoneEnvironment::PhoneQualifierTypeDescriptions[9] {{0}, {1}, {0}, {0}, {0}, {0}, {0}, {1}, {0}};

const QUALIFIER_INFO WindowsPhoneEnvironment::PhoneQualifierInfo[] {
    {0x7103FD70, 9, PhoneQualifierNames, PhoneQualifierDescriptions, 9, PhoneQualifierTypeNames, PhoneQualifierTypeDescriptions},
    {0xBADC0FFE, 9, PhoneQualifierNames, PhoneQualifierDescriptions, 9, PhoneQualifierTypeNames, PhoneQualifierTypeDescriptions},
};

const ENVIRONMENT_DESCRIPTION WindowsPhoneEnvironment::PhoneEnvironmentDescription {
    L"WindowsPhone",
    L"win://WindowsPhone/1",
    2,
    PhoneVersions};

const ENVIRONMENT_INITIALIZER WindowsPhoneEnvironment::PhoneEnvironmentInitializer {
    &PhoneEnvironmentDescription,
    -1,
    2,
    PhoneQualifierInfo};

HRESULT WindowsClientProfileBase::GetProviderForQualifier(
    const IEnvironment* const environment,
    const Atom qualifier,
    IQualifierValueProvider** const result) const
{
    static_cast<void>(environment);
    static_cast<void>(qualifier);
    return GenericQVProvider::CreateInstance(L"downlevel", reinterpret_cast<GenericQVProvider**>(result));
}

bool WindowsClientProfileBase::IsSupportedFileMagicNumber(const DEFFILE_MAGIC& magic) const
{
    return IsSupportedFileMagicNumber(/* unknown = */ true, magic);
}

WindowsClientProfileBase::WindowsClientProfileBase(const MrmPlatformVersionInternal platformVersion) : m_platformVersion(platformVersion) {}

HRESULT WindowsClientProfileBase::Initialize(const MrmPlatformVersionInternal requestedPlatformVersion)
{
    m_platformVersion = requestedPlatformVersion == MrmPlatformVersionInternal::DefaultPlatformVersion ?
                            MrmPlatformVersionInternal::WindowsCoreRS4 :
                            requestedPlatformVersion;

    if (m_platformVersion == MrmPlatformVersionInternal::WindowsClient8)
    {
        m_pEnvironmentInitializer = &WindowsClientEnvironment::ClientEnvironmentInitializer;
        m_pDefaultQualifierInfo = &WindowsClientEnvironment::ClientQualifierInfo[1];
        return S_OK;
    }
    if (m_platformVersion == MrmPlatformVersionInternal::WindowsClientBlue)
    {
        if (m_platformVersion == MrmPlatformVersionInternal::WindowsClient8 ||
            m_platformVersion == MrmPlatformVersionInternal::DefaultPlatformVersion)
        {
            m_pDefaultQualifierInfo = &WindowsClientEnvironment::ClientQualifierInfo[1];
        }
        else if (m_platformVersion == MrmPlatformVersionInternal::WindowsClientBlue)
        {
            m_pDefaultQualifierInfo = &WindowsClientEnvironment::ClientQualifierInfo[0];
        }
        else
        {
            // Original line: 24
            RETURN_HR(E_DEF_UNSUPPORTED_VERSION);
        }
        m_pEnvironmentInitializer = &WindowsClientEnvironment::ClientEnvironmentInitializer;
        return S_OK;
    }
    if (m_platformVersion == MrmPlatformVersionInternal::WindowsPhoneBlue)
    {
        m_pDefaultQualifierInfo = &WindowsPhoneEnvironment::PhoneQualifierInfo[0];
        m_pEnvironmentInitializer = &WindowsPhoneEnvironment::PhoneEnvironmentInitializer;
        return S_OK;
    }
    if (IsPlatformAtLeastTH1(m_platformVersion))
    {
        m_pDefaultQualifierInfo = &CoreEnvironment::QualifierInfo[0];
        m_pEnvironmentInitializer = &CoreEnvironment::EnvironmentInitializer;
        return S_OK;
    }
    return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_PRI_FILE);
}

bool WindowsClientProfileBase::IsSupportedFileMagicNumber(bool unknown, const DEFFILE_MAGIC& magic) const
{
    static_cast<void>(unknown);
    if (!m_isDesignMode)
    {
        if (m_platformVersion == MrmPlatformVersionInternal::WindowsClient8)
        {
            return (magic.ullMagic == gWin8PriFileMagic.ullMagic) || (magic.ullMagic == gWinBluePriFileMagic.ullMagic);
        }
        if (m_platformVersion == MrmPlatformVersionInternal::WindowsPhoneBlue)
        {
            if (magic.ullMagic == gWindowsPhoneBluePriFileMagic.ullMagic)
            {
                return true;
            }
            if (magic.ullMagic == gWinBluePriFileMagic.ullMagic)
            {
                return true;
            }
            return magic.ullMagic == gWin8PriFileMagic.ullMagic;
        }
        if (m_platformVersion == MrmPlatformVersionInternal::WindowsClientBlue)
        {
            if (magic.ullMagic == gWinBluePriFileMagic.ullMagic)
            {
                return true;
            }
            return magic.ullMagic == gWin8PriFileMagic.ullMagic;
        }
    }

    return (magic.ullMagic == gUniversalPriFileMagic.ullMagic) || (magic.ullMagic == gUniversalRS4PriFileMagic.ullMagic) ||
           (magic.ullMagic == gUniversalVNextPriFileMagic.ullMagic) || (magic.ullMagic == gWinBluePriFileMagic.ullMagic) ||
           (magic.ullMagic == gWindowsPhoneBluePriFileMagic.ullMagic) || (magic.ullMagic == gWin8PriFileMagic.ullMagic);
}

HRESULT WindowsClientProfileBase::GetTargetPlatformAndVersionForFileMagic(
    const DEFFILE_MAGIC& magic,
    StringResult* const platform,
    StringResult* const version)
{
    return MrmProfile::GetDefaultTargetOsVersionForFileMagic(magic, platform, version);
}

HRESULT WindowsClientProfileBase::GetTargetPlatformAndVersion(StringResult* const platform, StringResult* const version)
{
    MrmBuildConfiguration* const configuration = GetBuildConfiguration();
    if (configuration == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    return MrmProfile::GetDefaultTargetOsVersionForFileMagic(configuration->GetFileMagicNumber(), platform, version);
}

int WindowsClientProfileBase::GetNumEnvironments() const { return 1; }

HRESULT WindowsClientProfileBase::GetEnvironmentVersionInfo(
    const int index,
    IEnvironmentVersionInfo** const result,
    StringResult* const environmentName) const
{
    *result = nullptr;
    if (index != 0)
    {
        // Original line: 133
        RETURN_HR(E_INVALIDARG);
    }

    const MRMFILE_ENVIRONMENT_VERSION_INFO* version {};
    HRESULT operationResult =
        MrmEnvironment::FindInfoForCurrentVersion(m_pEnvironmentInitializer->pEnvironmentDescription, &version, nullptr, environmentName);
    if (FAILED(operationResult))
    {
        // Original line: 134
        RETURN_HR(operationResult);
    }
    EnvironmentVersionInfo* environmentVersionInfo = nullptr;
    operationResult = EnvironmentVersionInfo::CreateInstance(version, &environmentVersionInfo);
    if (SUCCEEDED(operationResult))
    {
        *result = environmentVersionInfo;
    }
    return operationResult;
}

HRESULT WindowsClientProfileBase::CreateEnvironment(const int index, AtomPoolGroup* const atomPoolGroup, IEnvironment** const result) const
{
    if (index != 0)
    {
        return CoreProfile::CreateEnvironment(index, atomPoolGroup, result);
    }
    if (m_platformVersion == MrmPlatformVersionInternal::DefaultPlatformVersion ||
        m_platformVersion == MrmPlatformVersionInternal::WindowsCore || m_platformVersion == MrmPlatformVersionInternal::WindowsCoreRS4 ||
        m_platformVersion == MrmPlatformVersionInternal::WindowsCoreVNext)
    {
        return MrmEnvironment::CreateInstance(
            atomPoolGroup, &CoreEnvironment::EnvironmentInitializer, 1, 2, reinterpret_cast<MrmEnvironment**>(result));
    }
    if (m_platformVersion == MrmPlatformVersionInternal::WindowsClient8)
    {
        return MrmEnvironment::CreateInstance(
            atomPoolGroup, &WindowsClientEnvironment::ClientEnvironmentInitializer, 1, 3, reinterpret_cast<MrmEnvironment**>(result));
    }
    if (m_platformVersion == MrmPlatformVersionInternal::WindowsClientBlue)
    {
        return MrmEnvironment::CreateInstance(
            atomPoolGroup, &WindowsClientEnvironment::ClientEnvironmentInitializer, 1, 4, reinterpret_cast<MrmEnvironment**>(result));
    }
    if (m_platformVersion == MrmPlatformVersionInternal::WindowsPhoneBlue)
    {
        return MrmEnvironment::CreateInstance(
            atomPoolGroup, &WindowsPhoneEnvironment::PhoneEnvironmentInitializer, 1, 1, reinterpret_cast<MrmEnvironment**>(result));
    }
    if (IsTestMrmPlatformVersionInternal(m_platformVersion))
    {
        return CoreProfile::CreateEnvironment(index, atomPoolGroup, result);
    }
    return MrmEnvironment::CreateInstance(
        atomPoolGroup, &WindowsClientEnvironment::ClientEnvironmentInitializer, 1, 4, reinterpret_cast<MrmEnvironment**>(result));
}

bool WindowsClientProfileBase::IsCompatibleEnvironment(
    const EnvironmentReference* const reference,
    const IEnvironment* const environment,
    const RemapAtomPool** const remap) const
{
    return IsTestMrmPlatformVersionInternal(m_platformVersion) || CoreProfile::IsCompatibleEnvironment(reference, environment, remap);
}

HRESULT WindowsClientProfileBase::RemapQualifierIndexToCore(
    const MrmPlatformVersionInternal platformVersion,
    const Atom qualifier,
    CoreEnvironment::QualifierIndex* const index,
    bool* const remapped)
{
    if (platformVersion == MrmPlatformVersionInternal::WindowsClient8 || platformVersion == MrmPlatformVersionInternal::WindowsClientBlue)
    {
        *remapped = true;
        *index = static_cast<CoreEnvironment::QualifierIndex>(WindowsClientEnvironment::ClientToCoreQualifierMapping[qualifier.GetIndex()]);
        return S_OK;
    }
    if (platformVersion == MrmPlatformVersionInternal::WindowsPhoneBlue)
    {
        *remapped = true;
        *index = static_cast<CoreEnvironment::QualifierIndex>(WindowsPhoneEnvironment::PhoneToCoreQualifierMapping[qualifier.GetIndex()]);
        return S_OK;
    }
    if (!IsPlatformAtLeastTH1(platformVersion))
    {
        // Original line: 270
        RETURN_HR(E_MRM_UNSUPPORTED_ENVIRONMENT);
    }
    *remapped = false;
    *index = static_cast<CoreEnvironment::QualifierIndex>(qualifier.GetIndex());
    return S_OK;
}

HRESULT WindowsClientProfileBase::GetDefaultTypeForQualifier(
    const IEnvironment* const environment,
    const MrmPlatformVersionInternal platformVersion,
    const Atom qualifier,
    IBuildQualifierType** const result)
{
    *result = nullptr;
    ResourceQualifier resourceQualifier {};
    // Original line: 289
    RETURN_IF_FAILED(environment->GetQualifier(qualifier, &resourceQualifier));

    CoreEnvironment::QualifierIndex coreIndex {};
    bool remapped {};
    // Original line: 294
    RETURN_IF_FAILED(RemapQualifierIndexToCore(platformVersion, qualifier, &coreIndex, &remapped));

    if (coreIndex == CoreEnvironment::QualifierIndex::Language)
    {
        LanguageQualifierType* qualifierType = nullptr;
        // Original line: 300
        RETURN_IF_FAILED(LanguageQualifierType::CreateInstance(&qualifierType));
        *result = qualifierType;
        return S_OK;
    }
    if (coreIndex == CoreEnvironment::QualifierIndex::HomeRegion)
    {
        RegionQualifierType* qualifierType = nullptr;
        // Original line: 304
        RETURN_IF_FAILED(RegionQualifierType::CreateInstance(&qualifierType));
        *result = qualifierType;
        return S_OK;
    }
    if (remapped && coreIndex == CoreEnvironment::QualifierIndex::Scale)
    {
        EnumeratedScaleQualifierType* qualifierType = nullptr;
        // Original line: 308
        RETURN_IF_FAILED(EnumeratedScaleQualifierType::CreateInstance(&qualifierType));
        *result = qualifierType;
        return S_OK;
    }
    // Original line: 312
    RETURN_IF_FAILED(CoreEnvironment::GetDefaultQualifierType(static_cast<CoreEnvironment::QualifierTypeIndex>(coreIndex), result));
    return S_OK;
}

HRESULT WindowsClientProfileBase::GetTypeForQualifier(
    const IEnvironment* const environment,
    const Atom qualifier,
    IBuildQualifierType** const result) const
{
    IBuildQualifierType* type {};
    const HRESULT operationResult = GetDefaultTypeForQualifier(environment, m_platformVersion, qualifier, &type);
    if (FAILED(operationResult))
    {
        // Original line: 325
        RETURN_HR(operationResult);
    }
    *result = type;
    return S_OK;
}

HRESULT WindowsClientProfileBase::GetMergeFolders(StringResult* const mergeFolder, StringResult* const systemMergeFolder) const
{
    if (mergeFolder != nullptr)
    {
        // Original line: 338
        RETURN_IF_FAILED(mergeFolder->SetRef(nullptr));
    }
    if (systemMergeFolder == nullptr)
    {
        return S_OK;
    }

    wchar_t windowsFolder[MAX_PATH] {};
    StringResult folder;
    // Original line: 348
    RETURN_IF_FAILED(folder.SetEmptyContents(MAX_PATH, nullptr, nullptr));
    // Original line: 350
    RETURN_IF_FAILED(systemMergeFolder->SetRef(nullptr));

    StringResult redirected;
    StringResult* source {};
    if (TryGetRedirectedSystemMergeFolder(&redirected) && redirected.GetRef() != nullptr)
    {
        source = &redirected;
    }
    else
    {
        // Original line: 359
        RETURN_LAST_ERROR_IF(GetSystemWindowsDirectoryW(windowsFolder, MAX_PATH) == 0);
        // Original line: 361
        RETURN_IF_FAILED(DefStringResult_SetCopy(folder.GetStringResult(), windowsFolder));
        // Original line: 362
        RETURN_IF_FAILED(DefStringResult_ConcatPathElement(folder.GetStringResult(), L"rescache\\_merged", L'\\'));
        source = &folder;
    }

    if (source != nullptr)
    {
        return DefStringResult_SetCopy(systemMergeFolder->GetStringResult(), source->GetRef());
    }
    return S_OK;
}

MrmBuildConfiguration* WindowsClientProfileBase::GetBuildConfiguration()
{
    if (m_pBuildConfiguration == nullptr)
    {
        MrmBuildConfiguration::CreateInstance(m_platformVersion, &m_pBuildConfiguration);
    }
    return m_pBuildConfiguration;
}

HRESULT WindowsClientProfileBase::GetQualifierBuildInfoByToken(
    const wchar_t* const token,
    const UnifiedEnvironment* const environment,
    QualifierBuildInfo* const info) const
{
    const IEnvironment* const defaultEnvironment = environment->GetDefaultEnvironment();
    const QUALIFIER_INFO* const qualifierInfo = defaultEnvironment->GetQualifierInfo();
    int qualifierIndex = 0;
    while (qualifierIndex < qualifierInfo->numQualifiers &&
           DefString_CompareWithOptions(token, qualifierInfo->pQualifiers[qualifierIndex].pQualifierToken, DefCompare_CaseInsensitive) !=
               Def_Equal)
    {
        ++qualifierIndex;
    }
    if (qualifierIndex < 0 || qualifierIndex >= qualifierInfo->numQualifiers)
    {
        // Original line: 401
        RETURN_HR(E_MRM_BAD_NAME);
    }

    ResourceQualifier resourceQualifier {};
    HRESULT operationResult = defaultEnvironment->GetQualifier(qualifierIndex, &resourceQualifier);
    if (FAILED(operationResult))
    {
        // Original line: 402
        RETURN_HR(operationResult);
    }
    const QUALIFIER_DESCRIPTION* const description = &qualifierInfo->pQualifiers[qualifierIndex];
    if (info != nullptr)
    {
        std::memcpy(info, &resourceQualifier, sizeof(resourceQualifier));
        info->pToken = description->pQualifierToken;
        info->pDefaultValue = description->pDefaultValue;
        info->alwaysMatches = (description->flags & 1) != 0;
        info->readOnlyForApp = (description->flags & 2) != 0;
        operationResult = environment->GetTypeOfQualifier(resourceQualifier.name, &info->pQualifierType);
        if (FAILED(operationResult))
        {
            // Original line: 414
            RETURN_HR(operationResult);
        }
    }
    return S_OK;
}

HRESULT WindowsClientProfileBase::GetQualifierBuildInfoByName(
    const Atom name,
    const UnifiedEnvironment* const environment,
    QualifierBuildInfo* const info) const
{
    const IEnvironment* const defaultEnvironment = environment->GetDefaultEnvironment();
    const QUALIFIER_INFO* const qualifierInfo = defaultEnvironment->GetQualifierInfo();
    ResourceQualifier resourceQualifier {};
    HRESULT operationResult = environment->GetResourceQualifier(name, &resourceQualifier);
    if (FAILED(operationResult))
    {
        // Original line: 432
        RETURN_HR(operationResult);
    }
    if (name.GetPoolIndex() != defaultEnvironment->GetQualifierNames()->GetPoolIndex())
    {
        // Original line: 433
        RETURN_HR(HRESULT_FROM_WIN32(ERROR_MRM_INVALID_FILE_TYPE));
    }
    if (name.GetIndex() < 0 || name.GetIndex() > qualifierInfo->numQualifiers - 1)
    {
        // Original line: 434
        RETURN_HR(HRESULT_FROM_WIN32(ERROR_RANGE_NOT_FOUND));
    }
    const QUALIFIER_DESCRIPTION* const description = &qualifierInfo->pQualifiers[name.GetIndex()];
    if (info == nullptr)
    {
        return S_OK;
    }
    std::memcpy(info, &resourceQualifier, sizeof(resourceQualifier));
    info->pToken = description->pQualifierToken;
    info->pDefaultValue = description->pDefaultValue;
    info->alwaysMatches = (description->flags & 1) != 0;
    info->readOnlyForApp = (description->flags & 2) != 0;
    operationResult = environment->GetTypeOfQualifier(resourceQualifier.name, &info->pQualifierType);
    if (FAILED(operationResult))
    {
        // Original line: 446
        RETURN_HR(operationResult);
    }
    return S_OK;
}

int WindowsClientProfileBase::GetNumSupportedTokens() const { return m_pDefaultQualifierInfo->numQualifiers; }

HRESULT WindowsClientProfileBase::GetToken(const int index, StringResult* const token) const
{
    if (m_pDefaultQualifierInfo == nullptr)
    {
        // Original line: 465
        RETURN_HR(E_INVALIDARG);
    }
    if (index < 0 || index > m_pDefaultQualifierInfo->numQualifiers - 1)
    {
        // Original line: 466
        RETURN_HR(E_INVALIDARG);
    }
    return token->SetRef(m_pDefaultQualifierInfo->pQualifiers[index].pQualifierToken);
}

} // namespace Microsoft::Resources
