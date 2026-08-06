#include "StdAfx.h"

#include <CEmbedFilesIndexer.h>

namespace Microsoft::Resources::Indexers
{
// clang-format off
const wchar_t* CEmbedFilesIndexer::s_pEmbedFilesSchema =
    LR"xml(<xs:schema id="embedfiles" xmlns:xs="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified">)xml"
        LR"xml(<xs:simpleType name="IndexerConfigEmbeddedFilesType">)xml"
            LR"xml(<xs:restriction base="xs:string">)xml"
                LR"xml(<xs:pattern value="((e|E)(m|M)(b|B)(e|E)(d|D)(f|F)(i|I)(l|L)(e|E)(s|S))"/>)xml"
            LR"xml(</xs:restriction>)xml"
        LR"xml(</xs:simpleType>)xml"
        LR"xml(<xs:element name="indexer-config">)xml"
            LR"xml(<xs:complexType>)xml"
                LR"xml(<xs:attribute  name="type"  type="IndexerConfigEmbeddedFilesType" use="required"/>)xml"
            LR"xml(</xs:complexType>)xml"
        LR"xml(</xs:element>)xml"
    LR"xml(</xs:schema>)xml";
// clang-format on

HRESULT CEmbedFilesIndexer::Init(
    const UnifiedEnvironment* const pEnvironment,
    const wchar_t* const pProjectRoot,
    IXMLDOMNode* const pIndexPassNode,
    CQualifierApplicator* const pQualApplicator,
    const IIndexOptions* const options,
    IDefStatusEx* const pStatus)
{
    static_cast<void>(pQualApplicator);
    static_cast<void>(options);
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    _pEnvironment = pEnvironment;
    _pProjectRoot = pProjectRoot;
    const HRESULT result = _ParseIndexPassNode(pIndexPassNode, pStatus);
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CEmbedFilesIndexer::Process(
    CItemInstanceEntry* const pEntry,
    CItemInstanceSink* const pTraversalSink,
    IDefStatusEx* const pStatus,
    bool* const pbRemoveContainerFromIndex)
{
    HRESULT result = S_OK;
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    *pbRemoveContainerFromIndex = false;
    if (DefString_CompareWithOptions(pEntry->source.GetRef(), L"Files", DefCompare_CaseInsensitive) == 0)
    {
        const wchar_t* path = nullptr;
        result = CUtilities::GetPathInAccessibleFormat(_pProjectRoot, pEntry->value.GetRef(), pStatus, &path);
        if (SUCCEEDED(result))
        {
            const DWORD attributes = GetFileAttributesW(path);
            if (attributes == INVALID_FILE_ATTRIBUTES)
            {
                pStatus->SetError(E_DEF_FILE_NOT_FOUND, path);
            }
            else if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                result = _AddEmbeddedFileEntry(path, pEntry, pTraversalSink, pStatus);
                if (SUCCEEDED(result))
                {
                    *pbRemoveContainerFromIndex = true;
                }
            }
        }
        delete[] path;
    }
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, ComputeHResult(result, pStatus));
    return ComputeHResult(result, pStatus);
}

HRESULT CEmbedFilesIndexer::_AddEmbeddedFileEntry(
    const wchar_t* const pEmbeddedFileName,
    CItemInstanceEntry* const pEntry,
    CItemInstanceSink* const pTraversalSink,
    IDefStatusEx* const pStatus)
{
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    BlobResult value;
    HRESULT result = GetFileContents(pEmbeddedFileName, pStatus, &value);
    if (SUCCEEDED(result))
    {
        AutoDeletePtr<CItemInstanceEntry> embedded(
            CItemInstanceEntry::NewForEmbeddedData(
                pEntry->source.GetRef(),
                pEntry->itemName.GetRef(),
                pEntry->resourceItemType,
                MrmEnvironment::ResourceValueType_EmbeddedData,
                &value,
                pEntry->qualifierSetIndex,
                1,
                pEmbeddedFileName,
                nullptr,
                pStatus));
        if (embedded.Data() != nullptr)
        {
            result = pTraversalSink->AddEntry(embedded.Data());
            if (SUCCEEDED(result))
            {
                embedded.Detach();
            }
        }
    }
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, ComputeHResult(result, pStatus));
    return ComputeHResult(result, pStatus);
}

HRESULT CEmbedFilesIndexer::GetFileContents(
    const wchar_t* const pEmbeddedFileName,
    IDefStatusEx* const pStatus,
    BlobResult* const pFileContents)
{
    HRESULT operationResult = pStatus->GetHResult();
    if (FAILED(operationResult))
    {
        return ComputeHResult(operationResult, pStatus);
    }

    std::FILE* const file = _wfopen(pEmbeddedFileName, L"rb");
    if (file != nullptr)
    {
        const __int64 length = _filelengthi64(_fileno(file));
        if (length < 0)
        {
            pStatus->SetError(E_DEF_UNABLE_TO_CALCULATE_SIZE, pEmbeddedFileName);
        }
        else if (length == 0 || length > 0x1000000)
        {
            operationResult = E_DEF_IBC_CANDIDATE_NOT_EMBEDDED;
        }
        else
        {
            void* buffer = nullptr;
            DefBlobResult_SetEmptyContents(pFileContents->GetBlobResult(), static_cast<std::size_t>(length), &buffer, nullptr);
            if (buffer != nullptr)
            {
                if (fread(buffer, 1, static_cast<std::size_t>(length), file) != static_cast<std::size_t>(length))
                {
                    pStatus->SetError(E_DEF_UNABLE_TO_READ_FILE, pEmbeddedFileName);
                }
            }
            else
            {
                pStatus->SetError(E_OUTOFMEMORY, pEmbeddedFileName);
            }
        }
        fclose(file);
        return ComputeHResult(operationResult, pStatus);
    }

    HRESULT openResult = ErrnoToHResult(errno);
    if (openResult == E_FAIL)
    {
        openResult = HRESULT_FROM_WIN32(ERROR_CANT_ACCESS_FILE);
    }
    pStatus->SetError(openResult, pEmbeddedFileName);
    return ComputeHResult(operationResult, pStatus);
}

HRESULT CEmbedFilesIndexer::_ParseIndexPassNode(IXMLDOMNode* const pIndexPassNode, IDefStatusEx* const pStatus)
{
    IXMLDOMNodeList* children = nullptr;
    CXmlHelper helper(pIndexPassNode);
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    bool found = false;
    HRESULT result =
        helper.ValidateChildNodeAgainstChildSchema(L"indexer-config", s_pEmbedFilesSchema, L"type", L"embedfiles", false, pStatus);
    if (SUCCEEDED(result))
    {
        helper.TryGetChildren(L"indexer-config", pStatus, &children);
        LONG length;
        children->get_length(&length);
        for (LONG index = 0; index < length && !found; ++index)
        {
            IXMLDOMNode* child = nullptr;
            result = children->get_item(index, &child);
            if (SUCCEEDED(result))
            {
                CXmlHelper childHelper(child);
                wchar_t* type = nullptr;
                childHelper.GetAttributeValue(L"type", pStatus, &type);
                if (DefString_CompareWithOptions(type, L"embedfiles", DefCompare_CaseInsensitive) == 0)
                {
                    found = true;
                }
                delete[] type;
            }
            SAFE_RELEASE(child);
        }
        SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
    }
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, ComputeHResult(result, pStatus));
    return ComputeHResult(result, pStatus);
}
} // namespace Microsoft::Resources::Indexers
