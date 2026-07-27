#include "StdAfx.h"

#include <IndexerBase.h>

namespace Microsoft::Resources::Indexers
{

HRESULT CBootStrapIndexer::New(
    const UnifiedEnvironment* const pEnvironment,
    Build::DecisionInfoBuilder* const pDecisionInfoBuilder,
    const wchar_t* const pProjectRootFolder,
    IXMLDOMNode* const pIndexPassNode,
    CQualifierApplicator* const pQualifierApplicator,
    IDefStatusEx* const pStatus,
    CBootStrapIndexer** const ppBootStrapIndexer)
{
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    HRESULT hr = E_OUTOFMEMORY;
    CBootStrapIndexer* const pIndexer = new (std::nothrow) CBootStrapIndexer;
    if (pIndexer != nullptr)
    {
        hr = pIndexer->_Init(pEnvironment, pDecisionInfoBuilder, pProjectRootFolder, pIndexPassNode, pQualifierApplicator, pStatus);
        if (FAILED(hr))
        {
            delete pIndexer;
        }
        else
        {
            *ppBootStrapIndexer = pIndexer;
        }
    }
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, hr);
    return hr;
}

CBootStrapIndexer::~CBootStrapIndexer()
{
    if (_pItemInstanceEntry != nullptr)
    {
        delete _pItemInstanceEntry;
    }
}

HRESULT CBootStrapIndexer::Process(CItemInstanceSink* const pTraversalSink, bool* const pbRemoveContainerFromIndex)
{
    if (_pItemInstanceEntry == nullptr)
    {
        return E_FAIL;
    }
    if (SUCCEEDED(pTraversalSink->AddEntry(_pItemInstanceEntry)))
    {
        *pbRemoveContainerFromIndex = (_pItemInstanceEntry->flags & 1) == 0;
        _pItemInstanceEntry = nullptr;
    }
    return S_OK;
}

HRESULT CBootStrapIndexer::GetIndexablePath(IDefStatusEx* const pStatus, StringResult* const strIndexablePath)
{
    StringResult strNormalizedPath;
    HRESULT hr = _NormalizePath(_strValue.GetRef(), strNormalizedPath, pStatus);
    if (SUCCEEDED(hr))
    {
        bool bIsAbsolute = false;
        strNormalizedPath.IsAbsolutePath(L'\\', &bIsAbsolute);
        if (bIsAbsolute)
        {
            Def_HrFailed0(strIndexablePath->SetContentsFromOther(&strNormalizedPath), pStatus);
        }
        else
        {
            hr = CUtilities::GetAbsolutePath(strNormalizedPath.GetRef(), pStatus, *strIndexablePath);
        }
    }
    return hr;
}

HRESULT CBootStrapIndexer::_Init(
    const UnifiedEnvironment* const pEnvironment,
    Build::DecisionInfoBuilder* const pDecisionInfoBuilder,
    const wchar_t* const pProjectRootFolder,
    IXMLDOMNode* const pIndexPassNode,
    CQualifierApplicator* const pQualifierApplicator,
    IDefStatusEx* const pStatus)
{
    static_cast<void>(pDecisionInfoBuilder);
    HRESULT hr = S_OK;
    _c_pEnvironment = pEnvironment;
    _pQualifierApplicator = pQualifierApplicator;
    _pProjectRootFolder = pProjectRootFolder;
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    _baseQualifierSetIndex = 0;
    if (pStatus->Succeeded())
    {
        hr = _ParseNode(pIndexPassNode, pStatus);
        if (SUCCEEDED(hr))
        {
            hr = CreateStringEntry(
                L"Files",
                L"",
                _strValue.GetRef(),
                MrmEnvironment::ResourceItemType_Path,
                MrmEnvironment::ResourceValueType_Utf16Path,
                _baseQualifierSetIndex,
                2,
                pStatus);
        }
    }
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, ComputeHResult(hr, pStatus));
    return ComputeHResult(hr, pStatus);
}

