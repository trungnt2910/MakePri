#pragma once

#include <cstdint>

#include <mrm/BaseInternal.h>
#include <mrm/Checksums.h>
#include <mrm/Collections.h>
#include <mrm/DefObject.h>
#include <mrm/Atoms.h>
#include <mrm/MrmEnvironment.h>
#include <mrm/MrmQualifiers.h>
#include <mrm/Results.h>
#include <mrm/common/MrmProfileData.h>
#include <mrm/common/file/MrmFiles.h>
#include <mrm/platform/base.h>
#include <mrm/platform/WindowsCore.h>

namespace Microsoft::Resources
{

class EnvironmentVersionInfo;
class IEnvironment;
class IEnvironmentVersionInfo;
class IRawResourceMap;
class StringResult;
class UnifiedEnvironment;

class MrmProfile : public CoreProfile
{
public:
    enum class ProfileType
    {
        EmptyInit = 0,
    };

    virtual ~MrmProfile() = default;

    [[nodiscard]] virtual bool IsPackageLoadAllowedDuringStartup(int packageIndex) const;
    [[nodiscard]] virtual bool GetIsChangeNotificationSupported(const wchar_t* resourceMapName) const;
    [[nodiscard]] virtual int GetNumPackages() const;

    static HRESULT ChooseDefaultProfile(
        ProfileType type,
        MrmPlatformVersionInternal platformVersion,
        const wchar_t* packageFamilyName,
        const wchar_t* packageFullName,
        const wchar_t* applicationId,
        MrmProfile** result);

    static HRESULT GetDefaultTargetOsVersionForFileMagic(const DEFFILE_MAGIC& magic, StringResult* platform, StringResult* version);

    using CoreProfile::IsLoadPriFileAllowed;
    [[nodiscard]] virtual bool IsLoadPriFileAllowed(const wchar_t* fileName, MRMPROFILE_PHASE phase);
    virtual HRESULT GetPackageRootFolder(const wchar_t* packageFullName, StringResult* result);
    virtual HRESULT GetMainPriFilePathForResourceMapName(const wchar_t* resourceMapName, StringResult* result, bool* isCurrent);
    [[nodiscard]] virtual bool TryGetPackageForFile(const wchar_t* fileName, int* packageIndex);
    virtual HRESULT GetPackageFullNameForFile(int packageIndex, const IRawResourceMap* resourceMap, StringResult* result);
    virtual HRESULT GetQualifierBuildInfoByToken(const wchar_t* token, const UnifiedEnvironment* environment, QualifierBuildInfo* info)
        const = 0;
    virtual HRESULT GetQualifierBuildInfoByName(Atom name, const UnifiedEnvironment* environment, QualifierBuildInfo* info) const = 0;
    [[nodiscard]] virtual int GetNumSupportedTokens() const = 0;
    virtual HRESULT GetToken(int index, StringResult* token) const = 0;
    virtual HRESULT GetObjectSharedState(MrtSharedObjectState* result) const;

