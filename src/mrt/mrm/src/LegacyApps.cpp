#include "StdAfx.h"

#include <LegacyApps.h>
#include <mrm/platform/CoreQualifierTypes.h>
#include <mrm/platform/WindowsCore.h>

namespace Microsoft::Resources
{

const EnumeratedScaleQualifierType::ProviderEntry EnumeratedScaleQualifierType::ResultEntries[] = {
    {80,
     L"80",
     {{80, 1.0},
      {100, 0.9},
      {120, 0.8},
      {140, 0.7},
      {150, 0.65},
      {160, 0.6},
      {180, 0.5},
      {200, 0.4},
      {220, 0.3},
      {225, 0.25},
      {240, 0.2},
      {300, 0.1},
      {400, 0.05}}},
    {100,
     L"100",
     {{80, 0.45},
      {100, 1.0},
      {120, 0.9},
      {140, 0.8},
      {150, 0.75},
      {160, 0.7},
      {180, 0.6},
      {200, 0.5},
      {220, 0.4},
      {225, 0.35},
      {240, 0.3},
      {300, 0.2},
      {400, 0.1}}},
    {120,
     L"120",
     {{80, 0.25},
      {100, 0.3},
      {120, 1.0},
      {140, 0.9},
      {150, 0.85},
      {160, 0.8},
      {180, 0.7},
      {200, 0.6},
      {220, 0.5},
      {225, 0.45},
      {240, 0.4},
      {300, 0.2},
      {400, 0.1}}},
    {140,
     L"140",
     {{80, 0.25},
      {100, 0.3},
      {120, 0.4},
      {140, 1.0},
      {150, 0.95},
      {160, 0.9},
      {180, 0.8},
      {200, 0.7},
      {220, 0.6},
      {225, 0.55},
      {240, 0.5},
      {300, 0.2},
      {400, 0.1}}},
    {150,
     L"150",
     {{80, 0.15},
      {100, 0.2},
      {120, 0.3},
      {140, 0.35},
      {150, 1.0},
      {160, 0.9},
      {180, 0.8},
      {200, 0.7},
      {220, 0.6},
      {225, 0.55},
      {240, 0.5},
      {300, 0.4},
      {400, 0.1}}},
    {160,
     L"160",
     {{80, 0.15},
      {100, 0.2},
      {120, 0.3},
      {140, 0.4},
      {150, 0.45},
      {160, 1.0},
      {180, 0.9},
      {200, 0.8},
      {220, 0.7},
      {225, 0.65},
      {240, 0.6},
      {300, 0.5},
      {400, 0.1}}},
    {180,
     L"180",
     {{80, 0.15},
      {100, 0.2},
      {120, 0.3},
      {140, 0.4},
      {150, 0.45},
      {160, 0.5},
      {180, 1.0},
      {200, 0.9},
      {220, 0.8},
      {225, 0.75},
      {240, 0.7},
      {300, 0.6},
      {400, 0.1}}},
    {200,
     L"200",
     {{80, 0.05},
      {100, 0.1},
      {120, 0.2},
      {140, 0.3},
      {150, 0.35},
      {160, 0.4},
      {180, 0.5},
      {200, 1.0},
      {220, 0.9},
      {225, 0.85},
      {240, 0.8},
      {300, 0.7},
      {400, 0.6}}},
    {220,
     L"220",
     {{80, 0.05},
      {100, 0.1},
      {120, 0.2},
      {140, 0.3},
      {150, 0.35},
      {160, 0.4},
      {180, 0.5},
      {200, 0.6},
      {220, 1.0},
      {225, 0.95},
      {240, 0.9},
      {300, 0.8},
      {400, 0.7}}},
    {225,
     L"225",
     {{80, 0.05},
      {100, 0.1},
      {120, 0.15},
      {140, 0.2},
      {150, 0.25},
      {160, 0.3},
      {180, 0.4},
      {200, 0.5},
      {220, 0.6},
      {225, 1.0},
      {240, 0.9},
      {300, 0.8},
      {400, 0.7}}},
    {240,
     L"240",
     {{80, 0.05},
      {100, 0.1},
      {120, 0.2},
      {140, 0.3},
      {150, 0.35},
      {160, 0.4},
      {180, 0.5},
      {200, 0.6},
      {220, 0.7},
      {225, 0.75},
      {240, 1.0},
      {300, 0.9},
      {400, 0.8}}},
    {300,
     L"300",
     {{80, 0.05},
      {100, 0.1},
      {120, 0.2},
      {140, 0.3},
      {150, 0.35},
      {160, 0.4},
      {180, 0.5},
      {200, 0.6},
      {220, 0.7},
      {225, 0.75},
      {240, 0.8},
      {300, 1.0},
      {400, 0.9}}},
    {400,
     L"400",
     {{80, 0.05},
      {100, 0.1},
      {120, 0.2},
      {140, 0.3},
      {150, 0.35},
      {160, 0.4},
      {180, 0.5},
      {200, 0.6},
      {220, 0.7},
      {225, 0.75},
      {240, 0.8},
      {300, 0.9},
      {400, 1.0}}},
};

HRESULT EnumeratedScaleQualifierType::CreateInstance(EnumeratedScaleQualifierType** type)
{
    *type = nullptr;

    auto* result = new EnumeratedScaleQualifierType();
    RETURN_IF_NULL_ALLOC(result);

    *type = result;
    return S_OK;
}

HRESULT
EnumeratedScaleQualifierType::ValidateSingleQualifierValue(const wchar_t* value) const
{
    if (DefString_IsEmpty(value))
    {
        return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_QUALIFIER_VALUE);
    }

