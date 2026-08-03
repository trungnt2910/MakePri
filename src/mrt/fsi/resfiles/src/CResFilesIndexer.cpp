#include "StdAfx.h"

#include <CResFilesIndexer.h>

namespace Microsoft::Resources::Indexers
{
// clang-format off
const wchar_t* CResFilesIndexer::s_pResFilesSchema =
    LR"xml(<xs:schema id="resx" xmlns:xs="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified">)xml"
        LR"xml(<xs:simpleType name="IndexerConfigResFilesType">)xml"
            LR"xml(<xs:restriction base="xs:string">)xml"
                LR"xml(<xs:pattern value="((r|R)(e|E)(s|S)(f|F)(i|I)(l|L)(e|E)(s|S))"/>)xml"
            LR"xml(</xs:restriction>)xml"
        LR"xml(</xs:simpleType>)xml"
        LR"xml(<xs:element name="indexer-config">)xml"
            LR"xml(<xs:complexType>)xml"
                LR"xml(<xs:attribute  name="type"  type="IndexerConfigResFilesType" use="required"/>)xml"
                LR"xml(<xs:attribute  name="qualifierDelimiter"  type="xs:string" use="required"/>)xml"
            LR"xml(</xs:complexType>)xml"
        LR"xml(</xs:element>)xml"
    LR"xml(</xs:schema>)xml";
// clang-format on

CResFilesIndexer::~CResFilesIndexer()
{
    _pEnvironment = nullptr;
    _pProjectRoot = nullptr;
    _pQualApplicator = nullptr;
}

HRESULT CResFilesIndexer::Init(
    const UnifiedEnvironment* const pEnvironment,
    const wchar_t* const pProjectRoot,
    IXMLDOMNode* const pIndexPassNode,
    CQualifierApplicator* const pApplicator,
    const IIndexOptions* const options,
    IDefStatusEx* const pStatus)
{
    static_cast<void>(options);
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    _pEnvironment = pEnvironment;
    _pProjectRoot = pProjectRoot;
    _pQualApplicator = pApplicator;
    const HRESULT result = _ParseIndexPassNode(pIndexPassNode, pStatus);
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CResFilesIndexer::_ParseIndexPassNode(IXMLDOMNode* const pIndexPassNode, IDefStatusEx* const pStatus)
{
    IXMLDOMNodeList* children = nullptr;
    CXmlHelper helper(pIndexPassNode);
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    HRESULT result = helper.ValidateChildNodeAgainstChildSchema(L"indexer-config", s_pResFilesSchema, L"type", L"resfiles", false, pStatus);
    if (SUCCEEDED(result))
    {
        helper.TryGetChildren(L"indexer-config", pStatus, &children);
        LONG length;
        children->get_length(&length);
        bool found = false;
        IXMLDOMNode* child;
        for (LONG index = 0; index < length && !found; ++index)
        {
            result = children->get_item(index, &child);
            if (SUCCEEDED(result))
            {
                CXmlHelper childHelper(child);
                wchar_t* type = nullptr;
                childHelper.GetAttributeValue(L"type", pStatus, &type);
                if (DefString_CompareWithOptions(type, L"resfiles", DefCompare_CaseInsensitive) == 0)
                {
                    wchar_t* delimiter = nullptr;
                    childHelper.GetAttributeValue(L"qualifierDelimiter", pStatus, &delimiter);
                    if (wcslen(delimiter) != 1 || *delimiter == L'-' || *delimiter == L'_')
                    {
                        pStatus->SetError(E_DEF_FSI_INVALID_DELIMITER, delimiter);
                        result = ComputeHResult(result, pStatus);
                    }
                    else
                    {
                        Def_HrFailed0(DefStringResult_SetCopy(_strQualifierDelimiter.GetStringResult(), delimiter), pStatus);
                    }
                    operator delete(delimiter);
                    found = true;
                }
                operator delete(type);
            }
            SAFE_RELEASE(child);
        }
        SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
    }
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, ComputeHResult(result, pStatus));
    return ComputeHResult(result, pStatus);
}

void CResFilesIndexer::_TrimSpaces(std::wstring& wszString)
{
    const std::size_t first = wszString.find_first_not_of(L" \t");
    const std::size_t last = wszString.find_last_not_of(L" \t");
    if (first == std::wstring::npos || last == std::wstring::npos)
    {
        wszString.clear();
    }
    else
    {
        wszString.assign(wszString.substr(first, last - first + 1), 0, std::wstring::npos);
    }
}

HRESULT CResFilesIndexer::_ParseResFile(
    const wchar_t* const pResFileName,
    std::wstring& contents,
    CItemInstanceEntry* const pEntry,
    CItemInstanceSink* const pTraversalSink,
    IDefStatusEx* const pStatus)
{
    HRESULT result = S_OK;
    int qualifierSetIndex = 0;
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);

