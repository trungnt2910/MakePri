#pragma once

#include <IFsIndexer.h>

namespace Microsoft::Resources
{
class BlobResult;
}

namespace Microsoft::Resources::Indexers
{
class CEmbedFilesIndexer : public IFormatSpecificIndexer
{
public:
    ~CEmbedFilesIndexer() override = default;

    HRESULT Init(
        const UnifiedEnvironment* pEnvironment,
        const wchar_t* pProjectRoot,
        IXMLDOMNode* pIndexPassNode,
        CQualifierApplicator* pQualApplicator,
        const IIndexOptions* options,
        IDefStatusEx* pStatus) override;
    HRESULT Process(CItemInstanceEntry* pEntry, CItemInstanceSink* pTraversalSink, IDefStatusEx* pStatus, bool* pbRemoveContainerFromIndex)
        override;

    static HRESULT GetFileContents(const wchar_t* pEmbeddedFileName, IDefStatusEx* pStatus, BlobResult* pFileContents);

private:
    static const wchar_t* s_pEmbedFilesSchema;

    HRESULT _ParseIndexPassNode(IXMLDOMNode* pIndexPassNode, IDefStatusEx* pStatus);
    HRESULT _AddEmbeddedFileEntry(
        const wchar_t* pEmbeddedFileName,
        CItemInstanceEntry* pEntry,
        CItemInstanceSink* pTraversalSink,
        IDefStatusEx* pStatus);

    const UnifiedEnvironment* _pEnvironment {};
    const wchar_t* _pProjectRoot {};
};
} // namespace Microsoft::Resources::Indexers
