#pragma once

#include <mrm/BaseInternal.h>
#include <mrm/Results.h>
#include <IFsIndexer.h>

#include <string>

namespace Microsoft::Resources::Indexers
{

class CResFilesIndexer : public IFormatSpecificIndexer
{
public:
    CResFilesIndexer() { DefStringResult_SetCopy(_strQualifierDelimiter.GetStringResult(), L"@"); }

    ~CResFilesIndexer() override;

    HRESULT Init(
        const UnifiedEnvironment* pEnvironment,
        const wchar_t* pProjectRoot,
        IXMLDOMNode* pIndexPassNode,
        CQualifierApplicator* pApplicator,
        const IIndexOptions* options,
        IDefStatusEx* pStatus) override;
    HRESULT Process(CItemInstanceEntry* pEntry, CItemInstanceSink* pTraversalSink, IDefStatusEx* pStatus, bool* pbRemoveContainerFromIndex)
        override;

private:
    static const wchar_t* s_pResFilesSchema;

    HRESULT _ParseIndexPassNode(IXMLDOMNode* pIndexPassNode, IDefStatusEx* pStatus);
    void _TrimSpaces(std::wstring& wszString);
    HRESULT _ParseResFile(
        const wchar_t* pResFileName,
        std::wstring& contents,
        CItemInstanceEntry* pEntry,
        CItemInstanceSink* pTraversalSink,
        IDefStatusEx* pStatus);
    HRESULT Redirect(
        const wchar_t* accessiblePath,
        std::wstring& contents,
        CItemInstanceEntry* entry,
        CItemInstanceSink* sink,
        IDefStatusEx* status);

    const UnifiedEnvironment* _pEnvironment {};
    const wchar_t* _pProjectRoot {};
    StringResult _strQualifierDelimiter;
    CQualifierApplicator* _pQualApplicator {};
};

} // namespace Microsoft::Resources::Indexers
