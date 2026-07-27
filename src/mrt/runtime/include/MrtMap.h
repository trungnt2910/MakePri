#pragma once

#include <cstdint>

#include <windows.h>

#include <cstddef>
#include <cstring>
#include <limits>
#include <new>

namespace Microsoft::Resources::Runtime
{
template<typename T>
struct Equal
{
    bool operator()(const T& left, const T& right) const { return left == right; }
};

template<>
struct Equal<const wchar_t*>
{
    bool operator()(const wchar_t* const left, const wchar_t* const right) const
    {
        return CompareStringOrdinal(left, -1, right, -1, TRUE) == CSTR_EQUAL;
    }
};

template<typename Key, typename Value, typename KeyEqual = Equal<Key>>
class MrtMap
{
public:
    struct Entry
    {
        Key key;
        Value value;
    };

    MrtMap() { _Initialize(0); }

    ~MrtMap()
    {
        if (m_entries != nullptr)
        {
            operator delete(m_entries);
            m_entries = nullptr;
        }
    }

    MrtMap(const MrtMap&) = delete;
    MrtMap& operator=(const MrtMap&) = delete;

    HRESULT FindKeyIndex(const Key& key, std::uint32_t* const index)
    {
        std::uint32_t current = 0;
        bool found = m_count != 0;
        if (m_count != 0)
        {
            while (!m_equal(m_entries[current].key, key))
            {
                ++current;
                if (current >= m_count)
                {
                    break;
                }
            }
            if (current < m_count)
            {
                *index = current;
            }
            found = current < m_count;
        }
        return found ? S_OK : HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }

    HRESULT Insert(const Key& key, const Value& value)
    {
        std::uint32_t index;
        if (SUCCEEDED(FindKeyIndex(key, &index)))
        {
            return HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
        }

        HRESULT result = S_OK;
        AcquireSRWLockExclusive(&m_lock);
        if (m_count != m_capacity || SUCCEEDED(result = _Expand()))
        {
            m_entries[m_count].key = key;
            m_entries[m_count].value = value;
            ++m_count;
        }
        ReleaseSRWLockExclusive(&m_lock);
        return result;
    }

    HRESULT Clear()
    {
        AcquireSRWLockShared(&m_lock);
        if (m_entries != nullptr)
        {
            operator delete(m_entries);
            m_entries = nullptr;
        }
        m_count = 0;
        m_capacity = 0;
        ReleaseSRWLockShared(&m_lock);
        return _Initialize(0);
    }

    [[nodiscard]] std::uint32_t GetCount() const { return m_count; }
    [[nodiscard]] const Entry& GetEntry(const std::uint32_t index) const { return m_entries[index]; }

private:
    HRESULT _Initialize(std::uint32_t ignoredCapacity)
    {
        static_cast<void>(ignoredCapacity);
        InitializeSRWLock(&m_lock);
        m_capacity = 5;
        m_entries = static_cast<Entry*>(operator new[](static_cast<std::size_t>(5) * sizeof(Entry), std::nothrow));
        HRESULT result = S_OK;
        if (m_entries == nullptr)
        {
            m_capacity = 0;
            result = E_OUTOFMEMORY;
        }
        m_count = 0;
        return result;
    }

    HRESULT _Expand()
    {
        std::size_t allocationSize;
        if (m_capacity > std::numeric_limits<std::size_t>::max() / (2 * sizeof(Entry)))
        {
            allocationSize = std::numeric_limits<std::size_t>::max();
        }
        else
        {
            allocationSize = 2 * static_cast<std::size_t>(m_capacity) * sizeof(Entry);
        }

        auto* const entries = static_cast<Entry*>(operator new[](allocationSize, std::nothrow));
        if (entries == nullptr)
        {
            return E_OUTOFMEMORY;
        }

        std::memcpy(entries, m_entries, m_count * sizeof(Entry));
        operator delete(m_entries);
        m_entries = entries;
        m_capacity *= 2;
        return S_OK;
    }

    Entry* m_entries {};
    SRWLOCK m_lock {};
    std::uint32_t m_capacity {};
    std::uint32_t m_count {};
    [[no_unique_address]] KeyEqual m_equal {};
};
} // namespace Microsoft::Resources::Runtime
