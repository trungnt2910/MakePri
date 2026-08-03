#include <algorithm>
#include <cstdlib>
#include <format>
#include <mutex>
#include <random>
#include <string>

#include <rpc.h>

#include <uni_algo/conv.h>

extern "C" RPC_STATUS WINAPI UuidCreate(UUID* const uuid)
{
    if (uuid == nullptr)
    {
        return ERROR_INVALID_PARAMETER;
    }
    static std::random_device randomDevice;
    static std::mutex randomDeviceMutex;
    const std::lock_guard lock(randomDeviceMutex);
    uuid->Data1 = randomDevice();
    uuid->Data2 = static_cast<USHORT>(randomDevice());
    uuid->Data3 = static_cast<USHORT>((randomDevice() & 0x0fff) | 0x4000);
    for (BYTE& byte : uuid->Data4)
    {
        byte = static_cast<BYTE>(randomDevice());
    }
    uuid->Data4[0] = static_cast<BYTE>((uuid->Data4[0] & 0x3f) | 0x80);
    return RPC_S_OK;
}

extern "C" RPC_STATUS WINAPI UuidToStringW(const UUID* const uuid, RPC_WSTR* const string)
{
    if (uuid == nullptr || string == nullptr)
    {
        return ERROR_INVALID_PARAMETER;
    }
    const std::string formatted = std::format(
        "{:08x}-{:04x}-{:04x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
        uuid->Data1,
        uuid->Data2,
        uuid->Data3,
        uuid->Data4[0],
        uuid->Data4[1],
        uuid->Data4[2],
        uuid->Data4[3],
        uuid->Data4[4],
        uuid->Data4[5],
        uuid->Data4[6],
        uuid->Data4[7]);
    const std::u16string converted = una::utf8to16<char, char16_t>(formatted);
    auto* const result = static_cast<char16_t*>(std::malloc(sizeof(char16_t) * (converted.size() + 1)));
    if (result == nullptr)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    std::copy(converted.begin(), converted.end(), result);
    result[converted.size()] = u'\0';
    *string = reinterpret_cast<RPC_WSTR>(result);
    return RPC_S_OK;
}

extern "C" RPC_STATUS WINAPI RpcStringFreeW(RPC_WSTR* const string)
{
    if (string == nullptr)
    {
        return ERROR_INVALID_PARAMETER;
    }
    std::free(*string);
    *string = nullptr;
    return RPC_S_OK;
}
