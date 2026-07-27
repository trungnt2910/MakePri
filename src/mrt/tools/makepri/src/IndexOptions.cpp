#include "StdAfx.h"

#include <ParameterManager.h>

namespace Microsoft::Resources
{
namespace
{
constexpr std::uint64_t ProcessHiddenFiles = 0x0001;
constexpr std::uint64_t ProcessLinkedFiles = 0x0002;
constexpr std::uint64_t BuildAutoMergePri = 0x0004;
constexpr std::uint64_t DisableDeploymentMergeablePri = 0x0008;
constexpr std::uint64_t DisableDefaultQualifierValidation = 0x0010;
constexpr std::uint64_t SuppressConsoleOutput = 0x0020;
constexpr std::uint64_t SuppressEmbeddedData = 0x0040;
constexpr std::uint64_t GenerateReverseMap = 0x0080;
constexpr std::uint64_t DisableOptimizedEncoding = 0x0100;
constexpr std::uint64_t IncludeSchemaInResourcePacks = 0x0200;
constexpr std::uint64_t DisableDeduplication = 0x0400;
constexpr std::uint64_t GranularResourceSplitting = 0x0800;
constexpr std::uint64_t SplitLanguageVariants = 0x1000;
constexpr std::uint64_t CreateContentChecksum = 0x2000;
} // namespace

const OptionsBase::OptionSpec IndexOptions::s_options[4] {
    {L"hiddenfiles", L"hf", ProcessHiddenFiles, ProcessHiddenFiles, 0},
    {L"linkedfiles", L"lf", ProcessLinkedFiles, ProcessLinkedFiles, 0},
    {
        L"disableDeduplication",
        L"dd",
        DisableDeduplication,
        DisableDeduplication,
        DisableDeduplication,
    },
    {
        L"contentChecksumCreation",
        L"cc",
        CreateContentChecksum,
        CreateContentChecksum,
        0,
    },
};

const OptionsBase::StringOptionSpec IndexOptions::s_stringOptions[1] {
    {L"indexerschema", L"is"},
};

IndexOptions::IndexOptions() : TOptionsBase(0, 4, s_options, 1, s_stringOptions) {}

IndexOptions::~IndexOptions()
{
    if (m_indexerSchemaCollection != nullptr)
    {
        m_indexerSchemaCollection->~PathSchemaCollection();
        HeapFree(GetProcessHeap(), 0, m_indexerSchemaCollection);
    }
}

bool IndexOptions::HasFlag(const std::uint64_t flag) const { return (m_flags & flag) != 0; }

bool IndexOptions::GetShouldProcessHiddenFiles() const { return HasFlag(ProcessHiddenFiles); }

bool IndexOptions::GetShouldProcessLinkedFiles() const { return HasFlag(ProcessLinkedFiles); }

bool IndexOptions::GetShouldBuildAutoMergePri() const { return HasFlag(BuildAutoMergePri); }

bool IndexOptions::GetShouldBuildDeploymentMergeablePri() const { return !HasFlag(DisableDeploymentMergeablePri); }

bool IndexOptions::GetShouldValidateDefaultQualifiers() const { return !HasFlag(DisableDefaultQualifierValidation); }

bool IndexOptions::GetShouldSuppressConsoleOutput() const { return HasFlag(SuppressConsoleOutput); }

bool IndexOptions::GetShouldSuppressEmbeddedData() const { return HasFlag(SuppressEmbeddedData); }

bool IndexOptions::GetShouldGenerateReverseMap() const { return HasFlag(GenerateReverseMap); }

bool IndexOptions::GetShouldUseOptimizedEncoding() const { return !HasFlag(DisableOptimizedEncoding); }

bool IndexOptions::GetShouldOmitSchemaFromResourcePacks() const { return !HasFlag(IncludeSchemaInResourcePacks); }

bool IndexOptions::GetShouldDisableDeduplication() const { return HasFlag(DisableDeduplication); }

bool IndexOptions::GetShouldUseGranularResourceSplitting() const { return HasFlag(GranularResourceSplitting); }

bool IndexOptions::GetShouldSplitLanguageVariants() const { return HasFlag(SplitLanguageVariants); }

bool IndexOptions::GetShouldCreateContentChecksum() const { return HasFlag(CreateContentChecksum); }

const wchar_t* IndexOptions::GetIndexerSchemaPath() const { return m_indexerSchemaPath.GetRef(); }

const ISchemaCollection* IndexOptions::GetIndexerSchemaCollection(IDefStatusEx* const status) const
{
    static_cast<void>(status);
    return m_indexerSchemaCollection;
}

bool IndexOptions::VerifyStringOptions(IDefStatusEx* const status)
{
    if (m_stringValues != nullptr && m_stringValues[0] != nullptr)
    {
        SetIndexerSchemaPath(m_stringValues[0], status);
    }
    return status->Succeeded();
}

bool IndexOptions::SetIndexerSchemaPath(const wchar_t* const path, IDefStatusEx* const status)
{
    const HRESULT copyResult = DefStringResult_SetCopy(m_indexerSchemaPath.GetStringResult(), path);
    if (FAILED(copyResult))
    {
        status->SetError(copyResult, L"" __FILE__, 62, L"", 0);
        return status->Succeeded();
    }

    if (path == nullptr)
    {
        if (m_indexerSchemaCollection != nullptr)
        {
            m_indexerSchemaCollection->~PathSchemaCollection();
            HeapFree(GetProcessHeap(), 0, m_indexerSchemaCollection);
        }
        m_indexerSchemaCollection = nullptr;
        return status->Succeeded();
    }

    void* const memory = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Indexers::PathSchemaCollection));
    if (memory == nullptr)
    {
        status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 80, L"", 0);
        return status->Succeeded();
    }

    auto* const collection = new (memory) Indexers::PathSchemaCollection();
    if (collection->Initialize(path, status))
    {
        m_indexerSchemaCollection = collection;
        return status->Succeeded();
    }

    if (status->GetHResult() != E_DEF_INVALID_ARG)
    {
        return status->Succeeded();
    }
    status->SetError(E_DEF_INDEXER_SCHEMA_NOT_FOUND, L"" __FILE__, 74, path, 0);
    return status->Succeeded();
}
} // namespace Microsoft::Resources
