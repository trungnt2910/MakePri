#include "StdAfx.h"

#include <CResJsonIndexer.h>

namespace Microsoft::Resources::Indexers
{
// clang-format off
const wchar_t* CResJsonIndexer::s_pResJsonSchema =
    LR"xml(<xs:schema id="resjson" xmlns:xs="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified">)xml"
        LR"xml(<xs:simpleType name="IndexerConfigResJsonType">)xml"
            LR"xml(<xs:restriction base="xs:string">)xml"
                LR"xml(<xs:pattern value="((r|R)(e|E)(s|S)(j|J)(s|S)(o|O)(n|N))"/>)xml"
            LR"xml(</xs:restriction>)xml"
        LR"xml(</xs:simpleType>)xml"
        LR"xml(<xs:element name="indexer-config">)xml"
            LR"xml(<xs:complexType>)xml"
                LR"xml(<xs:attribute  name="type"  type="IndexerConfigResJsonType" use="required"/>)xml"
                LR"xml(<xs:attribute  name="initialPath"  type="xs:string" use="optional"/>)xml"
            LR"xml(</xs:complexType>)xml"
        LR"xml(</xs:element>)xml"
    LR"xml(</xs:schema>)xml";
// clang-format on

CResJsonIndexer::~CResJsonIndexer()
{
    _pEnvironment = nullptr;
    _pProjectRoot = nullptr;
    _pQualApplicator = nullptr;
}

HRESULT CResJsonIndexer::Init(
    const UnifiedEnvironment* const pEnvironment,
    const wchar_t* const pProjectRoot,
    IXMLDOMNode* const pIndexPassNode,
    CQualifierApplicator* const pApplicator,
    const IIndexOptions* const options,
    IDefStatusEx* const pStatus)
{
    static_cast<void>(options);
    _pEnvironment = pEnvironment;
    _pProjectRoot = pProjectRoot;
    _pQualApplicator = pApplicator;

    CXmlHelper xmlHelper(pIndexPassNode);
    HRESULT result =
        xmlHelper.ValidateChildNodeAgainstChildSchema(L"indexer-config", s_pResJsonSchema, L"type", L"resjson", false, pStatus);
    if (SUCCEEDED(result))
    {
        IXMLDOMNodeList* children = nullptr;
        LONG length = 0;
        bool found = false;
        xmlHelper.TryGetChildren(L"indexer-config", pStatus, &children);
        children->get_length(&length);
        for (LONG index = 0; index < length && !found && SUCCEEDED(result); ++index)
        {
            IXMLDOMNode* child = nullptr;
            result = children->get_item(index, &child);
            if (SUCCEEDED(result) && child != nullptr)
            {
                CXmlHelper childHelper(child);
                wchar_t* type = nullptr;
                childHelper.GetAttributeValue(L"type", pStatus, &type);
                if (DefString_CompareWithOptions(L"resjson", type, DefCompare_CaseInsensitive) == 0)
                {
                    wchar_t* initialPath = nullptr;
                    result = childHelper.GetAttributeValue(L"initialPath", pStatus, &initialPath);
                    if (pStatus->GetWhat() == E_DEF_XML_ATTRIB_NOT_FOUND)
                    {
                        pStatus->Reset();
                        result = S_OK;
                    }
                    else if (SUCCEEDED(result))
                    {
                        PathRemoveBackslashW(initialPath);
                        Def_HrFailed0(DefStringResult_SetCopy(_strInitialPath.GetStringResult(), initialPath), pStatus);
                    }
                    operator delete(initialPath);
                    found = true;
                }
                operator delete(type);
                SAFE_RELEASE(child);
            }
        }
        SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
    }
    return ComputeHResult(result, pStatus);
}

CResJsonIndexer::ITEM_INFO* CResJsonIndexer::_CreateNewItemInfo() { return new ITEM_INFO; }

bool CResJsonIndexer::_IsProperty(std::wstring wszInputString, std::wstring* const pwszParent, std::wstring* const pwszProperty)
{
    bool result = false;
    if (wszInputString.find_first_of(L"_", 0) == 0)
    {
        const std::size_t dot = wszInputString.rfind(L".", wszInputString.length() - 1, 1);
        if (dot == std::wstring::npos)
        {
            pwszParent->assign(wszInputString.substr(1, std::wstring::npos), 0, std::wstring::npos);
            result = true;
        }
        else
        {
            pwszParent->assign(wszInputString.substr(1, dot - 1), 0, std::wstring::npos);
            pwszProperty->assign(wszInputString.substr(dot + 1, wszInputString.length() - dot - 1), 0, std::wstring::npos);
            result = true;
        }
    }
    return result;
}

void CResJsonIndexer::_AddMetaData(
    const wchar_t* const pszPropertyType,
    const wchar_t* const pszPropertyValue,
    ITEM_INFO* const pNode,
    const std::uint32_t nDepth)
{
    if (pNode != nullptr && pNode->bIsValid)
    {
        if (pNode->pNext != nullptr)
        {
            _AddMetaData(pszPropertyType, pszPropertyValue, pNode->pNext, nDepth + 1);
        }
        else
        {
            pNode->metadataMap.insert(std::pair<const std::wstring, std::wstring>(pszPropertyType, pszPropertyValue));
        }
        if (pNode->pAdjacent != nullptr && nDepth != 0)
        {
            _AddMetaData(pszPropertyType, pszPropertyValue, pNode->pAdjacent, nDepth + 1);
        }
    }
}

HRESULT CResJsonIndexer::_ParseValue(
    CJsonValue* const pJsonValue,
    ITEM_INFO* const pItem,
    const std::uint32_t nDepth,
    const wchar_t* const pResJsonFileName,
    IDefStatusEx* const pStatus)
{
    HRESULT result = S_OK;
    if (nDepth >= 5)
    {
        _SetDefError_WithLineColumn(
            E_DEF_FSI_RESJSON_NODE_DEPTH_MAX_EXCEEDED,
            pResJsonFileName,
            static_cast<int>(pJsonValue->_iLineNumber),
            static_cast<int>(pJsonValue->_iColumnNumber),
            pStatus);
        return ComputeHResult(result, pStatus);
    }

    const std::uint32_t nextDepth = nDepth + 1;
    std::list<std::wstring> names;
    std::map<std::wstring, CJsonValue*> values(pJsonValue->_pObjectValue->GetMap());
    ITEM_INFO* previous = nullptr;
    for (auto iterator = values.begin(); iterator != values.end(); ++iterator)
    {
        std::wstring name(iterator->first);
        for (auto existing = names.begin(); existing != names.end(); ++existing)
        {
            if (DefString_CompareWithOptions(existing->c_str(), name.c_str(), DefCompare_CaseInsensitive) == 0)
            {
                break;
            }
        }

        ITEM_INFO* const item = _CreateNewItemInfo();
        item->pItemName.assign(name, 0, std::wstring::npos);
        if (previous != nullptr)
        {
            previous->pAdjacent = item;
        }
        else
        {
            pItem->pNext = item;
        }

        CJsonValue* const childValue = iterator->second;
        if (childValue->_valueType == CJsonValue::JsonValueType_Object)
        {
            std::wstring targetName;
            std::wstring propertyName;
            if (_IsProperty(name, &targetName, &propertyName))
            {
                _SetDefError_WithLineColumn(
                    E_DEF_FSI_RESJSON_INVALID_PROP_OBJ,
                    pResJsonFileName,
                    static_cast<int>(childValue->_iLineNumber),
                    static_cast<int>(childValue->_iColumnNumber),
                    pStatus);
                break;
            }
            result = _ParseValue(childValue, item, nextDepth, pResJsonFileName, pStatus);
        }
        else if (childValue->_valueType == CJsonValue::JsonValueType_String)
        {
            item->pValue.assign(childValue->_szStringValue, 0, std::wstring::npos);
        }
        else
        {
            _SetDefError_WithLineColumn(
                E_DEF_FSI_RESJSON_INVALID_ITEM_TYPE,
                pResJsonFileName,
                static_cast<int>(childValue->_iLineNumber),
                static_cast<int>(childValue->_iColumnNumber),
                pStatus);
            break;
        }

        previous = item;
        names.push_back(name);
    }

    for (ITEM_INFO* item = pItem->pNext; item != nullptr; item = item->pAdjacent)
    {
        std::wstring targetName;
        std::wstring propertyName;
        if (item->bIsValid && _IsProperty(item->pItemName, &targetName, &propertyName))
        {
            item->bIsValid = false;
            if (!targetName.empty())
            {
                ITEM_INFO* target = pItem->pNext;
                while (target != nullptr)
                {
                    if (target->bIsValid && target->pItemName.compare(targetName) == 0)
                    {
                        break;
                    }
                    target = target->pAdjacent;
                }
                if (target != nullptr)
                {
                    _AddMetaData(propertyName.c_str(), item->pValue.c_str(), target, 0);
                }
            }
        }
    }
    return ComputeHResult(result, pStatus);
}

HRESULT CResJsonIndexer::_ProcessItemInfoAndDelete(
    const wchar_t* const pScopeName,
    const wchar_t* const pResJsonFileName,
    const wchar_t* const pItemName,
    ITEM_INFO* const pItem,
    CItemInstanceEntry* const pEntry,
    const std::uint32_t nDepth,
    CItemInstanceSink* const pTraversalSink)
{
    HRESULT result = S_OK;
    ITEM_INFO* item = pItem;
    while (item != nullptr)
    {
        std::wstring fullName(pItemName);
        if (nDepth != 0)
        {
            if (!fullName.empty())
            {
                fullName.append(L"\\");
            }
            fullName.append(item->pItemName, 0, std::wstring::npos);
        }

        if (item->pNext != nullptr)
        {
            if (item->bIsValid)
            {
                result = _ProcessItemInfoAndDelete(
                    pScopeName, pResJsonFileName, fullName.c_str(), item->pNext, pEntry, nDepth + 1, pTraversalSink);
            }
            if (item->pNext != nullptr)
            {
                delete item->pNext;
            }
            item->pNext = nullptr;
        }
        else if (item->bIsValid)
        {
            DefStatusEx status;
            AutoDeletePtr<CItemInstanceEntry> entry(
                CItemInstanceEntry::NewForString(
                    pScopeName,
                    fullName.c_str(),
                    MrmEnvironment::ResourceItemType_String,
                    MrmEnvironment::ResourceValueType_Utf16String,
                    item->pValue.c_str(),
                    pEntry->qualifierSetIndex,
                    1,
                    pResJsonFileName,
                    &item->metadataMap,
                    &status));
            if (entry.Data() != nullptr)
            {
                result = pTraversalSink->AddEntry(entry.Data());
                if (SUCCEEDED(result))
                {
                    entry.Detach();
                }
            }
            else
            {
                result = status.GetHResult();
            }
        }

        if (FAILED(result))
        {
            return result;
        }
        ITEM_INFO* const oldItem = item;
        item = item->pAdjacent;
        if (oldItem != pItem)
        {
            delete oldItem;
        }
    }
    return result;
}

CResJsonIndexer::ITEM_INFO::~ITEM_INFO() = default;

HRESULT CResJsonIndexer::_ParseResJson(
    const wchar_t* const pScopeName,
    const wchar_t* const pResJsonFileName,
    CItemInstanceEntry* const pEntry,
    CItemInstanceSink* const pTraversalSink,
    IDefStatusEx* const pStatus)
{
    HRESULT result = S_OK;
    if (_wszBuffer.length() == 0)
    {
        return ComputeHResult(result, pStatus);
    }

    CJsonParser parser;
    result = parser.SetInput(_wszBuffer.c_str(), static_cast<std::uint32_t>(_wszBuffer.length() + 1));
    if (SUCCEEDED(result))
    {
        CJsonValue rootValue;
        result = parser.Parse(&rootValue);
        if (FAILED(result))
        {
            _ReportParserError(pResJsonFileName, &parser, pStatus);
        }
        else if (rootValue._valueType == CJsonValue::JsonValueType_Object)
        {
            ITEM_INFO* const rootInfo = _CreateNewItemInfo();
            result = _ParseValue(&rootValue, rootInfo, 0, pResJsonFileName, pStatus);
            if (SUCCEEDED(result) && rootInfo->pNext != nullptr)
            {
                result = _ProcessItemInfoAndDelete(pScopeName, pResJsonFileName, L"", rootInfo, pEntry, 0, pTraversalSink);
            }
            delete rootInfo;
        }
        else
        {
            pStatus->SetError(E_DEF_FSI_RESJSON_MISSING_ROOT_OBJ, pResJsonFileName, 0, L"(0,0)");
        }
    }
    return ComputeHResult(result, pStatus);
}

HRESULT CResJsonIndexer::Process(
    CItemInstanceEntry* const pEntry,
    CItemInstanceSink* const pSink,
    IDefStatusEx* const pStatus,
    bool* const pbRemoveContainerFromIndex)
{
    HRESULT result = S_OK;
    if (pEntry->resourceItemType == MrmEnvironment::ResourceItemType_ResJson)
    {
        *pbRemoveContainerFromIndex = true;
        _wszBuffer.assign(pEntry->value.GetRef());
        result = _ParseResJson(pEntry->source.GetRef(), pEntry->valueTypeName.GetRef(), pEntry, pSink, pStatus);
    }
    else if (DefString_CompareWithOptions(pEntry->source.GetRef(), L"Files", DefCompare_CaseInsensitive) == 0)
    {
        const wchar_t* const filePath = pEntry->value.GetRef();
        *pbRemoveContainerFromIndex = false;
        StringResult resourceName;
        Def_HrFailed0(DefStringResult_InitRef(resourceName.GetStringResult(), _strInitialPath.GetRef()), pStatus);
        if (filePath != nullptr)
        {
            std::wstring path(filePath);
            const std::size_t extensionPosition = path.rfind(L".", std::wstring::npos, 1);
            if (extensionPosition != std::wstring::npos)
            {
                std::wstring extension = path.substr(extensionPosition, std::wstring::npos);
                if (DefString_CompareWithOptions(extension.c_str(), L".resjson", DefCompare_CaseInsensitive) == 0)
                {
                    const wchar_t* accessiblePath = nullptr;
                    result = CUtilities::GetPathInAccessibleFormat(_pProjectRoot, filePath, pStatus, &accessiblePath);
                    if (SUCCEEDED(result))
                    {
                        if (PathFileExistsW(accessiblePath))
                        {
                            std::wstring resourceBase;
                            std::wstring itemName(pEntry->itemName.GetRef());
                            const std::size_t dotPosition = itemName.rfind(L".", std::wstring::npos, 1);
                            const std::size_t slashPosition = itemName.rfind(L"\\", std::wstring::npos, 1);
                            if (slashPosition == std::wstring::npos)
                            {
                                resourceBase.assign(itemName.substr(0, dotPosition), 0, std::wstring::npos);
                            }
                            else
                            {
                                resourceBase.assign(
                                    itemName.substr(slashPosition + 1, dotPosition - slashPosition - 1), 0, std::wstring::npos);
                            }
                            Def_HrFailed0(
                                DefStringResult_ConcatPathElement(resourceName.GetStringResult(), resourceBase.c_str(), L'/'), pStatus);

                            std::wstring contents;
                            result = CUtilities::LoadFile(accessiblePath, contents, pStatus);
                            result = ComputeHResult(result, pStatus);
                            if (SUCCEEDED(result) && !contents.empty())
                            {
                                result = Redirect(resourceName.GetRef(), accessiblePath, contents, pEntry, pSink, pStatus);
                                if (SUCCEEDED(result))
                                {
                                    *pbRemoveContainerFromIndex = true;
                                }
                            }
                        }
                        else
                        {
                            pStatus->SetError(E_DEF_FILE_NOT_FOUND, accessiblePath);
                        }
                    }
                    operator delete(const_cast<wchar_t*>(accessiblePath));
                }
            }
        }
    }
    return ComputeHResult(result, pStatus);
}

HRESULT CResJsonIndexer::Redirect(
    const wchar_t* const resourceId,
    const wchar_t* const accessiblePath,
    std::wstring& contents,
    CItemInstanceEntry* const entry,
    CItemInstanceSink* const sink,
    IDefStatusEx* const status)
{
    AutoDeletePtr<CItemInstanceEntry> redirectedEntry(
        CItemInstanceEntry::NewForString(
            resourceId,
            entry->itemName.GetRef(),
            MrmEnvironment::ResourceItemType_ResJson,
            MrmEnvironment::ResourceValueType_Utf8String,
            contents.c_str(),
            entry->qualifierSetIndex,
            2,
            accessiblePath,
            nullptr,
            status));
    HRESULT result;
    if (redirectedEntry.Data() != nullptr)
    {
        result = sink->AddEntry(redirectedEntry.Data());
        if (SUCCEEDED(result))
        {
            redirectedEntry.Detach();
        }
    }
    else
    {
        result = status->GetHResult();
    }
    return result;
}

HRESULT CResJsonIndexer::_ReportParserError(
    const wchar_t* const pResJsonFileName,
    CJsonParser* const pJsonParser,
    IDefStatusEx* const pStatus)
{
    int line;
    int column;
    HRESULT error;
    switch (pJsonParser->GetLastJsonError(&line, &column))
    {
    case CJsonParser::ERR_DUPLICATE:
        error = E_DEF_FSI_RESJSON_DUPLICATE;
        break;
    case CJsonParser::ERR_MULTIPLE_OBJECTS:
        error = E_DEF_FSI_RESJSON_MULTIPLE_OBJECTS;
        break;
    case CJsonParser::ERR_MISSING_LBRACE:
        line = 0;
        column = 0;
        error = E_DEF_FSI_RESJSON_MISSING_ROOT_OBJ;
        break;
    case CJsonParser::ERR_MISSING_RBRACE:
        error = E_DEF_FSI_RESJSON_MISSING_COMMA_BRACE;
        break;
    case CJsonParser::ERR_MISSING_OBJECT_NAME:
        error = E_DEF_FSI_RESJSON_MISSING_OBJECT_NAME;
        break;
    case CJsonParser::ERR_MISSING_COLON:
        error = E_DEF_FSI_RESJSON_MISSING_COLON;
        break;
    case CJsonParser::ERR_INVALID_CHARACTER:
        error = E_DEF_FSI_RESJSON_INVALID_CHAR;
        break;
    case CJsonParser::ERR_INVALID_INPUTSTRING:
        error = E_DEF_FSI_RESJSON_INVALID_INPUTSTR;
        break;
    default:
        return pStatus->GetHResult();
    }
    _SetDefError_WithLineColumn(error, pResJsonFileName, line, column, pStatus);
    return pStatus->GetHResult();
}

bool CResJsonIndexer::_SetDefError_WithLineColumn(
    const HRESULT error,
    const wchar_t* const source,
    const int line,
    const int column,
    IDefStatusEx* const status)
{
    wchar_t lineAndColumn[10];
    StringCchPrintfW(lineAndColumn, 10, L"(%d,%d)", line, column);
    return status->SetError(error, source, 0, lineAndColumn);
}
} // namespace Microsoft::Resources::Indexers
