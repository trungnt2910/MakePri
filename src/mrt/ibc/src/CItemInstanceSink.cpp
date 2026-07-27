#include "StdAfx.h"

#include <ItemInstanceSink.h>

namespace Microsoft::Resources::Indexers
{
CItemInstanceEntry::CItemInstanceEntry(
    const MrmEnvironment::ResourceItemType resourceItemType,
    const MrmEnvironment::ResourceValueType resourceValueType,
    const int qualifierSetIndex,
    const ULONG flags) :
    projectRoot(nullptr),
    resourceItemType(resourceItemType),
    resourceValueType(resourceValueType),
    qualifierSetIndex(qualifierSetIndex),
    flags(flags)
{}

CItemInstanceEntry::~CItemInstanceEntry() = default;

CItemInstanceEntry* CItemInstanceEntry::NewForEmbeddedData(
    const wchar_t* const source,
    const wchar_t* const itemName,
    const MrmEnvironment::ResourceItemType resourceItemType,
    const MrmEnvironment::ResourceValueType resourceValueType,
    BlobResult* const value,
    const int qualifierSetIndex,
    const ULONG flags,
    const wchar_t* const valueTypeName,
    std::map<std::wstring, std::wstring>* const qualifiers,
    IDefStatusEx* const status)
{
    if (itemName == nullptr)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 27, L"itemName", 0);
        }
        return nullptr;
    }
    if (value == nullptr)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 28, L"value", 0);
        }
        return nullptr;
    }
    if (resourceValueType != MrmEnvironment::ResourceValueType_EmbeddedData)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 29, L"resourceValueType", 0);
        }
        return nullptr;
    }

    AutoDeletePtr<CItemInstanceEntry> cleanup(
        new (std::nothrow) CItemInstanceEntry(resourceItemType, MrmEnvironment::ResourceValueType_EmbeddedData, qualifierSetIndex, flags));
    CItemInstanceEntry* result = nullptr;
    HRESULT operationResult = S_OK;
    if (cleanup.Data() != nullptr)
    {
        operationResult = cleanup.Data()->_Init(source, itemName, valueTypeName, qualifiers);
        if (SUCCEEDED(operationResult))
        {
            operationResult = cleanup.Data()->value.SetRef(L"EmbeddedData");
            if (SUCCEEDED(operationResult))
            {
                operationResult = cleanup.Data()->_ReleaseAndSetBlobResult(value);
                if (SUCCEEDED(operationResult))
                {
                    result = cleanup.Detach();
                    return result;
                }
                if (status != nullptr)
                {
                    status->SetError(operationResult, L"" __FILE__, 60, L"", 0);
                }
                return result;
            }
            if (status != nullptr)
            {
                status->SetError(operationResult, L"" __FILE__, 53, L"", 0);
            }
            return result;
        }
        if (status != nullptr)
        {
            status->SetError(operationResult, L"" __FILE__, 45, L"", 0);
        }
        return result;
    }
    if (status != nullptr)
    {
        status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 38, L"", 0);
    }
    return result;
}

CItemInstanceEntry* CItemInstanceEntry::NewForString(
    const wchar_t* const source,
    const wchar_t* const itemName,
    const MrmEnvironment::ResourceItemType resourceItemType,
    const MrmEnvironment::ResourceValueType resourceValueType,
    const wchar_t* const value,
    const int qualifierSetIndex,
    const ULONG flags,
    const wchar_t* const valueTypeName,
    std::map<std::wstring, std::wstring>* const qualifiers,
    IDefStatusEx* const status)
{
    if (itemName == nullptr)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 82, L"itemName", 0);
        }
        return nullptr;
    }
    if (value == nullptr)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 83, L"value", 0);
        }
        return nullptr;
    }
    if (resourceValueType == MrmEnvironment::ResourceValueType_EmbeddedData)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 84, L"resourceValueType", 0);
        }
        return nullptr;
    }

    AutoDeletePtr<CItemInstanceEntry> cleanup(new (std::nothrow)
                                                  CItemInstanceEntry(resourceItemType, resourceValueType, qualifierSetIndex, flags));
    CItemInstanceEntry* result = nullptr;
    HRESULT operationResult = S_OK;
    if (cleanup.Data() != nullptr)
    {
        operationResult = cleanup.Data()->_Init(source, itemName, valueTypeName, qualifiers);
        if (SUCCEEDED(operationResult))
        {
            operationResult = cleanup.Data()->_CopyString(value, &cleanup.Data()->value);
            if (SUCCEEDED(operationResult))
            {
                return cleanup.Detach();
            }
            if (status != nullptr)
            {
                status->SetError(operationResult, L"" __FILE__, 106, L"", 0);
            }
            return result;
        }
        if (status != nullptr)
        {
            status->SetError(operationResult, L"" __FILE__, 99, L"", 0);
        }
        return result;
    }
    if (status != nullptr)
    {
        status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 92, L"", 0);
    }
    return result;
}

