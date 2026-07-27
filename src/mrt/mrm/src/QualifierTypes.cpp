#include "StdAfx.h"

#include <QualifierTypes.h>

extern "C" bool WINAPI IsWellFormedTag(const wchar_t* languageTag);
extern "C" HRESULT WINAPI
GetDistanceOfClosestLanguageInList(const wchar_t* languageList, const wchar_t* language, wchar_t delimiter, double* score);
extern "C" HRESULT WINAPI FormatLanguageTag(const wchar_t* languageTag, int maximumLength, const wchar_t* source, wchar_t* result);

namespace Microsoft::Resources
{
namespace
{

__declspec(noinline) std::uint32_t WINAPI GetCompositeRegionCode(const wchar_t* const region)
{
    if (region == nullptr)
    {
        return 0;
    }

    Windows::Internal::CRegion parsedRegion(region);
    return parsedRegion.GetCompositeRegionCode();
}

} // namespace

HRESULT LanguageQualifierType::CreateInstance(LanguageQualifierType** type)
{
    *type = nullptr;

    auto* result = new LanguageQualifierType();
    // Original line: 17
    RETURN_IF_NULL_ALLOC(result);

    *type = result;
    return S_OK;
}

HRESULT
LanguageQualifierType::ValidateSingleQualifierValue(const wchar_t* const value) const
{
    StringResult language;
    RETURN_IF_FAILED(language.SetRef(value));

    if ((language.GetRef() != nullptr) && (*language.GetRef() != L'\0') && !IsWellFormedTag(language.GetRef()))
    {
        return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_QUALIFIER_VALUE);
    }

    return S_OK;
}

HRESULT LanguageQualifierType::ValidateQualifierComparison(
    const Atom qualifierName,
    const ICondition::ConditionOperator conditionOperator,
    const wchar_t* const qualifierValue) const
{
    (void)qualifierName;

    if ((conditionOperator == ICondition::MatchOp) && IsWellFormedTag(qualifierValue))
    {
        return S_OK;
    }
    if (conditionOperator == ICondition::MatchOp)
    {
        return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_QUALIFIER_VALUE);
    }
    return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_QUALIFIER_OPERATOR);
}

HRESULT LanguageQualifierType::Evaluate(const IQualifier* const qualifier, const wchar_t* const providerValue, double* const score) const
{
    *score = 0.0;
    if (std::wcslen(providerValue) != 0)
    {
        StringResult qualifierValue;
        RETURN_IF_FAILED(ValidateQualifier(qualifier));
        RETURN_IF_FAILED(qualifier->GetOperand2Literal(&qualifierValue));

        GetDistanceOfClosestLanguageInList(qualifierValue.GetRef(), providerValue, L';', score);
    }
    return S_OK;
}

HRESULT LanguageQualifierType::InnerCompare(const IQualifier* const left, const IQualifier* const right, DEFCOMPARISON* const result) const
{
    *result = Def_CompareError;

    StringResult leftValue;
    StringResult rightValue;
    RETURN_IF_FAILED(left->GetOperand2Literal(&leftValue));
    RETURN_IF_FAILED(right->GetOperand2Literal(&rightValue));

    const int comparison = CompareBcp47Tags(leftValue.GetRef(), rightValue.GetRef());
    if (comparison < 0)
    {
        *result = Def_Greater;
    }
    else if (comparison > 0)
    {
        *result = Def_Less;
    }
    else
    {
        RETURN_IF_FAILED(QualifierTypeBase::InnerCompare(left, right, result));
    }

    return S_OK;
}

HRESULT LanguageQualifierType::GetPackagingInfo(
    const IQualifier* const qualifier,
    const std::uint32_t buildConfiguration,
    const wchar_t** const autoPackageValues,
    const size_t numAutoPackageValues,
    PackagingFlags* const flags,
    StringResult* const affinity) const
{
    double fallbackScore {};
    RETURN_IF_FAILED(qualifier->GetFallbackScore(&fallbackScore));

    *flags = static_cast<PackagingFlags>(PackagingAllowResourcePackage | PackagingReportQualifier);
    if ((fallbackScore > 0.0) && ((buildConfiguration & MrmBuildConfiguration::UseGranularResourceSplittingFlag) == 0) &&
        (((buildConfiguration & MrmBuildConfiguration::SplitLanguageVariantsFlag) == 0) || (fallbackScore == 1.0)))
    {
        return affinity->SetRef(L"default");
    }

    RETURN_IF_FAILED(ValidateQualifier(qualifier));

    StringResult qualifierValue;
    RETURN_IF_FAILED(qualifier->GetOperand2Literal(&qualifierValue));

    wchar_t formattedLanguage[85] {};
    RETURN_IF_FAILED(FormatLanguageTag(qualifierValue.GetRef(), 7, nullptr, formattedLanguage));

    if ((autoPackageValues == nullptr) || (numAutoPackageValues == 0))
    {
        const wchar_t* value =
            (buildConfiguration & MrmBuildConfiguration::SplitLanguageVariantsFlag) != 0 ? qualifierValue.GetRef() : formattedLanguage;
        return DefStringResult_SetCopy(affinity->GetStringResult(), value);
    }

    const wchar_t* closestQualifierValue {};
    double qualifierValueScore {};
    RETURN_IF_FAILED(_GetClosestLanguageInList(
        qualifierValue.GetRef(),
        autoPackageValues,
        static_cast<std::uint32_t>(numAutoPackageValues),
        &closestQualifierValue,
        &qualifierValueScore));

    if ((qualifierValueScore == 1.0) && (closestQualifierValue != nullptr))
    {
        return affinity->SetRef(closestQualifierValue);
    }

    const wchar_t* closestFormattedLanguage {};
    double formattedLanguageScore {};
    RETURN_IF_FAILED(_GetClosestLanguageInList(
        formattedLanguage,
        autoPackageValues,
        static_cast<std::uint32_t>(numAutoPackageValues),
        &closestFormattedLanguage,
        &formattedLanguageScore));

    if (((closestQualifierValue != nullptr) || (closestFormattedLanguage != nullptr)) &&
        (std::fmax(qualifierValueScore, formattedLanguageScore) >= 0.0))
    {
        if (closestQualifierValue < closestFormattedLanguage)
        {
            return affinity->SetRef(closestFormattedLanguage);
        }
        return affinity->SetRef(closestQualifierValue);
    }

    return affinity->SetRef(L"default");
}

