#pragma once

#include <mrm/BaseInternal.h>
#include <QualifierApplicator.h>
#include <IFsIndexer.h>

#include <string>

namespace Microsoft::Resources::Indexers
{

class CPriInfoIndexer final : public IFormatSpecificIndexer
{
public:
    ~CPriInfoIndexer() override;

    HRESULT Init(
        const UnifiedEnvironment* pEnvironment,
        const wchar_t* pProjectRootFolder,
        IXMLDOMNode* pIndexPassNode,
        CQualifierApplicator* pQualApplicator,
        const IIndexOptions* options,
        IDefStatusEx* pStatus) override;
    HRESULT Process(CItemInstanceEntry* pEntry, CItemInstanceSink* pTraversalSink, IDefStatusEx* pStatus, bool* pbRemoveContainerFromIndex)
        override;

private:
    HRESULT ParseIndexPassNode(IXMLDOMNode* pIndexPassNode, IDefStatusEx* pStatus);
    HRESULT ParseIndexConfigNode(IXMLDOMNode* pIndexConfigNode, IDefStatusEx* pStatus);
    HRESULT Redirect(
        const wchar_t* accessiblePath,
        std::wstring& contents,
        CItemInstanceEntry* entry,
        CItemInstanceSink* sink,
        IDefStatusEx* status);
    HRESULT ParsePriInfo(std::wstring& contents, CItemInstanceEntry* parentEntry, CItemInstanceSink* sink, IDefStatusEx* status);
    HRESULT ParseResourceMapNode(IXMLDOMNode* node, CItemInstanceEntry* parentEntry, CItemInstanceSink* sink, IDefStatusEx* status);
    HRESULT ParseScopeNode(
        const wchar_t* pszCollectionName,
        const wchar_t* pszPriorScopeString,
        IXMLDOMNode* pXmlNode,
        CItemInstanceEntry* pEntry,
        CItemInstanceSink* pTraversalSink,
        IDefStatusEx* pStatus);
    HRESULT ParseItemNode(
        const wchar_t* pszCollectionName,
        const wchar_t* pszScopeString,
        IXMLDOMNode* pXmlNode,
        CItemInstanceEntry* pEntry,
        CItemInstanceSink* pTraversalSink,
        IDefStatusEx* pStatus);
    HRESULT ParseLinkNode(
        const wchar_t* collectionName,
        const wchar_t* itemName,
        IXMLDOMNode* linkXmlNode,
        CItemInstanceSink* traversalSink,
        IDefStatusEx* status);
    HRESULT ParseCandidateNode(
        const wchar_t* pszCollectionName,
        const wchar_t* pszItemName,
        IXMLDOMNode* pXmlNode,
        CItemInstanceEntry* pEntry,
        CItemInstanceSink* pTraversalSink,
        IDefStatusEx* pStatus);
    HRESULT ParseQualifierSetNode(
        IXMLDOMNode* pXmlNode,
        bool fIsFile,
        CQualifierApplicator::CQualifierSetBuilder* pQualifierSetBuilder,
        IDefStatusEx* pStatus);
    HRESULT ParseQualifierNode(
        IXMLDOMNode* pXmlNode,
        bool fIsFile,
        CQualifierApplicator::CQualifierSetBuilder* pQualifierSetBuilder,
        IDefStatusEx* pStatus);

    const UnifiedEnvironment* _pEnvironment {};
    bool _fEmitStringResources {true};
    bool _fEmitPathResources {true};
    bool _fEmitEmbeddedDataResources {true};
    const wchar_t* _pProjectRoot {};
    std::wstring _strQualifierDelimiter;
    StringResult _unused;
    CQualifierApplicator* _pQualApplicator {};
};

} // namespace Microsoft::Resources::Indexers