CItemInstanceEntry* CItemInstanceEntry::NewForLink(
    const wchar_t* const source,
    const wchar_t* const itemName,
    const wchar_t* const link,
    const ULONG flags,
    const wchar_t* const valueTypeName,
    std::map<std::wstring, std::wstring>* const qualifiers,
    IDefStatusEx* const status)
{
    if (itemName == nullptr)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 125, L"itemName", 0);
        }
        return nullptr;
    }
    if (link == nullptr)
    {
        if (status != nullptr)
        {
            status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 126, L"link", 0);
        }
        return nullptr;
    }

    AutoDeletePtr<CItemInstanceEntry> cleanup(new (std::nothrow) CItemInstanceEntry(
        MrmEnvironment::ResourceItemType_String, MrmEnvironment::ResourceValueType_Utf16String, 0, flags));
    CItemInstanceEntry* result = nullptr;
    HRESULT operationResult = S_OK;
    if (cleanup.Data() != nullptr)
    {
        operationResult = cleanup.Data()->_Init(source, itemName, valueTypeName, qualifiers);
        if (SUCCEEDED(operationResult))
        {
            operationResult = cleanup.Data()->_CopyString(link, &cleanup.Data()->link);
            if (SUCCEEDED(operationResult))
            {
                return cleanup.Detach();
            }
            if (status != nullptr)
            {
                status->SetError(operationResult, L"" __FILE__, 151, L"", 0);
            }
            return result;
        }
        if (status != nullptr)
        {
            status->SetError(operationResult, L"" __FILE__, 144, L"", 0);
        }
        return result;
    }
    if (status != nullptr)
    {
        status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 137, L"", 0);
    }
    return result;
}

HRESULT CItemInstanceEntry::_Init(
    const wchar_t* const source,
    const wchar_t* const itemName,
    const wchar_t* const valueTypeName,
    std::map<std::wstring, std::wstring>* const qualifiers)
{
    HRESULT result = S_OK;
    if (source != nullptr)
    {
        result = _CopyString(source, &this->source);
        if (FAILED(result))
        {
            return result;
        }
    }
    result = _CopyString(itemName, &this->itemName);
    if (FAILED(result))
    {
        return result;
    }
    if (valueTypeName != nullptr)
    {
        result = _CopyString(valueTypeName, &this->valueTypeName);
        if (FAILED(result))
        {
            return result;
        }
    }
    if ((qualifiers != nullptr) && (&this->qualifiers != qualifiers))
    {
        this->qualifiers = *qualifiers;
    }
    return result;
}

HRESULT CItemInstanceEntry::_CopyString(const wchar_t* const source, StringResult* const result)
{
    StringResult stringResult;
    // Original line: 200
    RETURN_IF_FAILED(stringResult.Init(source));
    // Original line: 202
    RETURN_IF_FAILED(stringResult.GetCopy(result));
    return S_OK;
}

HRESULT CItemInstanceEntry::_ReleaseAndSetBlobResult(BlobResult* const value)
{
    void* data;
    std::size_t size;
    HRESULT result = value->ReleaseContents(&data, &size);
    if (SUCCEEDED(result))
    {
        result = blob.SetContents(data, size);
    }
    return result;
}

CItemInstanceSink::CItemInstanceSink(const bool sorted) : _bSorted(sorted) {}

CItemInstanceSink::~CItemInstanceSink()
{
    while (!_IIESink.empty())
    {
        delete _IIESink.back();
        _IIESink.pop_back();
    }
}

HRESULT CItemInstanceSink::AddEntry(CItemInstanceEntry* const entry)
{
    if (_bSorted)
    {
        const auto iterator = std::lower_bound(
            _IIESink.begin(), _IIESink.end(), entry, [](CItemInstanceEntry* const first, CItemInstanceEntry* const second) {
                std::wstring firstName(first->source.GetRef());
                firstName.append(L"\\");
                firstName.append(first->itemName.GetRef());
                std::wstring secondName(second->source.GetRef());
                secondName.append(L"\\");
                secondName.append(second->itemName.GetRef());
                return CompareStringOrdinal(
                           firstName.c_str(),
                           static_cast<int>(firstName.length()),
                           secondName.c_str(),
                           static_cast<int>(secondName.length()),
                           TRUE) == CSTR_LESS_THAN;
            });
        _IIESink.insert(iterator, entry);
    }
    else
    {
        _IIESink.push_back(entry);
    }
    return S_OK;
}

CItemInstanceEntry* CItemInstanceSink::PopEntry()
{
    CItemInstanceEntry* result = nullptr;
    if (!_IIESink.empty())
    {
        result = _IIESink.back();
        if (!_IIESink.empty())
        {
            _IIESink.pop_back();
        }
    }
    return result;
}

CItemInstanceEntry* CItemInstanceSink::GetEntry(const std::uint32_t index) { return _IIESink.at(index); }

std::uint32_t CItemInstanceSink::GetNumberOfEntries() const { return static_cast<std::uint32_t>(_IIESink.size()); }

bool CItemInstanceSink::empty() const { return _IIESink.empty(); }
} // namespace Microsoft::Resources::Indexers