HRESULT CBootStrapIndexer::_ParseNode(IXMLDOMNode* const pIndexPassNode, IDefStatusEx* const pStatus)
{
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    wchar_t* pNodeName = nullptr;
    IXMLDOMNode* pConditionsNode = nullptr;
    IXMLDOMNode* pUltFallbackNode = nullptr;
    CXmlHelper xmlHelper(pIndexPassNode);
    BSTR bstrNodeName;
    if (FAILED(pIndexPassNode->get_nodeName(&bstrNodeName)))
    {
        xmlHelper._WriteParseError(nullptr, pStatus);
    }
    else
    {
        xmlHelper._CreateString(bstrNodeName, &pNodeName);
        SysFreeString(bstrNodeName);
    }

    HRESULT hr = _ProcessIndexNode(pIndexPassNode, pStatus);
    if (SUCCEEDED(hr))
    {
        hr = xmlHelper.TryGetChildNode(L"default", pStatus, &pUltFallbackNode);
        if (SUCCEEDED(hr))
        {
            if (pUltFallbackNode != nullptr)
            {
                hr = _ProcessUltimateFallbackNode(pUltFallbackNode, pStatus);
                pUltFallbackNode->Release();
            }
            else
            {
                pStatus->Reset();
            }
            if (SUCCEEDED(hr))
            {
                hr = xmlHelper.TryGetChildNode(L"qualifiers", pStatus, &pConditionsNode);
                if (SUCCEEDED(hr))
                {
                    if (pConditionsNode != nullptr)
                    {
                        hr = _ProcessConditionsNode(pConditionsNode, pStatus);
                        pConditionsNode->Release();
                    }
                    else
                    {
                        pStatus->Reset();
                    }
                }
            }
        }
    }
    operator delete(pNodeName);
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, hr);
    return hr;
}

HRESULT CBootStrapIndexer::_ProcessIndexNode(IXMLDOMNode* const pIndexPassNode, IDefStatusEx* const pStatus)
{
    CXmlHelper xmlHelper(pIndexPassNode);
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    wchar_t* startIndexAt = nullptr;
    xmlHelper.GetAttributeValue(L"startIndexAt", pStatus, &startIndexAt);
    wchar_t* root = nullptr;
    xmlHelper.GetAttributeValue(L"root", pStatus, &root);
    HRESULT result = _NormalizePath(root, _strNewProjectRootFolder, pStatus);
    if (SUCCEEDED(result))
    {
        result = _AdjustSlashes(startIndexAt, _strValue, pStatus);
        if (SUCCEEDED(result))
        {
            Def_HrFailed0(DefStringResult_SetCopy(_strStartIndexAt.GetStringResult(), _strValue.GetRef()), pStatus);
        }
    }
    operator delete(root);
    operator delete(startIndexAt);
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CBootStrapIndexer::_AdjustSlashes(const wchar_t* const pPath, StringResult& strAdjustedPath, IDefStatusEx* const pStatus)
{
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    wchar_t* pPathNoSlash = const_cast<wchar_t*>(pPath);
    if (pPathNoSlash != nullptr)
    {
        for (wchar_t* pCh = pPathNoSlash; *pCh != L'\0'; ++pCh)
        {
            if (*pCh == L'/')
            {
                *pCh = L'\\';
            }
        }
        if (wcsncmp(pPathNoSlash, L"\\\\?\\", 4) != 0 && !PathIsNetworkPathW(pPathNoSlash) && *pPathNoSlash == L'\\')
        {
            ++pPathNoSlash;
        }
        Def_HrFailed0(DefStringResult_SetCopy(strAdjustedPath.GetStringResult(), pPathNoSlash), pStatus);
    }
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, pStatus->GetHResult());
    return pStatus->GetHResult();
}

