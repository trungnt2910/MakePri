#if __has_include_next(<guiddef.h>)
#include_next <guiddef.h>
#else

#pragma once

#include <cstdint>

struct GUID
{
    std::uint32_t Data1;
    std::uint16_t Data2;
    std::uint16_t Data3;
    std::uint8_t Data4[8];
};
using IID = GUID;
using CLSID = GUID;
using REFGUID = const GUID&;
using REFIID = const IID&;
using REFCLSID = const CLSID&;

constexpr bool operator==(const GUID& left, const GUID& right) noexcept
{
    return left.Data1 == right.Data1 && left.Data2 == right.Data2 && left.Data3 == right.Data3 && left.Data4[0] == right.Data4[0] &&
           left.Data4[1] == right.Data4[1] && left.Data4[2] == right.Data4[2] && left.Data4[3] == right.Data4[3] &&
           left.Data4[4] == right.Data4[4] && left.Data4[5] == right.Data4[5] && left.Data4[6] == right.Data4[6] &&
           left.Data4[7] == right.Data4[7];
}

constexpr bool operator!=(const GUID& left, const GUID& right) noexcept { return !(left == right); }

#define DEFINE_GUID(name, data1, data2, data3, b0, b1, b2, b3, b4, b5, b6, b7) \
    inline constexpr GUID name \
    { \
        data1, data2, data3, { b0, b1, b2, b3, b4, b5, b6, b7 } \
    }

#endif // __has_include_next(<guiddef.h>)