    contents.insert(contents.length(), 1, static_cast<wchar_t>(0xFFFF));
    const wchar_t* current = contents.c_str();
    std::list<std::wstring> normalizedPaths;
    wchar_t terminator;
    do
    {
        std::wstring line;
        do
        {
            terminator = *current++;
            if (terminator != L'\n' && terminator != static_cast<wchar_t>(0xFFFF))
            {
                line.append(1, terminator);
            }
        } while (terminator != L'\n' && terminator != static_cast<wchar_t>(0xFFFF));

        const wchar_t* const next = current;
        std::wstring comment;
        std::wstring resourcePath;
        std::wstring normalizedPath;
        std::wstring itemName;
        qualifierSetIndex = 0;

        _TrimSpaces(line);
        if (!line.empty())
        {
            const std::size_t commentPosition = line.find(L"//", 0, 2);
            if (commentPosition == std::wstring::npos)
            {
                resourcePath.assign(line, 0, std::wstring::npos);
            }
            else
            {
                comment.append(line.substr(commentPosition + 2, line.length() - commentPosition - 2), 0, std::wstring::npos);
                resourcePath.append(line.substr(0, commentPosition), 0, std::wstring::npos);
                _TrimSpaces(resourcePath);
            }

            if (!resourcePath.empty())
            {
                const std::size_t firstQuote = resourcePath.find_first_not_of(L"\"");
                const std::size_t lastQuote = resourcePath.find_last_not_of(L"\"");
                if (firstQuote == std::wstring::npos || lastQuote == std::wstring::npos)
                {
                    resourcePath.clear();
                }
                else
                {
                    resourcePath.assign(resourcePath.substr(firstQuote, lastQuote - firstQuote + 1), 0, std::wstring::npos);
                }
            }

            if (!resourcePath.empty())
            {
                if (!pStatus->Succeeded())
                {
                    break;
                }

                CUtilities::AdjustForProjectRoot(_pProjectRoot, pStatus, resourcePath);
                if (!PathIsRelativeW(resourcePath.c_str()) && resourcePath.find(L"..\\", 0, 3) != std::wstring::npos)
                {
                    itemName.assign(resourcePath, 0, std::wstring::npos);
                }
                else
                {
                    StringResult normalizedBuffer;
                    wchar_t* buffer = nullptr;
                    std::size_t bufferSize = 0;
                    Def_HrFailed0(normalizedBuffer.SetEmptyContents(resourcePath.length() + 1, &buffer, &bufferSize), pStatus);
                    LCMapStringEx(
                        LOCALE_NAME_INVARIANT,
                        LCMAP_UPPERCASE,
                        resourcePath.c_str(),
                        static_cast<int>(resourcePath.length()),
                        buffer,
                        static_cast<int>(bufferSize),
                        nullptr,
                        nullptr,
                        0);
                    normalizedPath.clear();
                    normalizedPath.append(buffer);

                    bool duplicate = false;
                    for (const std::wstring& existing : normalizedPaths)
                    {
                        if (existing.compare(normalizedPath.c_str()) == 0)
                        {
                            duplicate = true;
                            break;
                        }
                    }
                    if (duplicate)
                    {
                        current = next;
                        continue;
                    }
                    normalizedPaths.push_back(normalizedPath);

                    std::size_t position = 0;
                    std::size_t separator = normalizedPath.find_first_of(L"\\", 0);
                    do
                    {
                        std::wstring qualifier;
                        bool isQualifier = false;
                        bool isLast = false;
                        bool delimiterAtBeginning = false;
                        std::wstring normalizedComponent = normalizedPath.substr(position, separator - position);
                        std::wstring originalComponent = resourcePath.substr(position, separator - position);
                        std::wstring modifiedComponent;

                        if (separator == std::wstring::npos)
                        {
                            isLast = true;
                            position = std::wstring::npos;
                            const wchar_t* const delimiter = _strQualifierDelimiter.GetRef();
                            std::size_t delimiterPosition =
                                normalizedComponent.rfind(delimiter, normalizedComponent.length(), wcslen(delimiter));
                            if (*delimiter == L'.')
                            {
                                delimiterPosition = normalizedComponent.rfind(delimiter, delimiterPosition - 1, wcslen(delimiter));
                            }
                            if (delimiterPosition != std::wstring::npos)
                            {
                                delimiterAtBeginning = delimiterPosition == 0;
                                const std::size_t dotPosition = normalizedComponent.find_first_of(L".", delimiterPosition + 1);
                                if (dotPosition != std::wstring::npos)
                                {
                                    qualifier.append(
                                        normalizedComponent.substr(delimiterPosition + 1, dotPosition - delimiterPosition - 1),
                                        0,
                                        std::wstring::npos);
                                    modifiedComponent.append(originalComponent, 0, std::wstring::npos);
                                    modifiedComponent.replace(delimiterPosition, dotPosition - delimiterPosition, L"", 0);
                                }
                            }
                        }
                        else
                        {
                            qualifier.assign(normalizedComponent, 0, std::wstring::npos);
                            position = ++separator;
                        }

                        int newQualifierSetIndex = 0;
                        result = _pQualApplicator->ApplyQualifier(
                            qualifier.c_str(),
                            qualifierSetIndex,
                            isLast ? CQualifierApplicator::tagTOKEN_TYPE::tokenFile : CQualifierApplicator::tagTOKEN_TYPE::tokenFolder,
                            &newQualifierSetIndex,
                            &isQualifier,
                            pStatus);
                        if (SUCCEEDED(result))
                        {
                            if (isQualifier)
                            {
                                if (!delimiterAtBeginning)
                                {
                                    qualifierSetIndex = newQualifierSetIndex;
                                    if (isLast)
                                    {
                                        itemName.append(modifiedComponent, 0, std::wstring::npos);
                                    }
                                    separator = normalizedPath.find_first_of(L"\\", separator);
                                    continue;
                                }
                                pStatus->AddWarning(E_DEF_QUALAPPL_INVALID_QUAL_FILENAME, pResFileName, 0, resourcePath.c_str());
                            }
                            itemName.append(originalComponent, 0, std::wstring::npos);
                            if (!isLast)
                            {
                                itemName.append(L"\\");
                            }
                        }
                        separator = normalizedPath.find_first_of(L"\\", separator);
                    } while (position != std::wstring::npos);
                }

                _TrimSpaces(itemName);
                _TrimSpaces(resourcePath);
                _TrimSpaces(comment);
                std::map<std::wstring, std::wstring> metadata;
                metadata.insert(std::pair<const std::wstring, std::wstring>(L"comment", comment));
                AutoDeletePtr<CItemInstanceEntry> entry(
                    CItemInstanceEntry::NewForString(
                        pEntry->source.GetRef(),
                        itemName.c_str(),
                        MrmEnvironment::ResourceItemType_Path,
                        MrmEnvironment::ResourceValueType_Utf16Path,
                        resourcePath.c_str(),
                        qualifierSetIndex,
                        3,
                        pResFileName,
                        &metadata,
                        pStatus));
                if (entry.Data() != nullptr)
                {
                    result = pTraversalSink->AddEntry(entry.Data());
                    if (SUCCEEDED(result))
                    {
                        entry.Detach();
                    }
                }
            }
        }
        current = next;
    } while (terminator != static_cast<wchar_t>(0xFFFF));

    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, ComputeHResult(result, pStatus));
    return ComputeHResult(result, pStatus);
}

