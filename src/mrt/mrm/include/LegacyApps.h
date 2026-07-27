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

class EnumeratedScaleQualifierType : public IntegerQualifierType
{
public:
    static HRESULT CreateInstance(EnumeratedScaleQualifierType** type);

    ~EnumeratedScaleQualifierType() override = default;

    HRESULT Evaluate(const IQualifier* qualifier, const wchar_t* providerValue, double* score) const override;

    static double CalculateScaleFactorScore(int providerValue, int assetValue);

protected:
    EnumeratedScaleQualifierType() : IntegerQualifierType(50, 1000, false) {}

    HRESULT ValidateSingleQualifierValue(const wchar_t* value) const override;
    HRESULT
    ValidateOrMakeCompatibleSingleQualifierValue(const wchar_t* value, StringResult* compatibleValue) const override;
    HRESULT InnerCompare(const IQualifier* left, const IQualifier* right, DEFCOMPARISON* result) const override;

private:
    struct ResultEntry
    {
        std::int32_t value;
        double score;
    };

    struct ProviderEntry
    {
        std::int32_t value;
        const wchar_t* stringValue;
        ResultEntry resultEntries[13];
    };

    static const ProviderEntry ResultEntries[13];
};

} // namespace Microsoft::Resources
