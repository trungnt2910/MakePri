#pragma once

#include <cstdint>

#include <IFsIndexer.h>

namespace Microsoft::Resources
{

class AtomPoolGroup;
class MrmProfile;
class NamedResourceResult;
class ResourceMapSubtree;
class StandalonePriFile;

} // namespace Microsoft::Resources

namespace Microsoft::Resources::Indexers
{

class CPriFileIndexer final : public IFormatSpecificIndexer
{
public:
    ~CPriFileIndexer() override = default;

    HRESULT Init(
        const UnifiedEnvironment* pEnvironment,
        const wchar_t* pProjectRootFolder,
        IXMLDOMNode* pIndexPassNode,
        CQualifierApplicator* pApplicator,
        const IIndexOptions* options,
        IDefStatusEx* pStatus) override;
    HRESULT Process(CItemInstanceEntry* pEntry, CItemInstanceSink* pTraversalSink, IDefStatusEx* pStatus, bool* pbRemoveContainerFromIndex)
        override;

private:
    static const wchar_t* const s_pszPriSchema;

    HRESULT Redirect(const wchar_t* path, CItemInstanceEntry* entry, CItemInstanceSink* sink, IDefStatusEx* status);
    HRESULT _ReadPriFileContentsFromBlob(
        const wchar_t* valueTypeName,
        CItemInstanceEntry* entry,
        IDefStatusEx* status,
        CItemInstanceSink* sink);
    HRESULT _ReadPriFileObject(
        StandalonePriFile* file,
        MrmProfile* profile,
        const wchar_t* valueTypeName,
        CItemInstanceEntry* parentEntry,
        IDefStatusEx* status,
        CItemInstanceSink* sink);
    HRESULT _ReadRecursiveScopeTree(
        const ResourceMapSubtree* pResourceMapSubtree,
        AtomPoolGroup* pPoolGroup,
        CItemInstanceEntry* pEntry,
        std::uint32_t nDepth,
        const wchar_t* pszParentScope,
        const wchar_t* pszScopedItemName,
        const wchar_t* pszFileName,
        IDefStatusEx* pStatus,
        CItemInstanceSink* pTraversalSink);
    HRESULT _ReadNamedResult(
        NamedResourceResult* pNamedResourceResult,
        const wchar_t* pszParentScope,
        const wchar_t* pszItemName,
        AtomPoolGroup* pPoolGroup,
        CItemInstanceEntry* pEntry,
        const wchar_t* pszFileName,
        IDefStatusEx* pStatus,
        CItemInstanceSink* pTraversalSink);

    const UnifiedEnvironment* _pEnvironment {};
    const wchar_t* _pProjectRoot {};
    CQualifierApplicator* _pQualApplicator {};
    const IIndexOptions* _options {};
};

} // namespace Microsoft::Resources::Indexers