int LanguageQualifierType::GetMaxQualifierEntries() const { return 256; }

HRESULT LanguageQualifierType::_GetClosestLanguageInList(
    const wchar_t* const language,
    const wchar_t** const languages,
    const std::uint32_t numLanguages,
    const wchar_t** const closestLanguage,
    double* const closestScore) const
{
    double bestScore = -1.0;
    *closestLanguage = nullptr;

    for (std::uint32_t i = 0; i < numLanguages; ++i)
    {
        const size_t languageSize = std::wcslen(languages[i]) + 1;
        wchar_t* const copiedLanguage = new (std::nothrow) wchar_t[languageSize + 1];
        RETURN_IF_NULL_ALLOC(copiedLanguage);

        const HRESULT copyStatus = StringCchCopyW(copiedLanguage, languageSize, languages[i]);
        if (FAILED(copyStatus))
        {
            delete[] copiedLanguage;
            RETURN_HR(copyStatus);
        }

        copiedLanguage[languageSize] = L'\0';

        double score {};
        const HRESULT status = GetDistanceOfClosestLanguageInList(language, copiedLanguage, L'\0', &score);
        if (FAILED(status) && (status != HRESULT_FROM_WIN32(ERROR_NO_MATCH)))
        {
            delete[] copiedLanguage;
            return status;
        }
        if (SUCCEEDED(status) && (score > 0.0) && (score > bestScore))
        {
            bestScore = score;
            *closestLanguage = languages[i];
        }

        delete[] copiedLanguage;
    }

    *closestScore = bestScore;
    return S_OK;
}

HRESULT RegionQualifierType::CreateInstance(RegionQualifierType** type)
{
    *type = nullptr;

    auto* result = new RegionQualifierType();
    RETURN_IF_NULL_ALLOC(result);

    *type = result;
    return S_OK;
}

HRESULT
RegionQualifierType::ValidateSingleQualifierValue(const wchar_t* const value) const
{
    if (DefString_IsEmpty(value) || (value == nullptr))
    {
        return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_QUALIFIER_VALUE);
    }

    return Windows::Internal::CRegion::TryFindRegionId(value) != 0 ? S_OK : HRESULT_FROM_WIN32(ERROR_MRM_INVALID_QUALIFIER_VALUE);
}

HRESULT RegionQualifierType::Evaluate(const IQualifier* const qualifier, const wchar_t* const providerValue, double* const score) const
{
    *score = 0.0;

    StringResult qualifierValue;
    RETURN_IF_FAILED(ValidateQualifier(qualifier));
    RETURN_IF_FAILED(ValidateQualifierValue(providerValue));
    RETURN_IF_FAILED(qualifier->GetOperand2Literal(&qualifierValue));

    const wchar_t* const assetValue = qualifierValue.GetRef();
    RETURN_HR_IF(E_POINTER, (providerValue == nullptr) || (assetValue == nullptr));

    Windows::Internal::CRegion providerRegion(providerValue);
    Windows::Internal::CRegion assetRegion(assetValue);
    RETURN_IF_FAILED(providerRegion.Compare(assetRegion, score));
    return S_OK;
}

HRESULT RegionQualifierType::InnerCompare(const IQualifier* const left, const IQualifier* const right, DEFCOMPARISON* const result) const
{
    *result = Def_CompareError;

    StringResult leftValue;
    StringResult rightValue;
    RETURN_IF_FAILED(left->GetOperand2Literal(&leftValue));
    RETURN_IF_FAILED(right->GetOperand2Literal(&rightValue));

    std::uint32_t leftCompositeRegionCode = GetCompositeRegionCode(leftValue.GetRef());
    std::uint32_t rightCompositeRegionCode = GetCompositeRegionCode(rightValue.GetRef());
    while ((leftCompositeRegionCode != 0) && (rightCompositeRegionCode != 0))
    {
        leftCompositeRegionCode = Windows::Internal::CRegion::GetCompositeRegionCode(leftCompositeRegionCode);
        rightCompositeRegionCode = Windows::Internal::CRegion::GetCompositeRegionCode(rightCompositeRegionCode);
    }

    int difference = static_cast<int>(leftCompositeRegionCode) - static_cast<int>(rightCompositeRegionCode);
    if (difference > 0)
    {
        *result = Def_Greater;
        return S_OK;
    }

    if (difference < 0)
    {
        *result = Def_Less;
        return S_OK;
    }

    RETURN_IF_FAILED(QualifierTypeBase::InnerCompare(left, right, result));
    return S_OK;
}

} // namespace Microsoft::Resources
