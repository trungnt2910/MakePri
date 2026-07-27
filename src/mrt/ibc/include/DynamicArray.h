#pragma once

#include <cstdint>

#include <mrm/common/BaseInternal.h>
#include <mrm/DefObject.h>

#include <windows.h>

#include <wil/result.h>

#include <cstring>
#include <new>

namespace Microsoft::Resources::Indexers
{

class CHIndexerBase;

template<typename T>
class DynamicArray : public DefObject
{
public:
    ~DynamicArray() { HeapFree(GetProcessHeap(), 0, m_data); }

    static HRESULT CreateInstance(const std::uint32_t initialCapacity, DynamicArray** const result)
    {
        *result = nullptr;

        void* const memory = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DynamicArray));
        auto* const array = memory != nullptr ? new (memory) DynamicArray() : nullptr;
        if (array == nullptr)
        {
            RETURN_HR(E_OUTOFMEMORY);
        }

        const std::size_t allocationSize = _DefArray_Size(sizeof(T), initialCapacity);
        if (allocationSize != 0)
        {
            array->m_data = static_cast<T*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, allocationSize));
            if (array->m_data != nullptr)
            {
                array->m_capacity = initialCapacity;
                *result = array;
                return S_OK;
            }
        }
        else
        {
            array->m_data = nullptr;
        }

        delete array;
        RETURN_HR(E_OUTOFMEMORY);
    }

    HRESULT Add(const T value, int* const index)
    {
        const std::uint32_t count = m_count;
        std::uint32_t capacity = m_capacity;
        if (count >= capacity)
        {
            const std::uint32_t requiredCapacity = count + 1;
            if (requiredCapacity > capacity)
            {
                if (capacity == 0)
                {
                    capacity = requiredCapacity;
                }
                while (capacity < requiredCapacity)
                {
                    capacity += capacity;
                }

                if (!_DefArray_TryEnsureSizeByElemSize(m_data, sizeof(T), count, capacity, reinterpret_cast<void**>(&m_data)))
                {
                    RETURN_HR(E_OUTOFMEMORY);
                }
                m_capacity = capacity;
            }
        }

        m_data[m_count] = value;
        if (index != nullptr)
        {
            *index = static_cast<int>(m_count);
        }
        ++m_count;
        return S_OK;
    }

    HRESULT Get(const std::uint32_t index, T* const result) const
    {
        *result = T {};
        if (index >= m_count)
        {
            return E_INVALIDARG;
        }

        *result = m_data[index];
        return S_OK;
    }

    bool TryGet(const std::uint32_t index, T* const result) const
    {
        *result = T {};
        if (index >= m_count)
        {
            return false;
        }

        *result = m_data[index];
        return true;
    }

    HRESULT Insert(const T value, const std::uint32_t index)
    {
        if (index > m_count)
        {
            return E_INVALIDARG;
        }

        std::uint32_t capacity = m_capacity;
        if (m_count >= capacity)
        {
            const std::uint32_t requiredCapacity = m_count + 1;
            if (requiredCapacity > capacity)
            {
                if (capacity == 0)
                {
                    capacity = requiredCapacity;
                }
                while (capacity < requiredCapacity)
                {
                    capacity += capacity;
                }

                if (!_DefArray_TryEnsureSizeByElemSize(m_data, sizeof(T), m_count, capacity, reinterpret_cast<void**>(&m_data)))
                {
                    RETURN_HR(E_OUTOFMEMORY);
                }
                m_capacity = capacity;
            }
        }

        if (index < m_count)
        {
            std::memmove(&m_data[index + 1], &m_data[index], sizeof(T) * (m_count - index));
        }
        m_data[index] = value;
        ++m_count;
        return S_OK;
    }

    HRESULT SetExtent(const std::uint32_t extent)
    {
        if (extent < m_count)
        {
            return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_PRI_FILE);
        }
        if (extent > m_capacity)
        {
            RETURN_IF_FAILED(Extend(extent, true));
        }

        m_count = extent;
        return S_OK;
    }

    HRESULT ExtendAndSet(const std::uint32_t index, const T value, T* const previousValue)
    {
        if (index >= m_count)
        {
            const std::uint32_t extent = index + 1;
            if (extent < m_count)
            {
                return HRESULT_FROM_WIN32(ERROR_MRM_INVALID_PRI_FILE);
            }
            if (extent > m_capacity)
            {
                if (!_DefArray_TryEnsureSizeByElemSize(m_data, sizeof(T), m_count, extent, reinterpret_cast<void**>(&m_data)))
                {
                    RETURN_HR(E_OUTOFMEMORY);
                }
                m_capacity = extent;
            }
            m_count = extent;
        }

        if (previousValue != nullptr)
        {
            *previousValue = m_data[index];
        }
        m_data[index] = value;
        return S_OK;
    }

    [[nodiscard]] std::uint32_t GetCount() const { return m_count; }

    [[nodiscard]] T* GetData() const { return m_data; }

    HRESULT Extend(const std::uint32_t capacity, const bool)
    {
        if (capacity > m_capacity)
        {
            if (!_DefArray_TryEnsureSizeByElemSize(m_data, sizeof(T), m_count, capacity, reinterpret_cast<void**>(&m_data)))
            {
                RETURN_HR(E_OUTOFMEMORY);
            }
            m_capacity = capacity;
        }
        return S_OK;
    }

private:
    DynamicArray() = default;

public:
    T* m_data {};
    std::uint32_t m_capacity {};
    std::uint32_t m_count {};
};

} // namespace Microsoft::Resources::Indexers
