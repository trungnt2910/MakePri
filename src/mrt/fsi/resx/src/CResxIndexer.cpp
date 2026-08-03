#include "StdAfx.h"

#include <CResxIndexer.h>

namespace Microsoft::Resources::Indexers
{

CResxIndexer::~CResxIndexer()
{
    m_environment = nullptr;
    m_projectRoot = nullptr;
    m_indexerConfig = nullptr;
    m_qualifierApplicator = nullptr;
    if (m_resxXmlConfigHelper != nullptr)
    {
        m_resxXmlConfigHelper->CResxXmlConfig::~CResxXmlConfig();
        ::operator delete(m_resxXmlConfigHelper);
    }
    m_resxXmlConfigHelper = nullptr;
}

HRESULT CResxIndexer::CollectItemInstanceEntry(
    CItemInstanceEntry* const entry,
    IXMLDOMNode* const dataDomNode,
    const wchar_t* const fileName,
    const wchar_t* const scopeName,
    IDefStatusEx* const status,
    CItemInstanceSink* const sink)
{
    CXmlHelper helper(dataDomNode);
    wchar_t* name = nullptr;
    HRESULT result = helper.GetAttributeValue(L"name", status, &name);
    if (SUCCEEDED(result) && name != nullptr)
    {
        if (m_resxXmlConfigHelper->GetConvertDotsToSlashesFlag())
        {
            ConvertDotsToSlashes(name);
        }

        std::map<std::wstring, std::wstring> metadata;
        IXMLDOMNode* commentNode = nullptr;
        if (helper.TryGetChildNode(L"comment", status, &commentNode) != S_OK || commentNode == nullptr)
        {
            status->Reset();
        }
        else
        {
            BSTR comment;
            commentNode->get_text(&comment);
            metadata.insert(std::pair<const std::wstring, std::wstring>(L"comment", comment));
            SysFreeString(comment);
            SAFE_RELEASE(commentNode);
        }

        IXMLDOMNode* valueNode = nullptr;
        HRESULT valueResult = helper.TryGetChildNode(L"value", status, &valueNode);
        if (SUCCEEDED(valueResult) && valueNode != nullptr)
        {
            BSTR valueText;
            valueResult = valueNode->get_text(&valueText);
            if (SUCCEEDED(valueResult))
            {
                std::wstring value(valueText);
                SysFreeString(valueText);
                AutoDeletePtr<CItemInstanceEntry> newEntry(
                    CItemInstanceEntry::NewForString(
                        scopeName,
                        name,
                        MrmEnvironment::ResourceItemType_String,
                        MrmEnvironment::ResourceValueType_Utf16String,
                        value.c_str(),
                        entry->qualifierSetIndex,
                        1,
                        fileName,
                        &metadata,
                        status));
                if (newEntry.Data() != nullptr)
                {
                    if (SUCCEEDED(sink->AddEntry(newEntry.Data())))
                    {
                        newEntry.Detach();
                    }
                }
                else
                {
                    valueResult = status->GetHResult();
                }
            }
            SAFE_RELEASE(valueNode);
        }
        status->Reset();

        IXMLDOMNode* linkNode = nullptr;
        result = helper.TryGetChildNode(L"link", status, &linkNode);
        if (SUCCEEDED(result) && linkNode != nullptr)
        {
            BSTR linkText;
            result = linkNode->get_text(&linkText);
            if (SUCCEEDED(result))
            {
                std::wstring link(linkText);
                SysFreeString(linkText);
                AutoDeletePtr<CItemInstanceEntry> newEntry(
                    CItemInstanceEntry::NewForLink(scopeName, name, link.c_str(), 1, fileName, &metadata, status));
                if (newEntry.Data() != nullptr)
                {
                    if (SUCCEEDED(sink->AddEntry(newEntry.Data())))
                    {
                        newEntry.Detach();
                    }
                }
                else
                {
                    result = status->GetHResult();
                }
            }
            SAFE_RELEASE(linkNode);
        }
        status->Reset();

        if (SUCCEEDED(valueResult))
        {
            result = valueResult;
        }
        operator delete(name);
    }
    return result;
}

HRESULT CResxIndexer::Init(
    const UnifiedEnvironment* const environment,
    const wchar_t* const projectRoot,
    IXMLDOMNode* const indexPassNode,
    CQualifierApplicator* const qualifierApplicator,
    const IIndexOptions* const options,
    IDefStatusEx* const status)
{
    static_cast<void>(options);
    m_environment = environment;
    m_projectRoot = projectRoot;
    m_indexerConfig = indexPassNode;
    m_qualifierApplicator = qualifierApplicator;
    m_resxXmlConfigHelper = new (std::nothrow) CResxXmlConfig(indexPassNode);
    if (m_resxXmlConfigHelper != nullptr)
    {
        return m_resxXmlConfigHelper->Parse(status);
    }
    return E_OUTOFMEMORY;
}

HRESULT CResxIndexer::Process(
    CItemInstanceEntry* const entry,
    CItemInstanceSink* const sink,
    IDefStatusEx* const status,
    bool* const removeContainer)
{
    HRESULT result = S_OK;
    if (entry->resourceItemType == MrmEnvironment::ResourceItemType_Resw)
    {
        *removeContainer = true;
        result = ProcessPayload(entry, entry->valueTypeName.GetRef(), entry->value.GetRef(), entry->source.GetRef(), sink, status);
    }
    else
    {
        const wchar_t* const filePath = entry->value.GetRef();
        if (filePath != nullptr && DefString_CompareWithOptions(entry->source.GetRef(), L"Files", DefCompare_CaseInsensitive) == 0)
        {
            std::wstring path(filePath);
            const std::size_t extensionPosition = path.rfind(L".", std::wstring::npos, 1);
            if (extensionPosition != std::wstring::npos)
            {
                std::wstring extension = path.substr(extensionPosition, std::wstring::npos);
                if (DefString_CompareWithOptions(extension.c_str(), L".Resw", DefCompare_CaseInsensitive) == 0)
                {
                    const wchar_t* accessiblePath = nullptr;
                    result = CUtilities::GetPathInAccessibleFormat(m_projectRoot, filePath, status, &accessiblePath);
                    if (SUCCEEDED(result))
                    {
                        if (PathFileExistsW(accessiblePath))
                        {
                            std::wstring itemName(entry->itemName.GetRef());
                            const std::size_t dotPosition = itemName.rfind(L".", std::wstring::npos, 1);
                            const std::size_t slashPosition = itemName.rfind(L"\\", std::wstring::npos, 1);
                            std::wstring resourceBase = itemName.substr(slashPosition + 1, dotPosition - slashPosition - 1);

                            StringResult resourceName;
                            Def_HrFailed0(
                                DefStringResult_InitRef(resourceName.GetStringResult(), m_resxXmlConfigHelper->GetInitialPath()), status);
                            Def_HrFailed0(
                                DefStringResult_ConcatPathElement(resourceName.GetStringResult(), resourceBase.c_str(), L'/'), status);

                            std::wstring contents;
                            result = CUtilities::LoadFile(accessiblePath, contents, status);
                            if (SUCCEEDED(result))
                            {
                                result = Redirect(resourceName.GetRef(), accessiblePath, contents, entry, sink, status);
                            }
                            *removeContainer = true;
                        }
                        else
                        {
                            status->SetError(E_DEF_FILE_NOT_FOUND, accessiblePath);
                        }
                    }
                    operator delete(const_cast<wchar_t*>(accessiblePath));
                }
            }
        }
    }
    return ComputeHResult(result, status);
}

void CResxIndexer::ConvertDotsToSlashes(wchar_t* const value)
{
    wchar_t* current = value;
    wchar_t* const end = value + wcslen(value);
    while (current < end)
    {
        if (*current == L'.')
        {
            *current = L'\\';
        }
        else if (*current == L'[')
        {
            wchar_t* const closingBracket = wcsstr(current, L"]");
            if (closingBracket != nullptr)
            {
                current = closingBracket;
            }
        }
        ++current;
    }
}

HRESULT CResxIndexer::Redirect(
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
            MrmEnvironment::ResourceItemType_Resw,
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

HRESULT CResxIndexer::ProcessPayload(
    CItemInstanceEntry* const parentEntry,
    const wchar_t* const valueTypeName,
    const wchar_t* const payload,
    const wchar_t* const source,
    CItemInstanceSink* const sink,
    IDefStatusEx* const status)
{
    CXmlHelper* const helper = new (std::nothrow) CXmlHelper;
    HRESULT result = helper != nullptr ? S_OK : E_OUTOFMEMORY;
    if (helper != nullptr)
    {
        result = helper->Init(payload, CXmlHelper::INPUT_XML_STR_TYPE::XML_STR_BLOB, L"root", status);
        if (SUCCEEDED(result))
        {
            IXMLDOMNodeList* children = nullptr;
            result = helper->TryGetChildren(L"data", status, &children);
            if (SUCCEEDED(result) && children != nullptr)
            {
                LONG length = 0;
                result = children->get_length(&length);
                if (SUCCEEDED(result) && length > 0)
                {
                    IXMLDOMNode* child = nullptr;
                    for (LONG index = 0; index < length; ++index)
                    {
                        result = children->get_item(index, &child);
                        if (SUCCEEDED(result) && child != nullptr)
                        {
                            result = CollectItemInstanceEntry(parentEntry, child, valueTypeName, source, status, sink);
                            SAFE_RELEASE(child);
                        }
                    }
                }
                SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
            }
        }
        delete helper;
    }
    return result;
}
} // namespace Microsoft::Resources::Indexers
