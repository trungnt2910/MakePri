#pragma once

#include <windows.h>

#include <cstdint>

struct RegionsTableEntry
{
    std::uint32_t regionId;
    std::uint16_t regionCode;
    std::uint16_t compositeRegionCode;
};

extern const RegionsTableEntry regionsTable[281];

namespace Microsoft::Resources
{

class RegionQualifierType;

}

namespace Windows::Internal
{

class CRegion
{
public:
    explicit CRegion(const wchar_t* region);

    HRESULT Compare(const CRegion& other, double* score) const;

    [[nodiscard]] std::uint32_t GetCompositeRegionCode() const { return regionsTable[m_tableIndex].compositeRegionCode; }

    static std::uint32_t GetCompositeRegionCode(std::uint32_t regionCode);

private:
    friend class Microsoft::Resources::RegionQualifierType;

    static std::uint32_t TryFindRegionId(const wchar_t* region);

    std::uint32_t m_regionId;
    std::uint32_t m_tableIndex;
};

} // namespace Windows::Internal
