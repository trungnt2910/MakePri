#pragma once

#include <cstdint>

#include <mrm/BaseInternal.h>
#include <mrm/Collections.h>
#include <mrm/common/file/MrmFiles.h>
#include <mrm/common/MrmProfileData.h>
#include <mrm/Checksums.h>
#include <mrm/MrmEnvironment.h>
#include <mrm/MrmQualifiers.h>
#include <mrm/platform/base.h>

namespace Microsoft::Resources
{

class WindowsClientProfileBase;

class LanguageQualifierType : public QualifierTypeBase
{
public:
    static HRESULT CreateInstance(LanguageQualifierType** type);

    ~LanguageQualifierType() override = default;

    PackagingFlags GetDefaultPackagingFlags() const override
    {
        return static_cast<PackagingFlags>(PackagingAllowResourcePackage | PackagingReportQualifier);
    }

    HRESULT ValidateQualifierComparison(Atom qualifierName, ICondition::ConditionOperator conditionOperator, const wchar_t* qualifierValue)
        const override;
    HRESULT Evaluate(const IQualifier* qualifier, const wchar_t* providerValue, double* score) const override;
    HRESULT GetPackagingInfo(
        const IQualifier* qualifier,
        std::uint32_t buildConfiguration,
        const wchar_t** autoPackageValues,
        size_t numAutoPackageValues,
        PackagingFlags* flags,
        StringResult* affinity) const override;
    int GetMaxQualifierEntries() const override;

protected:
    LanguageQualifierType() : QualifierTypeBase(ListValuesAllowed | EmptyValuesNotAllowed) {}

    HRESULT ValidateSingleQualifierValue(const wchar_t* value) const override;
    HRESULT InnerCompare(const IQualifier* left, const IQualifier* right, DEFCOMPARISON* result) const override;

private:
    HRESULT _GetClosestLanguageInList(
        const wchar_t* language,
        const wchar_t** languages,
        std::uint32_t numLanguages,
        const wchar_t** closestLanguage,
        double* closestScore) const;
};

class RegionQualifierType : public QualifierTypeBase
{
public:
    static HRESULT CreateInstance(RegionQualifierType** type);

    ~RegionQualifierType() override = default;

    HRESULT Evaluate(const IQualifier* qualifier, const wchar_t* providerValue, double* score) const override;

protected:
    RegionQualifierType() : QualifierTypeBase(RequiredValueQualifierTypeFlags) {}

    HRESULT ValidateSingleQualifierValue(const wchar_t* value) const override;
    HRESULT InnerCompare(const IQualifier* left, const IQualifier* right, DEFCOMPARISON* result) const override;
};

} // namespace Microsoft::Resources