HRESULT CBootStrapIndexer::_NormalizePath(const wchar_t* const pPath, StringResult& strNormalizedPath, IDefStatusEx* const pStatus)
{
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    const wchar_t* const pProjectRoot = GetProjectRoot(pStatus);
    if (pPath != nullptr)
    {
        StringResult strFilePath;
        if (SUCCEEDED(_AdjustSlashes(pPath, strFilePath, pStatus)))
        {
            bool bIsAbsolute = false;
            strFilePath.IsAbsolutePath(L'\\', &bIsAbsolute);
            HRESULT hr;
            if (bIsAbsolute)
            {
                hr = DefStringResult_SetCopy(strNormalizedPath.GetStringResult(), strFilePath.GetRef());
            }
            else
            {
                hr = DefStringResult_SetCopy(strNormalizedPath.GetStringResult(), pProjectRoot);
                Def_HrFailed0(hr, pStatus);
                hr = DefStringResult_ConcatPathElement(strNormalizedPath.GetStringResult(), strFilePath.GetRef(), L'\\');
            }
            Def_HrFailed0(hr, pStatus);
        }
    }
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, pStatus->GetHResult());
    return pStatus->GetHResult();
}

const wchar_t* CBootStrapIndexer::GetProjectRoot(IDefStatusEx* const pStatus)
{
    static_cast<void>(pStatus);
    if (_strNewProjectRootFolder.GetLength() != 0)
    {
        return _strNewProjectRootFolder.GetRef();
    }
    return _pProjectRootFolder;
}

HRESULT CBootStrapIndexer::_ProcessConditionsNode(IXMLDOMNode* const pConditionsNode, IDefStatusEx* const pStatus)
{
    std::uint32_t length = 0;
    int qualifierSetIndex = 0;
    IXMLDOMNodeList* children = nullptr;
    IXMLDOMNode* child = nullptr;
    bool valid = true;
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);

    CXmlHelper* helper = new (std::nothrow) CXmlHelper(pConditionsNode);
    HRESULT result = helper != nullptr ? S_OK : E_OUTOFMEMORY;
    if (helper != nullptr)
    {
        helper->TryGetChildren(L"qualifier", pStatus, &children);
        children->get_length(reinterpret_cast<LONG*>(&length));
        CQualifierApplicator::CQualifierSetBuilder* builder = nullptr;
        result = _pQualifierApplicator->GetQualifierSetBuilder(_baseQualifierSetIndex, pStatus, &builder);
        if (SUCCEEDED(result))
        {
            for (int index = 0; index < static_cast<int>(length); ++index)
            {
                result = children->get_item(index, &child);
                if (SUCCEEDED(result))
                {
                    CXmlHelper* childHelper = new (std::nothrow) CXmlHelper(child);
                    if (childHelper != nullptr)
                    {
                        wchar_t* name = nullptr;
                        wchar_t* value = nullptr;
                        childHelper->GetAttributeValue(L"name", pStatus, &name);
                        result = childHelper->GetAttributeValue(L"value", pStatus, &value);
                        builder->_AddQualifier(
                            name, value, nullptr, nullptr, CQualifierApplicator::tagTOKEN_TYPE::tokenDefault, &valid, pStatus);
                        if (!valid)
                        {
                            StringResult description;
                            Def_HrFailed0(DefStringResult_InitRef(description.GetStringResult(), name), pStatus);
                            Def_HrFailed0(DefStringResult_Concat(description.GetStringResult(), L"-"), pStatus);
                            Def_HrFailed0(DefStringResult_Concat(description.GetStringResult(), value), pStatus);
                            pStatus->SetError(E_DEF_PRICONFIG_INVALID_QUAL, description.GetRef());
                        }
                        operator delete(name);
                        operator delete(value);
                        delete childHelper;
                        if (!valid)
                        {
                            break;
                        }
                    }
                    else
                    {
                        result = E_OUTOFMEMORY;
                    }
                    SAFE_RELEASE(child);
                }
                if (FAILED(result))
                {
                    break;
                }
            }
            SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
            if (SUCCEEDED(result) && valid)
            {
                _pQualifierApplicator->ApplyQualifierSetFromBuilder(builder, pStatus, &qualifierSetIndex);
                _baseQualifierSetIndex = qualifierSetIndex;
            }
            if (builder != nullptr)
            {
                delete builder;
            }
        }
        delete helper;
    }
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, ComputeHResult(result, pStatus));
    return ComputeHResult(result, pStatus);
}