    if ((DefString_CompareWithOptions(value, L"500", DefCompare_CaseInsensitive) == Def_Equal) ||
        (DefString_CompareWithOptions(value, L"400", DefCompare_CaseInsensitive) == Def_Equal) ||
        (DefString_CompareWithOptions(value, L"300", DefCompare_CaseInsensitive) == Def_Equal) ||
        (DefString_CompareWithOptions(value, L"250", DefCompare_CaseInsensitive) == Def_Equal) ||
        (DefString_CompareWithOptions(value, L"240", DefCompare_CaseInsensitive) == Def_Equal) ||
        (DefString_CompareWithOptions(value, L"225", DefCompare_CaseInsensitive) == Def_Equal) ||
        (DefString_CompareWithOptions(value, L"220", DefCompare_CaseInsensitive) == Def_Equal) ||
        (DefString_CompareWithOptions(value, L"200", DefCompare_CaseInsensitive) == Def_Equal) ||
        (DefString_CompareWithOptions(value, L"180", DefCompare_CaseInsensitive) == Def_Equal) ||
        (DefString_CompareWithOptions(value, L"160", DefCompare_CaseInsensitive) == Def_Equal) ||
        (DefString_CompareWithOptions(value, L"150", DefCompare_CaseInsensitive) == Def_Equal) ||
        (DefString_CompareWithOptions(value, L"140", DefCompare_CaseInsensitive) == Def_Equal) ||
        (DefString_CompareWithOptions(value, L"125", DefCompare_CaseInsensitive) == Def_Equal) ||
        (DefString_CompareWithOptions(value, L"120", DefCompare_CaseInsensitive) == Def_Equal) ||
        (DefString_CompareWithOptions(value, L"100", DefCompare_CaseInsensitive) == Def_Equal) ||
        (DefString_CompareWithOptions(value, L"80", DefCompare_CaseInsensitive) == Def_Equal))
    {
        return S_OK;
    }

    return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_QUALIFIER_VALUE);
}

HRESULT EnumeratedScaleQualifierType::ValidateOrMakeCompatibleSingleQualifierValue(const wchar_t* value, StringResult* compatibleValue)
    const
{
    if (SUCCEEDED(ValidateSingleQualifierValue(value)))
    {
        RETURN_IF_FAILED(compatibleValue->SetRef(L""));
        return S_OK;
    }

    int numericValue;
    if (DefString_ToInteger(value, &numericValue) && (numericValue > 0) && (static_cast<std::uint32_t>(numericValue - 50) <= 950))
    {
        int index = 0;
        while ((index < 12) && (ResultEntries[index].value < numericValue))
        {
            ++index;
        }

        RETURN_IF_FAILED(DefStringResult_SetCopy(compatibleValue->GetStringResult(), ResultEntries[index].stringValue));
        return S_OK;
    }

    return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_QUALIFIER_VALUE);
}

double EnumeratedScaleQualifierType::CalculateScaleFactorScore(int providerValue, int assetValue)
{
    if (static_cast<double>(providerValue) == 0.0)
    {
        return 0.0;
    }

    if (providerValue == assetValue)
    {
        return 1.0;
    }

    for (const ProviderEntry& providerEntry : ResultEntries)
    {
        if (providerEntry.value == providerValue)
        {
            for (const ResultEntry& resultEntry : providerEntry.resultEntries)
            {
                if (resultEntry.value == assetValue)
                {
                    return resultEntry.score;
                }
            }

            break;
        }
    }

    return ScaleQualifierType::CalculateScaleFactorScore(providerValue, assetValue);
}

HRESULT EnumeratedScaleQualifierType::Evaluate(const IQualifier* qualifier, const wchar_t* providerValue, double* score) const
{
    *score = 0.0;

    StringResult qualifierValue;
    RETURN_IF_FAILED(ValidateQualifier(qualifier));
    RETURN_IF_FAILED(ValidateQualifierValue(providerValue));
    RETURN_IF_FAILED(qualifier->GetOperand2Literal(&qualifierValue));

    int providerScale = static_cast<int>(_wtof(providerValue));
    int assetScale = static_cast<int>(_wtof(qualifierValue.GetRef()));
    *score = CalculateScaleFactorScore(providerScale, assetScale);
    return S_OK;
}

HRESULT EnumeratedScaleQualifierType::InnerCompare(const IQualifier* left, const IQualifier* right, DEFCOMPARISON* result) const
{
    *result = Def_CompareError;

    StringResult leftValue;
    StringResult rightValue;
    RETURN_IF_FAILED(left->GetOperand2Literal(&leftValue));
    RETURN_IF_FAILED(right->GetOperand2Literal(&rightValue));

    int difference = static_cast<int>(_wtof(leftValue.GetRef())) - static_cast<int>(_wtof(rightValue.GetRef()));
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

    return QualifierTypeBase::InnerCompare(left, right, result);
}

} // namespace Microsoft::Resources
