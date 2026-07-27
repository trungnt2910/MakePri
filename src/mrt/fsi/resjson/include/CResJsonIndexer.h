#pragma once

#include <cstdint>

#include <mrm/BaseInternal.h>
#include <mrm/Results.h>
#include <IFsIndexer.h>

#include <map>
#include <string>

class CJsonParser;
class CJsonValue;

namespace Microsoft::Resources::Indexers
{

class CResJsonIndexer : public IFormatSpecificIndexer
{
public:
    CResJsonIndexer() { DefStringResult_SetCopy(_strInitialPath.GetStringResult(), L""); }

    ~CResJsonIndexer() override;

    HRESULT Init(
        const UnifiedEnvironment* pEnvironment,
        const wchar_t* pProjectRoot,
        IXMLDOMNode* pIndexPassNode,
        CQualifierApplicator* pApplicator,
        const IIndexOptions* options,
        IDefStatusEx* pStatus) override;
    HRESULT Process(CItemInstanceEntry* pEntry, CItemInstanceSink* pSink, IDefStatusEx* pStatus, bool* pbRemoveContainerFromIndex) override;

private:
    static const wchar_t* s_pResJsonSchema;

    struct ITEM_INFO
    {
        ~ITEM_INFO();

        std::wstring pItemName;
        std::wstring pValue;
        std::map<std::wstring, std::wstring> metadataMap;
        ITEM_INFO* pAdjacent {};
        ITEM_INFO* pNext {};
        bool bIsValid {true};
    };

    ITEM_INFO* _CreateNewItemInfo();
    bool _IsProperty(std::wstring wszInputString, std::wstring* pwszParent, std::wstring* pwszProperty);
    void _AddMetaData(const wchar_t* pszPropertyType, const wchar_t* pszPropertyValue, ITEM_INFO* pNode, std::uint32_t nDepth);
    HRESULT _ParseValue(
        CJsonValue* pJsonValue,
        ITEM_INFO* pItem,
        std::uint32_t nDepth,
        const wchar_t* pResJsonFileName,
        IDefStatusEx* pStatus);
    HRESULT _ProcessItemInfoAndDelete(
        const wchar_t* pScopeName,
        const wchar_t* pResJsonFileName,
        const wchar_t* pItemName,
        ITEM_INFO* pItem,
        CItemInstanceEntry* pEntry,
        std::uint32_t nDepth,
        CItemInstanceSink* pTraversalSink);
    HRESULT _ParseResJson(
        const wchar_t* pScopeName,
        const wchar_t* pResJsonFileName,
        CItemInstanceEntry* pEntry,
        CItemInstanceSink* pTraversalSink,
        IDefStatusEx* pStatus);
    HRESULT Redirect(
        const wchar_t* resourceId,
        const wchar_t* accessiblePath,
        std::wstring& contents,
        CItemInstanceEntry* entry,
        CItemInstanceSink* sink,
        IDefStatusEx* status);
    HRESULT _ReportParserError(const wchar_t* pResJsonFileName, CJsonParser* pJsonParser, IDefStatusEx* pStatus);
    bool _SetDefError_WithLineColumn(HRESULT error, const wchar_t* source, int line, int column, IDefStatusEx* status);

    const UnifiedEnvironment* _pEnvironment {};
    const wchar_t* _pProjectRoot {};
    CQualifierApplicator* _pQualApplicator {};
    std::wstring _wszBuffer;
    StringResult _strInitialPath;
};

} // namespace Microsoft::Resources::Indexers