HRESULT CBootStrapIndexer::_ProcessUltimateFallbackNode(IXMLDOMNode* const pUltFallbackNode, IDefStatusEx* const pStatus)
{
    std::uint32_t length = 0;
    IXMLDOMNodeList* children = nullptr;
    IXMLDOMNode* child = nullptr;
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);

    CXmlHelper* helper = new (std::nothrow) CXmlHelper(pUltFallbackNode);
    HRESULT result = helper != nullptr ? S_OK : E_OUTOFMEMORY;
    if (helper != nullptr)
    {
        result = helper->TryGetChildren(L"qualifier", pStatus, &children);
        children->get_length(reinterpret_cast<LONG*>(&length));
        for (int index = 0; SUCCEEDED(result) && index < static_cast<int>(length); ++index)
        {
            result = children->get_item(index, &child);
            if (SUCCEEDED(result) && child != nullptr)
            {
                CXmlHelper* childHelper = new (std::nothrow) CXmlHelper(child);
                if (childHelper != nullptr)
                {
                    wchar_t* name = nullptr;
                    wchar_t* value = nullptr;
                    childHelper->GetAttributeValue(L"name", pStatus, &name);
                    const wchar_t* attribute = L"DeviceFamily";
                    if (CompareStringOrdinal(name, -1, L"Platform", -1, true) != CSTR_EQUAL)
                    {
                        attribute = name;
                    }
                    childHelper->GetAttributeValue(L"value", pStatus, &value);
                    result = _pQualifierApplicator->AddUltFallbackAttrValuePair(attribute, value, nullptr, pStatus);
                    operator delete(name);
                    operator delete(value);
                    delete childHelper;
                }
                else
                {
                    result = E_OUTOFMEMORY;
                }
                SAFE_RELEASE(child);
            }
        }
        if (SUCCEEDED(result))
        {
            result = _pQualifierApplicator->ValidateUltFallbackQualifiers(pStatus);
        }
        SAFE_RELEASE(reinterpret_cast<IXMLDOMNode*>(children));
        delete helper;
    }
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, result);
    return result;
}

HRESULT CBootStrapIndexer::CreateStringEntry(
    const wchar_t* const pSource,
    const wchar_t* const pItemName,
    const wchar_t* const pValue,
    const MrmEnvironment::ResourceItemType resourceItemType,
    const MrmEnvironment::ResourceValueType resourceValueType,
    const int qualifierSetIndex,
    const ULONG ulActionFlags,
    IDefStatusEx* const pStatus)
{
    static_cast<void>(pSource);
    static_cast<void>(pItemName);
    static_cast<void>(resourceItemType);
    static_cast<void>(resourceValueType);
    static_cast<void>(ulActionFlags);
    pStatus->DiagnosticLogWithPrefixA("Start - ", __FUNCTION__);
    wcsnlen(L"", MAX_PATH + 1);
    const wchar_t* pEntryValue = pValue;
    if (pEntryValue == nullptr || wcsnlen(pEntryValue, MAX_PATH + 1) == 0)
    {
        pEntryValue = L"";
    }
    if (pStatus->Succeeded())
    {
        if (_pItemInstanceEntry != nullptr)
        {
            delete _pItemInstanceEntry;
        }
        _pItemInstanceEntry = CItemInstanceEntry::NewForString(
            L"Files",
            L"",
            MrmEnvironment::ResourceItemType_Path,
            MrmEnvironment::ResourceValueType_Utf16Path,
            pEntryValue,
            qualifierSetIndex,
            2,
            nullptr,
            nullptr,
            pStatus);
    }
    pStatus->DiagnosticLogWithErrorCodeA(__FUNCTION__, ComputeHResult(S_OK, pStatus));
    return ComputeHResult(S_OK, pStatus);
}
} // namespace Microsoft::Resources::Indexers