HRESULT CResFilesIndexer::Process(
    CItemInstanceEntry* const pEntry,
    CItemInstanceSink* const pTraversalSink,
    IDefStatusEx* const pStatus,
    bool* const pbRemoveContainerFromIndex)
{
    HRESULT result = S_OK;
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    if (pEntry->resourceItemType == MrmEnvironment::ResourceItemType_ResFile)
    {
        *pbRemoveContainerFromIndex = true;
        std::wstring contents(pEntry->value.GetRef());
        result = _ParseResFile(pEntry->valueTypeName.GetRef(), contents, pEntry, pTraversalSink, pStatus);
        return ComputeHResult(result, pStatus);
    }

    if (DefString_CompareWithOptions(pEntry->source.GetRef(), L"Files", DefCompare_CaseInsensitive) == 0)
    {
        const wchar_t* const filePath = pEntry->value.GetRef();
        *pbRemoveContainerFromIndex = false;
        const std::size_t length = wcslen(filePath);
        if (length > 9 && DefString_CompareWithOptions(filePath + length - 9, L".resfiles", DefCompare_CaseInsensitive) == 0)
        {
            const wchar_t* accessiblePath = nullptr;
            result = CUtilities::GetPathInAccessibleFormat(_pProjectRoot, filePath, pStatus, &accessiblePath);
            if (SUCCEEDED(result))
            {
                if (PathFileExistsW(accessiblePath))
                {
                    std::wstring contents;
                    result = CUtilities::LoadFile(accessiblePath, contents, pStatus);
                    if (SUCCEEDED(result) && !contents.empty())
                    {
                        result = Redirect(accessiblePath, contents, pEntry, pTraversalSink, pStatus);
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
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, ComputeHResult(result, pStatus));
    return ComputeHResult(result, pStatus);
}

HRESULT CResFilesIndexer::Redirect(
    const wchar_t* const accessiblePath,
    std::wstring& contents,
    CItemInstanceEntry* const entry,
    CItemInstanceSink* const sink,
    IDefStatusEx* const status)
{
    AutoDeletePtr<CItemInstanceEntry> redirectedEntry(
        CItemInstanceEntry::NewForString(
            entry->source.GetRef(),
            entry->itemName.GetRef(),
            MrmEnvironment::ResourceItemType_ResFile,
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
} // namespace Microsoft::Resources::Indexers
