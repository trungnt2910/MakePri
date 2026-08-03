#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <winuser.h>

#include <uni_algo/conv.h>

#include "internal/resources.h"

namespace
{
constexpr std::uint16_t OrdinalMarker = 0xffff;
constexpr std::uint16_t StringResourceType = 6;
constexpr unsigned StringsPerBlock = 16;

struct ResourcePrefix
{
    std::uint32_t dataSize;
    std::uint32_t headerSize;
};

struct ResourceMetadata
{
    std::uint32_t dataVersion;
    std::uint16_t memoryFlags;
    std::uint16_t languageId;
    std::uint32_t version;
    std::uint32_t characteristics;
};

struct ResourceIdentifier
{
    bool isOrdinal;
    std::uint16_t ordinal;
};

struct ResourceRecord
{
    ResourceIdentifier type;
    ResourceIdentifier name;
    std::span<const unsigned char> contents;
};

std::vector<std::span<const unsigned char>>& ResourceDataSets()
{
    static std::vector<std::span<const unsigned char>> data;
    return data;
}

std::mutex& ResourceDataMutex()
{
    static std::mutex mutex;
    return mutex;
}

bool Contains(const std::span<const unsigned char> data, const std::size_t offset, const std::size_t size)
{
    return offset <= data.size() && size <= data.size() - offset;
}

template<typename Value>
std::optional<Value> Read(const std::span<const unsigned char> data, const std::size_t offset)
{
    if (!Contains(data, offset, sizeof(Value)))
    {
        return std::nullopt;
    }
    Value value;
    std::memcpy(&value, data.data() + offset, sizeof(value));
    return value;
}

std::size_t AlignToDword(const std::size_t value)
{
    constexpr std::size_t Alignment = sizeof(std::uint32_t);
    return (value + Alignment - 1) & ~(Alignment - 1);
}

std::optional<ResourceIdentifier> ReadIdentifier(const std::span<const unsigned char> data, std::size_t& cursor, const std::size_t end)
{
    const auto first = Read<std::uint16_t>(data, cursor);
    if (!first || cursor + sizeof(*first) > end)
    {
        return std::nullopt;
    }
    cursor += sizeof(*first);
    if (*first == OrdinalMarker)
    {
        const auto ordinal = Read<std::uint16_t>(data, cursor);
        if (!ordinal || cursor + sizeof(*ordinal) > end)
        {
            return std::nullopt;
        }
        cursor += sizeof(*ordinal);
        return ResourceIdentifier {true, *ordinal};
    }

    std::uint16_t character = *first;
    while (character != 0)
    {
        const auto next = Read<std::uint16_t>(data, cursor);
        if (!next || cursor + sizeof(*next) > end)
        {
            return std::nullopt;
        }
        cursor += sizeof(*next);
        character = *next;
    }
    return ResourceIdentifier {false, 0};
}

std::optional<ResourceRecord> ReadRecord(const std::span<const unsigned char> data, std::size_t& recordOffset)
{
    const std::size_t recordStart = recordOffset;
    const auto prefix = Read<ResourcePrefix>(data, recordStart);
    if (!prefix || prefix->headerSize < sizeof(ResourcePrefix) + sizeof(ResourceMetadata) ||
        !Contains(data, recordStart, prefix->headerSize))
    {
        return std::nullopt;
    }

    const std::size_t headerEnd = recordStart + prefix->headerSize;
    const std::size_t identifiersEnd = headerEnd - sizeof(ResourceMetadata);
    std::size_t cursor = recordStart + sizeof(ResourcePrefix);
    const auto type = ReadIdentifier(data, cursor, identifiersEnd);
    const auto name = ReadIdentifier(data, cursor, identifiersEnd);
    if (!type || !name || !Contains(data, headerEnd, prefix->dataSize))
    {
        return std::nullopt;
    }

    const std::size_t dataEnd = headerEnd + prefix->dataSize;
    recordOffset = AlignToDword(dataEnd);
    return ResourceRecord {*type, *name, data.subspan(headerEnd, prefix->dataSize)};
}

std::optional<std::u16string> FindString(const UINT id)
{
    const std::uint16_t blockId = static_cast<std::uint16_t>(id / StringsPerBlock + 1);
    const unsigned indexInBlock = id % StringsPerBlock;

    for (const std::span<const unsigned char> resourceData : win32_compat::RegisteredResourceData())
    {
        std::size_t recordOffset = 0;
        while (recordOffset < resourceData.size())
        {
            const auto record = ReadRecord(resourceData, recordOffset);
            if (!record)
            {
                break;
            }
            if (!record->type.isOrdinal || record->type.ordinal != StringResourceType || !record->name.isOrdinal ||
                record->name.ordinal != blockId)
            {
                continue;
            }

            std::size_t cursor = 0;
            for (unsigned stringIndex = 0; stringIndex < StringsPerBlock; ++stringIndex)
            {
                const auto length = Read<std::uint16_t>(record->contents, cursor);
                if (!length)
                {
                    return std::nullopt;
                }
                cursor += sizeof(*length);
                const std::size_t bytes = static_cast<std::size_t>(*length) * sizeof(char16_t);
                if (!Contains(record->contents, cursor, bytes))
                {
                    return std::nullopt;
                }
                if (stringIndex == indexInBlock)
                {
                    std::u16string result(*length, u'\0');
                    for (std::size_t index = 0; index < result.size(); ++index)
                    {
                        result[index] = static_cast<char16_t>(*Read<std::uint16_t>(record->contents, cursor + index * sizeof(char16_t)));
                    }
                    return result;
                }
                cursor += bytes;
            }
            return std::nullopt;
        }
    }
    return std::nullopt;
}
} // namespace

void win32_compat::RegisterResourceData(const std::span<const unsigned char> data)
{
    std::scoped_lock lock(ResourceDataMutex());
    ResourceDataSets().push_back(data);
}

std::vector<std::span<const unsigned char>> win32_compat::RegisteredResourceData()
{
    std::scoped_lock lock(ResourceDataMutex());
    return ResourceDataSets();
}

extern "C" int WINAPI LoadStringW(HINSTANCE, const UINT id, LPWSTR const buffer, const int bufferLength)
{
    if (buffer == nullptr || bufferLength <= 0)
    {
        return 0;
    }
    const auto value = FindString(id);
    if (!value)
    {
        return 0;
    }
    const std::size_t copied = std::min(value->size(), static_cast<std::size_t>(bufferLength - 1));
    auto* const output = reinterpret_cast<char16_t*>(buffer);
    std::copy_n(value->data(), copied, output);
    output[copied] = u'\0';
    return static_cast<int>(copied);
}

extern "C" int WINAPI LoadStringA(HINSTANCE, const UINT id, LPSTR const buffer, const int bufferLength)
{
    if (buffer == nullptr || bufferLength <= 0)
    {
        return 0;
    }
    const auto value = FindString(id);
    if (!value)
    {
        return 0;
    }
    const std::string text = una::utf16to8<char16_t, char>(*value);
    const std::size_t copied = std::min(text.size(), static_cast<std::size_t>(bufferLength - 1));
    std::copy_n(text.data(), copied, buffer);
    buffer[copied] = '\0';
    return static_cast<int>(copied);
}
