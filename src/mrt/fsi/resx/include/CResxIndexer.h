#pragma once

#include <IFsIndexer.h>

#include <string>

namespace Microsoft::Resources::Indexers
{
class CResxXmlConfig;

class CResxIndexer : public IFormatSpecificIndexer
{
public:
    ~CResxIndexer() override;

    HRESULT Init(
        const UnifiedEnvironment* environment,
        const wchar_t* projectRoot,
        IXMLDOMNode* indexPassNode,
        CQualifierApplicator* qualifierApplicator,
        const IIndexOptions* options,
        IDefStatusEx* status) override;
    HRESULT Process(CItemInstanceEntry* entry, CItemInstanceSink* sink, IDefStatusEx* status, bool* removeContainer) override;

private:
    HRESULT CollectItemInstanceEntry(
        CItemInstanceEntry* entry,
        IXMLDOMNode* dataDomNode,
        const wchar_t* fileName,
        const wchar_t* scopeName,
        IDefStatusEx* status,
        CItemInstanceSink* sink);
    void ConvertDotsToSlashes(wchar_t* value);
    HRESULT Redirect(
        const wchar_t* resourceId,
        const wchar_t* accessiblePath,
        std::wstring& contents,
        CItemInstanceEntry* entry,
        CItemInstanceSink* sink,
        IDefStatusEx* status);
    HRESULT ProcessPayload(
        CItemInstanceEntry* parentEntry,
        const wchar_t* valueTypeName,
        const wchar_t* payload,
        const wchar_t* source,
        CItemInstanceSink* sink,
        IDefStatusEx* status);

    const UnifiedEnvironment* m_environment {};
    const wchar_t* m_projectRoot {};
    IXMLDOMNode* m_indexerConfig {};
    CQualifierApplicator* m_qualifierApplicator {};
    CResxXmlConfig* m_resxXmlConfigHelper {};
};
} // namespace Microsoft::Resources::Indexers