    HRESULT GetTargetPlatformVersionForFileMagic(const DEFFILE_MAGIC& magic, MrmPlatformVersionInternal* result) override;
    HRESULT GetPackageInfo(int packageIndex, StringResult* packageFamilyName, StringResult* packageFullName, StringResult* packageRoot)
        const override;
    HRESULT GetQualifierInfoForEnvironment(
        const wchar_t* simpleName,
        const IEnvironmentVersionInfo* version,
        const IEnvironment* environment,
        int* numQualifiers,
        const wchar_t* const** qualifierNames,
        const Atom::SmallIndex** qualifierMappings) const override;
    HRESULT GetDefaultEnvironmentForFileMagic(const DEFFILE_MAGIC& magic, StringResult* environmentName, EnvironmentVersionInfo* result)
        const override;
};

namespace CoreEnvironment
{
extern const ENVIRONMENT_INITIALIZER& EnvironmentInitializer;
extern const ENVIRONMENT_DESCRIPTION& EnvironmentDescription;
extern const QUALIFIER_INFO* const QualifierInfo;
} // namespace CoreEnvironment

class WindowsClientEnvironment
{
public:
    static const wchar_t* const ClientQualifierNames[9];
    static const std::uint16_t ClientToCoreQualifierMapping[9];
    static const std::uint16_t CoreToClientQualifierMapping[12];
    static const MRMFILE_ENVIRONMENT_VERSION_INFO ClientVersions[5];
    static const wchar_t* const ClientQualifierTypeNames[9];
    static const QUALIFIER_DESCRIPTION ClientQualifierDescriptions[9];
    static const QUALIFIER_TYPE_DESCRIPTION ClientQualifierTypeDescriptions[9];
    static const QUALIFIER_INFO ClientQualifierInfo[];
    static const ENVIRONMENT_DESCRIPTION ClientEnvironmentDescription;
    static const ENVIRONMENT_INITIALIZER ClientEnvironmentInitializer;
};

class WindowsPhoneEnvironment
{
public:
    static const wchar_t* const PhoneQualifierNames[9];
    static const std::uint16_t PhoneToCoreQualifierMapping[9];
    static const std::uint16_t CoreToPhoneQualifierMapping[12];
    static const MRMFILE_ENVIRONMENT_VERSION_INFO PhoneVersions[2];
    static const wchar_t* const PhoneQualifierTypeNames[9];
    static const QUALIFIER_DESCRIPTION PhoneQualifierDescriptions[9];
    static const QUALIFIER_TYPE_DESCRIPTION PhoneQualifierTypeDescriptions[9];
    static const QUALIFIER_INFO PhoneQualifierInfo[];
    static const ENVIRONMENT_DESCRIPTION PhoneEnvironmentDescription;
    static const ENVIRONMENT_INITIALIZER PhoneEnvironmentInitializer;
};

class IQualifierValueProvider;

class WindowsClientProfileBase : public MrmProfile
{
public:
    ~WindowsClientProfileBase() override = default;

    using CoreProfile::GetEnvironmentVersionInfo;

    [[nodiscard]] bool IsSupportedFileMagicNumber(const DEFFILE_MAGIC& magic) const override;
    HRESULT GetTargetPlatformAndVersionForFileMagic(const DEFFILE_MAGIC& magic, StringResult* platform, StringResult* version) override;
    HRESULT GetTargetPlatformAndVersion(StringResult* platform, StringResult* version) override;
    [[nodiscard]] int GetNumEnvironments() const override;
    HRESULT CreateEnvironment(int index, AtomPoolGroup* atomPoolGroup, IEnvironment** result) const override;
    HRESULT GetTypeForQualifier(const IEnvironment* environment, Atom qualifier, IBuildQualifierType** result) const override;
    HRESULT GetProviderForQualifier(const IEnvironment* environment, Atom qualifier, IQualifierValueProvider** result) const override;
    [[nodiscard]] bool IsCompatibleEnvironment(
        const EnvironmentReference* reference,
        const IEnvironment* environment,
        const RemapAtomPool** remap) const override;
    HRESULT GetMergeFolders(StringResult* mergeFolder, StringResult* systemMergeFolder) const override;
    MrmBuildConfiguration* GetBuildConfiguration() override;
    HRESULT GetQualifierBuildInfoByToken(const wchar_t* token, const UnifiedEnvironment* environment, QualifierBuildInfo* info)
        const override;
    HRESULT GetQualifierBuildInfoByName(Atom name, const UnifiedEnvironment* environment, QualifierBuildInfo* info) const override;
    [[nodiscard]] int GetNumSupportedTokens() const override;
    HRESULT GetToken(int index, StringResult* token) const override;

    virtual HRESULT GetEnvironmentVersionInfo(int index, IEnvironmentVersionInfo** result, StringResult* environmentName) const;

protected:
    explicit WindowsClientProfileBase(MrmPlatformVersionInternal platformVersion);

    HRESULT Initialize(MrmPlatformVersionInternal platformVersion);

private:
    [[nodiscard]] bool IsSupportedFileMagicNumber(bool unknown, const DEFFILE_MAGIC& magic) const;
    static HRESULT RemapQualifierIndexToCore(
        MrmPlatformVersionInternal platformVersion,
        Atom qualifier,
        CoreEnvironment::QualifierIndex* index,
        bool* remapped);
    static HRESULT GetDefaultTypeForQualifier(
        const IEnvironment* environment,
        MrmPlatformVersionInternal platformVersion,
        Atom qualifier,
        IBuildQualifierType** result);

    MrmPlatformVersionInternal m_platformVersion;
    const ENVIRONMENT_INITIALIZER* m_pEnvironmentInitializer {};
    const QUALIFIER_INFO* m_pDefaultQualifierInfo {};
    bool m_isDesignMode {};
};

} // namespace Microsoft::Resources
