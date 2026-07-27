#include "StdAfx.h"

#include <IndexerBase.h>

namespace Microsoft::Resources::Indexers
{
CContentChecksumData* CContentChecksumData::New(
    const bool enabled,
    const ContentChecksumOperation operation,
    const NeutralLanguageCandidateCreation neutralLanguageCandidateCreation,
    std::vector<std::wstring>* const qualifierValues,
    const std::uint32_t contentChecksumValue,
    const bool checksumValueProvided,
    IDefStatusEx* const status)
{
    static_cast<void>(operation);
    if (status == nullptr)
    {
        return nullptr;
    }
    if (contentChecksumValue == 0)
    {
        status->SetError(E_DEF_INVALID_ARG, L"" __FILE__, 23, L"contentChecksumValue", 0);
        return nullptr;
    }

    auto* const result = new (std::nothrow) CContentChecksumData {
        enabled,
        ContentChecksumOperation::MainPackage,
        neutralLanguageCandidateCreation,
        qualifierValues,
        contentChecksumValue,
        0,
        checksumValueProvided};
    if (status->Failed())
    {
        delete result;
        return nullptr;
    }
    if (result == nullptr)
    {
        status->SetError(E_DEF_OUT_OF_MEMORY, L"" __FILE__, 36, L"", 0);
        return nullptr;
    }
    return result;
}
} // namespace Microsoft::Resources::Indexers
